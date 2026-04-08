cmake_minimum_required(VERSION 3.16)

project(SysMonitor
    VERSION 2.0
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# VY Add include directories so target can find the QML_ELEMENTS
set(QML_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core)
# find_package(Qt6 REQUIRED COMPONENTS Qml)
# find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Graphs)


qt_standard_project_setup(REQUIRES 6.8)

set(CPP_OS_COMMON
    shared/stru.h
    shared/TempLib.h
    shared/procprovider.h shared/procprovider.cpp
    shared/memprovider.h shared/memprovider.cpp
    shared/logger.h shared/logger.cpp
)

set(CPP_MAC_SPECIFIC
    spec/mackernel.h
    spec/mackernel.cpp
)

set(CPP_LINUX_SPECIFIC
    linux/linuxkernel.h
    linux/linuxkernel.cpp
)

set(CPP_WIN_SPECIFIC
    spec/winkernel.h
    spec/winkernel.cpp
)

set(JS_FILES
    qt/qml/lib.js
)

# Includes OS-specific C++ sources
set(CPP_OS_SPECIFIC )

qt_add_executable(appSysMonitor  WIN32 MACOSX_BUNDLE
    main.cpp
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "macOS-specific QML and sources.")
    # macOS-specific files
    set(CPP_OS_SPECIFIC
        spec/mackernel.h
        spec/mackernel.cpp)

    # list(APPEND QML_SOURCES_COMMON macos/kernelproxy.h macos/kernelproxy.cpp)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows-specific QML and sources.")
    # Windows-specific files
    # target_link_libraries(${PROJECT_NAME} PRIVATE Psapi)
    set(CPP_OS_SPECIFIC
        spec/winkernel.h
        spec/winkernel.cpp
`   )
    # list(APPEND QML_SOURCES_COMMON winos/kernelproxy.h winos/kernelproxy.cpp)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Linux-specific QML and sources.")
    set(HAS_PROC_FS, FALSE)
    # Linux-specific files
    if(IS_DIRECTORY "/proc")
        set(CPP_OS_SPECIFIC
            spec/linuxkernel.h
            spec/linuxkernel.cpp
        )
        # list(APPEND QML_SOURCES_COMMON linux/kernelproxy.h linux/kernelproxy.cpp)
        # target_link_libraries(appSysMonitor PRIVATE proc2)
        set(HAS_PROC_FS, TRUE)
    endif()
    if (!HAS_PROC_FS)
        message(FATAL_ERROR "Unspported linux distro. Folder /proc doesn't exist.")
    endif()
else()
    message(FATAL_ERROR "Unsupported OS. No kernelproxy.h defined for ${CMAKE_SYSTEM_NAME}.")
endif()

# if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     message(STATUS "Linux-specific QML and sources.")
#     set(HAS_PROC_FS, FALSE)
#     # Linux-specific files
#     if(IS_DIRECTORY "/proc")
#         set(HAS_PROC_FS, TRUE)
#     endif()
#     if (!HAS_PROC_FS)
#         message(FATAL_ERROR "Unspported linux distro. Folder /proc doesn't exist.")
#     endif()
# endif()

qt_add_qml_module(appSysMonitor
    URI SysMonitor
    VERSION 1.0
    QML_FILES
        qt/qml/Main.qml
        qt/qml/ProcViewDelegate.qml
        qt/qml/ProcViewHighlight.qml
        qt/qml/HeaderElem.qml
        qt/qml/MemView.qml
        ${JS_FILES}
    SOURCES
        ${CPP_OS_COMMON}
        # ${CPP_MAC_SPECIFIC}
        # ${CPP_LINUX_SPECIFIC}
        # ${CPP_WIN_SPECIFIC}
        ${CPP_OS_SPECIFIC}
        # ${QML_SOURCES_COMMON}
    RESOURCES
        TODO.txt
        README.md
        .gitignore
)

# qt_add_qml_module(appSysMonitor
#   URI com.sysmonitor
#   VERSION 1.0
# )

# VY Add includepaths
target_include_directories(appSysMonitor PRIVATE
   ${QML_INCLUDES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSysMonitor PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSysMonitor
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSysMonitor PRIVATE Qt6::Quick)
target_link_libraries(appSysMonitor PRIVATE Qt6::Core)
# target_link_libraries(appSysMonitor PRIVATE Qt6::Qml)
target_link_libraries(appSysMonitor PRIVATE Qt6::Graphs)



include(GNUInstallDirs)
install(TARGETS appSysMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
