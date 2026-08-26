#pragma once

#include <QString>
#include <QStringList>

class NvComputer;

class ServerCommandManager
{
public:
    explicit ServerCommandManager(NvComputer* computer);

    bool hasPermission() const;
    QStringList availableCommands() const;
    bool execute(int commandIndex, QString& result) const;

    static bool isDestructive(const QString& commandName);

private:
    NvComputer* m_Computer;
};
