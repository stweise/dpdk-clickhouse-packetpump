# dpdk-clickhouse-packetpump

A DPDK based packet dumping utility that puts all packet details into a clickhouse database.

## Description

Sometimes all you want to do is connect a device to a network port and get all the packets on there into a nice fast database to run queries on it as you like. This project achieves this using a Network Switch, an Intel NUC, Linux, DPDK, and Clickhouse.

TODO: put image of setup here

This is a minimal setup only, the solution provided here, will work with much more performant hardware too.

TODO: put some claims about performance in a table

## Getting Started

### Dependencies

* A network switch - model used: HPE OfficeConnect 1820 Series Switch J9979A
** but you can use any network switch that supports port mirroring (or full switch mirroring if you want)
* An Intel NUC - model used: BXNUC10i3FNH2
** but you can use any notebook or PC with a network card supported by DPDK [DPDK supported network cards](https://core.dpdk.org/supported/)
* Ubuntu Linux 20.04
* DPDK 19.11
* Clickhouse (including driver) 
** TODO: clickhouse version
** TODO: clickhouse-cpp driver repo
** TODO: mention multithreading limitation of driver and link github issue

### Installing

* Install Ubuntu Linux 20.04 on Intel NUC
** TODO: find simple guide how to do this
* TODO: Install ansible
* TODO: clone this repo on the machine
* TODO: run ansible and reboot
* TODO: configure network switch, screenshots + model I used
* TODO: setup clickhouse
* TODO: setup grafana
* use CMake to build the application
```
# mkdir build
# cd build
# cmake ..
# make
```

### Executing program

* TODO: How to run the program
```
# ./run_pcap_test.sh
```
* TODO: How to query clickhouse + maybe Grafana?

## Version History

* TODO: release 0.1

## License
[BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)
2021 Steffen Weise
