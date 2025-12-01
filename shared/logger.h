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
    enum SMLOG {NOLOG =0, INFO =1, WARN =2, ERROR =4, FATAL =8} ;
    explicit Logger(QObject *parent = nullptr);
    explicit Logger(const QString& sysLogPath, QObject *parent = nullptr);
    ~Logger() =default;

    bool isValid() const
    { return m_valid; }

    bool consoleMode(bool mode)
    { m_toConsole = mode; }

    const QString lastError() const { return m_lastError; }
    const QString logPath() const { return generateFileName(); }

signals:
    void terminate();

public slots:
    void log(const QString& msg, int level);


private:
    int m_level{SMLOG::INFO | SMLOG::FATAL | SMLOG::ERROR};

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
