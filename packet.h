#ifndef PACKET_H
#define PACKET_H
#include <stdint.h>

struct packet {
  int pkt_length;
  int ip_total_length;
  char ip_src_addr[40];  // we want to make it simple therefore: reuse the
  // same field for ipv4 and ipv6 we will use a string representation as the
  // clickhouse driver insists on either string or in6_addr spec, the string
  // conversion is the format available in both cases 39+1 (for null character)
  char ip_dst_addr[40];
};
#endif  // PACKET_H
