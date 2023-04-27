#include <mysql/service_command.h>
#include "mysql/com_data.h"
struct COM_INIT_DB_DATA {
  const char *db_name;
  unsigned long length;
};
struct COM_REFRESH_DATA {
  unsigned char options;
};
struct COM_KILL_DATA {
  unsigned long id;
};
struct COM_SET_OPTION_DATA {
  unsigned int opt_command;
};
struct PS_PARAM {
  unsigned char null_bit;
  enum enum_field_types type;
  unsigned char unsigned_type;
  const unsigned char *value;
  unsigned long length;
};
struct COM_STMT_EXECUTE_DATA {
  unsigned long stmt_id;
  unsigned long open_cursor;
  PS_PARAM *parameters;
  unsigned long parameter_count;
  unsigned char has_new_types;
};
struct COM_STMT_FETCH_DATA {
  unsigned long stmt_id;
  unsigned long num_rows;
};
struct COM_STMT_SEND_LONG_DATA_DATA {
  unsigned long stmt_id;
  unsigned int param_number;
  unsigned char *longdata;
  unsigned long length;
};
struct COM_STMT_PREPARE_DATA {
  const char *query;
  unsigned int length;
};
struct COM_STMT_CLOSE_DATA {
  unsigned int stmt_id;
};
struct COM_STMT_RESET_DATA {
  unsigned int stmt_id;
};
struct COM_QUERY_DATA {
  const char *query;
  unsigned int length;
  const char *query_attrs;
  unsigned int query_attrs_length;
};
struct COM_FIELD_LIST_DATA {
  unsigned char *table_name;
  unsigned int table_name_length;
  const unsigned char *query;
  unsigned int query_length;
};
union COM_DATA {
  COM_QUERY_DATA com_query;
  COM_INIT_DB_DATA com_init_db;
  COM_REFRESH_DATA com_refresh;
  COM_KILL_DATA com_kill;
  COM_SET_OPTION_DATA com_set_option;
  COM_STMT_EXECUTE_DATA com_stmt_execute;
  COM_STMT_FETCH_DATA com_stmt_fetch;
  COM_STMT_SEND_LONG_DATA_DATA com_stmt_send_long_data;
  COM_STMT_PREPARE_DATA com_stmt_prepare;
  COM_STMT_CLOSE_DATA com_stmt_close;
  COM_STMT_RESET_DATA com_stmt_reset;
  COM_FIELD_LIST_DATA com_field_list;
  COM_DATA() : com_query() {}
};
#include "mysql/service_srv_session.h"
#include "mysql/service_srv_session_bits.h"
struct Srv_session;
typedef struct Srv_session *MYSQL_SESSION;
typedef void (*srv_session_error_cb)(void *ctx, unsigned int sql_errno,
                                     const char *err_msg);
extern "C" struct srv_session_service_st {
  int (*init_session_thread)(const void *plugin);
  void (*deinit_session_thread)();
  MYSQL_SESSION(*open_session)
  (srv_session_error_cb error_cb, void *plugix_ctx);
  int (*detach_session)(MYSQL_SESSION session);
  int (*close_session)(MYSQL_SESSION session);
  int (*server_is_available)();
  int (*attach_session)(MYSQL_SESSION session, MYSQL_THD *ret_previous_thd);
} * srv_session_service;
int srv_session_init_thread(const void *plugin);
void srv_session_deinit_thread();
MYSQL_SESSION srv_session_open(srv_session_error_cb error_cb, void *plugin_ctx);
int srv_session_detach(MYSQL_SESSION session);
int srv_session_close(MYSQL_SESSION session);
int srv_session_server_is_available();
int srv_session_attach(MYSQL_SESSION session, MYSQL_THD *ret_previous_thd);
#include "decimal.h"
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
typedef enum {
  TRUNCATE = 0,
  HALF_EVEN,
  HALF_UP,
  CEILING,
  FLOOR
} decimal_round_mode;
typedef int32 decimal_digit_t;
struct decimal_t {
  int intg, frac, len;
  bool sign;
  decimal_digit_t *buf;
};
#include "mysql_time.h"
enum enum_mysql_timestamp_type {
  MYSQL_TIMESTAMP_NONE = -2,
  MYSQL_TIMESTAMP_ERROR = -1,
  MYSQL_TIMESTAMP_DATE = 0,
  MYSQL_TIMESTAMP_DATETIME = 1,
  MYSQL_TIMESTAMP_TIME = 2,
  MYSQL_TIMESTAMP_DATETIME_TZ = 3
};
typedef struct MYSQL_TIME {
  unsigned int year, month, day, hour, minute, second;
  unsigned long second_part;
  bool neg;
  enum enum_mysql_timestamp_type time_type;
  int time_zone_displacement;
} MYSQL_TIME;
struct st_send_field {
  const char *db_name;
  const char *table_name;
  const char *org_table_name;
  const char *col_name;
  const char *org_col_name;
  unsigned long length;
  unsigned int charsetnr;
  unsigned int flags;
  unsigned int decimals;
  enum_field_types type;
};
typedef int (*start_result_metadata_t)(void *ctx, uint num_cols, uint flags,
                                       const CHARSET_INFO *resultcs);
