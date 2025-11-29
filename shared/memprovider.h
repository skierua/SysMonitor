#ifndef MEMPROVIDER_H
#define MEMPROVIDER_H

#include <QObject>
#include <QPoint>

#include <QDebug>

// #include "macos/macmonitor.h"

class MemProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(unsigned long long totalRAM MEMBER m_totalRAM NOTIFY totalRAMChanged)

public:
    explicit MemProvider(QObject *parent = nullptr);

    void addData(unsigned long long);
    void setTotalRAM(unsigned long long v)
    { m_totalRAM = v; }
    Q_INVOKABLE QList<unsigned long long> dataList() const
    { return m_data; }

signals:
    void totalRAMChanged(unsigned long long);
    void usageChanged(QList<QPoint>);

private:
    int m_maxDataLen{60};
    QList<unsigned long long> m_data;
    unsigned long long m_totalRAM{0};
};

#endif // MEMPROVIDER_H
