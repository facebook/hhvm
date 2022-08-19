typedef uint64_t my_ulonglong;
typedef int my_socket;
#include "field_types.h"
enum enum_field_types
{ MYSQL_TYPE_DECIMAL,
  MYSQL_TYPE_TINY,
  MYSQL_TYPE_SHORT,
  MYSQL_TYPE_LONG,
  MYSQL_TYPE_FLOAT,
  MYSQL_TYPE_DOUBLE,
  MYSQL_TYPE_NULL,
  MYSQL_TYPE_TIMESTAMP,
  MYSQL_TYPE_LONGLONG,
  MYSQL_TYPE_INT24,
  MYSQL_TYPE_DATE,
  MYSQL_TYPE_TIME,
  MYSQL_TYPE_DATETIME,
  MYSQL_TYPE_YEAR,
  MYSQL_TYPE_NEWDATE,
  MYSQL_TYPE_VARCHAR,
  MYSQL_TYPE_BIT,
  MYSQL_TYPE_TIMESTAMP2,
  MYSQL_TYPE_DATETIME2,
  MYSQL_TYPE_TIME2,
  MYSQL_TYPE_TYPED_ARRAY,
  MYSQL_TYPE_JSON = 245,
  MYSQL_TYPE_NEWDECIMAL = 246,
  MYSQL_TYPE_ENUM = 247,
  MYSQL_TYPE_SET = 248,
  MYSQL_TYPE_TINY_BLOB = 249,
  MYSQL_TYPE_MEDIUM_BLOB = 250,
  MYSQL_TYPE_LONG_BLOB = 251,
  MYSQL_TYPE_BLOB = 252,
  MYSQL_TYPE_VAR_STRING = 253,
  MYSQL_TYPE_STRING = 254,
  MYSQL_TYPE_GEOMETRY = 255 };
typedef enum enum_field_types enum_field_types;
#include "my_list.h"
typedef struct LIST {
  struct LIST *prev, *next;
  void *data;
} LIST;
typedef int (*list_walk_action)(void *, void *);
extern LIST *list_add(LIST *root, LIST *element);
extern LIST *list_delete(LIST *root, LIST *element);
extern LIST *list_cons(void *data, LIST *root);
extern LIST *list_reverse(LIST *root);
extern void list_free(LIST *root, unsigned int free_data);
extern unsigned int list_length(LIST *);
extern int list_walk(LIST *, list_walk_action action, unsigned char *argument);
#include "mysql_com.h"
#include "my_command.h"
enum enum_server_command {
  COM_SLEEP,
  COM_QUIT,
  COM_INIT_DB,
  COM_QUERY,
  COM_FIELD_LIST,
  COM_CREATE_DB,
  COM_DROP_DB,
  COM_REFRESH,
  COM_DEPRECATED_1,
  COM_STATISTICS,
  COM_PROCESS_INFO,
  COM_CONNECT,
  COM_PROCESS_KILL,
  COM_DEBUG,
  COM_PING,
  COM_TIME,
  COM_DELAYED_INSERT,
  COM_CHANGE_USER,
  COM_BINLOG_DUMP,
  COM_TABLE_DUMP,
  COM_CONNECT_OUT,
  COM_REGISTER_SLAVE,
  COM_STMT_PREPARE,
  COM_STMT_EXECUTE,
  COM_STMT_SEND_LONG_DATA,
  COM_STMT_CLOSE,
  COM_STMT_RESET,
  COM_SET_OPTION,
  COM_STMT_FETCH,
  COM_DAEMON,
  COM_BINLOG_DUMP_GTID,
  COM_RESET_CONNECTION,
  COM_CLONE,
  COM_END,
  COM_TOP_BEGIN = 253,
  COM_SEND_REPLICA_STATISTICS = 254,
  COM_QUERY_ATTRS = 255,
  COM_TOP_END = 256,
};
#include "my_compress.h"
enum enum_compression_algorithm {
  MYSQL_UNCOMPRESSED = 1,
  MYSQL_ZLIB,
  MYSQL_ZSTD,
  MYSQL_ZSTD_STREAM,
  MYSQL_LZ4F_STREAM,
  MYSQL_INVALID
};
typedef struct mysql_zlib_compress_context {
  unsigned int compression_level;
} mysql_zlib_compress_context;
typedef struct LZ4F_cctx_s LZ4F_compressionContext;
typedef struct LZ4F_dctx_s LZ4F_decompressionContext;
typedef struct mysql_lz4f_compress_context {
  LZ4F_compressionContext *cctx;
  LZ4F_decompressionContext *dctx;
  bool reset_cctx;
  unsigned char *compress_buf;
  unsigned long compress_buf_len;
  unsigned int compression_level;
} mysql_lz4f_compress_context;
typedef struct ZSTD_CCtx_s ZSTD_CCtx;
typedef struct ZSTD_DCtx_s ZSTD_DCtx;
typedef struct mysql_zstd_compress_context {
  ZSTD_CCtx *cctx;
  ZSTD_DCtx *dctx;
  bool reset_cctx;
  unsigned char *compress_buf;
  unsigned long compress_buf_len;
  unsigned int compression_level;
} mysql_zstd_compress_context;
typedef struct mysql_compress_context {
  enum enum_compression_algorithm algorithm;
  union {
    mysql_lz4f_compress_context lz4f_ctx;
    mysql_zlib_compress_context zlib_ctx;
    mysql_zstd_compress_context zstd_ctx;
  } u;
} mysql_compress_context;
unsigned int mysql_default_compression_level(
    enum enum_compression_algorithm algorithm);