typedef int (*field_metadata_t)(void *ctx, struct st_send_field *field,
                                const CHARSET_INFO *charset);
typedef int (*end_result_metadata_t)(void *ctx, uint server_status,
                                     uint warn_count);
typedef int (*start_row_t)(void *ctx);
typedef int (*end_row_t)(void *ctx);
typedef void (*abort_row_t)(void *ctx);
typedef ulong (*get_client_capabilities_t)(void *ctx);
typedef int (*get_null_t)(void *ctx);
typedef int (*get_integer_t)(void *ctx, longlong value);
typedef int (*get_longlong_t)(void *ctx, longlong value, uint is_unsigned);
typedef int (*get_decimal_t)(void *ctx, const decimal_t *value);
typedef int (*get_double_t)(void *ctx, double value, uint32_t decimals);
typedef int (*get_date_t)(void *ctx, const MYSQL_TIME *value);
typedef int (*get_time_t)(void *ctx, const MYSQL_TIME *value, uint decimals);
typedef int (*get_datetime_t)(void *ctx, const MYSQL_TIME *value,
                              uint decimals);
typedef int (*get_string_t)(void *ctx, const char *value, size_t length,
                            const CHARSET_INFO *valuecs);
typedef void (*handle_ok_t)(void *ctx, uint server_status,
                            uint statement_warn_count, ulonglong affected_rows,
                            ulonglong last_insert_id, const char *message);
typedef void (*handle_error_t)(void *ctx, uint sql_errno, const char *err_msg,
                               const char *sqlstate);
typedef void (*shutdown_t)(void *ctx, int server_shutdown);
struct st_command_service_cbs {
  start_result_metadata_t start_result_metadata;
  field_metadata_t field_metadata;
  end_result_metadata_t end_result_metadata;
  start_row_t start_row;
  end_row_t end_row;
  abort_row_t abort_row;
  get_client_capabilities_t get_client_capabilities;
  get_null_t get_null;
  get_integer_t get_integer;
  get_longlong_t get_longlong;
  get_decimal_t get_decimal;
  get_double_t get_double;
  get_date_t get_date;
  get_time_t get_time;
  get_datetime_t get_datetime;
  get_string_t get_string;
  handle_ok_t handle_ok;
  handle_error_t handle_error;
  shutdown_t shutdown;
};
enum cs_text_or_binary {
  CS_TEXT_REPRESENTATION = 1,
  CS_BINARY_REPRESENTATION = 2,
};
extern "C" struct command_service_st {
  int (*run_command)(MYSQL_SESSION session, enum enum_server_command command,
                     const union COM_DATA *data, const CHARSET_INFO *client_cs,
                     const struct st_command_service_cbs *callbacks,
                     enum cs_text_or_binary text_or_binary,
                     void *service_callbacks_ctx);
} * command_service;
int command_service_run_command(MYSQL_SESSION session,
                                enum enum_server_command command,
                                const union COM_DATA *data,
                                const CHARSET_INFO *client_cs,
                                const struct st_command_service_cbs *callbacks,
                                enum cs_text_or_binary text_or_binary,
                                void *service_callbacks_ctx);
