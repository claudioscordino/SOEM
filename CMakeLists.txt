cmake_minimum_required(VERSION 2.8.4)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
project(SOEM C)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # Default to installing in SOEM source directory
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)
endif()

set(SOEM_INCLUDE_INSTALL_DIR include/soem)
set(SOEM_LIB_INSTALL_DIR lib)
set(BUILD_TESTS TRUE)

if(WIN32)
  set(OS "win32")
  include_directories(oshw/win32/wpcap/Include)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    link_directories(${CMAKE_SOURCE_DIR}/oshw/win32/wpcap/Lib/x64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    link_directories(${CMAKE_SOURCE_DIR}/oshw/win32/wpcap/Lib)
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  /WX")
  set(OS_LIBS wpcap.lib Packet.lib Ws2_32.lib Winmm.lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "linux")
  set(OS "linux")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(OS_LIBS pthread rt)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rt-kernel")
  set(OS "rtk")
  message("ARCH is ${ARCH}")
  message("BSP is ${BSP}")
  include_directories(oshw/${OS}/${ARCH})
  file(GLOB OSHW_EXTRA_SOURCES oshw/${OS}/${ARCH}/*.c)
  set(OSHW_SOURCES "${OS_HW_SOURCES} ${OSHW_ARCHSOURCES}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-but-set-variable")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format")
  set(OS_LIBS "-Wl,--start-group -l${BSP} -l${ARCH} -lkern -ldev -lsio -lblock -lfs -lusb -llwip -leth -li2c -lrtc -lcan -lnand -lspi -lnor -lpwm -ladc -ltrace -lc -lm -Wl,--end-group")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "rtems")
  message("Building for RTEMS")
  set(OS "rtems")
  set(SOEM_LIB_INSTALL_DIR ${LIB_DIR})
  set(BUILD_TESTS FALSE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "erika")
  message("Building for ERIKA Enterprise")
  set(OS "erika")
  set(LIB_DIR "out")
  set(X86_COMPILER_CC $ENV{CROSS_COMPILE}gcc)
  set(X86_COMPILER_CPP $ENV{CROSS_COMPILE}g++)
  message("CROSS_COMPILE = $ENV{CROSS_COMPILE}")
  if(X86_COMPILER_CC STREQUAL "gcc")
        message(FATAL_ERROR "CROSS_COMPILE not set!")
   endif()

  find_program(X86_BARE_CC ${X86_COMPILER_CC})
  find_program(X86_BARE_CPP ${X86_COMPILER_CPP})
  if(${X86_BARE_CC} MATCHES "X86_BARE_CC-NOTFOUND")
        message(FATAL_ERROR "${X86_COMPILER_CC} not found in path!")
  elseif(${X86_BARE_CPP} MATCHES "X86_BARE_CPP-NOTFOUND")
        message(FATAL_ERROR "${X86_COMPILER_CPP} not found in path!")
  endif()

  set(CMAKE_C_COMPILER ${X86_BARE_CC})
  set(CMAKE_CXX_COMPILER ${X86_BARE_CPP})
  set(CMAKE_FIND_ROOT_PATH ${X86_BARE_CC}/../)
  set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
  set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
  set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
  set(CMAKE_C_COMPILER_FORCED true)
  set(CMAKE_CXX_COMPILER_FORCED true)
##   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(SOEM_LIB_INSTALL_DIR ${LIB_DIR})
  set(BUILD_TESTS FALSE)
  add_definitions( -fno-pie -g -Os
        -Wstrict-prototypes -Wtype-limits -Wmissing-declarations -Wmissing-prototypes -fno-strict-aliasing
        -fomit-frame-pointer -fno-pic -fno-common -fno-stack-protector -m64 -mno-red-zone )
  set(CMAKE_EXE_LINKER_FLAGS "-Xlinker -melf_x86_64 -Wl,--gc-sections")
  message ("x86 compiler located as ${X86_BARE_CC}")
  message ("CMAKE_FIND_ROOT_PATH is ${CMAKE_FIND_ROOT_PATH}")
endif()

message("OS is ${OS}")

file(GLOB SOEM_SOURCES soem/*.c)
file(GLOB OSAL_SOURCES osal/${OS}/*.c)
file(GLOB OSHW_SOURCES oshw/${OS}/*.c)

file(GLOB SOEM_HEADERS soem/*.h)
file(GLOB OSAL_HEADERS osal/osal.h osal/${OS}/*.h)
file(GLOB OSHW_HEADERS oshw/${OS}/*.h)

include_directories(soem)
include_directories(osal)
include_directories(osal/${OS})
include_directories(oshw/${OS})

add_library(soem STATIC
  ${SOEM_SOURCES}
  ${OSAL_SOURCES}
  ${OSHW_SOURCES}
  ${OSHW_EXTRA_SOURCES})
target_link_libraries(soem ${OS_LIBS})

message("LIB_DIR: ${SOEM_LIB_INSTALL_DIR}")

install(TARGETS soem DESTINATION ${SOEM_LIB_INSTALL_DIR})
install(FILES
  ${SOEM_HEADERS}
  ${OSAL_HEADERS}
  ${OSHW_HEADERS}
  DESTINATION ${SOEM_INCLUDE_INSTALL_DIR})

if(BUILD_TESTS) 
  add_subdirectory(test/linux/slaveinfo)
  add_subdirectory(test/linux/eepromtool)
  add_subdirectory(test/linux/simple_test)
endif()
