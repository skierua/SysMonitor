#ifndef LOGGER_H
#define LOGGER_H

// #include <iostream>

#include <QObject>
#include <QFile>
#include <QDir>
#include <QtLogging>

#include <QDebug>

class Logger : public QObject
{
    Q_OBJECT
public:
    enum class SMLOG {ENOLOG =0, EINFO =1, EWARN =2, EERROR =4, EFATAL =8} ;

    Logger(QObject *parent = nullptr);
    explicit Logger(const QString& sysLogPath, QObject *parent = nullptr);

    // ~Logger() noexcept =default;

    bool isValid() const
    { return m_valid; }

    void consoleMode(bool mode)
    { m_toConsole = mode; }

    const QString lastError() const { return m_lastError; }
    const QString logPath() const { return generateFileName(); }

signals:
    void terminate();

public slots:
    void log(const QString& msg, int level);


private:
    int m_level{static_cast<int>(SMLOG::EINFO) | static_cast<int>(SMLOG::EFATAL) | static_cast<int>(SMLOG::EERROR)};

    unsigned long long m_logMaxSize{1 * 1024 * 1024};
    // duplicate log to consol
    bool m_toConsole{false};
    // is class properly construced
    bool m_valid{false};
    // OS path for logs
    QString m_sysLogPath{"./Logs"}; // testing
    QString m_appName{"SystemMonitor"}; // testing
    // template for log string
    QString m_template{"%1: %2 %3"};

    QString m_lastError{""};

    QString generateFileName() const { return QString("%1/%2/%3.log").arg(m_sysLogPath,m_appName,m_appName);}
    bool checkPath();
    // rotation for logs
    bool rotate();
};

#endif // LOGGER_H
