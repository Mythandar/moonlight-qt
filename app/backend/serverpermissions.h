#pragma once

#include <QString>

class ServerPermissions
{
public:
    enum Permission : quint32 {
        ControllerInput = 0x00000100,
        TouchInput      = 0x00000200,
        PenInput        = 0x00000400,
        MouseInput      = 0x00000800,
        KeyboardInput   = 0x00001000,

        ClipboardSet    = 0x00010000,
        ClipboardRead   = 0x00020000,
        FileUpload      = 0x00040000,
        FileDownload    = 0x00080000,
        ServerCommand   = 0x00100000,

        ListApps        = 0x01000000,
        ViewStreams     = 0x02000000,
        LaunchApps      = 0x04000000,
    };

    static quint32 parse(const QString& value, bool* ok = nullptr);
    static bool has(quint32 permissions, Permission permission);
    static QString formatDetailed(quint32 permissions);
};
