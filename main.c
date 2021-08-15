/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdint.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include "use_clickhouse_driver.h"
#include "helpers.h"

#define DEBUG
#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static inline size_t get_vlan_offset(struct rte_ether_hdr *eth_hdr, uint16_t *proto);

static const struct rte_eth_conf port_conf_default =
{
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};

/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
static inline int
port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 1;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if(!rte_eth_dev_is_valid_port(port))
	{
		return -1;
	}

	retval = rte_eth_dev_info_get(port, &dev_info);
	if(retval != 0)
	{
		printf("Error during getting device (port %u) info: %s\n",
		       port, strerror(-retval));
		return retval;
	}

#ifdef DEBUG
	printf("Port: %u Driver name %s\n", port, dev_info.driver_name);
#endif

	if(dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
		    DEV_TX_OFFLOAD_MBUF_FAST_FREE;

	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if(retval != 0)
	{
		return retval;
	}

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if(retval != 0)
	{
		return retval;
	}

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for(q = 0; q < rx_rings; q++)
	{
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
		                                rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if(retval < 0)
		{
			return retval;
		}
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for(q = 0; q < tx_rings; q++)
	{
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
		                                rte_eth_dev_socket_id(port), &txconf);
		if(retval < 0)
		{
			return retval;
		}
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if(retval < 0)
	{
		return retval;
	}

	/* Display the port MAC address. */
	struct rte_ether_addr addr;
	retval = rte_eth_macaddr_get(port, &addr);
	if(retval != 0)
	{
		return retval;
	}

	printf("Port %u set up for use with DPDK MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
	       " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
	       port,
	       addr.addr_bytes[0], addr.addr_bytes[1],
	       addr.addr_bytes[2], addr.addr_bytes[3],
	       addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	retval = rte_eth_promiscuous_enable(port);
	if(retval != 0)
	{
		return retval;
	}

	return 0;
}

/*
 * The lcore main. This is the main thread that does the work, reading from
 * an input port and writing to an output port.
 */
static __attribute__((noreturn)) void lcore_main(void)
{
	//Receive packets on the port
	uint16_t port = 0;

	/*
	 * Check that the port is on the same NUMA node as the polling thread
	 * for best performance.
	 */
		if(rte_eth_dev_socket_id(port) > 0 &&
		        rte_eth_dev_socket_id(port) !=
		        (int)rte_socket_id())
			printf("WARNING, port %u is on remote NUMA node to "
			       "polling thread.\n\tPerformance will "
			       "not be optimal.\n", port);

	printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n",
	       rte_lcore_id());

	CHconstruct();
	/* Run until the application is quit or killed. */
	for(;;)
	{
		/* Get burst of RX packets, from first port of pair. */
		struct rte_mbuf *pkts_burst[BURST_SIZE];
		uint16_t nb_rx = rte_eth_rx_burst(port, 0,
		                                  pkts_burst, BURST_SIZE);

		//break loop if there are no packets in the buffer
		if(unlikely(nb_rx == 0))
		{
			continue;
		}

#ifdef DEBUG
		printf("------------------\n");
		printf("Packets rcv: %u\n", nb_rx);
#endif
		uint16_t p;
		uint16_t offset, ether_type;
		struct rte_ipv4_hdr *ipv4_hdr;
		for(p = 0; p < nb_rx; p++)
		{
#ifdef DEBUG
			printf("--------------------------------------\n");
			printf("\tEther\n");
#endif
			uint32_t pkt_length = rte_pktmbuf_pkt_len(pkts_burst[p]);// - sizeof(struct rte_ether_hdr);
#ifdef DEBUG
			printf("\tPacket length: %u\n", pkt_length);
#endif
			//Clickhouse
			struct packet pa;
			pa.pkt_length = pkt_length;
			struct rte_ether_hdr *eth_hdr;
			eth_hdr = rte_pktmbuf_mtod(pkts_burst[p], struct rte_ether_hdr *);
#ifdef DEBUG
			printf("\tSRC-MAC: %02" PRIx8 ":%02" PRIx8 ":%02" PRIx8
			       ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 "\n",
			       eth_hdr->s_addr.addr_bytes[0], eth_hdr->s_addr.addr_bytes[1],
			       eth_hdr->s_addr.addr_bytes[2], eth_hdr->s_addr.addr_bytes[3],
			       eth_hdr->s_addr.addr_bytes[4], eth_hdr->s_addr.addr_bytes[5]);
			printf("\tDST-MAC: %02" PRIx8 ":%02" PRIx8 ":%02" PRIx8
			       ":%02" PRIx8 ":%02" PRIx8 ":%02" PRIx8 "\n",
			       eth_hdr->d_addr.addr_bytes[0], eth_hdr->d_addr.addr_bytes[1],
			       eth_hdr->d_addr.addr_bytes[2], eth_hdr->d_addr.addr_bytes[3],
			       eth_hdr->d_addr.addr_bytes[4], eth_hdr->d_addr.addr_bytes[5]);
#endif
			ether_type = eth_hdr->ether_type;
			offset = get_vlan_offset(eth_hdr, &ether_type);
#ifdef DEBUG
			if(offset > 0)
			{
				printf("\tVLAN tagged frame, offset:%u\n", offset);
			}
#endif
			printf("\t----------------------------------\n");
			if(ether_type == rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4))
			{
				ipv4_hdr = (struct rte_ipv4_hdr *)((char *)(eth_hdr + 1) + offset);
				printf("\t\tIpv4\n");
				printf("\t\tIp_version=0x%02x\n", (ipv4_hdr->version_ihl & 0xf0) >> 4);
				printf("\t\tIp_ihl=0x%02x\n", ipv4_hdr->version_ihl & 0x0f);
				printf("\t\tIp_tos=0x%02x\n", ipv4_hdr->type_of_service);
				printf("\t\tIp_totallength=0x%02x\n", ipv4_hdr->total_length);
			  pa.ip_total_length = rte_be_to_cpu_16(ipv4_hdr->total_length);
				printf("\t\tIp_totallength=%d\n", rte_be_to_cpu_16(ipv4_hdr->total_length));
				printf("\t\tIp_id=0x%02x\n", ipv4_hdr->packet_id);
				printf("\t\tIp_flags=0x%02x\n", (ipv4_hdr->fragment_offset & 0xe0) >> 13);
				printf("\t\tIp_fragentoffset=0x%02x\n", (ipv4_hdr->fragment_offset & 0x1f));
				printf("\t\tIp_ttl=0x%02x\n", ipv4_hdr->time_to_live);
				printf("\t\tIp_proto=0x%02x\n",  ipv4_hdr->next_proto_id);
				printf("\t\tIp_chksum=0x%02x\n", ipv4_hdr->hdr_checksum);
				printf("\t\tIp_src="IPv4_BYTES_FMT"\n", IPv4_BYTES(rte_cpu_to_be_32(ipv4_hdr->src_addr)));
				snprintf(pa.ip_src_addr, 23, IPv4_BYTES_FMT, IPv4_BYTES(rte_cpu_to_be_32(ipv4_hdr->src_addr))); // 15 + 7("::ffff:") + 1
				printf("\t\tIp_dst="IPv4_BYTES_FMT"\n", IPv4_BYTES(rte_cpu_to_be_32(ipv4_hdr->dst_addr)));
				snprintf(pa.ip_dst_addr, 23, IPv4_BYTES_FMT, IPv4_BYTES(rte_cpu_to_be_32(ipv4_hdr->dst_addr)));
				// for now: only submit data if it is an IPv4 packet 
				CHappend(&pa);
			}
			else
			{
				printf("\t\tNOTIPv4\n");
			}
			printf("\t----------------------------------\n");
			printf("--------------------------------------\n");
		}
		CHinsert();

		/* Free any unsent packets. */
		uint16_t buf;
		for(buf = 0; buf < nb_rx; buf++)
		{
			rte_pktmbuf_free(pkts_burst[buf]);
		}
	}
}

