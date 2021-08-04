CC = gcc 

src = $(wildcard *.c )
obj = $(src:.c=.o)

LDFLAGS = -L/usr/local/lib64 -lrte_bpf -lrte_flow_classify -lrte_pipeline -lrte_table -lrte_port -lrte_fib -lrte_ipsec -lrte_vhost -lrte_stack -lrte_security -lrte_sched -lrte_reorder -lrte_rib -lrte_rcu -lrte_rawdev -lrte_pdump -lrte_power -lrte_member -lrte_lpm -lrte_latencystats -lrte_kni -lrte_jobstats -lrte_ip_frag -lrte_gso -lrte_gro -lrte_eventdev -lrte_efd -lrte_distributor -lrte_cryptodev -lrte_compressdev -lrte_cfgfile -lrte_bitratestats -lrte_bbdev -lrte_acl -lrte_timer -lrte_hash -lrte_metrics -lrte_cmdline -lrte_pci -lrte_ethdev -lrte_meter -lrte_net -lrte_mbuf -lrte_mempool -lrte_ring -lrte_eal -lrte_kvargs
CFLAGS = -fno-omit-frame-pointer -O2 -I/usr/local/include -include rte_config.h -march=native -g -p -Wall -Wextra

TARGET = basicfwd.x

basicfwd.x: $(obj)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm basicfwd.x $(obj)
