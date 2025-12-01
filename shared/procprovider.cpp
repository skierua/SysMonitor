#include "procprovider.h"

ProcProvider::ProcProvider(QObject *parent)
    : QAbstractListModel{parent}
{
    // std::function<std::vector<vk_proc_info>()> m_procFn = [](){
    //     return std::vector<vk_proc_info>(); };
    // QObject::connect(this, &ProcProvider::procChanged,
    //                  this, &ProcProvider::prnProc);
}
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

bool ProcProvider::terminate()
{
    lock(105);
    auto msg = QString("ProcProvider::terminate() %1/%2")
                   .arg(QString::number(m_crntPID),
                        QString::fromStdString(m_procList.at(m_crntPIDIndex).comm));
    int ok = m_procTerm(m_crntPID);
    // qDebug() << "ProcProvider::terminate 1";
    if (ok == 0){
        auto row = m_crntPIDIndex;
        beginRemoveRows(QModelIndex(), row, row);
        // qDebug() << "ProcProvider::terminate 2";
        m_crntPIDIndex = -1;
        m_crntPID = 0;
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

QString ProcProvider::procPath() {
    QString res = m_procPath(m_crntPID);
    if (res.isEmpty()){
        auto msg = QString("ProcProvider::procPath() %1/%2 FAILED")
                       .arg(QString::number(m_crntPID),
                            QString::fromStdString(m_procList.at(m_crntPIDIndex).comm));
        emit message(msg, 4);
    }
    return res;
}

void ProcProvider::addProcList(QList<vk_proc_info> &&proc){
    // to avoid reset until previous not finished
    // TODO std::atomic ?
    if (m_lock) return;
    lock();
    if (proc.size() == 0){
        emit message("Kernel error. Processess list not retrieved.", 4);
        return;
    }
    std::sort(proc.begin(), proc.end(), [](vk_proc_info a,vk_proc_info b){ return a.mem > b.mem;});
    // beginResetModel();
    // m_procList.clear();
    m_procList = proc;
    if (proc.size() != m_procList.size()) {
        if (proc.size() > m_procList.size()) {
            beginInsertRows(QModelIndex(),  m_procList.size(),  proc.size()-1);
            while (proc.size() > m_procList.size()) m_procList << proc.at(m_procList.size() -1);
            endInsertRows();
        } else if (proc.size() < m_procList.size()) {
            beginRemoveRows(QModelIndex(), proc.size(), m_procList.size()-1);
            while (proc.size() < m_procList.size()) m_procList.remove(m_procList.size()-1);
            endRemoveRows();
        }
    }
    auto nextIndex{-1};
    if(m_crntPID != 0) {
        nextIndex = 0;
        for ( ; nextIndex < m_procList.size() && m_procList[nextIndex].pid != m_crntPID; ++nextIndex) {}
        if (nextIndex == m_procList.size()) nextIndex = -1;
    }
    // endResetModel();
    if (nextIndex != m_crntPIDIndex){
        m_crntPIDIndex = nextIndex;
        emit crntPIDIndexChanged(m_crntPIDIndex);
    }
    dataChanged(index(0,0)
                ,index(m_procList.size()-1,0));
    // for (auto i{0}; i < 30; ++i) addProc(proc[i]);
    unlock();
}

/*void ProcProvider::start()
{
    std::cout << "procprovider started" << std::endl;
    int n{0};
    while (true && n < 10) {
        ++n;
        m_procData = m_procFn();
        std::sort(m_procData.begin(), m_procData.end(), [](vk_proc_info a,vk_proc_info b){ return a.mem > b.mem;});
        std::vector<vk_proc_info> tmp = m_procData;
        if (tmp.size()) {
            emit procChanged(tmp);
            emit emitTest(tmp.size());
            // emit procChanged(std::move(tmp));
        } else {
            // TODO error
            std::cout << "#6g63 procprovider error";
        }

        beginRemoveRows(QModelIndex(), 0, rowCount());
        m_procList.clear();
        endRemoveRows();
        for (auto i{0}; i < 30; ++i) addProc(m_procData[i]);


        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}*/

void ProcProvider::prnProc(std::vector<vk_proc_info>) const{
    std::cout << "========= prmProc =======" << std::endl;
    for (auto& v: m_procData){
        std::cout << "PID: " << v.pid
                  << ", comm: " << v.comm
                  << ",\tmem=" << v.mem << "/" << v.vm
                  << ",\tth=" << v.th_all << "/" << v.th_active
                  << ",\ttm=" << v.tm
                  << std::endl;
    }
}

/*void ProcProvider::addProc(const vk_proc_info& proc)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_procList << proc;
    endInsertRows();
} */

int ProcProvider::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_procList.count();
}

#if 0
int ProcProvider::columnCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return COLUMN_COUNT;
}

QModelIndex ProcProvider::parent(const QModelIndex &) const {
    return QModelIndex();
}

QModelIndex ProcProvider::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (row < 0 || row >= m_procList.count() || column < 0 || column >= COLUMN_COUNT )
        return QModelIndex();
    return createIndex(row, column);
}

QVariant ProcProvider::data(const QModelIndex & index, int role) const {
    // if (index.row() < 0 || index.row() >= m_procList.count())
    //     return QVariant();
    if (!index.isValid()) return QVariant();

    const vk_proc_info &proc = m_procList[index.row()];
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return proc.pid;
        } else if (index.column() == 1) {
            return QString::fromStdString(proc.comm);
        } else if (index.column() == 2) {
            return humanMem(proc.mem);
        } else if (index.column() == 3) {
            return proc.th_all;
        } else if (index.column() == 4) {
            return proc.vm;
        } else if (index.column() == 5) {
            return proc.th_active;
        } else if (index.column() == 6) {
            return QDateTime::fromSecsSinceEpoch(proc.tm).toString(Qt::ISODate);
        } else return QVariant();
    } else  return QVariant();
}
#endif

// for QML/ListModel
QVariant ProcProvider::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= m_procList.count())
        return QVariant();

    const vk_proc_info &proc = m_procList[index.row()];
    if (role == PidRole)
        return proc.pid;
    else if (role == CommRole)
        return QString::fromStdString(proc.comm);
    else if (role == MemRole)
        return humanMem(proc.mem);
    else if (role == Th_allRole)
        return proc.th_all;
    else if (role == VmRole)
        return proc.vm;
    else if (role == Th_activeRole)
        return proc.th_active;
    else if (role == TmRole)
        return QDateTime::fromSecsSinceEpoch(proc.tm).toString(Qt::ISODate);
    else return QVariant();
}

QHash<int, QByteArray> ProcProvider::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[PidRole] = "pid";
    roles[CommRole] = "comm";
    roles[MemRole] = "mem";
    roles[VmRole] = "vm";
    roles[Th_allRole] = "th_all";
    roles[Th_activeRole] = "th_active";
    roles[TmRole] = "tm";
    return roles;
}

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
