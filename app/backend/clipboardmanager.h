#pragma once

#include <QObject>
#include <QByteArray>
#include <QList>

class NvComputer;
class NvHTTP;
class QClipboard;

class ClipboardManager : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardManager(NvComputer* computer, QObject* parent = nullptr);

    void onStreamStarted();
    void onStreamStopped();
    void onFocusLost();
    void onLocalClipboardChanged();

    bool uploadNow();
    bool fetchNow();
    bool canUpload() const;
    bool canFetch() const;

private:
    static const int MAX_CLIPBOARD_SIZE = 1024 * 1024;
    static const int MAX_OWN_CONTENT_HASHES = 10;

    bool sendClipboard();
    bool receiveClipboard();
    bool getLocalClipboardText(QString& text) const;
    bool isOwnClipboardChange(const QString& text) const;
    void markAsOwnClipboardChange(const QString& text);
    static QByteArray contentHash(const QString& text);

    QClipboard* m_Clipboard;
    NvComputer* m_Computer;
    NvHTTP* m_Http;
    bool m_Streaming;
    bool m_SyncInProgress;
    QString m_LastReceivedContent;
    QList<QByteArray> m_OwnContentHashes;
};
