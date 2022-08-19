#include "mysql/psi/psi_idle.h"
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
#include "mysql/components/services/psi_idle_bits.h"
struct PSI_idle_locker;
typedef struct PSI_idle_locker PSI_idle_locker;
struct PSI_idle_locker_state_v1 {
  unsigned int m_flags;
  struct PSI_thread *m_thread;
  unsigned long long m_timer_start;
  unsigned long long (*m_timer)(void);
  void *m_wait;
};
typedef struct PSI_idle_locker_state_v1 PSI_idle_locker_state_v1;
typedef struct PSI_idle_locker *(*start_idle_wait_v1_t)(
    struct PSI_idle_locker_state_v1 *state, const char *src_file,
    unsigned int src_line);
typedef void (*end_idle_wait_v1_t)(struct PSI_idle_locker *locker);
typedef struct PSI_idle_locker_state_v1 PSI_idle_locker_state;
struct PSI_idle_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_idle_bootstrap PSI_idle_bootstrap;
struct PSI_idle_service_v1 {
  start_idle_wait_v1_t start_idle_wait;
  end_idle_wait_v1_t end_idle_wait;
};
typedef struct PSI_idle_service_v1 PSI_idle_service_t;
extern PSI_idle_service_t *psi_idle_service;
