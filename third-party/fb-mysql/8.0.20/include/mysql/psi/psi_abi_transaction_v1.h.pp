#include "mysql/psi/psi_transaction.h"
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
#include "mysql/components/services/psi_transaction_bits.h"
struct PSI_transaction_locker;
typedef struct PSI_transaction_locker PSI_transaction_locker;
struct PSI_transaction_locker_state_v1 {
  unsigned int m_flags;
  void *m_class;
  struct PSI_thread *m_thread;
  unsigned long long m_timer_start;
  unsigned long long (*m_timer)(void);
  void *m_transaction;
  bool m_read_only;
  bool m_autocommit;
  unsigned long m_statement_count;
  unsigned long m_savepoint_count;
  unsigned long m_rollback_to_savepoint_count;
  unsigned long m_release_savepoint_count;
};
typedef struct PSI_transaction_locker_state_v1 PSI_transaction_locker_state_v1;
typedef struct PSI_transaction_locker *(*get_thread_transaction_locker_v1_t)(
    struct PSI_transaction_locker_state_v1 *state, const void *xid,
    const unsigned long long *trxid, int isolation_level, bool read_only,
    bool autocommit);
typedef void (*start_transaction_v1_t)(struct PSI_transaction_locker *locker,
                                       const char *src_file,
                                       unsigned int src_line);
typedef void (*set_transaction_xid_v1_t)(struct PSI_transaction_locker *locker,
                                         const void *xid, int xa_state);
typedef void (*set_transaction_xa_state_v1_t)(
    struct PSI_transaction_locker *locker, int xa_state);
typedef void (*set_transaction_gtid_v1_t)(struct PSI_transaction_locker *locker,
                                          const void *sid,
                                          const void *gtid_spec);
typedef void (*set_transaction_trxid_v1_t)(
    struct PSI_transaction_locker *locker, const unsigned long long *trxid);
typedef void (*inc_transaction_savepoints_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);
typedef void (*inc_transaction_rollback_to_savepoint_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);
typedef void (*inc_transaction_release_savepoint_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);
typedef void (*end_transaction_v1_t)(struct PSI_transaction_locker *locker,
                                     bool commit);
typedef struct PSI_transaction_locker_state_v1 PSI_transaction_locker_state;
struct PSI_transaction_bootstrap {
  void *(*get_interface)(int version);
};
typedef struct PSI_transaction_bootstrap PSI_transaction_bootstrap;
struct PSI_transaction_service_v1 {
  get_thread_transaction_locker_v1_t get_thread_transaction_locker;
  start_transaction_v1_t start_transaction;
  set_transaction_xid_v1_t set_transaction_xid;
  set_transaction_xa_state_v1_t set_transaction_xa_state;
  set_transaction_gtid_v1_t set_transaction_gtid;
  set_transaction_trxid_v1_t set_transaction_trxid;
  inc_transaction_savepoints_v1_t inc_transaction_savepoints;
  inc_transaction_rollback_to_savepoint_v1_t
      inc_transaction_rollback_to_savepoint;
  inc_transaction_release_savepoint_v1_t inc_transaction_release_savepoint;
  end_transaction_v1_t end_transaction;
};
typedef struct PSI_transaction_service_v1 PSI_transaction_service_t;
extern PSI_transaction_service_t *psi_transaction_service;
