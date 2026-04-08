#ifndef LINUXKERNEL_H
#define LINUXKERNEL_H

#include <unistd.h>     // geteuid

#include "../shared/TempLib.h"

using VProcInfoList = QList<vk_proc_info>;

class LinuxKernel : public StaticBase<LinuxKernel>
{

public:
    static LinuxKernel & getSelf() {
        static LinuxKernel self;
        return self;
    }
    ~LinuxKernel() noexcept = default;

    int test() {return 42;}
    int crntEUID() { return geteuid(); }
    int canTerminate(int pid);
    int termProc(int pid);
    VProcInfoList procList();
    QString procPath(int pid);
    uint64_t sizeRAM();
    uint64_t usageRAM();
    const QString& lastError() { return m_lastError; }
    void setLogPath(const QString& path) { m_logPath = path; }
    const QString& logPath() { return m_logPath; }

private:
    static LinuxKernel * self;

    long int m_pageSize{sysconf(_SC_PAGESIZE)};
    long int m_ticsPerSec{sysconf(_SC_CLK_TCK)};
    QString m_lastError{QString("")};
    QString m_logPath{QString("./Logs")};
};

#endif // LINUXKERNEL_H
