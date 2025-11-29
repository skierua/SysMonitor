#ifndef LIB_H
#define LIB_H

// #include <iostream>
#include <vector>
#include <cstdint>
#include <libproc.h>
#include <unistd.h>
#include <signal.h> // For kill()
#include <sys/sysctl.h> // For sysctl() and related definitions
#include <mach/mach.h>  // for host_statistics64

#include "../shared/stru.h"

// using std::vector;

namespace MacLib {

std::string sysLogPath()
// { return "~Library/Logs"; }
{ return "./Logs"; }    // default for testing

int getCrntEUID(){
    return geteuid();
}

int termProc(int pid){
    int signal = SIGTERM;
    // std::cout << "BEFORE lib termProc pid=" << pid
    //           << ". Reason: " << strerror(errno) << std::endl;
    int ok = proc_terminate(pid, &signal);
    if (ok) return ok;
    usleep(50000);
    if (kill(pid, 0) == -1 && errno == ESRCH){
        ok = 0;
    } else {
        ok = -1;
    }
    // std::cout << "AFTER lib termProc pid=" << pid << " res=" << ok
    // << ". Reason: " << strerror(errno) << std::endl;
    return ok;
}
// sysctl based
std::vector<vk_proc_info> getProc(){
    int mib[4]; // Management Information Base (MIB) array
    size_t len;
    // vector<vk_proc_info> res;
    std::vector<vk_proc_info> res;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    mib[3] = 0;

    // Get the len of the data first
    if (sysctl(mib, 4, NULL, &len, NULL, 0) == -1) {
        // std::cerr << "1 sysctl (len):" << std::strerror(errno) << std::endl;
        return res;
    }
    // Allocate memory for the process information
    std::vector<kinfo_proc> processes(len / sizeof(kinfo_proc));

    // Second call to sysctl to retrieve the process information
    if (sysctl(mib, 4, processes.data(), &len, NULL, 0) == -1) {
        // std::cerr << "2 sysctl (data):" << std::strerror(errno) << std::endl;
        return res;
    }
    // vk_proc_info irow;
    struct proc_taskinfo procInfo; // Structure to store BSD process information
    for (size_t i = 0; i < len / sizeof(kinfo_proc); ++i) {

        int pti = proc_pidinfo(processes[i].kp_proc.p_pid, PROC_PIDTASKINFO, 0, &procInfo, sizeof(procInfo));

        if (pti <= 0) {
            // std::cerr << "3 proc_pidinfo["<< processes[i].kp_proc.p_pid
            //           << ", " << processes[i].kp_proc.p_comm << "]:" << std::strerror(errno) << std::endl;
            // return 1;
        }
        res.push_back(vk_proc_info());
        res[res.size()-1].pid = processes[i].kp_proc.p_pid;
        res[res.size()-1].ppid = 0;
        res[res.size()-1].comm = processes[i].kp_proc.p_comm;
        res[res.size()-1].mem = procInfo.pti_resident_size;
        res[res.size()-1].vm = procInfo.pti_virtual_size;
        res[res.size()-1].th_all = procInfo.pti_threadnum;
        res[res.size()-1].th_active = procInfo.pti_numrunning;
        res[res.size()-1].tm = processes[i].kp_proc.p_starttime.tv_sec;
        res[res.size()-1].uid = processes[i].kp_eproc.e_pcred.p_ruid; // Real UID
        // std::cout << "PID: " << res.back().pid
        //           << ", comm: " << res.back().comm
        //           << ",\tmem=" << res.back().mem << "/" << res.back().vm
        //           << ",\tth=" << res.back().th_all << "/" << res.back().th_active
        //           << ",\ttm=" << (procInfo.pti_total_user + procInfo.pti_total_system)/1e9
        //           << ",\ttm=" << res.back().tm
        //           << std::endl;
    }

    // std::sort(res.begin(), res.end(), [](vk_proc_info a,vk_proc_info b){ return a.mem > b.mem;});
    return res;
}

// procinfo.h based
VProcInfoList getProcList(){
    VProcInfoList res;
    size_t len = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    if (len <= 0) {
        // perror("proc_listpids failed to estimate size");
        return res;
    }

    std::vector<pid_t> pids(len / sizeof(pid_t));
    int ret = proc_listpids(PROC_ALL_PIDS, 0, pids.data(), len);
    if (ret <= 0) {
        // perror ("proc_listpids failed to retrieve PIDs");
        return res;
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
        vpri.comm = bsd_info.pbi_comm;
        vpri.mem = procInfo.pti_resident_size;
        vpri.vm = procInfo.pti_virtual_size;
        vpri.th_all = procInfo.pti_threadnum;
        vpri.th_active = procInfo.pti_numrunning;
        vpri.tm = bsd_info.pbi_start_tvsec;
        vpri.uid = bsd_info.pbi_uid;
        res << vpri;
        // std::cout << pids[i] << std::endl;
    }

    return res;
}

QString getProcPath(int pid) {
    // pid_t pid = getpid();
    int buffer_size = PROC_PIDPATHINFO_MAXSIZE; // Max size defined in libproc.h
    std::vector<char> buffer(buffer_size);

    // Get the path of the executable for the current PID
    int ret = proc_pidpath(pid, buffer.data(), buffer_size);

    if (ret > 0) {
        // ret is the length of the path string
        // return std::string(buffer.data());
        return QString(buffer.data());
    } else {
        // Error handling: process not found, permission denied, etc.
        // perror("proc_pidpath failed");
        return QString("");
    }
}

uint64_t getRAMSize() {
    uint64_t res{0};  // same as unsigned long long
    size_t len = sizeof(res);

    int mib[2]; // Management Information Base (MIB) array
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    if (sysctl(mib, 2, &res, &len, NULL, 0) == -1) {
        // std::cerr << "1 sysctl (len):" << std::strerror(errno) << std::endl;
        return 0;
    }

    return res;
}

// res =0 for error, but it's not fair enought
uint64_t getRAMUsage() {
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

}


#endif // LIB_H
