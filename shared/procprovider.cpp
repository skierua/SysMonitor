#include <iostream>
#include <chrono> // For std::chrono::seconds, milliseconds, etc.
#include <thread> // For std::this_thread::sleep_for
#include <QDateTime>

#include "procprovider.h"

#if defined(__APPLE__)          //definedQ_OS_MAC)
#include "../spec/mackernel.h"
using Kernel = MacKernel;
#elif defined(_WIN64)           //defined(Q_OS_WIN)
static_assert(false, "Windows is not supported");
#include "../spec/winkernel.h"
using Kernel = WinKernel;
#elif defined(__linux__)            //defined(Q_OS_LINUX)
static_assert(false, "Linux is not supported");
#include "../spec/linuxkernel.h"
using Kernel = LinuxKernel;
#else
#error "Platform not supported"
#endif

ProcProvider::ProcProvider(QObject *parent)
    : QAbstractListModel{parent}
{ }

/*
 * param(t) ms for wait
 */
void ProcProvider::lock(int t){
    while (m_lock) std::this_thread::sleep_for(std::chrono::milliseconds(t));
    m_lock = true;
}

void ProcProvider::unlock(){
    m_lock = false;
}

bool ProcProvider::canTerminate() const{
    return Kernel::getSelf().canTerminate(m_crntPID) == 0;
}

/*
 * terminate process for current PID
 */
bool ProcProvider::terminate()
{
    lock(105);
    auto msg = QString("ProcProvider::terminate() %1/%2")
                   .arg(QString::number(m_crntPID),
                        m_procList.at(m_crntPIDIndex).qcomm);
    int ok = Kernel::getSelf().termProc(m_crntPID);
    // qDebug() << "ProcProvider::terminate 1";
    if (ok == 0){
        // to avoid compiler optimiztion
        volatile auto row = m_crntPIDIndex;
        beginRemoveRows(QModelIndex(), row, row);
        // qDebug() << "ProcProvider::terminate 2";
        m_crntPIDIndex = -1;
        m_crntPID = -1;
        m_procList.remove(row);
        endRemoveRows();
        // qDebug() << "ProcProvider::terminate 3";
        emit message(QString(msg + " SUCCESSFUL"), 1);
    } else {
        emit message(QString(msg + " FAILED"), 4);
    }
    // qDebug() << "ProcProvider::terminate 4";
    unlock();
    return ok;
}

/*
 * get full system pass process for current PID
 */
QString ProcProvider::procPath() {
    // qDebug()<< "procPath() pid=" << m_crntPID << " ind=" << m_crntPIDIndex;
    if (m_crntPID < 0) return QString("");
    QString res = Kernel::getSelf().procPath(m_crntPID);
    // qDebug()<< " pid=" << m_crntPID << " path=" << res;
    if (res.isEmpty()){
        auto msg = QString("ProcProvider::procPath() %1/%2 FAILED")
                       .arg(QString::number(m_crntPID), m_procList.at(m_crntPIDIndex).qcomm);
        emit message(msg, 4);
    }
    return res;
}

/*
 * ppulate process's list
 */
