#ifndef __loadconfig_h__
#define __loadconfig_h__ 1

#include "include.h"

extern int  read_config_file(char *filename, REQUEST_CBF cbf);
extern void  read_config_server(REQUEST_CBF cbf);
extern int  config_set_server(char *hostport, const char* key);

#endif /* #ifndef __loadconfig_h__ */
