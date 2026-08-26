#include "quickmenumanager.h"

#include "clipboardmanager.h"
#include "servercommandmanager.h"
#include "streaming/session.h"

#include <algorithm>

QuickMenuManager::QuickMenuManager(Session* session,
                                   ServerCommandManager* serverCommandManager,
                                   ClipboardManager* clipboardManager)
    : m_Session(session),
      m_ServerCommandManager(serverCommandManager),
      m_ClipboardManager(clipboardManager),
      m_Page(Page::Main),
      m_SelectedIndex(0),
      m_PendingCommandIndex(-1),
      m_Visible(false),
      m_MessageExpiry(0)
{
}

bool QuickMenuManager::isVisible() const
{
    return m_Visible;
}

void QuickMenuManager::toggle()
{
    m_Visible ? hide() : show();
}

void QuickMenuManager::show()
{
    m_MessageExpiry = 0;
    m_Visible = true;
    m_Page = Page::Main;
    m_SelectedIndex = 0;
    m_PendingCommandIndex = -1;
    m_Session->m_InputHandler->raiseAllKeys();
    rebuildItems();
    updateOverlay();
}

void QuickMenuManager::hide()
{
    m_Visible = false;
    m_Page = Page::Main;
    m_Items.clear();
    m_Session->getOverlayManager().setOverlayState(Overlay::OverlayQuickMenu, false);
}

bool QuickMenuManager::handleEvent(const SDL_Event& event)
{
    if (!m_Visible) {
        return false;
    }

    if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
        const SDL_Keycode key = event.key.keysym.sym;
        const SDL_Keymod modifiers = static_cast<SDL_Keymod>(event.key.keysym.mod);

        if (key == SDLK_BACKSLASH &&
                (modifiers & KMOD_CTRL) &&
                (modifiers & KMOD_SHIFT)) {
            hide();
        }
        else if (key == SDLK_UP) {
            moveSelection(-1);
        }
        else if (key == SDLK_DOWN) {
            moveSelection(1);
        }
        else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            activateSelection();
        }
        else if (key == SDLK_ESCAPE || key == SDLK_BACKSPACE) {
            if (m_Page == Page::Main) {
                hide();
            }
            else {
                m_Page = Page::Main;
                m_SelectedIndex = 0;
                rebuildItems();
                updateOverlay();
            }
        }
    }
    else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
        switch (event.cbutton.button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            moveSelection(-1);
            break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            moveSelection(1);
            break;
        case SDL_CONTROLLER_BUTTON_A:
            activateSelection();
            break;
        case SDL_CONTROLLER_BUTTON_B:
            if (m_Page == Page::Main) {
                hide();
            }
            else {
                m_Page = Page::Main;
                m_SelectedIndex = 0;
                rebuildItems();
                updateOverlay();
            }
            break;
        default:
            break;
        }
    }

    return consumeWhileVisible(event.type);
}

void QuickMenuManager::tick()
{
    if (m_MessageExpiry != 0 && SDL_TICKS_PASSED(SDL_GetTicks(), m_MessageExpiry)) {
        m_MessageExpiry = 0;
        m_Session->getOverlayManager().setOverlayState(Overlay::OverlayQuickMenu, false);
    }
}