void mysql_compress_context_init(mysql_compress_context *cmp_ctx,
                                 enum enum_compression_algorithm algorithm,
                                 unsigned int compression_level);
void mysql_compress_context_deinit(mysql_compress_context *mysql_compress_ctx);
void reset_compress_status(void);
enum SERVER_STATUS_flags_enum {
  SERVER_STATUS_IN_TRANS = 1,
  SERVER_STATUS_AUTOCOMMIT = 2,
  SERVER_MORE_RESULTS_EXISTS = 8,
  SERVER_QUERY_NO_GOOD_INDEX_USED = 16,
  SERVER_QUERY_NO_INDEX_USED = 32,
  SERVER_STATUS_CURSOR_EXISTS = 64,
  SERVER_STATUS_LAST_ROW_SENT = 128,
  SERVER_STATUS_DB_DROPPED = 256,
  SERVER_STATUS_NO_BACKSLASH_ESCAPES = 512,
  SERVER_STATUS_METADATA_CHANGED = 1024,
  SERVER_QUERY_WAS_SLOW = 2048,
  SERVER_PS_OUT_PARAMS = 4096,
  SERVER_STATUS_IN_TRANS_READONLY = 8192,
  SERVER_SESSION_STATE_CHANGED = (1UL << 14)
};
struct Vio;
typedef struct {
  uint value_ms_;
} timeout_t;
typedef struct NET {
  struct Vio * vio;
  unsigned char *buff, *buff_end, *write_pos, *read_pos;
  my_socket fd;
  unsigned long remain_in_buf, length, buf_length, where_b;
  unsigned long max_packet, max_packet_size;
  unsigned int pkt_nr, compress_pkt_nr;
  timeout_t write_timeout, read_timeout;
  unsigned int retry_count;
  int fcntl;
  unsigned int *return_status;
  unsigned char reading_or_writing;
  unsigned char save_char;
  bool compress;
  unsigned int last_errno;
  unsigned char error;
  char last_error[512];
  char sqlstate[5 + 1];
  void *extension;
  unsigned int receive_buffer_size;
} NET;
enum mysql_enum_shutdown_level {
  SHUTDOWN_DEFAULT = 0,
  SHUTDOWN_WAIT_CONNECTIONS = (unsigned char)(1 << 0),
  SHUTDOWN_WAIT_TRANSACTIONS = (unsigned char)(1 << 1),
  SHUTDOWN_WAIT_UPDATES = (unsigned char)(1 << 3),
  SHUTDOWN_WAIT_ALL_BUFFERS = ((unsigned char)(1 << 3) << 1),
  SHUTDOWN_WAIT_CRITICAL_BUFFERS = ((unsigned char)(1 << 3) << 1) + 1,
  KILL_QUERY = 254,
  KILL_CONNECTION = 255
};
enum enum_resultset_metadata {
  RESULTSET_METADATA_NONE = 0,
  RESULTSET_METADATA_FULL = 1
};
enum enum_cursor_type {
  CURSOR_TYPE_NO_CURSOR = 0,
  CURSOR_TYPE_READ_ONLY = 1,
  CURSOR_TYPE_FOR_UPDATE = 2,
  CURSOR_TYPE_SCROLLABLE = 4
};
enum enum_mysql_set_option {
  MYSQL_OPTION_MULTI_STATEMENTS_ON,
  MYSQL_OPTION_MULTI_STATEMENTS_OFF
};
enum connect_stage {
  CONNECT_STAGE_INVALID = 0,
  CONNECT_STAGE_NOT_STARTED,
  CONNECT_STAGE_NET_BEGIN_CONNECT,
  CONNECT_STAGE_NET_WAIT_CONNECT,
  CONNECT_STAGE_NET_COMPLETE_CONNECT,
  CONNECT_STAGE_READ_GREETING,
  CONNECT_STAGE_PARSE_HANDSHAKE,
  CONNECT_STAGE_ESTABLISH_SSL,
  CONNECT_STAGE_AUTHENTICATE,
  CONNECT_STAGE_PREP_SELECT_DATABASE,
  CONNECT_STAGE_PREP_INIT_COMMANDS,
  CONNECT_STAGE_SEND_ONE_INIT_COMMAND,
  CONNECT_STAGE_COMPLETE
};
enum enum_session_state_type {
  SESSION_TRACK_SYSTEM_VARIABLES,
  SESSION_TRACK_SCHEMA,
  SESSION_TRACK_STATE_CHANGE,
  SESSION_TRACK_GTIDS,
  SESSION_TRACK_TRANSACTION_CHARACTERISTICS,
  SESSION_TRACK_TRANSACTION_STATE,
  SESSION_TRACK_RESP_ATTR = 32
};
bool my_net_init(struct NET *net, struct Vio * vio);
void my_net_local_init(struct NET *net);
void net_end(struct NET *net);
void net_clear(struct NET *net, bool check_buffer);
void net_claim_memory_ownership(struct NET *net);
bool net_realloc(struct NET *net, size_t length);
bool net_flush(struct NET *net);
bool my_net_write(struct NET *net, const unsigned char *packet, size_t len);
bool net_write_command(struct NET *net, unsigned char command,
                       const unsigned char *header, size_t head_len,
                       const unsigned char *packet, size_t len);
