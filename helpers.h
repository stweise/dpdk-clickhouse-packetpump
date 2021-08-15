#ifndef HELPERS_H
#define HELPERS_H
#endif //HELPERS_H

#ifndef IPv4_BYTES
#define IPv4_BYTES_FMT "::ffff:%" PRIu8 ".%" PRIu8 ".%" PRIu8 ".%" PRIu8
#define IPv4_BYTES(addr) \
		(uint8_t) (((addr) >> 24) & 0xFF),\
		(uint8_t) (((addr) >> 16) & 0xFF),\
		(uint8_t) (((addr) >> 8) & 0xFF),\
		(uint8_t) ((addr) & 0xFF)
#endif

#ifndef IPv6_BYTES
#define IPv6_BYTES_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"\
                       "%02x%02x:%02x%02x:%02x%02x:%02x%02x"
#define IPv6_BYTES(addr) \
    addr[0],  addr[1], addr[2],  addr[3], \
    addr[4],  addr[5], addr[6],  addr[7], \
    addr[8],  addr[9], addr[10], addr[11],\
    addr[12], addr[13],addr[14], addr[15]
#endif
