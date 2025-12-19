## SysMonitor ##
MultiOS system monitor, pet project

### Features ###

- info about processes (permitted only)
    - PID
    - executable's name
    - executable's full system path
    - RAM usage
    - thread open/active
    - start up time
- possibility to terminate processes (PressAndHold for context menu)
- last minute RAM average usage chart

### MacOS ##
beta, ready to use
- Tahoe
- clang version 17.0.0 (clang-1700.4.4.1)
- Qt 6.10.0 for Desktop
- CMake 3.27.7(Qt)


### Windows ###
beta, ready to use
- Win10
    - MinGW 13.1.0 64-bit for c++ - it almost works. Unfortunautly unstable ))
    - MSVC2022 64bit
- Qt 6.10.0 for Desktop
- CMake 3.27.7(Qt)

### Linux ###
beta, ready to use
- kdeneon(ubuntu-latest)
- gcc version 13.3.0 (Ubuntu x86 64bit)
- Qt 6.10.0 for Desktop
- CMake 3.30.5
- /proc file system is mandatory


### Branches ###
- stable: stable
- main: current beta
- devel: unstable alpha
