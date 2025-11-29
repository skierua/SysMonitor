#ifndef PROCPROVIDER_H
#define PROCPROVIDER_H

#include <QObject>
#include <QtQML>
#include <QAbstractListModel>
#include <QModelIndex>

#include <iostream>
#include <chrono> // For std::chrono::seconds, milliseconds, etc.
#include <thread> // For std::this_thread::sleep_for
#include <vector>

#include "stru.h"

class ProcProvider : public QAbstractListModel
{
    Q_OBJECT

    // QML_ELEMENT
    Q_PROPERTY(int crntPID MEMBER m_crntPID NOTIFY crntPIDChanged)
    Q_PROPERTY(int crntPIDIndex MEMBER m_crntPIDIndex NOTIFY crntPIDIndexChanged)

public:
    enum ProcRoles {
        PidRole = Qt::UserRole + 1,
        CommRole, MemRole, VmRole, Th_allRole,Th_activeRole, TmRole
    };
    explicit ProcProvider(QObject *parent = nullptr);

    // kernel/proclib.h leyer
    // func for processes info retrieving
    void setProcFn(std::function<std::vector<vk_proc_info>()> fn){
        m_procFn = std::move(fn); }

    // func for processes full system path
    void setProcPath(std::function<QString(int)> fn){
        m_procPath = std::move(fn); }

    // func for terminate process
    void setProcTerm(std::function<int(int)> fn){
        m_procTerm = std::move(fn); }

    // EUID current app
    void setEUID(int v) { m_crntEUID = v; }


    // QML adaptors for kernel/proclib.h leyer
    Q_INVOKABLE bool terminate() ;          // terminate current
    Q_INVOKABLE QString procPath();   // path for current
    Q_INVOKABLE int getPID(int row) const { return m_procList[row].pid; }
    Q_INVOKABLE bool canTerminate(int row) const
    { return (m_crntPID != 0
               && (m_crntEUID == 0 || m_crntEUID == m_procList[row].uid)); }

    // populate model
    void addProcList(VProcInfoList&& proc);

    // redefinition model's abstract func
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    // garbage
    // void start();
    // void addProc(const vk_proc_info &proc);
    // int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    // QModelIndex parent(const QModelIndex &) const override;
    // QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;


protected:
    QHash<int, QByteArray> roleNames() const override;


signals:
    // void emitTest(int);
    void crntPIDChanged();
    void crntPIDIndexChanged(int);
    void message(QString, int);

private:
    // uint m_pageCapacity{10};
    int m_crntPID{0};
    int m_crntPIDIndex{-1};
    // int m_crntPIDAttr{0};       // only for canTerminate

    int m_crntEUID{std::numeric_limits<int>::max()};
    bool m_lock{false};
    QList<vk_proc_info> m_procList;

    std::vector<vk_proc_info> m_procData;
    std::function<std::vector<vk_proc_info>()> m_procFn = [](){
             return std::vector<vk_proc_info>(); };
    std::function<QString(int)> m_procPath = [](int){
        return QString(""); };

    std::function<int(int)> m_procTerm = [](int){
        return -1; };
    QString humanMem(unsigned int mem) const;   // RAM size in B/kB/MB/GB

    void prnProc(std::vector<vk_proc_info>) const;  // in test purpose

    void lock(int t =100);  // lock data for update (kind of guard)
    void unlock();
};

#endif // PROCPROVIDER_H