void QuickMenuManager::rebuildItems()
{
    m_Items.clear();

    if (m_Page == Page::Main) {
        m_Items.append({QStringLiteral("Disconnect"), Action::Disconnect});
        m_Items.append({QStringLiteral("Quit stream"), Action::QuitStream});

        if (!m_ServerCommandManager->availableCommands().isEmpty()) {
            m_Items.append({QStringLiteral("Server Commands"), Action::OpenServerCommands});
        }
        if (m_ClipboardManager != nullptr && m_ClipboardManager->canUpload()) {
            m_Items.append({QStringLiteral("Clipboard Upload"), Action::ClipboardUpload});
        }
        if (m_ClipboardManager != nullptr && m_ClipboardManager->canFetch()) {
            m_Items.append({QStringLiteral("Fetch Clipboard"), Action::ClipboardFetch});
        }

        const bool statsVisible = m_Session->getOverlayManager().isOverlayEnabled(Overlay::OverlayDebug);
        const bool mouseCaptured = m_Session->m_InputHandler->isCaptureActive();
        const bool keyboardCaptured = m_Session->m_InputHandler->isSystemKeyCaptureActive();
        const bool fullscreen = (SDL_GetWindowFlags(m_Session->m_Window) & SDL_WINDOW_FULLSCREEN) != 0;

        m_Items.append({QStringLiteral("Performance Stats: %1").arg(statsVisible ? "On" : "Off"),
                        Action::ToggleStats});
        m_Items.append({QStringLiteral("Mouse Capture: %1").arg(mouseCaptured ? "On" : "Off"),
                        Action::ToggleMouseCapture});
        m_Items.append({QStringLiteral("Keyboard/Input Capture: %1").arg(keyboardCaptured ? "On" : "Off"),
                        Action::ToggleKeyboardCapture});
        m_Items.append({QStringLiteral("Fullscreen: %1").arg(fullscreen ? "On" : "Off"),
                        Action::ToggleFullscreen});
    }
    else if (m_Page == Page::ServerCommands) {
        const QStringList commands = m_ServerCommandManager->availableCommands();
        for (int i = 0; i < commands.size(); ++i) {
            m_Items.append({commands.at(i), Action::ExecuteServerCommand, i});
        }
        m_Items.append({QStringLiteral("Back"), Action::Back});
    }
    else {
        m_Items.append({QStringLiteral("Execute command"), Action::ConfirmServerCommand,
                        m_PendingCommandIndex});
        m_Items.append({QStringLiteral("Cancel"), Action::CancelConfirmation});
    }

    if (m_Items.isEmpty()) {
        m_SelectedIndex = 0;
    }
    else {
        m_SelectedIndex = std::clamp(m_SelectedIndex, 0, static_cast<int>(m_Items.size()) - 1);
    }
}

void QuickMenuManager::updateOverlay()
{
    QStringList lines;
    if (m_Page == Page::Main) {
        lines << QStringLiteral("QUICK MENU")
              << QStringLiteral("Up/Down + Enter   Esc/B to close") << QString();
    }
    else if (m_Page == Page::ServerCommands) {
        lines << QStringLiteral("SERVER COMMANDS")
              << QStringLiteral("Only commands advertised by the host are shown") << QString();
    }
    else {
        const QStringList commands = m_ServerCommandManager->availableCommands();
        const QString command = m_PendingCommandIndex >= 0 && m_PendingCommandIndex < commands.size() ?
                    commands.at(m_PendingCommandIndex) : QStringLiteral("Unknown command");
        lines << QStringLiteral("CONFIRM SERVER COMMAND")
              << command << QString();
    }

    const int firstVisible = std::max(0, m_SelectedIndex - 8);
    const int lastVisible = std::min(static_cast<int>(m_Items.size()), firstVisible + 12);
    for (int i = firstVisible; i < lastVisible; ++i) {
        lines << QStringLiteral("%1 %2")
                 .arg(i == m_SelectedIndex ? QStringLiteral(">") : QStringLiteral(" "),
                      m_Items.at(i).label);
    }

    const QByteArray text = lines.join('\n').toUtf8();
    m_Session->getOverlayManager().updateOverlayText(Overlay::OverlayQuickMenu,
                                                      text.constData());
    m_Session->getOverlayManager().setOverlayState(Overlay::OverlayQuickMenu, true);
}

void QuickMenuManager::moveSelection(int delta)
{
    if (m_Items.isEmpty()) {
        return;
    }
    m_SelectedIndex = (m_SelectedIndex + delta + m_Items.size()) % m_Items.size();
    updateOverlay();
}

