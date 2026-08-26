#include "serverpermissions.h"

#include <QStringList>

quint32 ServerPermissions::parse(const QString& value, bool* ok)
{
    const QString trimmed = value.trimmed();
    const int base = trimmed.startsWith("0x", Qt::CaseInsensitive) ? 16 : 10;
    const QString digits = base == 16 ? trimmed.mid(2) : trimmed;

    bool parsed = false;
    const quint32 permissions = digits.toUInt(&parsed, base);
    if (ok != nullptr) {
        *ok = parsed;
    }
    return parsed ? permissions : 0;
}

bool ServerPermissions::has(quint32 permissions, Permission permission)
{
    return (permissions & static_cast<quint32>(permission)) != 0;
}

QString ServerPermissions::formatDetailed(quint32 permissions)
{
    const auto state = [permissions](Permission permission) {
        return has(permissions, permission) ? QStringLiteral("Allowed") : QStringLiteral("Denied");
    };

    QStringList lines;
    lines << QStringLiteral("Input:")
          << QStringLiteral("  Controller: %1").arg(state(ControllerInput))
          << QStringLiteral("  Touch: %1").arg(state(TouchInput))
          << QStringLiteral("  Pen: %1").arg(state(PenInput))
          << QStringLiteral("  Mouse: %1").arg(state(MouseInput))
          << QStringLiteral("  Keyboard: %1").arg(state(KeyboardInput))
          << QStringLiteral("Operations:")
          << QStringLiteral("  Set Clipboard: %1").arg(state(ClipboardSet))
          << QStringLiteral("  Read Clipboard: %1").arg(state(ClipboardRead))
          << QStringLiteral("  File Upload: %1").arg(state(FileUpload))
          << QStringLiteral("  File Download: %1").arg(state(FileDownload))
          << QStringLiteral("  Server Command: %1").arg(state(ServerCommand))
          << QStringLiteral("Actions:")
          << QStringLiteral("  List Apps: %1").arg(state(ListApps))
          << QStringLiteral("  View Streams: %1").arg(state(ViewStreams))
          << QStringLiteral("  Launch Apps: %1").arg(state(LaunchApps));
    return lines.join('\n');
}
