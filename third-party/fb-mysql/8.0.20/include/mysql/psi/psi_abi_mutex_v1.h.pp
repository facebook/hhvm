#include "mysql/psi/psi_mutex.h"
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
#include "mysql/components/services/psi_mutex_bits.h"
typedef unsigned int PSI_mutex_key;
struct PSI_mutex_info_v1 {
  PSI_mutex_key *m_key;
  const char *m_name;
  unsigned int m_flags;
  int m_volatility;
  const char *m_documentation;
};
struct PSI_mutex;
typedef struct PSI_mutex PSI_mutex;
struct PSI_mutex_locker;
typedef struct PSI_mutex_locker PSI_mutex_locker;
enum PSI_mutex_operation {
  PSI_MUTEX_LOCK = 0,
  PSI_MUTEX_TRYLOCK = 1
};
typedef enum PSI_mutex_operation PSI_mutex_operation;
struct PSI_mutex_locker_state_v1 {
  unsigned int m_flags;
  enum PSI_mutex_operation m_operation;
  struct PSI_mutex *m_mutex;
  struct PSI_thread *m_thread;
  unsigned long long m_timer_start{0};
  unsigned long long (*m_timer)(void);
  void *m_wait{nullptr};
};
typedef struct PSI_mutex_locker_state_v1 PSI_mutex_locker_state_v1;
typedef void (*register_mutex_v1_t)(const char *category,
                                    struct PSI_mutex_info_v1 *info, int count);
typedef struct PSI_mutex *(*init_mutex_v1_t)(PSI_mutex_key key,
                                             const void *identity);
typedef void (*destroy_mutex_v1_t)(struct PSI_mutex *mutex);
typedef void (*unlock_mutex_v1_t)(struct PSI_mutex *mutex);
typedef struct PSI_mutex_locker *(*start_mutex_wait_v1_t)(
    struct PSI_mutex_locker_state_v1 *state, struct PSI_mutex *mutex,
    enum PSI_mutex_operation op, const char *src_file, unsigned int src_line);
typedef void (*end_mutex_wait_v1_t)(struct PSI_mutex_locker *locker, int rc);
typedef struct PSI_mutex_info_v1 PSI_mutex_info_v1;
typedef PSI_mutex_info_v1 PSI_mutex_info;
typedef struct PSI_mutex_locker_state_v1 PSI_mutex_locker_state;
struct PSI_mutex_bootstrap {
  void *(*get_interface)(int version);
};
struct PSI_mutex_service_v1 {
  register_mutex_v1_t register_mutex;
  init_mutex_v1_t init_mutex;
  destroy_mutex_v1_t destroy_mutex;
  start_mutex_wait_v1_t start_mutex_wait;
  end_mutex_wait_v1_t end_mutex_wait;
  unlock_mutex_v1_t unlock_mutex;
};
typedef struct PSI_mutex_service_v1 PSI_mutex_service_t;
extern PSI_mutex_service_t *psi_mutex_service;