#include <mysql/service_locking.h>
#include "my_inttypes.h"
enum enum_locking_service_lock_type {
  LOCKING_SERVICE_READ,
  LOCKING_SERVICE_WRITE
};
typedef int (*mysql_acquire_locks_t)(
    void * opaque_thd, const char *lock_namespace, const char **lock_names,
    size_t lock_num, enum enum_locking_service_lock_type lock_type,
    uint64_t lock_timeout);
typedef int (*mysql_release_locks_t)(void * opaque_thd,
                                     const char *lock_namespace);
extern "C" struct mysql_locking_service_st {
  mysql_acquire_locks_t mysql_acquire_locks_nsec;
  mysql_release_locks_t mysql_release_locks;
} * mysql_locking_service;
int mysql_acquire_locking_service_locks_nsec(
    void * opaque_thd, const char *lock_namespace, const char **lock_names,
    size_t lock_num, enum enum_locking_service_lock_type lock_type,
    uint64_t lock_timeout);
int mysql_release_locking_service_locks(void * opaque_thd,
                                        const char *lock_namespace);
#include <mysql/service_my_plugin_log.h>
enum plugin_log_level {
  MY_ERROR_LEVEL,
  MY_WARNING_LEVEL,
  MY_INFORMATION_LEVEL
};
extern "C" struct my_plugin_log_service {
  int (*my_plugin_log_message)(MYSQL_PLUGIN *, enum plugin_log_level,
                               const char *, ...)
      MY_ATTRIBUTE((format(printf, 3, 4)));
} * my_plugin_log_service;
int my_plugin_log_message(MYSQL_PLUGIN *plugin, enum plugin_log_level level,
                          const char *format, ...)
    MY_ATTRIBUTE((format(printf, 3, 4)));
#include <mysql/service_mysql_alloc.h>
#include "mysql/components/services/psi_memory_bits.h"
typedef unsigned int PSI_memory_key;
struct PSI_thread;
struct PSI_memory_info_v1 {
  PSI_memory_key *m_key;
  const char *m_name;
  unsigned int m_flags;
  int m_volatility;
  const char *m_documentation;
};
typedef struct PSI_memory_info_v1 PSI_memory_info_v1;
typedef void (*register_memory_v1_t)(const char *category,
                                     struct PSI_memory_info_v1 *info,
                                     int count);
typedef PSI_memory_key (*memory_alloc_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);
typedef PSI_memory_key (*memory_realloc_v1_t)(PSI_memory_key key,
                                              size_t old_size, size_t new_size,
                                              struct PSI_thread **owner);
typedef PSI_memory_key (*memory_claim_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);
typedef void (*memory_free_v1_t)(PSI_memory_key key, size_t size,
                                 struct PSI_thread *owner);
typedef struct PSI_memory_info_v1 PSI_memory_info;
typedef int myf_t;
typedef void *(*mysql_malloc_t)(PSI_memory_key key, size_t size, myf_t flags);
typedef void *(*mysql_realloc_t)(PSI_memory_key key, void *ptr, size_t size,
                                 myf_t flags);
typedef void (*mysql_claim_t)(const void *ptr);
typedef void (*mysql_free_t)(void *ptr);
typedef void *(*my_memdup_t)(PSI_memory_key key, const void *from,
                             size_t length, myf_t flags);
typedef char *(*my_strdup_t)(PSI_memory_key key, const char *from, myf_t flags);
typedef char *(*my_strndup_t)(PSI_memory_key key, const char *from,
                              size_t length, myf_t flags);