bool net_write_packet(struct NET *net, const unsigned char *packet,
                      size_t length);
unsigned long my_net_read(struct NET *net);
void my_net_set_write_timeout(struct NET *net, const timeout_t timeout);
void my_net_set_read_timeout(struct NET *net, const timeout_t timeout);
void my_net_set_retry_count(struct NET *net, unsigned int retry_count);
timeout_t timeout_from_seconds(uint seconds);
timeout_t timeout_from_millis(uint ms);
timeout_t timeout_infinite(void);
bool timeout_is_infinite(const timeout_t t);
int timeout_is_nonzero(const timeout_t t);
unsigned int timeout_to_millis(const timeout_t t);
unsigned int timeout_to_seconds(const timeout_t t);
struct rand_struct {
  unsigned long seed1, seed2, max_value;
  double max_value_dbl;
};
#include <mysql/udf_registration_types.h>
enum Item_result {
  INVALID_RESULT = -1,
  STRING_RESULT = 0,
  REAL_RESULT,
  INT_RESULT,
  ROW_RESULT,
  DECIMAL_RESULT
};
typedef struct UDF_ARGS {
  unsigned int arg_count;
  enum Item_result *arg_type;
  char **args;
  unsigned long *lengths;
  char *maybe_null;
  char **attributes;
  unsigned long *attribute_lengths;
  void *extension;
} UDF_ARGS;
typedef struct UDF_INIT {
  bool maybe_null;
  unsigned int decimals;
  unsigned long max_length;
  char *ptr;
  bool const_item;
  void *extension;
} UDF_INIT;
enum Item_udftype { UDFTYPE_FUNCTION = 1, UDFTYPE_AGGREGATE };
typedef void (*Udf_func_clear)(UDF_INIT *, unsigned char *, unsigned char *);
typedef void (*Udf_func_add)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                             unsigned char *);
typedef void (*Udf_func_deinit)(UDF_INIT *);
typedef bool (*Udf_func_init)(UDF_INIT *, UDF_ARGS *, char *);
typedef void (*Udf_func_any)(void);
typedef double (*Udf_func_double)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                                  unsigned char *);
typedef long long (*Udf_func_longlong)(UDF_INIT *, UDF_ARGS *, unsigned char *,
                                       unsigned char *);
typedef char *(*Udf_func_string)(UDF_INIT *, UDF_ARGS *, char *,
                                 unsigned long *, unsigned char *,
                                 unsigned char *);
void randominit(struct rand_struct *, unsigned long seed1, unsigned long seed2);
double my_rnd(struct rand_struct *);
void create_random_string(char *to, unsigned int length,
                          struct rand_struct *rand_st);
void hash_password(unsigned long *to, const char *password,
                   unsigned int password_len);
void make_scrambled_password_323(char *to, const char *password);
void scramble_323(char *to, const char *message, const char *password);
bool check_scramble_323(const unsigned char *reply, const char *message,
                        unsigned long *salt);
void get_salt_from_password_323(unsigned long *res, const char *password);
void make_password_from_salt_323(char *to, const unsigned long *salt);
void make_scrambled_password(char *to, const char *password);
void scramble(char *to, const char *message, const char *password);
bool check_scramble(const unsigned char *reply, const char *message,
                    const unsigned char *hash_stage2);
void get_salt_from_password(unsigned char *res, const char *password);
void make_password_from_salt(char *to, const unsigned char *hash_stage2);
char *octet2hex(char *to, const char *str, unsigned int len);
bool generate_sha256_scramble(unsigned char *dst, size_t dst_size,
                              const char *src, size_t src_size, const char *rnd,
                              size_t rnd_size);
char *get_tty_password(const char *opt_message);
const char *mysql_errno_to_sqlstate(unsigned int mysql_errno);
bool my_thread_init(void);
void my_thread_end(void);
unsigned long net_field_length(unsigned char **packet);
unsigned long net_field_length_checked(unsigned char **packet,
                                               unsigned long max_length);