void QuickMenuManager::activateSelection()
{
    if (m_Items.isEmpty()) {
        return;
    }

    const Item item = m_Items.at(m_SelectedIndex);
    switch (item.action) {
    case Action::Disconnect: {
        hide();
        SDL_Event disconnectEvent;
        SDL_zero(disconnectEvent);
        disconnectEvent.type = SDL_QUIT;
        SDL_PushEvent(&disconnectEvent);
        break;
    }
    case Action::QuitStream: {
        hide();
        m_Session->setShouldExit(true);
        SDL_Event quitEvent;
        SDL_zero(quitEvent);
        quitEvent.type = SDL_QUIT;
        SDL_PushEvent(&quitEvent);
        break;
    }
    case Action::OpenServerCommands:
        m_Page = Page::ServerCommands;
        m_SelectedIndex = 0;
        rebuildItems();
        updateOverlay();
        break;
    case Action::ExecuteServerCommand: {
        const QStringList commands = m_ServerCommandManager->availableCommands();
        if (item.value >= 0 && item.value < commands.size() &&
                ServerCommandManager::isDestructive(commands.at(item.value))) {
            m_PendingCommandIndex = item.value;
            m_Page = Page::ConfirmCommand;
            m_SelectedIndex = 1;
            rebuildItems();
            updateOverlay();
        }
        else {
            executeServerCommand(item.value);
        }
        break;
    }
    case Action::ConfirmServerCommand:
        executeServerCommand(item.value);
        break;
    case Action::CancelConfirmation:
        m_Page = Page::ServerCommands;
        m_SelectedIndex = 0;
        rebuildItems();
        updateOverlay();
        break;
    case Action::ClipboardUpload:
        showMessage(m_ClipboardManager->uploadNow() ?
                        QStringLiteral("Clipboard uploaded") :
                        QStringLiteral("Clipboard upload failed or was denied"));
        break;
    case Action::ClipboardFetch:
        showMessage(m_ClipboardManager->fetchNow() ?
                        QStringLiteral("Clipboard fetched") :
                        QStringLiteral("Clipboard fetch failed or was denied"));
        break;
    case Action::ToggleStats:
        m_Session->getOverlayManager().setOverlayState(
                    Overlay::OverlayDebug,
                    !m_Session->getOverlayManager().isOverlayEnabled(Overlay::OverlayDebug));
        rebuildItems();
        updateOverlay();
        break;
    case Action::ToggleMouseCapture:
        m_Session->m_InputHandler->toggleCaptureActive();
        rebuildItems();
        updateOverlay();
        break;
    case Action::ToggleKeyboardCapture:
        m_Session->m_InputHandler->toggleSystemKeyCapture();
        rebuildItems();
        updateOverlay();
        break;
    case Action::ToggleFullscreen:
        m_Session->toggleFullscreen();
        rebuildItems();
        updateOverlay();
        break;
    case Action::Back:
        m_Page = Page::Main;
        m_SelectedIndex = 0;
        rebuildItems();
        updateOverlay();
        break;
    }
}

void QuickMenuManager::executeServerCommand(int commandIndex)
{
    QString result;
    m_ServerCommandManager->execute(commandIndex, result);
    showMessage(result);
}

void QuickMenuManager::showMessage(const QString& message)
{
    m_Visible = false;
    m_Items.clear();
    const QByteArray text = message.toUtf8();
    m_Session->getOverlayManager().updateOverlayText(Overlay::OverlayQuickMenu,
                                                      text.constData());
    m_Session->getOverlayManager().setOverlayState(Overlay::OverlayQuickMenu, true);
    m_MessageExpiry = SDL_GetTicks() + 3000;
}

bool QuickMenuManager::consumeWhileVisible(Uint32 eventType) const
{
    switch (eventType) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_TEXTINPUT:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEWHEEL:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
#if SDL_VERSION_ATLEAST(2, 0, 14)
    case SDL_CONTROLLERSENSORUPDATE:
    case SDL_CONTROLLERTOUCHPADDOWN:
    case SDL_CONTROLLERTOUCHPADUP:
    case SDL_CONTROLLERTOUCHPADMOTION:
#endif
    case SDL_FINGERDOWN:
    case SDL_FINGERMOTION:
    case SDL_FINGERUP:
        return true;
    default:
        return false;
    }
}
