#include "macmonitor.h"



/*MacMonitor::~MacMonitor() noexcept {
    // qDebug() << "~MacMonitor() terminated";
    // prc->terminate();
}*/

/*
void MacMonitor::start(){
}
*/

/*
vector<vk_proc_info> MacMonitor::proc(){
    int mib[4]; // Management Information Base (MIB) array
    size_t len;
    vector<vk_proc_info> res;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    mib[3] = 0;

    // Get the len of the data first
    if (sysctl(mib, 4, NULL, &len, NULL, 0) == -1) {
        std::cerr << "1 sysctl (len):" << std::strerror(errno) << std::endl;
        return res;
    }
    // Allocate memory for the process information
    std::vector<kinfo_proc> processes(len / sizeof(kinfo_proc));

    // Second call to sysctl to retrieve the process information
    if (sysctl(mib, 4, processes.data(), &len, NULL, 0) == -1) {
        std::cerr << "2 sysctl (data):" << std::strerror(errno) << std::endl;
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
        // std::cout << "PID: " << res.back().pid
        //           << ", comm: " << res.back().comm
        //           << ",\tmem=" << res.back().mem << "/" << res.back().vm
        //           << ",\tth=" << res.back().th_all << "/" << res.back().th_active
        //           << ",\ttm=" << (procInfo.pti_total_user + procInfo.pti_total_system)/1e9
        //           << ",\ttm=" << res.back().tm
        //           << std::endl;
    }


    std::sort(res.begin(), res.end(), [](vk_proc_info a,vk_proc_info b){ return a.mem > b.mem;});
    return res;
}
*/