uint64_t net_field_length_ll(unsigned char **packet);
unsigned char *net_store_length(unsigned char *pkg, unsigned long long length);
unsigned int net_length_size(unsigned long long num);
unsigned int net_field_length_size(const unsigned char *pos);
#include "mysql/client_plugin.h"
struct st_mysql_client_plugin {
  int type; unsigned int interface_version; const char *name; const char *author; const char *desc; unsigned int version[3]; const char *license; void *mysql_api; int (*init)(char *, size_t, int, va_list); int (*deinit)(void); int (*options)(const char *option, const void *);
};
struct MYSQL;
#include "plugin_auth_common.h"
struct MYSQL_PLUGIN_VIO_INFO {
  enum {
    MYSQL_VIO_INVALID,
    MYSQL_VIO_TCP,
    MYSQL_VIO_SOCKET,
    MYSQL_VIO_PIPE,
    MYSQL_VIO_MEMORY
  } protocol;
  int socket;
};
enum net_async_status {
  NET_ASYNC_COMPLETE = 0,
  NET_ASYNC_NOT_READY,
  NET_ASYNC_ERROR,
  NET_ASYNC_COMPLETE_NO_MORE_RESULTS
};
typedef struct MYSQL_PLUGIN_VIO {
  int (*read_packet)(struct MYSQL_PLUGIN_VIO *vio, unsigned char **buf);
  int (*write_packet)(struct MYSQL_PLUGIN_VIO *vio, const unsigned char *packet,
                      int packet_len);
  void (*info)(struct MYSQL_PLUGIN_VIO *vio,
               struct MYSQL_PLUGIN_VIO_INFO *info);
  enum net_async_status (*read_packet_nonblocking)(struct MYSQL_PLUGIN_VIO *vio,
                                                   unsigned char **buf,
                                                   int *result);
  enum net_async_status (*write_packet_nonblocking)(
      struct MYSQL_PLUGIN_VIO *vio, const unsigned char *pkt, int pkt_len,
      int *result);
} MYSQL_PLUGIN_VIO;
struct auth_plugin_t {
  int type; unsigned int interface_version; const char *name; const char *author; const char *desc; unsigned int version[3]; const char *license; void *mysql_api; int (*init)(char *, size_t, int, va_list); int (*deinit)(void); int (*options)(const char *option, const void *);
  int (*authenticate_user)(MYSQL_PLUGIN_VIO *vio, struct MYSQL *mysql);
  enum net_async_status (*authenticate_user_nonblocking)(MYSQL_PLUGIN_VIO *vio,
                                                         struct MYSQL *mysql,
                                                         int *result);
};
typedef struct auth_plugin_t st_mysql_client_plugin_AUTHENTICATION;
struct st_mysql_client_plugin *mysql_load_plugin(struct MYSQL *mysql,
                                                 const char *name, int type,
                                                 int argc, ...);
struct st_mysql_client_plugin *mysql_load_plugin_v(struct MYSQL *mysql,
                                                   const char *name, int type,
                                                   int argc, va_list args);
struct st_mysql_client_plugin *mysql_client_find_plugin(struct MYSQL *mysql,
                                                        const char *name,
                                                        int type);
struct st_mysql_client_plugin *mysql_client_register_plugin(
    struct MYSQL *mysql, struct st_mysql_client_plugin *plugin);
int mysql_plugin_options(struct st_mysql_client_plugin *plugin,
                         const char *option, const void *value);
