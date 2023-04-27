#include "mysql/psi/psi_rwlock.h"
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
#include "mysql/components/services/psi_rwlock_bits.h"
typedef unsigned int PSI_rwlock_key;
struct PSI_rwlock;
typedef struct PSI_rwlock PSI_rwlock;
struct PSI_rwlock_locker;
typedef struct PSI_rwlock_locker PSI_rwlock_locker;
enum PSI_rwlock_operation {
  PSI_RWLOCK_READLOCK = 0,
  PSI_RWLOCK_WRITELOCK = 1,
  PSI_RWLOCK_TRYREADLOCK = 2,
  PSI_RWLOCK_TRYWRITELOCK = 3,
  PSI_RWLOCK_UNLOCK = 4,
  PSI_RWLOCK_SHAREDLOCK = 5,
  PSI_RWLOCK_SHAREDEXCLUSIVELOCK = 6,
  PSI_RWLOCK_EXCLUSIVELOCK = 7,
  PSI_RWLOCK_TRYSHAREDLOCK = 8,
  PSI_RWLOCK_TRYSHAREDEXCLUSIVELOCK = 9,
  PSI_RWLOCK_TRYEXCLUSIVELOCK = 10,
  PSI_RWLOCK_SHAREDUNLOCK = 11,
  PSI_RWLOCK_SHAREDEXCLUSIVEUNLOCK = 12,
  PSI_RWLOCK_EXCLUSIVEUNLOCK = 13
};
typedef enum PSI_rwlock_operation PSI_rwlock_operation;
struct PSI_rwlock_info_v1 {
  PSI_rwlock_key *m_key;
  const char *m_name;
  unsigned int m_flags;
  int m_volatility;
  const char *m_documentation;
};
typedef struct PSI_rwlock_info_v1 PSI_rwlock_info_v1;
struct PSI_rwlock_locker_state_v1 {
  unsigned int m_flags;
  enum PSI_rwlock_operation m_operation;
  struct PSI_rwlock *m_rwlock;
  struct PSI_thread *m_thread;
  unsigned long long m_timer_start{0};
  unsigned long long (*m_timer)(void);
  void *m_wait{nullptr};
};
typedef struct PSI_rwlock_locker_state_v1 PSI_rwlock_locker_state_v1;
typedef void (*register_rwlock_v1_t)(const char *category,
                                     struct PSI_rwlock_info_v1 *info,
                                     int count);
typedef struct PSI_rwlock *(*init_rwlock_v1_t)(PSI_rwlock_key key,
                                               const void *identity);
typedef void (*destroy_rwlock_v1_t)(struct PSI_rwlock *rwlock);
typedef struct PSI_rwlock_locker *(*start_rwlock_rdwait_v1_t)(
    struct PSI_rwlock_locker_state_v1 *state, struct PSI_rwlock *rwlock,
    enum PSI_rwlock_operation op, const char *src_file, unsigned int src_line);
typedef void (*end_rwlock_rdwait_v1_t)(struct PSI_rwlock_locker *locker,
                                       int rc);
typedef struct PSI_rwlock_locker *(*start_rwlock_wrwait_v1_t)(
    struct PSI_rwlock_locker_state_v1 *state, struct PSI_rwlock *rwlock,
    enum PSI_rwlock_operation op, const char *src_file, unsigned int src_line);
typedef void (*end_rwlock_wrwait_v1_t)(struct PSI_rwlock_locker *locker,
                                       int rc);
typedef void (*unlock_rwlock_v1_t)(struct PSI_rwlock *rwlock);
typedef void (*unlock_rwlock_v2_t)(struct PSI_rwlock *rwlock,
                                   enum PSI_rwlock_operation op);
typedef struct PSI_rwlock_info_v1 PSI_rwlock_info;
typedef struct PSI_rwlock_locker_state_v1 PSI_rwlock_locker_state;
#include "psi_base.h"
#include "my_psi_config.h"
struct PSI_placeholder {
  int m_placeholder;
};
struct PSI_rwlock_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_rwlock_bootstrap PSI_rwlock_bootstrap;
struct PSI_rwlock_service_v2 {
  register_rwlock_v1_t register_rwlock;
  init_rwlock_v1_t init_rwlock;
  destroy_rwlock_v1_t destroy_rwlock;
  start_rwlock_rdwait_v1_t start_rwlock_rdwait;
  end_rwlock_rdwait_v1_t end_rwlock_rdwait;
  start_rwlock_wrwait_v1_t start_rwlock_wrwait;
  end_rwlock_wrwait_v1_t end_rwlock_wrwait;
  unlock_rwlock_v2_t unlock_rwlock;
};
typedef struct PSI_rwlock_service_v2 PSI_rwlock_service_t;
extern PSI_rwlock_service_t *psi_rwlock_service;
