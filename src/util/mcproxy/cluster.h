#ifndef CLUSTER_H_
#define CLUSTER_H_

#include "include.h"
#include "request.h"

typedef void (*CLUSTER_FUNC)(int, void *);

int  cluster_determine_id(void);
void cluster_set_id(int id);
int  cluster_id(void);
void cluster_add(int id, in_addr_t *network, in_addr_t *netmask);
int  cluster_find_id(in_addr_t *addr);
void cluster_replicate(REQUEST *req, int reliable);
void cluster_clear(void);

#endif /*CLUSTER_H_*/
