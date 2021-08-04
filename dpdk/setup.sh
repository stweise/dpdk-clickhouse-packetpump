sudo modprobe uio_pci_generic
sudo dpdk-devbind.py -u 0000:00:1f.6
sudo dpdk-devbind.py -b uio_pci_generic 0000:00:1f.6
sudo dpdk-devbind.py --status
sudo mount -t hugetlbfs pagesize=1GB /mnt/huge
