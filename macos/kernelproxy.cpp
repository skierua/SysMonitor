#include "kernelproxy.h"

// KernelProxy::KernelProxy() {}

int KernelProxy::canTerminate(int pid) {
    if (pid < 2) return -1;

    return kill(pid, 0);
    // errno
    // ESRCH no such process(pid doesn't exist
    // EPERM no permission
    // EINVAL invalid signal

    // isn't better approach ???
    // geteuid() == 0 || geteuid() == pid
}

int KernelProxy::termProc(int pid) {
    int signal = SIGTERM;
    // std::cout << "BEFORE lib termProc pid=" << pid
    //           << ". Reason: " << strerror(errno) << std::endl;
    // int ok = proc_terminate(pid, &signal);
    // std::cout << "AFTER0 lib termProc pid=" << pid << " res=" << ok
    //           << ". Reason: " << strerror(errno) << std::endl;
    if (!proc_terminate(pid, &signal)) {
        m_lastError = strerror(errno);
        return -1;
    }
    usleep(50000);
    if (kill(pid, 0) == -1 && errno == ESRCH){
        return 0;
    } else {
        m_lastError = strerror(errno);
        return -1;
    }
    // std::cout << "AFTER lib termProc pid=" << pid << " res=" << ok
    // << ". Reason: " << strerror(errno) << std::endl;
}

VProcInfoList KernelProxy::procList() {
    VProcInfoList res;
    size_t len = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    if (len <= 0) {
        m_lastError = strerror(errno);
        return std::move(res);
    }

    std::vector<pid_t> pids(len / sizeof(pid_t));
    int ret = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), len);
    if (ret <= 0) {
        m_lastError = strerror(errno);
        return std::move(res);
    }

    int num_pids = ret / sizeof(pid_t);

    struct proc_taskinfo procInfo;
    struct proc_bsdinfo bsd_info;
    // std::cout << "lib getProcList ====================" << std::endl;
    for (int i = 0; i < num_pids; ++i) {
        vk_proc_info vpri;
        if (proc_pidinfo(pids[i], PROC_PIDTASKINFO, 0, &procInfo, sizeof(procInfo)) <= 0) continue;
        if (proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, &bsd_info, sizeof(bsd_info)) <= 0) continue;

        vpri.pid = pids[i];
        vpri.ppid = bsd_info.pbi_ppid;
        // vpri.comm = bsd_info.pbi_comm;
        vpri.qcomm = QString(bsd_info.pbi_comm);
        vpri.mem = procInfo.pti_resident_size;
        vpri.vm = procInfo.pti_virtual_size;
        vpri.th_all = procInfo.pti_threadnum;
        vpri.th_active = procInfo.pti_numrunning;
        vpri.tm = bsd_info.pbi_start_tvsec;
        vpri.uid = bsd_info.pbi_uid;
        res << vpri;
        // std::cout << pids[i] << std::endl;
    }

    return std::move(res);
}

QString KernelProxy::procPath(int pid) {
    if (pid < 1) return QString("");

    int buffer_size = PROC_PIDPATHINFO_MAXSIZE; // Max size defined in libproc.h
    std::vector<char> buffer(buffer_size);

    // Get the path of the executable for the current PID
    int ret = proc_pidpath(pid, buffer.data(), buffer_size);

    if (ret > 0) {
        // ret is the length of the path string
        return QString(buffer.data());
    } else {
        m_lastError = strerror(errno);
        return QString("");
    }
}

uint64_t KernelProxy::sizeRAM() {
    uint64_t res{0};  // same as unsigned long long
    size_t len = sizeof(res);

    int mib[2]; // Management Information Base (MIB) array
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    if (sysctl(mib, 2, &res, &len, NULL, 0) == -1) {
        m_lastError = strerror(errno);
        return 0;
    }

    return res;
}

uint64_t KernelProxy::usageRAM() {
    uint64_t res{0};  // same as unsigned long long

    vm_statistics64_data_t vm_stats;
    // mach_port_t host_port = mach_host_self();
    mach_msg_type_number_t len = HOST_VM_INFO64_COUNT;

    vm_size_t page_size;
    host_page_size(mach_host_self(), &page_size);

    kern_return_t ret = host_statistics64(
        mach_host_self(),
        HOST_VM_INFO64,
        (host_info64_t)&vm_stats,
        &len
        );

    if (ret != KERN_SUCCESS) {
        m_lastError = strerror(errno);
        // std::cerr << "host_statistics64 failed. Error code: " << kern_ret << std::endl;
        return res;
    }

    // uint64_t free_mem = (uint64_t)vm_stats.free_count * page_size;
    // uint64_t active_mem = (uint64_t)vm_stats.active_count * page_size;
    // uint64_t inactive_mem = (uint64_t)vm_stats.inactive_count * page_size;
    // uint64_t wired_mem = (uint64_t)vm_stats.wire_count * page_size;


    // not my idea, just expression from internet
    // yet confirmed by different sources
    res = (vm_stats.active_count
           + vm_stats.inactive_count
           + vm_stats.wire_count
           + vm_stats.speculative_count
           - vm_stats.purgeable_count) * page_size;


    return res;
}
