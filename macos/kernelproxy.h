#ifndef KERNELPROXY_H
#define KERNELPROXY_H

#include <iostream>
#include <cerrno>
#include <libproc.h> // Include for macOS specific functions
#include <sys/sysctl.h> // For sysctl() and related definitions
#include <mach/mach.h>  // for host_statistics64
#include <signal.h> // For kill()
#include <sys/types.h>
#include <system_error>
#include <unistd.h>     // geteuid

#include "../shared/TempLib.h"

class KernelProxy : public StaticBase<KernelProxy>
{

public:
    static KernelProxy & getSelf() {
        static KernelProxy self;
        return self;
    };
    ~KernelProxy() noexcept = default;

    int test() const {return 42;}
    int crntEUID() const { return geteuid(); }
    int canTerminate(int pid);
    int termProc(int pid);
    VProcInfoList procList();
    QString procPath(int pid);
    uint64_t sizeRAM();
    uint64_t usageRAM();
    const QString& lastError();

private:
    static KernelProxy * self;

    QString m_lastError{QString("")};
    // KernelProxy() = default;
    // KernelProxy(const KernelProxy&)= delete;
    // KernelProxy& operator=(const KernelProxy&)= delete;
    // KernelProxy(KernelProxy&&)= delete;
    // KernelProxy& operator=(KernelProxy&&)= delete;
};

#endif // KERNELPROXY_H
