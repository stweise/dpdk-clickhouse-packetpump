cmake_minimum_required(VERSION 3.19)
project (dpdk-clickhouse-packetdump)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

# We will use pkg-config to find dpdk
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

add_compile_options( -O2 -Wall -Wextra )


include_directories( /usr/local/include )
set_source_files_properties(clickhouse_driver.cpp PROPERTIES COMPILE_FLAGS -std=c++20)
add_executable(test_clickhouse_driver.x test_clickhouse_driver.c clickhouse_driver.cpp)
link_directories(/usr/local/lib)
target_link_libraries(test_clickhouse_driver.x clickhouse-cpp-lib)


add_executable(main.x main.c clickhouse_driver.cpp)
target_link_libraries(main.x ${DPDK_LIBRARIES} clickhouse-cpp-lib)
target_include_directories(main.x PUBLIC ${DPDK_INCLUDE_DIRS})
target_compile_options(main.x PUBLIC ${DPDK_CFLAGS_OTHER})

configure_file(scripts/run_pcap_test.sh run_pcap_test.sh COPYONLY)
