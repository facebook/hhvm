#include "mysql/psi/psi_file.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "mysql/components/services/psi_file_bits.h"
#include <mysql/components/services/my_io_bits.h>
typedef int File;
typedef mode_t MY_MODE;
typedef socklen_t socket_len_t;
typedef int my_socket;
typedef unsigned int PSI_file_key;
struct PSI_file;
typedef struct PSI_file PSI_file;
struct PSI_file_locker;
typedef struct PSI_file_locker PSI_file_locker;
enum PSI_file_operation {
  PSI_FILE_CREATE = 0,
  PSI_FILE_CREATE_TMP = 1,
  PSI_FILE_OPEN = 2,
  PSI_FILE_STREAM_OPEN = 3,
  PSI_FILE_CLOSE = 4,
  PSI_FILE_STREAM_CLOSE = 5,
  PSI_FILE_READ = 6,
  PSI_FILE_WRITE = 7,
  PSI_FILE_SEEK = 8,
  PSI_FILE_TELL = 9,
  PSI_FILE_FLUSH = 10,
  PSI_FILE_STAT = 11,
  PSI_FILE_FSTAT = 12,
  PSI_FILE_CHSIZE = 13,
  PSI_FILE_DELETE = 14,
  PSI_FILE_RENAME = 15,
  PSI_FILE_SYNC = 16
};
typedef enum PSI_file_operation PSI_file_operation;
struct PSI_file_info_v1 {
  PSI_file_key *m_key;
  const char *m_name;
  unsigned int m_flags;
  int m_volatility;
  const char *m_documentation;
};
typedef struct PSI_file_info_v1 PSI_file_info_v1;
struct PSI_file_locker_state_v1 {
  unsigned int m_flags;
  enum PSI_file_operation m_operation;
  struct PSI_file *m_file;
  const char *m_name;
  void *m_class;
  struct PSI_thread *m_thread;
  size_t m_number_of_bytes;
  unsigned long long m_timer_start;
  unsigned long long (*m_timer)(void);
  void *m_wait;
};
typedef struct PSI_file_locker_state_v1 PSI_file_locker_state_v1;
typedef void (*register_file_v1_t)(const char *category,
                                   struct PSI_file_info_v1 *info, int count);
typedef void (*create_file_v1_t)(PSI_file_key key, const char *name, File file);
typedef struct PSI_file_locker *(*get_thread_file_name_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, PSI_file_key key,
    enum PSI_file_operation op, const char *name, const void *identity);
typedef struct PSI_file_locker *(*get_thread_file_stream_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, struct PSI_file *file,
    enum PSI_file_operation op);
typedef struct PSI_file_locker *(*get_thread_file_descriptor_locker_v1_t)(
    struct PSI_file_locker_state_v1 *state, File file,
    enum PSI_file_operation op);
typedef void (*start_file_open_wait_v1_t)(struct PSI_file_locker *locker,
                                          const char *src_file,
                                          unsigned int src_line);
typedef struct PSI_file *(*end_file_open_wait_v1_t)(
    struct PSI_file_locker *locker, void *result);
typedef void (*end_file_open_wait_and_bind_to_descriptor_v1_t)(
    struct PSI_file_locker *locker, File file);
typedef void (*end_temp_file_open_wait_and_bind_to_descriptor_v1_t)(
    struct PSI_file_locker *locker, File file, const char *filename);
typedef void (*start_file_wait_v1_t)(struct PSI_file_locker *locker,
                                     size_t count, const char *src_file,
                                     unsigned int src_line);
typedef void (*end_file_wait_v1_t)(struct PSI_file_locker *locker,
                                   size_t count);
typedef void (*start_file_close_wait_v1_t)(struct PSI_file_locker *locker,
                                           const char *src_file,
                                           unsigned int src_line);
typedef void (*end_file_close_wait_v1_t)(struct PSI_file_locker *locker,
                                         int rc);
typedef void (*start_file_rename_wait_v1_t)(struct PSI_file_locker *locker,
                                            size_t count, const char *old_name,
                                            const char *new_name,
                                            const char *src_file,
                                            unsigned int src_line);
typedef void (*end_file_rename_wait_v1_t)(struct PSI_file_locker *locker,
                                          const char *old_name,
                                          const char *new_name, int rc);
typedef struct PSI_file_info_v1 PSI_file_info;
typedef struct PSI_file_locker_state_v1 PSI_file_locker_state;
struct PSI_file_bootstrap {
  void *(*get_interface)(int version);
};
struct PSI_file_service_v2 {
  register_file_v1_t register_file;
  create_file_v1_t create_file;
  get_thread_file_name_locker_v1_t get_thread_file_name_locker;
  get_thread_file_stream_locker_v1_t get_thread_file_stream_locker;
  get_thread_file_descriptor_locker_v1_t get_thread_file_descriptor_locker;
  start_file_open_wait_v1_t start_file_open_wait;
  end_file_open_wait_v1_t end_file_open_wait;
  end_file_open_wait_and_bind_to_descriptor_v1_t
      end_file_open_wait_and_bind_to_descriptor;
  end_temp_file_open_wait_and_bind_to_descriptor_v1_t
      end_temp_file_open_wait_and_bind_to_descriptor;
  start_file_wait_v1_t start_file_wait;
  end_file_wait_v1_t end_file_wait;
  start_file_close_wait_v1_t start_file_close_wait;
  end_file_close_wait_v1_t end_file_close_wait;
  start_file_rename_wait_v1_t start_file_rename_wait;
  end_file_rename_wait_v1_t end_file_rename_wait;
};
typedef struct PSI_file_service_v2 PSI_file_service_t;
extern PSI_file_service_t *psi_file_service;
