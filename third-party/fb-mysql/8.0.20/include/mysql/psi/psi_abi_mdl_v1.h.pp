#include "mysql/psi/psi_mdl.h"
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
#include "mysql/components/services/psi_mdl_bits.h"
struct MDL_key;
typedef int opaque_mdl_type;
typedef int opaque_mdl_duration;
typedef int opaque_mdl_status;
struct PSI_metadata_lock;
typedef struct PSI_metadata_lock PSI_metadata_lock;
struct PSI_metadata_locker;
typedef struct PSI_metadata_locker PSI_metadata_locker;
struct PSI_metadata_locker_state_v1 {
  unsigned int m_flags;
  struct PSI_metadata_lock *m_metadata_lock;
  struct PSI_thread *m_thread;
  unsigned long long m_timer_start;
  unsigned long long (*m_timer)(void);
  void *m_wait;
};
typedef struct PSI_metadata_locker_state_v1 PSI_metadata_locker_state_v1;
typedef PSI_metadata_lock *(*create_metadata_lock_v1_t)(
    void *identity, const struct MDL_key *key, opaque_mdl_type mdl_type,
    opaque_mdl_duration mdl_duration, opaque_mdl_status mdl_status,
    const char *src_file, unsigned int src_line);
typedef void (*set_metadata_lock_status_v1_t)(PSI_metadata_lock *lock,
                                              opaque_mdl_status mdl_status);
typedef void (*destroy_metadata_lock_v1_t)(PSI_metadata_lock *lock);
typedef struct PSI_metadata_locker *(*start_metadata_wait_v1_t)(
    struct PSI_metadata_locker_state_v1 *state, struct PSI_metadata_lock *mdl,
    const char *src_file, unsigned int src_line);
typedef void (*end_metadata_wait_v1_t)(struct PSI_metadata_locker *locker,
                                       int rc);
typedef struct PSI_metadata_locker_state_v1 PSI_metadata_locker_state;
struct PSI_mdl_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_mdl_bootstrap PSI_mdl_bootstrap;
struct PSI_mdl_service_v1 {
  create_metadata_lock_v1_t create_metadata_lock;
  set_metadata_lock_status_v1_t set_metadata_lock_status;
  destroy_metadata_lock_v1_t destroy_metadata_lock;
  start_metadata_wait_v1_t start_metadata_wait;
  end_metadata_wait_v1_t end_metadata_wait;
};
typedef struct PSI_mdl_service_v1 PSI_mdl_service_t;
extern PSI_mdl_service_t *psi_mdl_service;
