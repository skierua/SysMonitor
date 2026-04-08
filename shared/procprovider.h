#ifndef PROCPROVIDER_H
#define PROCPROVIDER_H

#include <QObject>
#include <QAbstractListModel>
#include <QModelIndex>

#include "stru.h"

using VProcInfoList = QList<vk_proc_info>;

class ProcProvider : public QAbstractListModel
{
    Q_OBJECT

    // QML_ELEMENT
    Q_PROPERTY(int crntPID MEMBER m_crntPID NOTIFY crntPIDChanged)
    Q_PROPERTY(int crntPIDIndex MEMBER m_crntPIDIndex NOTIFY crntPIDIndexChanged)
    Q_PROPERTY(QString nameFilter MEMBER m_nameFilter NOTIFY nameFilterChanged)

public:
    enum ProcRoles {
        PidRole = Qt::UserRole + 1,
        CommRole, MemRole, VmRole, Th_allRole,Th_activeRole, Th_strRole, TmRole
    };
    explicit ProcProvider(QObject *parent = nullptr);


    // QML adaptors for kernel/proclib.h leyer
    Q_INVOKABLE bool terminate() ;          // terminate current
    Q_INVOKABLE QString procPath();   // path for current
    Q_INVOKABLE int getPID(int row) const { return m_procList[row].pid; }
    Q_INVOKABLE int getPPID(int row) const { return m_procList[row].ppid; }
    Q_INVOKABLE int getEUID(int row) const { return m_procList[row].uid; }
    Q_INVOKABLE bool canTerminate() const;

    // populate model
    void addProcList(VProcInfoList&& proc);

    // redefinition model's abstract func
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;



protected:
    QHash<int, QByteArray> roleNames() const override;
    void lock(int t =100);  // lock data for update (kind of guard)
    void unlock();
    QString humanMem(unsigned int mem) const;   // RAM size in B/kB/MB/GB

    void prnProc() const;  // in test purpose


signals:
    // void emitTest(int);
    void crntPIDChanged();
    void crntPIDIndexChanged(int);
    void nameFilterChanged();
    void message(QString, int);

private:
    uint m_refreshCounter{0};
    pid_t m_crntPID{0};
    int m_crntPIDIndex{-1};
    // int m_crntPIDAttr{0};       // only for canTerminate

    QString m_nameFilter = QString();

    // int m_crntEUID{std::numeric_limits<int>::max()};
    bool m_lock{false};

    QList<vk_proc_info> m_procList;


};

#endif // PROCPROVIDER_H
