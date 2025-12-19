#include "kernelproxy.h"

// KernelProxy::KernelProxy(){}

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
    if (!kill(pid, signal)) {
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
}

VProcInfoList KernelProxy::procList() {
    VProcInfoList res;
    DIR* dir = opendir("/proc");
    if (dir == nullptr){
        m_lastError = strerror(errno);
        return std::move(res);
    }
    auto l_isPid = [](const char* name){
        for (auto i{0}; name[i] != '\0'; ++i){
            if (!isdigit(name[i])) return false;
        }
        return true;
    };

    auto l_bootTime = []() -> unsigned long long{
        std::ifstream file("/proc/stat", std::ios_base::in);
        std::string line;
        if (!file.is_open()) return 0;
        while (std::getline(file, line)) {
            if (line.substr(0,5) == "btime") {
                return std::stoull(line.substr(6));
            }
        }

        return 0;
    };

    auto l_uid = [](auto pid) ->int{
        std::ifstream file("/proc/"+ std::to_string(pid) + "/status", std::ios_base::in);
        std::string line;
        if (!file.is_open()) return -1;
        while (std::getline(file, line)) {
            if (line.substr(0,4) == "Uid:") {
                std::stringstream ss(line);
                std::string tmp;
                int realUid;
                ss >> tmp >> realUid;
                return realUid;
            }
        }

        return -1;
    };

    struct dirent* entry;
    // auto statShift{1};
    auto testShift{3};
    // auto r{0};
    while ((entry = readdir(dir)) != nullptr){
        if (entry->d_type == DT_DIR && l_isPid(entry->d_name)){
            vk_proc_info vpri;
            std::string spid = entry->d_name;
            std::string sfname = "/proc/"+ spid + "/stat";
            /*
            std::ifstream stat_file(sfname, std::ios_base::in);
            std::string stat_tmp;
            std::vector<std::string> stat_data;
            // std::cout << "sfname=" << sfname << " ps=" << m_pageSize << std::endl;
            if (stat_file.is_open()) {
                ++r;
                while (std::getline(stat_file, stat_tmp, ' ')){
                    stat_data.push_back(stat_tmp);
                }
            } else continue;
            vpri.pid = std::stoi(spid) ;
            vpri.ppid = std::stoi(stat_data[4 - statShift]);
            vpri.qcomm = QString::fromStdString(stat_data[2 - statShift]);   // QString(PIDS_VAL(PIDS_CMD, str, stack, info));
            vpri.mem = std::stoul(stat_data[24 - statShift]) * m_pageSize;
            vpri.vm = std::stoul(stat_data[23 - statShift]) * m_pageSize;
            vpri.th_all = std::stoi(stat_data[20 - statShift]);
            vpri.th_active = 0;
            vpri.tm = l_bootTime() + std::stoull(stat_data[22 - statShift]);    //bsd_info.pbi_start_tvsec;
            vpri.uid = l_uid(vpri.pid);
*/
            std::ifstream test_stream(sfname, std::ios_base::in);
            std::string test_stat;
            std::string comm;
            std::vector<std::string> test_data;

            if (test_stream.is_open()){
                std::getline(test_stream,test_stat);
                auto commSta{test_stat.find('(')};
                auto commFin{test_stat.rfind(')')};
                if (!(commSta < test_stat.size()-2) || !(commFin < test_stat.size() - 2)) continue;
                ++commSta;
                comm = test_stat.substr(commSta,commFin - commSta);
                commFin += 2;
                for (auto prev{commFin}, curr{commFin}; curr < test_stat.size(); ++curr){
                    if (test_stat[curr] == ' ') {
                        test_data.push_back(test_stat.substr(prev, curr - prev));
                        prev = curr;
                    }
                }
                // for (auto& v: test_data) std::cout << v << " ";
                // std::cout << std::endl;
            }

            vpri.pid = std::stoi(spid) ;
            vpri.ppid = std::stoi(test_data[4 - testShift]);
            vpri.qcomm = QString::fromStdString(comm);   // QString(PIDS_VAL(PIDS_CMD, str, stack, info));
            vpri.mem = std::stoul(test_data[24 - testShift]) * m_pageSize;
            vpri.vm = std::stoul(test_data[23 - testShift]) * m_pageSize;
            vpri.th_all = std::stoi(test_data[20 - testShift]);
            vpri.th_active = 0;
            vpri.tm = l_bootTime() + std::stoull(test_data[22 - testShift]) / m_ticsPerSec;    //bsd_info.pbi_start_tvsec;
            vpri.uid = l_uid(vpri.pid);

            res << vpri;
            // std::cout << "pid=" << vpri.pid << "\t"
            //           << "" << stat_data[2 - statShift] << "\t"
            //           << " uid=" << vpri.uid << "\t"
            //           << "" << comm << "\t"
            //           // << " bt=" << l_bootTime() << "\t"
            //           << std::endl;
        }
    }

    return std::move(res);
}