struct mysql_malloc_service_st {
  mysql_malloc_t mysql_malloc;
  mysql_realloc_t mysql_realloc;
  mysql_claim_t mysql_claim;
  mysql_free_t mysql_free;
  my_memdup_t my_memdup;
  my_strdup_t my_strdup;
  my_strndup_t my_strndup;
};
extern "C" struct mysql_malloc_service_st *mysql_malloc_service;
extern void *my_malloc(PSI_memory_key key, size_t size, myf_t flags);
extern void *my_realloc(PSI_memory_key key, void *ptr, size_t size,
                        myf_t flags);
extern void my_claim(const void *ptr);
extern void my_free(void *ptr);
extern void *my_memdup(PSI_memory_key key, const void *from, size_t length,
                       myf_t flags);
extern char *my_strdup(PSI_memory_key key, const char *from, myf_t flags);
extern char *my_strndup(PSI_memory_key key, const char *from, size_t length,
                        myf_t flags);
#include <mysql/service_mysql_keyring.h>
extern "C" struct mysql_keyring_service_st {
  int (*my_key_store_func)(const char *, const char *, const char *,
                           const void *, size_t);
  int (*my_key_fetch_func)(const char *, char **, const char *, void **,
                           size_t *);
  int (*my_key_remove_func)(const char *, const char *);
  int (*my_key_generate_func)(const char *, const char *, const char *, size_t);
} * mysql_keyring_service;
int my_key_store(const char *, const char *, const char *, const void *,
                 size_t);
int my_key_fetch(const char *, char **, const char *, void **, size_t *);
int my_key_remove(const char *, const char *);
int my_key_generate(const char *, const char *, const char *, size_t);
#include <mysql/service_mysql_password_policy.h>
extern "C" struct mysql_password_policy_service_st {
  int (*my_validate_password_policy_func)(const char *, unsigned int);
  int (*my_calculate_password_strength_func)(const char *, unsigned int);
} * mysql_password_policy_service;
int my_validate_password_policy(const char *, unsigned int);
int my_calculate_password_strength(const char *, unsigned int);
#include <mysql/service_mysql_string.h>
typedef void *mysql_string_iterator_handle;
typedef void *mysql_string_handle;
extern "C" struct mysql_string_service_st {
  int (*mysql_string_convert_to_char_ptr_type)(mysql_string_handle,
                                               const char *, char *,
                                               unsigned int, int *);
  mysql_string_iterator_handle (*mysql_string_get_iterator_type)(
      mysql_string_handle);
  int (*mysql_string_iterator_next_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_isupper_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_islower_type)(mysql_string_iterator_handle);
  int (*mysql_string_iterator_isdigit_type)(mysql_string_iterator_handle);
  mysql_string_handle (*mysql_string_to_lowercase_type)(mysql_string_handle);
  void (*mysql_string_free_type)(mysql_string_handle);
  void (*mysql_string_iterator_free_type)(mysql_string_iterator_handle);
} * mysql_string_service;
int mysql_string_convert_to_char_ptr(mysql_string_handle string_handle,
                                     const char *charset_name, char *buffer,
                                     unsigned int buffer_size, int *error);
mysql_string_iterator_handle mysql_string_get_iterator(
    mysql_string_handle string_handle);
int mysql_string_iterator_next(mysql_string_iterator_handle iterator_handle);
int mysql_string_iterator_isupper(mysql_string_iterator_handle iterator_handle);
int mysql_string_iterator_islower(mysql_string_iterator_handle iterator_handle);
int mysql_string_iterator_isdigit(mysql_string_iterator_handle iterator_handle);
mysql_string_handle mysql_string_to_lowercase(
    mysql_string_handle string_handle);
void mysql_string_free(mysql_string_handle);
void mysql_string_iterator_free(mysql_string_iterator_handle);
#include <mysql/service_parser.h>
#include <mysql/mysql_lex_string.h>
struct MYSQL_LEX_STRING {
  char *str;
  size_t length;
};
struct MYSQL_LEX_CSTRING {
  const char *str;
  size_t length;
};
class THD;
class Item;
typedef Item *MYSQL_ITEM;
typedef int (*parse_node_visit_function)(MYSQL_ITEM item, unsigned char *arg);
typedef int (*sql_condition_handler_function)(int sql_errno,
                                              const char *sqlstate,
                                              const char *msg, void *state);
