#include "logger.h"

Logger::Logger(QObject *parent)
    : QObject{parent}
{
    auto ok{true};
    if (m_appName.isEmpty()) m_appName = "SystemMonitor";
    QFile logFile(generateFileName());
    if (!logFile.exists()){
        ok &= checkPath();
    }
    ok &= rotate();
    m_valid = ok;
}

Logger::Logger(const QString &sysLogPath, QObject *parent)
    : m_sysLogPath{sysLogPath}, QObject{parent}
{
    auto ok{true};
    if (m_appName.isEmpty()) m_appName = "SystemMonitor";
    QFile logFile(generateFileName());
    if (!logFile.exists()){
        ok &= checkPath();
    }
    ok &= rotate();
    m_valid = ok;
}

void Logger::log(const QString &msg, int type =0)
{
    // if (msg.isEmpty()) return;
    if (!(type & m_level)) return; // nothing to log
    QString lvl{""};
    if (type & SMLOG::INFO) lvl = "II";
    else if (type & SMLOG::ERROR) lvl = "EE";
    else if (type & SMLOG::FATAL) lvl = "FF";
    else if (type & SMLOG::WARN) lvl = "WW";
    else lvl = "??";
    QFile logFile(generateFileName());
    if (!logFile.open(QIODevice::WriteOnly | QIODeviceBase::Append | QIODevice::Text)) {
        qCritical() << "Error: Could not open file for writing:" << logFile.errorString();
        emit terminate();
        return;
    }
    QString str = m_template
                      .arg(lvl
                        , QDateTime::currentDateTime().toString("yyyy/MM/ddThh:mm:ss")
                        , (msg.isEmpty() ? "BLANK message" : msg));

    QTextStream out(&logFile);

    out << str << "\n";

    logFile.close();

    if (m_toConsole) {
        if (type & SMLOG::INFO) qInfo() << str;
        else if (type & SMLOG::ERROR) qCritical() << str;
        else if (type & SMLOG::FATAL) qCritical() << str;
        else if (type & SMLOG::WARN) qWarning() << str;
        else qDebug() << str;

        // terminate in case of FATAL
        if (type & SMLOG::FATAL) emit terminate();
    }

}

bool Logger::checkPath()
{
    auto ok{true};
    auto dir = QDir(m_sysLogPath);
    // qDebug() << "Logger::Logger dir=" << dir.exists();

    // if Sys LogDir not exist -- turn to ./Logs in current dit
    if (!dir.exists()) {
        m_sysLogPath = "./Logs";
        dir = QDir(m_sysLogPath);
        if (!dir.exists()) {
            if (!dir.mkdir(m_sysLogPath)) {
                ok &= false;
                m_lastError = QString("System Log dir not exist and can't be created .");
                // throw std::invalid_argument("AppLog dir not exist and can't be created .");
            }
        }
    }

    // if App LogDir not exist -- mkdir
    if (ok && !dir.exists(m_appName)) {
        if (!dir.mkdir(m_appName)) {
            ok &= false;
            m_lastError = QString("AppLog dir not exist and can't be created .");
            // throw std::invalid_argument("AppLog dir not exist and can't be created .");
        }
    }
    return ok;
}

// rotate log when size > 10MB
bool Logger::rotate() {
    auto logName = generateFileName();
    QFile file(logName);
    if (!file.exists()) return true;

    auto fi = QFileInfo(logName);
    // rotate log if > 10MB
    if (fi.size() < m_logMaxSize) return true;

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Log rotation error: Could not open input file for reading: %1").arg(logName);
        qCritical() << "Log rotation error: Could not open input file for reading:" << logName;
        return false;
    }
    QByteArray rawData = file.readAll();
    file.close();
    // if (rawData.isEmpty())  return true;

    QByteArray comprData = qCompress(rawData, -1);
    if (comprData.isEmpty()) {
        m_lastError = QString("Log rotation error: Compression failed.");
        qCritical() << "Log rotation error: Compression failed.";
        return false;
    }
    // qDebug() << "Compressed size:" << comprData.size() << "bytes";
    QString outName = QString("%1/%2/%3.log.archive").arg(m_sysLogPath, m_appName, m_appName);
    QFile outFile(outName);
    if (outFile.exists() && !outFile.remove()) {
        m_lastError = QString("Log rotation error: Could not remove existed archive file: %1").arg(outName);
        qCritical() << "Log rotation error: Could not remove existed archive file:" << outName;
        return false;
    }
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = QString("Log rotation error: Could not open output file for writing: %1").arg(outName);
        qCritical() << "Log rotation error: Could not open output file for writing:" << outName;
        return false;
    }
    qint64 bytesWritten = outFile.write(comprData);
    outFile.close();

    if (bytesWritten != comprData.size()) {
        m_lastError = QString("Error: Failed to write all compressed data.");
        qCritical() << "Error: Failed to write all compressed data.";
        return false;
    }
    if (!file.remove()) {
        m_lastError = QString("Log rotation error: Could not remove existed log file: %1").arg(logName);
        qCritical() << "Log rotation error: Could not remove existed log file:" << logName;
        return false;
    }

    return true;
}