void ProcProvider::addProcList(QList<vk_proc_info> &&proc){
    // to avoid reset until previous not finished
    // TODO std::atomic ?
    if (m_lock) return;
    lock();
    if (proc.size() == 0){
        emit message("Kernel error. Processess list not retrieved.", 4);
        return;
    }
    if (m_refreshCounter++ % 5) {
        std::sort(proc.begin(), proc.end(), [](vk_proc_info a,vk_proc_info b){ return a.mem > b.mem;});
    }
    // 2 ways for velocity
    if (m_nameFilter.isEmpty()){
        if (proc.size() != m_procList.size()) {
            // size_t len = proc.size();
            if (proc.size() > m_procList.size()) {
                struct vk_proc_info blankRow;
                beginInsertRows(QModelIndex(),  m_procList.size(),  proc.size()-1);
                while (proc.size() > m_procList.size()) m_procList << blankRow;
                endInsertRows();
            } else if (proc.size() < m_procList.size()) {
                beginRemoveRows(QModelIndex(), proc.size(), m_procList.size()-1);
                while (proc.size() < m_procList.size()) m_procList.remove(m_procList.size()-1);
                endRemoveRows();
            }
            m_procList = proc;
        }
    } else {
        QList<vk_proc_info> filteredProc;
        for (auto& v: proc) {
            if (~v.qcomm.toLower().indexOf(m_nameFilter)){
                filteredProc.append(v);
            }
        }
        if (filteredProc.size() != m_procList.size()) {
            if (filteredProc.size() > m_procList.size()) {
                struct vk_proc_info blankRow;
                beginInsertRows(QModelIndex(),  m_procList.size(),  filteredProc.size()-1);
                while (filteredProc.size() > m_procList.size()) m_procList << blankRow;
                endInsertRows();
            } else if (filteredProc.size() < m_procList.size()) {
                beginRemoveRows(QModelIndex(), filteredProc.size(), m_procList.size()-1);
                while (filteredProc.size() < m_procList.size()) m_procList.remove(m_procList.size()-1);
                endRemoveRows();
            }
            m_procList = filteredProc;
        }
    }
 // return;
    auto nextIndex{-1};
    if (!(m_crntPID < 0)) {
        for ( nextIndex = 0; nextIndex < m_procList.size() && m_procList[nextIndex].pid != m_crntPID; ++nextIndex) {}
        if (nextIndex == m_procList.size()) {
            nextIndex = -1;
            m_crntPID = -1;
            emit crntPIDChanged();
        }
    }
    if (nextIndex != m_crntPIDIndex){
        m_crntPIDIndex = nextIndex;
        emit crntPIDIndexChanged(m_crntPIDIndex);
    }
    dataChanged(index(0,0)
                ,index(m_procList.size()-1,0));
    unlock();
    // prnProc();
}

/*
 * for debugging
 */
void ProcProvider::prnProc() const {
    std::cout << "========= prmProc =======" << std::endl;
    for (const auto& v: m_procList){
        std::cout << "PID: " << v.pid
                   << ",\tPPID: " << v.ppid
                  << ",\tqcomm: " << v.qcomm.toStdString()
                  << ",\tmem=" << v.mem << "/" << v.vm
                  << ",\tth=" << v.th_all << "/" << v.th_active
                  << ",\ttm=" << v.tm
                  << ",\tuid=" << v.uid
                  << std::endl;
    }
}


/*
 * virtual function redefinition
 */
int ProcProvider::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_procList.count();
}

/*
 * virtual function redefinition
 */
QVariant ProcProvider::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_procList.count())
        return QVariant();

    const vk_proc_info &proc = m_procList[index.row()];
    if (role == PidRole)
        return proc.pid;
    else if (role == CommRole)
        // return QString::fromStdString(proc.comm);
        return (proc.qcomm);
    else if (role == MemRole)
        return humanMem(proc.mem);
    else if (role == VmRole)
        return proc.vm;
    else if (role == Th_allRole)
        return proc.th_all;
    else if (role == Th_activeRole)
        return proc.th_active;
    else if (role == Th_strRole)
        return QString::number(proc.th_all) + (proc.th_active == 0 ? "" : ("/"+ QString::number(proc.th_active)));
    else if (role == TmRole)
        return QDateTime::fromSecsSinceEpoch(proc.tm).toString(Qt::ISODate);
    else return QVariant();
}

// for QML/ListModel
QHash<int, QByteArray> ProcProvider::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[PidRole] = "pid";
    roles[CommRole] = "comm";
    roles[MemRole] = "mem";
    roles[VmRole] = "vm";
    roles[Th_allRole] = "th_all";
    roles[Th_activeRole] = "th_active";
    roles[Th_strRole] = "th_str";
    roles[TmRole] = "tm";
    return roles;
}

/*
 * human readable memory info
 */
QString ProcProvider::humanMem(unsigned int mem) const {
    auto i{1};
    auto pow{1024};
    auto tmp{mem};
    QString res;
    for ( ; tmp > pow * 1024 && i < 4; ++i, pow *= 1024 ){ }
    tmp = (tmp + pow/2)/pow;
    // std::cout << "i=" << i << " pow=" << pow << " mem=" << mem << " tmp=" << tmp << std::endl;
    switch (i){
    case 2: res = QString("%1 MB").arg(QString::number(tmp)); break;
    case 1: res = QString("%1 kB").arg(QString::number(tmp)); break;
    case 3: res = QString("%1 GB").arg(QString::number(tmp)); break;
    case 0: res = QString("%1 B").arg(QString::number(tmp)); break;
    default: res = QString::number(mem);
    }
    return res;
}