#include "mysql_version.h"
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
#include "errmsg.h"
void init_client_errs(void);
void finish_client_errs(void);
extern const char *client_errors[];
static inline bool isPlaceHolder(int client_errno) {
  return client_errno >= 2068 &&
         client_errno <= 2199;
}
static inline const char *ER_CLIENT(int client_errno) {
  if (client_errno >= 2000 && client_errno <= 2202 &&
      !isPlaceHolder(client_errno)) {
    return client_errors[client_errno - 2000];
  }
  return client_errors[2000 - 2000];
}
extern unsigned int mysql_port;
extern char *mysql_unix_port;
typedef struct MYSQL_FIELD {
  char *name;
  char *org_name;
  char *table;
  char *org_table;
  char *db;
  char *catalog;
  char *def;
  unsigned long length;
  unsigned long max_length;
  unsigned int name_length;
  unsigned int org_name_length;
  unsigned int table_length;
  unsigned int org_table_length;
  unsigned int db_length;
  unsigned int catalog_length;
  unsigned int def_length;
  unsigned int flags;
  unsigned int decimals;
  unsigned int charsetnr;
  enum enum_field_types type;
  void *extension;
} MYSQL_FIELD;
typedef char **MYSQL_ROW;
typedef unsigned int MYSQL_FIELD_OFFSET;
typedef struct MYSQL_ROWS {
  struct MYSQL_ROWS *next;
  MYSQL_ROW data;
  unsigned long length;
} MYSQL_ROWS;
typedef MYSQL_ROWS *MYSQL_ROW_OFFSET;
struct MEM_ROOT;
typedef struct MYSQL_DATA {
  MYSQL_ROWS *data;
  struct MEM_ROOT *alloc;
  uint64_t rows;
  unsigned int fields;
} MYSQL_DATA;
enum mysql_option {
  MYSQL_OPT_CONNECT_TIMEOUT,
  MYSQL_OPT_COMPRESS,
  MYSQL_OPT_NAMED_PIPE,
  MYSQL_INIT_COMMAND,
  MYSQL_READ_DEFAULT_FILE,
  MYSQL_READ_DEFAULT_GROUP,
  MYSQL_SET_CHARSET_DIR,
  MYSQL_SET_CHARSET_NAME,
  MYSQL_OPT_LOCAL_INFILE,
  MYSQL_OPT_PROTOCOL,
  MYSQL_SHARED_MEMORY_BASE_NAME,
  MYSQL_OPT_READ_TIMEOUT,
  MYSQL_OPT_WRITE_TIMEOUT,
  MYSQL_OPT_USE_RESULT,
  MYSQL_REPORT_DATA_TRUNCATION,
  MYSQL_OPT_RECONNECT,
  MYSQL_PLUGIN_DIR,
  MYSQL_DEFAULT_AUTH,
  MYSQL_OPT_BIND,
  MYSQL_OPT_SSL_KEY,
  MYSQL_OPT_SSL_CERT,
  MYSQL_OPT_SSL_CA,
  MYSQL_OPT_SSL_CAPATH,
  MYSQL_OPT_SSL_CIPHER,
  MYSQL_OPT_SSL_CRL,
  MYSQL_OPT_SSL_CRLPATH,
  MYSQL_OPT_CONNECT_ATTR_RESET,
  MYSQL_OPT_CONNECT_ATTR_ADD,
  MYSQL_OPT_CONNECT_ATTR_DELETE,
  MYSQL_OPT_QUERY_ATTR_RESET,
  MYSQL_OPT_QUERY_ATTR_ADD,
  MYSQL_OPT_QUERY_ATTR_DELETE,
  MYSQL_SERVER_PUBLIC_KEY,
  MYSQL_ENABLE_CLEARTEXT_PLUGIN,
  MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
  MYSQL_OPT_MAX_ALLOWED_PACKET,
  MYSQL_OPT_NET_BUFFER_LENGTH,
  MYSQL_OPT_TLS_VERSION,
  MYSQL_OPT_SSL_MODE,
  MYSQL_OPT_GET_SERVER_PUBLIC_KEY,
  MYSQL_OPT_RETRY_COUNT,
  MYSQL_OPT_OPTIONAL_RESULTSET_METADATA,
  MYSQL_OPT_SSL_FIPS_MODE,
  MYSQL_OPT_TLS_CIPHERSUITES,
  MYSQL_OPT_COMPRESSION_ALGORITHMS,
  MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
  MYSQL_OPT_SSL_SESSION,
  MYSQL_OPT_SSL_CONTEXT,
  MYSQL_OPT_NET_RECEIVE_BUFFER_SIZE,
  MYSQL_OPT_CONNECT_TIMEOUT_MS,
  MYSQL_OPT_READ_TIMEOUT_MS,
  MYSQL_OPT_WRITE_TIMEOUT_MS,
  MYSQL_OPT_TLS_SNI_SERVERNAME,
  MYSQL_OPT_TLS_CERT_CALLBACK,
  MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT,
  MYSQL_OPT_TOS,
};
struct st_mysql_options_extention;
struct st_mysql_options {
  timeout_t connect_timeout, read_timeout, write_timeout;
  unsigned int port, protocol;
  unsigned long client_flag;
  char *host, *user, *password, *unix_socket, *db;
  struct Init_commands_array *init_commands;
  char *my_cnf_file, *my_cnf_group, *charset_dir, *charset_name;
  char *ssl_key;
  char *ssl_cert;
  char *ssl_ca;
  char *ssl_capath;
  char *ssl_cipher;
  char *shared_memory_base_name;
  unsigned long max_allowed_packet;
  bool compress, named_pipe;
  char *bind_address;
  bool report_data_truncation;
  int (*local_infile_init)(void **, const char *, void *);
  int (*local_infile_read)(void *, char *, unsigned int);
  void (*local_infile_end)(void *);
  int (*local_infile_error)(void *, char *, unsigned int);
  void *local_infile_userdata;
  struct st_mysql_options_extention *extension;
};
enum mysql_status {
  MYSQL_STATUS_READY,
  MYSQL_STATUS_GET_RESULT,
  MYSQL_STATUS_USE_RESULT,
  MYSQL_STATUS_STATEMENT_GET_RESULT
};
enum mysql_protocol_type {
  MYSQL_PROTOCOL_DEFAULT,
  MYSQL_PROTOCOL_TCP,
  MYSQL_PROTOCOL_SOCKET,
  MYSQL_PROTOCOL_PIPE,
  MYSQL_PROTOCOL_MEMORY
};
enum mysql_ssl_mode {
  SSL_MODE_DISABLED = 1,
  SSL_MODE_PREFERRED,
  SSL_MODE_REQUIRED,
  SSL_MODE_VERIFY_CA,
  SSL_MODE_VERIFY_IDENTITY
};
enum mysql_ssl_fips_mode {
  SSL_FIPS_MODE_OFF = 0,
  SSL_FIPS_MODE_ON = 1,
  SSL_FIPS_MODE_STRICT
};
typedef struct character_set {
  unsigned int number;
  unsigned int state;
  const char *csname;
  const char *name;
  const char *comment;
  const char *dir;
  unsigned int mbminlen;
  unsigned int mbmaxlen;
} MY_CHARSET_INFO;
struct MYSQL_METHODS;
struct MYSQL_STMT;
typedef struct MYSQL {
  NET net;
  unsigned char *connector_fd;
  char *host, *user, *passwd, *unix_socket, *server_version, *host_info;
  char *info, *db;
  struct CHARSET_INFO *charset;
  MYSQL_FIELD *fields;
  struct MEM_ROOT *field_alloc;
  uint64_t affected_rows;
  uint64_t insert_id;
  uint64_t extra_info;
  bool should_record_checksum;
  unsigned long checksum;
  unsigned long thread_id;
  unsigned long packet_length;
  unsigned int port;
  unsigned long client_flag, server_capabilities;
  unsigned int protocol_version;
  unsigned int field_count;
  unsigned int server_status;
  unsigned int server_language;
  unsigned int warning_count;
  struct st_mysql_options options;
  enum mysql_status status;
  enum enum_resultset_metadata resultset_metadata;
  bool free_me;
  bool reconnect;
  char scramble[20 + 1];
  LIST *stmts;
  const struct MYSQL_METHODS *methods;
  void *thd;
  bool *unbuffered_fetch_owner;
  void *extension;
} MYSQL;
typedef struct MYSQL_RES {
  uint64_t row_count;
  MYSQL_FIELD *fields;
  struct MYSQL_DATA *data;
  MYSQL_ROWS *data_cursor;
  unsigned long *lengths;
  MYSQL *handle;
  const struct MYSQL_METHODS *methods;
  MYSQL_ROW row;
  MYSQL_ROW current_row;
  struct MEM_ROOT *field_alloc;
  unsigned int field_count, current_field;
  bool eof;
  bool unbuffered_fetch_cancelled;
  enum enum_resultset_metadata metadata;
  void *extension;
} MYSQL_RES;
typedef struct MYSQL_RPL {
  size_t file_name_length;
  const char *file_name;
  uint64_t start_position;
  unsigned int server_id;
  unsigned int flags;
  size_t gtid_set_encoded_size;
  void (*fix_gtid_set)(struct MYSQL_RPL *rpl, unsigned char *packet_gtid_set);
  void *gtid_set_arg;
  unsigned long size;
  const unsigned char *buffer;
} MYSQL_RPL;
int mysql_server_init(int argc, char **argv, char **groups);
void mysql_server_end(void);
bool mysql_thread_init(void);
void mysql_thread_end(void);
uint64_t mysql_num_rows(MYSQL_RES *res);
unsigned int mysql_num_fields(MYSQL_RES *res);
bool mysql_eof(MYSQL_RES *res);
MYSQL_FIELD * mysql_fetch_field_direct(MYSQL_RES *res,
                                              unsigned int fieldnr);