struct my_thread_handle;
typedef THD * (*mysql_current_session_t)();
typedef THD * (*mysql_open_session_t)();
typedef void (*mysql_start_thread_t)(THD * thd,
                                     void *(*callback_fun)(void *), void *arg,
                                     struct my_thread_handle *thread_handle);
typedef void (*mysql_join_thread_t)(struct my_thread_handle *thread_handle);
typedef void (*mysql_set_current_database_t)(THD * thd,
                                             const MYSQL_LEX_STRING db);
typedef int (*mysql_parse_t)(THD * thd, const MYSQL_LEX_STRING query,
                             unsigned char is_prepared,
                             sql_condition_handler_function handle_condition,
                             void *condition_handler_state);
typedef int (*mysql_get_statement_type_t)(THD * thd);
typedef int (*mysql_get_statement_digest_t)(THD * thd,
                                            unsigned char *digest);
typedef int (*mysql_get_number_params_t)(THD * thd);
typedef int (*mysql_extract_prepared_params_t)(THD * thd, int *positions);
typedef int (*mysql_visit_tree_t)(THD * thd,
                                  parse_node_visit_function processor,
                                  unsigned char *arg);
typedef MYSQL_LEX_STRING (*mysql_item_string_t)(MYSQL_ITEM item);
typedef void (*mysql_free_string_t)(MYSQL_LEX_STRING string);
typedef MYSQL_LEX_STRING (*mysql_get_query_t)(THD * thd);
typedef MYSQL_LEX_STRING (*mysql_get_normalized_query_t)(THD * thd);
extern "C" struct mysql_parser_service_st {
  mysql_current_session_t mysql_current_session;
  mysql_open_session_t mysql_open_session;
  mysql_start_thread_t mysql_start_thread;
  mysql_join_thread_t mysql_join_thread;
  mysql_set_current_database_t mysql_set_current_database;
  mysql_parse_t mysql_parse;
  mysql_get_statement_type_t mysql_get_statement_type;
  mysql_get_statement_digest_t mysql_get_statement_digest;
  mysql_get_number_params_t mysql_get_number_params;
  mysql_extract_prepared_params_t mysql_extract_prepared_params;
  mysql_visit_tree_t mysql_visit_tree;
  mysql_item_string_t mysql_item_string;
  mysql_free_string_t mysql_free_string;
  mysql_get_query_t mysql_get_query;
  mysql_get_normalized_query_t mysql_get_normalized_query;
} * mysql_parser_service;
typedef void *(*callback_function)(void *);
THD * mysql_parser_current_session();
THD * mysql_parser_open_session();
void mysql_parser_start_thread(THD * thd, callback_function fun, void *arg,
                               struct my_thread_handle *thread_handle);
void mysql_parser_join_thread(struct my_thread_handle *thread_handle);
void mysql_parser_set_current_database(THD * thd,
                                       const MYSQL_LEX_STRING db);
int mysql_parser_parse(THD * thd, const MYSQL_LEX_STRING query,
                       unsigned char is_prepared,
                       sql_condition_handler_function handle_condition,
                       void *condition_handler_state);
int mysql_parser_get_statement_type(THD * thd);
int mysql_parser_get_statement_digest(THD * thd, unsigned char *digest);
int mysql_parser_get_number_params(THD * thd);
int mysql_parser_extract_prepared_params(THD * thd, int *positions);
int mysql_parser_visit_tree(THD * thd, parse_node_visit_function processor,
                            unsigned char *arg);
MYSQL_LEX_STRING mysql_parser_item_string(MYSQL_ITEM item);
void mysql_parser_free_string(MYSQL_LEX_STRING string);
MYSQL_LEX_STRING mysql_parser_get_query(THD * thd);
MYSQL_LEX_STRING mysql_parser_get_normalized_query(THD * thd);
#include <mysql/service_plugin_registry.h>
#include <mysql/components/services/registry.h>
#include <mysql/components/service.h>
typedef int mysql_service_status_t;
#include <stdint.h>
