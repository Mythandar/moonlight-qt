#include "servercommandmanager.h"

#include "nvcomputer.h"
#include "serverpermissions.h"

#include <Limelight.h>

#include <QReadLocker>

ServerCommandManager::ServerCommandManager(NvComputer* computer)
    : m_Computer(computer)
{
}

bool ServerCommandManager::hasPermission() const
{
    QReadLocker lock(&m_Computer->lock);
    return m_Computer->serverPermissionsAvailable &&
            ServerPermissions::has(m_Computer->serverPermissions,
                                   ServerPermissions::ServerCommand);
}

QStringList ServerCommandManager::availableCommands() const
{
    QReadLocker lock(&m_Computer->lock);
    if (!m_Computer->serverPermissionsAvailable ||
            !ServerPermissions::has(m_Computer->serverPermissions,
                                    ServerPermissions::ServerCommand)) {
        return {};
    }
    return m_Computer->serverCommands;
}

bool ServerCommandManager::execute(int commandIndex, QString& result) const
{
    const QStringList commands = availableCommands();
    if (commandIndex < 0 || commandIndex >= commands.size() || commandIndex > 255) {
        result = QStringLiteral("Command is not available");
        return false;
    }

    if (LiSendExecServerCmd(static_cast<uint8_t>(commandIndex)) != 0) {
        result = QStringLiteral("Unable to send command request");
        return false;
    }

    result = QStringLiteral("Command request sent: %1").arg(commands.at(commandIndex));
    return true;
}

bool ServerCommandManager::isDestructive(const QString& commandName)
{
    const QString name = commandName.toLower();
    return name.contains(QStringLiteral("shutdown")) ||
            name.contains(QStringLiteral("power off")) ||
            name.contains(QStringLiteral("poweroff")) ||
            name.contains(QStringLiteral("restart")) ||
            name.contains(QStringLiteral("reboot")) ||
            name.contains(QStringLiteral("suspend")) ||
            name.contains(QStringLiteral("sleep")) ||
            name.contains(QStringLiteral("hibernate"));
}