MYSQL_FIELD * mysql_fetch_fields(MYSQL_RES *res);
MYSQL_ROW_OFFSET mysql_row_tell(MYSQL_RES *res);
MYSQL_FIELD_OFFSET mysql_field_tell(MYSQL_RES *res);
enum enum_resultset_metadata mysql_result_metadata(MYSQL_RES *result);
unsigned int mysql_field_count(MYSQL *mysql);
uint64_t mysql_affected_rows(MYSQL *mysql);
uint64_t mysql_insert_id(MYSQL *mysql);
unsigned int mysql_errno(MYSQL *mysql);
const char * mysql_error(MYSQL *mysql);
const char * mysql_sqlstate(MYSQL *mysql);
unsigned int mysql_warning_count(MYSQL *mysql);
const char * mysql_info(MYSQL *mysql);
unsigned long mysql_thread_id(MYSQL *mysql);
const char * mysql_character_set_name(MYSQL *mysql);
int mysql_set_character_set(MYSQL *mysql, const char *csname);
MYSQL * mysql_init(MYSQL *mysql);
bool mysql_ssl_set(MYSQL *mysql, const char *key, const char *cert,
                           const char *ca, const char *capath,
                           const char *cipher);
const char * mysql_get_ssl_version(MYSQL *mysql);
const char * mysql_get_ssl_cipher(MYSQL *mysql);
void * mysql_get_ssl_session(MYSQL *mysql);
bool mysql_get_ssl_session_reused(MYSQL *mysql);
void * mysql_take_ssl_context_ownership(MYSQL *mysql);
bool mysql_change_user(MYSQL *mysql, const char *user,
                               const char *passwd, const char *db);
enum net_async_status mysql_change_user_nonblocking(MYSQL *mysql,
                                                    const char *user,
                                                    const char *passwd,
                                                    const char *db);
MYSQL * mysql_real_connect(MYSQL *mysql, const char *host,
                                  const char *user, const char *passwd,
                                  const char *db, unsigned int port,
                                  const char *unix_socket,
                                  unsigned long clientflag);
int mysql_select_db(MYSQL *mysql, const char *db);
int mysql_query(MYSQL *mysql, const char *q);
int mysql_send_query(MYSQL *mysql, const char *q, unsigned long length);
int mysql_real_query(MYSQL *mysql, const char *q, unsigned long length);
MYSQL_RES * mysql_store_result(MYSQL *mysql);
MYSQL_RES * mysql_use_result(MYSQL *mysql);
enum net_async_status mysql_real_connect_nonblocking(
    MYSQL *mysql, const char *host, const char *user, const char *passwd,
    const char *db, unsigned int port, const char *unix_socket,
    unsigned long clientflag);
enum net_async_status mysql_send_query_nonblocking(
    MYSQL *mysql, const char *query, unsigned long length);
