#ifndef USE_CLICKHOUSE_DRIVER_H
#define USE_CLICKHOUSE_DRIVER_H

#include <time.h>
#include <stdlib.h>
#include "packet.h"

void CHconstruct();
void CHdestruct();
void CHappend(struct packet *);
void CHinsert();
void CHshow();

#endif //USE_CLICKHOUSE_DRIVER_H