/*
 * unfortumatly can't done
 * unpredictable behaviour & info/man leak
VProcInfoList KernelProxy::procList_libproc2() {
    VProcInfoList res;
    struct pids_info *info = nullptr;
    struct pids_stack *stack{nullptr};
    enum pids_item items[] = {
        PIDS_ID_PID,            //    s_int        from /proc/<pid>
        PIDS_ID_PPID,           //    s_int        stat: ppid or status: PPid
        PIDS_ID_TGID,           //    s_int        status: Tgid
        PIDS_CMD,               //      str        stat: comm or status: Name
        // , PIDS_CMDLINE
        // , PIDS_EXE
        PIDS_MEM_RES,           //   ul_int        derived from MEM_RES_PGS, as KiB
        PIDS_MEM_VIRT,          //   ul_int        derived from MEM_VIRT_PGS, as KiB
        PIDS_ID_FUID,           //    u_int        status: Uid
        PIDS_NS_TIME,           //   ul_int         "
    };
    int count = sizeof(items) / sizeof(items[0]);
    fatal_proc_unmounted(NULL, 0);
    if (procps_pids_new(&info, items, count) < 0){
        m_lastError = strerror(errno);
        return std::move(res);
    }
    // std::cout << "size=" << sizeof(info) << std::endl;
    auto ic{0};
    // while ((stack = procps_pids_get(info, PIDS_FETCH_THREADS_TOO))){
    while ((stack = procps_pids_get(info, PIDS_FETCH_TASKS_ONLY))){
        ++ic;
        if (PIDS_VAL(PIDS_ID_PID, s_int, stack, info) == 0) continue;
        vk_proc_info vpri;
        // const char* comm = get_str_item(stack, PIDS_EXE);
        // QString comm = QString(PIDS_VAL(PIDS_EXE, str, stack, info));
        // std::cout << PIDS_VAL(PIDS_ID_PID, s_int, stack, info) << "\t"
        //           << PIDS_VAL(PIDS_ID_PPID, s_int, stack, info) << "\t"
        //           // << PIDS_VAL(PIDS_CMD, str, stack, info) << "\t"
        //           // << PIDS_VAL(PIDS_CMDLINE, str, stack, info) << "\t"
        //           // << comm << "\t"
        //           << std::endl;
        // qDebug() << comm;
        vpri.pid = PIDS_VAL(PIDS_ID_TGID, s_int, stack, info);
        vpri.ppid = PIDS_VAL(PIDS_ID_PPID, s_int, stack, info);
        // vpri.qcomm = QString("");   // QString(PIDS_VAL(PIDS_CMD, str, stack, info));
        vpri.mem = PIDS_VAL(PIDS_MEM_RES, ul_int, stack, info);    // procInfo.pti_resident_size;
        vpri.vm = PIDS_VAL(PIDS_MEM_VIRT, ul_int, stack, info);    // procInfo.pti_virtual_size;
        vpri.th_all = 0;
        vpri.th_active = 0;
        vpri.tm = 0;    //bsd_info.pbi_start_tvsec;
        vpri.uid = PIDS_VAL(PIDS_ID_FUID, u_int, stack, info);
        res << vpri;
        std::cout << "stack=" << stack << std::endl;
    }
    std::cout << "ic=" << std::to_string(ic) << std::endl;

    procps_pids_unref(&info);
    return std::move(res);

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
*/

QString KernelProxy::procPath(int pid) {
    if (pid < 1) return QString("");
    std::string path = "/proc/" + std::to_string(pid) + "/exe";
    // std::vector<char> buffer(PATH_MAX);
    char buffer[PATH_MAX];
    auto ret = readlink(path.c_str(), buffer, sizeof(buffer)-1);
    if (ret > 0) {
        // ret is the length of the path string
        buffer[ret] = '\0';
        return QString(buffer);
    } else {
        m_lastError = strerror(errno);
        return QString("");
    }
}

uint64_t KernelProxy::sizeRAM() {
    uint64_t res{0};  // same as unsigned long long
    struct sysinfo info;
    if (sysinfo(&info) != 0){
        m_lastError = strerror(errno);
        return res;
    }
    res = (info.totalram) * info.mem_unit;
    return res;
}

uint64_t KernelProxy::usageRAM() {
    uint64_t res{0};  // same as unsigned long long
    struct sysinfo info;
    if (sysinfo(&info) != 0){
        m_lastError = strerror(errno);
        return res;
    }
    // std::cout << "used=" << info.totalram - info.freeram
    //           << " unit=" << info.mem_unit << std::endl;
    res = (info.totalram - info.freeram) * info.mem_unit;
    return res;
}
