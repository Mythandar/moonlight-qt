#pragma once

#include "SDL_compat.h"

#include <QString>
#include <QVector>

class ClipboardManager;
class ServerCommandManager;
class Session;

class QuickMenuManager
{
public:
    QuickMenuManager(Session* session,
                     ServerCommandManager* serverCommandManager,
                     ClipboardManager* clipboardManager);

    bool isVisible() const;
    void toggle();
    void show();
    void hide();
    bool handleEvent(const SDL_Event& event);
    void tick();

private:
    enum class Page {
        Main,
        ServerCommands,
        ConfirmCommand,
    };

    enum class Action {
        Disconnect,
        QuitStream,
        OpenServerCommands,
        ExecuteServerCommand,
        ConfirmServerCommand,
        CancelConfirmation,
        ClipboardUpload,
        ClipboardFetch,
        ToggleStats,
        ToggleMouseCapture,
        ToggleKeyboardCapture,
        ToggleFullscreen,
        Back,
    };

    struct Item {
        QString label;
        Action action;
        int value = -1;
    };

    void rebuildItems();
    void updateOverlay();
    void moveSelection(int delta);
    void activateSelection();
    void executeServerCommand(int commandIndex);
    void showMessage(const QString& message);
    bool consumeWhileVisible(Uint32 eventType) const;

    Session* m_Session;
    ServerCommandManager* m_ServerCommandManager;
    ClipboardManager* m_ClipboardManager;
    Page m_Page;
    QVector<Item> m_Items;
    int m_SelectedIndex;
    int m_PendingCommandIndex;
    bool m_Visible;
    Uint32 m_MessageExpiry;
};