enum net_async_status mysql_real_query_nonblocking(
    MYSQL *mysql, const char *query, unsigned long length);
enum net_async_status
mysql_store_result_nonblocking(MYSQL *mysql, MYSQL_RES **result);
enum net_async_status mysql_next_result_nonblocking(MYSQL *mysql);
enum net_async_status mysql_select_db_nonblocking(MYSQL *mysql,
                                                          const char *db,
                                                          bool *error);
int mysql_get_socket_descriptor(MYSQL *mysql);
void mysql_get_character_set_info(MYSQL *mysql,
                                          MY_CHARSET_INFO *charset);
int mysql_session_track_get_first(MYSQL *mysql,
                                          enum enum_session_state_type type,
                                          const char **data, size_t *length);
int mysql_session_track_get_next(MYSQL *mysql,
                                         enum enum_session_state_type type,
                                         const char **data, size_t *length);
int mysql_resp_attr_find(MYSQL *mysql, const char *lookup,
                                 const char **data, size_t *length);
enum connect_stage mysql_get_connect_stage(MYSQL *mysql);
void mysql_set_local_infile_handler(
    MYSQL *mysql, int (*local_infile_init)(void **, const char *, void *),
    int (*local_infile_read)(void *, char *, unsigned int),
    void (*local_infile_end)(void *),
    int (*local_infile_error)(void *, char *, unsigned int), void *);
void mysql_set_local_infile_default(MYSQL *mysql);
int mysql_shutdown(MYSQL *mysql,
                           enum mysql_enum_shutdown_level shutdown_level);
int mysql_dump_debug_info(MYSQL *mysql);
int mysql_refresh(MYSQL *mysql, unsigned int refresh_options);
int mysql_kill(MYSQL *mysql, unsigned long pid);
int mysql_set_server_option(MYSQL *mysql,
                                    enum enum_mysql_set_option option);
int mysql_ping(MYSQL *mysql);
const char * mysql_stat(MYSQL *mysql);
const char * mysql_get_server_info(MYSQL *mysql);
const char * mysql_get_client_info(void);
unsigned long mysql_get_client_version(void);
const char * mysql_get_host_info(MYSQL *mysql);
unsigned long mysql_get_server_version(MYSQL *mysql);
unsigned int mysql_get_proto_info(MYSQL *mysql);
MYSQL_RES * mysql_list_dbs(MYSQL *mysql, const char *wild);
MYSQL_RES * mysql_list_tables(MYSQL *mysql, const char *wild);
MYSQL_RES * mysql_list_processes(MYSQL *mysql);
int mysql_options(MYSQL *mysql, enum mysql_option option,
                          const void *arg);
int mysql_options4(MYSQL *mysql, enum mysql_option option,
                           const void *arg1, const void *arg2);
int mysql_get_option(MYSQL *mysql, enum mysql_option option,
                             const void *arg);
void mysql_free_result(MYSQL_RES *result);
enum net_async_status mysql_free_result_nonblocking(MYSQL_RES *result);
void mysql_data_seek(MYSQL_RES *result, uint64_t offset);
MYSQL_ROW_OFFSET mysql_row_seek(MYSQL_RES *result,
                                        MYSQL_ROW_OFFSET offset);
MYSQL_FIELD_OFFSET mysql_field_seek(MYSQL_RES *result,
                                            MYSQL_FIELD_OFFSET offset);
MYSQL_ROW mysql_fetch_row(MYSQL_RES *result);
enum net_async_status mysql_fetch_row_nonblocking(MYSQL_RES *res,
                                                          MYSQL_ROW *row);
unsigned long * mysql_fetch_lengths(MYSQL_RES *result);
MYSQL_FIELD * mysql_fetch_field(MYSQL_RES *result);
MYSQL_RES * mysql_list_fields(MYSQL *mysql, const char *table,
                                     const char *wild);
unsigned long mysql_escape_string(char *to, const char *from,
                                          unsigned long from_length);
unsigned long mysql_hex_string(char *to, const char *from,
                                       unsigned long from_length);
unsigned long mysql_real_escape_string(MYSQL *mysql, char *to,
                                               const char *from,
                                               unsigned long length);
unsigned long mysql_real_escape_string_quote(MYSQL *mysql, char *to,
                                                     const char *from,
                                                     unsigned long length,
                                                     char quote);
void mysql_debug(const char *debug);
void myodbc_remove_escape(MYSQL *mysql, char *name);
unsigned int mysql_thread_safe(void);
bool mysql_read_query_result(MYSQL *mysql);
int mysql_reset_connection(MYSQL *mysql);
enum net_async_status mysql_reset_connection_nonblocking(MYSQL *mysql);
unsigned long cli_safe_read(MYSQL *mysql, bool *is_data_packet);
enum net_async_status cli_safe_read_nonblocking(MYSQL *mysql,
                                                        bool *is_data_packet,
                                                        unsigned long *res);
