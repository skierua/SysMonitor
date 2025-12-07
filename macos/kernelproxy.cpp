#include "kernelproxy.h"

// KernelProxy::KernelProxy() {}

int KernelProxy::canTerminate(int pid) {
    return -1;
}

int KernelProxy::termProc(int pid) {
    return -1;
}

VProcInfoList KernelProxy::procList() {
    VProcInfoList res;
    return res;
}

QString KernelProxy::procPath(int pid) {
    QString res{QString("")};
    return res;
}

uint64_t KernelProxy::sizeRAM() {
    return 0;
}

uint64_t KernelProxy::usageRAM() {
    return 0;
}
