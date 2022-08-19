#include "mysql/psi/psi_system.h"
#include "my_inttypes.h"
#include "my_config.h"
typedef unsigned char uchar;
typedef long long int longlong;
typedef unsigned long long int ulonglong;
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef intptr_t intptr;
typedef ulonglong my_off_t;
typedef int myf;
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "mysql/components/services/psi_system_bits.h"
typedef void (*unload_plugin_v1_t)(const char *plugin_name);
struct PSI_system_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_system_bootstrap PSI_system_bootstrap;
struct PSI_system_service_v1 {
  unload_plugin_v1_t unload_plugin;
};
typedef struct PSI_system_service_v1 PSI_system_service_t;
extern PSI_system_service_t *psi_system_service;