int mysql_binlog_open(MYSQL *mysql, MYSQL_RPL *rpl);
int mysql_binlog_fetch(MYSQL *mysql, MYSQL_RPL *rpl);
void mysql_binlog_close(MYSQL *mysql, MYSQL_RPL *rpl);
enum enum_mysql_stmt_state {
  MYSQL_STMT_INIT_DONE = 1,
  MYSQL_STMT_PREPARE_DONE,
  MYSQL_STMT_EXECUTE_DONE,
  MYSQL_STMT_FETCH_DONE
};
typedef struct MYSQL_BIND {
  unsigned long *length;
  bool *is_null;
  void *buffer;
  bool *error;
  unsigned char *row_ptr;
  void (*store_param_func)(NET *net, struct MYSQL_BIND *param);
  void (*fetch_result)(struct MYSQL_BIND *, MYSQL_FIELD *, unsigned char **row);
  void (*skip_result)(struct MYSQL_BIND *, MYSQL_FIELD *, unsigned char **row);
  unsigned long buffer_length;
  unsigned long offset;
  unsigned long length_value;
  unsigned int param_number;
  unsigned int pack_length;
  enum enum_field_types buffer_type;
  bool error_value;
  bool is_unsigned;
  bool long_data_used;
  bool is_null_value;
  void *extension;
} MYSQL_BIND;
struct MYSQL_STMT_EXT;
typedef struct MYSQL_STMT {
  struct MEM_ROOT *mem_root;
  LIST list;
  MYSQL *mysql;
  MYSQL_BIND *params;
  MYSQL_BIND *bind;
  MYSQL_FIELD *fields;
  MYSQL_DATA result;
  MYSQL_ROWS *data_cursor;
  int (*read_row_func)(struct MYSQL_STMT *stmt, unsigned char **row);
  uint64_t affected_rows;
  uint64_t insert_id;
  unsigned long stmt_id;
  unsigned long flags;
  unsigned long prefetch_rows;
  unsigned int server_status;
  unsigned int last_errno;
  unsigned int param_count;
  unsigned int field_count;
  enum enum_mysql_stmt_state state;
  char last_error[512];
  char sqlstate[5 + 1];
  bool send_types_to_server;
  bool bind_param_done;
  unsigned char bind_result_done;
  bool unbuffered_fetch_cancelled;
  bool update_max_length;
  struct MYSQL_STMT_EXT *extension;
} MYSQL_STMT;
enum enum_stmt_attr_type {
  STMT_ATTR_UPDATE_MAX_LENGTH,
  STMT_ATTR_CURSOR_TYPE,
  STMT_ATTR_PREFETCH_ROWS
};
MYSQL_STMT * mysql_stmt_init(MYSQL *mysql);
int mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query,
                               unsigned long length);
int mysql_stmt_execute(MYSQL_STMT *stmt);
int mysql_stmt_fetch(MYSQL_STMT *stmt);
int mysql_stmt_fetch_column(MYSQL_STMT *stmt, MYSQL_BIND *bind_arg,
                                    unsigned int column, unsigned long offset);
int mysql_stmt_store_result(MYSQL_STMT *stmt);
unsigned long mysql_stmt_param_count(MYSQL_STMT *stmt);
bool mysql_stmt_attr_set(MYSQL_STMT *stmt,
                                 enum enum_stmt_attr_type attr_type,
                                 const void *attr);
bool mysql_stmt_attr_get(MYSQL_STMT *stmt,
                                 enum enum_stmt_attr_type attr_type,
                                 void *attr);
bool mysql_stmt_bind_param(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
bool mysql_stmt_bind_result(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
bool mysql_stmt_close(MYSQL_STMT *stmt);
bool mysql_stmt_reset(MYSQL_STMT *stmt);
bool mysql_stmt_free_result(MYSQL_STMT *stmt);
bool mysql_stmt_send_long_data(MYSQL_STMT *stmt,
                                       unsigned int param_number,
                                       const char *data, unsigned long length);
MYSQL_RES * mysql_stmt_result_metadata(MYSQL_STMT *stmt);
MYSQL_RES * mysql_stmt_param_metadata(MYSQL_STMT *stmt);
unsigned int mysql_stmt_errno(MYSQL_STMT *stmt);
const char * mysql_stmt_error(MYSQL_STMT *stmt);
const char * mysql_stmt_sqlstate(MYSQL_STMT *stmt);
MYSQL_ROW_OFFSET mysql_stmt_row_seek(MYSQL_STMT *stmt,
                                             MYSQL_ROW_OFFSET offset);
MYSQL_ROW_OFFSET mysql_stmt_row_tell(MYSQL_STMT *stmt);
void mysql_stmt_data_seek(MYSQL_STMT *stmt, uint64_t offset);
uint64_t mysql_stmt_num_rows(MYSQL_STMT *stmt);
uint64_t mysql_stmt_affected_rows(MYSQL_STMT *stmt);
uint64_t mysql_stmt_insert_id(MYSQL_STMT *stmt);
unsigned int mysql_stmt_field_count(MYSQL_STMT *stmt);
bool mysql_commit(MYSQL *mysql);
bool mysql_rollback(MYSQL *mysql);
bool mysql_autocommit(MYSQL *mysql, bool auto_mode);
bool mysql_more_results(MYSQL *mysql);
int mysql_next_result(MYSQL *mysql);
int mysql_stmt_next_result(MYSQL_STMT *stmt);
void mysql_close(MYSQL *sock);
void mysql_reset_server_public_key(void);
