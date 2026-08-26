#include "clipboardmanager.h"

#include "nvcomputer.h"
#include "nvhttp.h"
#include "serverpermissions.h"

#include <QClipboard>
#include <QCryptographicHash>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>

ClipboardManager::ClipboardManager(NvComputer* computer, QObject* parent)
    : QObject(parent),
      m_Clipboard(QGuiApplication::clipboard()),
      m_Computer(computer),
      m_Http(new NvHTTP(computer, nullptr)),
      m_Streaming(false),
      m_SyncInProgress(false)
{
    m_Http->setParent(this);
    connect(m_Clipboard, &QClipboard::dataChanged,
            this, &ClipboardManager::onLocalClipboardChanged);
}

bool ClipboardManager::uploadNow()
{
    return sendClipboard();
}

bool ClipboardManager::fetchNow()
{
    return receiveClipboard();
}

bool ClipboardManager::canUpload() const
{
    QReadLocker lock(&m_Computer->lock);
    return m_Computer->serverPermissionsAvailable &&
            ServerPermissions::has(m_Computer->serverPermissions,
                                   ServerPermissions::ClipboardSet);
}

bool ClipboardManager::canFetch() const
{
    QReadLocker lock(&m_Computer->lock);
    return m_Computer->serverPermissionsAvailable &&
            ServerPermissions::has(m_Computer->serverPermissions,
                                   ServerPermissions::ClipboardRead);
}

void ClipboardManager::onStreamStarted()
{
    m_Streaming = true;

    // Match Artemis smart-sync behavior by seeding the host with the current
    // client clipboard after the streaming connection becomes active.
    sendClipboard();
}

void ClipboardManager::onStreamStopped()
{
    m_Streaming = false;
}

void ClipboardManager::onFocusLost()
{
    if (m_Streaming) {
        // A user commonly copies on the host and then leaves the stream to
        // paste locally, so focus loss is a useful event-driven pull point.
        receiveClipboard();
    }
}

void ClipboardManager::onLocalClipboardChanged()
{
    if (!m_Streaming || m_SyncInProgress || !canUpload()) {
        return;
    }

    QString text;
    if (!getLocalClipboardText(text) || isOwnClipboardChange(text)) {
        return;
    }

    sendClipboard();
}

bool ClipboardManager::sendClipboard()
{
    if (!m_Streaming || m_SyncInProgress || !canUpload()) {
        return false;
    }

    QString text;
    if (!getLocalClipboardText(text)) {
        return false;
    }

    if (isOwnClipboardChange(text)) {
        return false;
    }

    m_SyncInProgress = true;
    const bool success = m_Http->sendClipboardContent(text);
    m_SyncInProgress = false;
    return success;
}

bool ClipboardManager::receiveClipboard()
{
    if (!m_Streaming || m_SyncInProgress || !canFetch()) {
        return false;
    }

    m_SyncInProgress = true;

    QString text;
    const bool success = m_Http->getClipboardContent(text);
    const QByteArray encodedText = text.toUtf8();

    if (success && encodedText.size() <= MAX_CLIPBOARD_SIZE) {
        if (m_Clipboard->text(QClipboard::Clipboard) != text) {
            markAsOwnClipboardChange(text);
            m_Clipboard->setText(text, QClipboard::Clipboard);
        }
        m_LastReceivedContent = text;
    }
    else if (success && encodedText.size() > MAX_CLIPBOARD_SIZE) {
        qWarning() << "Ignoring host clipboard content larger than 1 MB";
    }

    m_SyncInProgress = false;
    return success && encodedText.size() <= MAX_CLIPBOARD_SIZE;
}

bool ClipboardManager::getLocalClipboardText(QString& text) const
{
    const QMimeData* mimeData = m_Clipboard->mimeData(QClipboard::Clipboard);
    if (mimeData == nullptr || !mimeData->hasText()) {
        return false;
    }

    text = mimeData->text();
    if (text.toUtf8().size() > MAX_CLIPBOARD_SIZE) {
        qWarning() << "Ignoring local clipboard content larger than 1 MB";
        return false;
    }

    return true;
}

bool ClipboardManager::isOwnClipboardChange(const QString& text) const
{
    return text == m_LastReceivedContent ||
            m_OwnContentHashes.contains(contentHash(text));
}

void ClipboardManager::markAsOwnClipboardChange(const QString& text)
{
    m_OwnContentHashes.append(contentHash(text));
    while (m_OwnContentHashes.size() > MAX_OWN_CONTENT_HASHES) {
        m_OwnContentHashes.removeFirst();
    }
}

QByteArray ClipboardManager::contentHash(const QString& text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Sha256);
}
