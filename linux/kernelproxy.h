#ifndef KERNELPROXY_H
#define KERNELPROXY_H

#include <iostream>
#include <cerrno>
#include <dirent.h>
#include <fstream>
#include <limits.h>
// #include <libproc2/pids.h>
// #include <ranges>    // unsupported by compiller
#include <sstream>
#include <signal.h> // For kill()
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>     // geteuid

#include <QString>
#include <QDebug>

#include "../shared/TempLib.h"

class KernelProxy : public StaticBase<KernelProxy>
{

public:
    static KernelProxy & getSelf() {
        static KernelProxy self;
        return self;
    }
    ~KernelProxy() noexcept = default;

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
    static KernelProxy * self;

    long int m_pageSize{sysconf(_SC_PAGESIZE)};
    long int m_ticsPerSec{sysconf(_SC_CLK_TCK)};
    QString m_lastError{QString("")};
    QString m_logPath{QString("./Logs")};
    // KernelProxy() = default;
    // KernelProxy(const KernelProxy&)= delete;
    // KernelProxy& operator=(const KernelProxy&)= delete;
    // KernelProxy(KernelProxy&&)= delete;
    // KernelProxy& operator=(KernelProxy&&)= delete;

// unfortumatly can't done
// unpredictable behaviour & info/man leak
    // VProcInfoList procList_libproc2();
};

#endif // KERNELPROXY_H