static inline size_t get_vlan_offset(struct rte_ether_hdr *eth_hdr, uint16_t *proto)
{
	size_t vlan_offset = 0;
	if(rte_cpu_to_be_16(RTE_ETHER_TYPE_VLAN) == *proto)
	{
		struct rte_vlan_hdr *vlan_hdr =
		    (struct rte_vlan_hdr *)(eth_hdr + 1);
		vlan_offset = sizeof(struct rte_vlan_hdr);
		*proto = vlan_hdr->eth_proto;
		if(rte_cpu_to_be_16(RTE_ETHER_TYPE_VLAN) == *proto)
		{
			vlan_hdr = vlan_hdr + 1;
			*proto = vlan_hdr->eth_proto;
			vlan_offset += sizeof(struct rte_vlan_hdr);
		}
	}
	return vlan_offset;
}


/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
int
main(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if(ret < 0)
	{
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
	}

	argc -= ret;
	argv += ret;

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count_avail();
	printf("INFO: Number of ports: %i.\n", nb_ports);
	//if (nb_ports < 2 || (nb_ports & 1))
	//		rte_exit(EXIT_FAILURE, "Error: number of ports must be even\n");

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS,
	                                    MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if(mbuf_pool == NULL)
	{
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
	}

	port_init(0, mbuf_pool);
	printf("INIT: Successfully initialized PCAP port %" PRIu16 "\n", 0);
	//fill array of pcap port pair, with valid portid
	lcore_main();
	return 0;
}
