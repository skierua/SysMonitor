#include "memprovider.h"

MemProvider::MemProvider(QObject *parent)
    : QObject{parent}
{}

void MemProvider::addData(unsigned long long v)
{
    // qDebug() << "memprovider.cpp data=" << v/(1024*1024) << "MB";
    while (m_data.size() >= m_maxDataLen) {
        m_data.remove(0);
    }
    m_data.append(v);
    QList<QPoint> plist;
    for (auto i{0}; i < m_data.size(); ++i) {
        // y-value in kB to avoid overflow for QPoint.y (int)
        plist.append(QPoint(i - m_data.size(), m_data.at(i)/1024));
    }
    emit usageChanged(plist);
}
