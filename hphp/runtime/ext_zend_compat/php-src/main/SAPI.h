/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:  Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef SAPI_H
#define SAPI_H

#include "zend.h"
#include "zend_API.h"
#include "zend_llist.h"
#include "zend_operators.h"
#ifdef PHP_WIN32
#include "win95nt.h"
#endif
#include <sys/stat.h>

#define SAPI_OPTION_NO_CHDIR 1

#define SAPI_POST_BLOCK_SIZE 4000

#ifdef PHP_WIN32
#  ifdef SAPI_EXPORTS
#    define SAPI_API __declspec(dllexport) 
#  else
#    define SAPI_API __declspec(dllimport) 
#  endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define SAPI_API __attribute__ ((visibility("default")))
#else
#  define SAPI_API
#endif

#undef shutdown

typedef struct {
  char *header;
  uint header_len;
} sapi_header_struct;


typedef struct {
  zend_llist headers;
  int http_response_code;
  unsigned char send_default_content_type;
  char *mimetype;
  char *http_status_line;
} sapi_headers_struct;


typedef struct _sapi_post_entry sapi_post_entry;
typedef struct _sapi_module_struct sapi_module_struct;

BEGIN_EXTERN_C()
extern SAPI_API sapi_module_struct sapi_module;  /* true global */
END_EXTERN_C()

/* Some values in this structure needs to be filled in before
 * calling sapi_activate(). We WILL change the `char *' entries,
 * so make sure that you allocate a separate buffer for them
 * and that you free them after sapi_deactivate().
 */

typedef struct {
  const char *request_method;
  char *query_string;
  char *post_data, *raw_post_data;
  char *cookie_data;
  long content_length;
  uint post_data_length, raw_post_data_length;

  char *path_translated;
  char *request_uri;

  const char *content_type;

  zend_bool headers_only;
  zend_bool no_headers;
  zend_bool headers_read;

  sapi_post_entry *post_entry;

  char *content_type_dup;

  /* for HTTP authentication */
  char *auth_user;
  char *auth_password;
  char *auth_digest;

  /* this is necessary for the CGI SAPI module */
  char *argv0;

  char *current_user;
  int current_user_length;

  /* this is necessary for CLI module */
  int argc;
  char **argv;
  int proto_num;
} sapi_request_info;


typedef struct _sapi_globals_struct {
  void *server_context;
  sapi_request_info request_info;
  sapi_headers_struct sapi_headers;
  int read_post_bytes;
  unsigned char headers_sent;
  struct stat global_stat;
  char *default_mimetype;
  char *default_charset;
  HashTable *rfc1867_uploaded_files;
  long post_max_size;
  int options;
  zend_bool sapi_started;
  double global_request_time;
  HashTable known_post_content_types;
  zval *callback_func;
  zend_fcall_info_cache fci_cache;
  zend_bool callback_run;
} sapi_globals_struct;


BEGIN_EXTERN_C()
#ifdef ZTS
# define SG(v) TSRMG(sapi_globals_id, sapi_globals_struct *, v)
SAPI_API extern int sapi_globals_id;
#else
# define SG(v) (sapi_globals.v)
extern SAPI_API sapi_globals_struct sapi_globals;
#endif

SAPI_API void sapi_startup(sapi_module_struct *sf);
SAPI_API void sapi_shutdown(void);
SAPI_API void sapi_activate(TSRMLS_D);
SAPI_API void sapi_deactivate(TSRMLS_D);
SAPI_API void sapi_initialize_empty_request(TSRMLS_D);
END_EXTERN_C()

/*
 * This is the preferred and maintained API for 
 * operating on HTTP headers.
 */

/*
 * Always specify a sapi_header_line this way:
 *
 *     sapi_header_line ctr = {0};
 */
 
typedef struct {
  char *line; /* If you allocated this, you need to free it yourself */
  uint line_len;
  long response_code; /* long due to zend_parse_parameters compatibility */
} sapi_header_line;

typedef enum {          /* Parameter:       */
  SAPI_HEADER_REPLACE,    /* sapi_header_line*   */
  SAPI_HEADER_ADD,      /* sapi_header_line*   */
  SAPI_HEADER_DELETE,      /* sapi_header_line*   */
  SAPI_HEADER_DELETE_ALL,    /* void          */
  SAPI_HEADER_SET_STATUS    /* int           */
} sapi_header_op_enum;

BEGIN_EXTERN_C()
SAPI_API int sapi_header_op(sapi_header_op_enum op, void *arg TSRMLS_DC);

/* Deprecated functions. Use sapi_header_op instead. */
SAPI_API int sapi_add_header_ex(char *header_line, uint header_line_len, zend_bool duplicate, zend_bool replace TSRMLS_DC);
#define sapi_add_header(a, b, c) sapi_add_header_ex((a),(b),(c),1 TSRMLS_CC)


SAPI_API int sapi_send_headers(TSRMLS_D);
SAPI_API void sapi_free_header(sapi_header_struct *sapi_header);
SAPI_API void sapi_handle_post(void *arg TSRMLS_DC);

SAPI_API int sapi_register_post_entries(sapi_post_entry *post_entry TSRMLS_DC);
SAPI_API int sapi_register_post_entry(sapi_post_entry *post_entry TSRMLS_DC);
SAPI_API void sapi_unregister_post_entry(sapi_post_entry *post_entry TSRMLS_DC);
SAPI_API int sapi_register_default_post_reader(void (*default_post_reader)(TSRMLS_D) TSRMLS_DC);
SAPI_API int sapi_register_treat_data(void (*treat_data)(int arg, char *str, zval *destArray TSRMLS_DC) TSRMLS_DC);
SAPI_API int sapi_register_input_filter(unsigned int (*input_filter)(int arg, char *var, char **val, unsigned int val_len, unsigned int *new_val_len TSRMLS_DC), unsigned int (*input_filter_init)(TSRMLS_D) TSRMLS_DC);

SAPI_API int sapi_flush(TSRMLS_D);
SAPI_API struct stat *sapi_get_stat(TSRMLS_D);
SAPI_API char *sapi_getenv(char *name, size_t name_len TSRMLS_DC);

SAPI_API char *sapi_get_default_content_type(TSRMLS_D);
SAPI_API void sapi_get_default_content_type_header(sapi_header_struct *default_header TSRMLS_DC);
SAPI_API size_t sapi_apply_default_charset(char **mimetype, size_t len TSRMLS_DC);
SAPI_API void sapi_activate_headers_only(TSRMLS_D);

SAPI_API int sapi_get_fd(int *fd TSRMLS_DC);
SAPI_API int sapi_force_http_10(TSRMLS_D);

SAPI_API int sapi_get_target_uid(uid_t * TSRMLS_DC);
SAPI_API int sapi_get_target_gid(gid_t * TSRMLS_DC);
SAPI_API double sapi_get_request_time(TSRMLS_D);
SAPI_API void sapi_terminate_process(TSRMLS_D);
END_EXTERN_C()

struct _sapi_module_struct {
  char *name;
  char *pretty_name;

  int (*startup)(struct _sapi_module_struct *sapi_module);
  int (*shutdown)(struct _sapi_module_struct *sapi_module);

  int (*activate)(TSRMLS_D);
  int (*deactivate)(TSRMLS_D);

  int (*ub_write)(const char *str, unsigned int str_length TSRMLS_DC);
  void (*flush)(void *server_context);
  struct stat *(*get_stat)(TSRMLS_D);
  char *(*getenv)(char *name, size_t name_len TSRMLS_DC);

  void (*sapi_error)(int type, const char *error_msg, ...);

  int (*header_handler)(sapi_header_struct *sapi_header, sapi_header_op_enum op, sapi_headers_struct *sapi_headers TSRMLS_DC);
  int (*send_headers)(sapi_headers_struct *sapi_headers TSRMLS_DC);
  void (*send_header)(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC);

  int (*read_post)(char *buffer, uint count_bytes TSRMLS_DC);
  char *(*read_cookies)(TSRMLS_D);

  void (*register_server_variables)(zval *track_vars_array TSRMLS_DC);
  void (*log_message)(char *message TSRMLS_DC);
  double (*get_request_time)(TSRMLS_D);
  void (*terminate_process)(TSRMLS_D);

  char *php_ini_path_override;

  void (*block_interruptions)(void);
  void (*unblock_interruptions)(void);

  void (*default_post_reader)(TSRMLS_D);
  void (*treat_data)(int arg, char *str, zval *destArray TSRMLS_DC);
  char *executable_location;

  int php_ini_ignore;
  int php_ini_ignore_cwd; /* don't look for php.ini in the current directory */

  int (*get_fd)(int *fd TSRMLS_DC);

  int (*force_http_10)(TSRMLS_D);

  int (*get_target_uid)(uid_t * TSRMLS_DC);
  int (*get_target_gid)(gid_t * TSRMLS_DC);

  unsigned int (*input_filter)(int arg, char *var, char **val, unsigned int val_len, unsigned int *new_val_len TSRMLS_DC);
  
  void (*ini_defaults)(HashTable *configuration_hash);
  int phpinfo_as_text;

  char *ini_entries;
  const zend_function_entry *additional_functions;
  unsigned int (*input_filter_init)(TSRMLS_D);
};


struct _sapi_post_entry {
  char *content_type;
  uint content_type_len;
  void (*post_reader)(TSRMLS_D);
  void (*post_handler)(char *content_type_dup, void *arg TSRMLS_DC);
};

/* header_handler() constants */
#define SAPI_HEADER_ADD      (1<<0)


#define SAPI_HEADER_SENT_SUCCESSFULLY  1
#define SAPI_HEADER_DO_SEND        2
#define SAPI_HEADER_SEND_FAILED      3

#define SAPI_DEFAULT_MIMETYPE    "text/html"
#define SAPI_DEFAULT_CHARSET    ""
#define SAPI_PHP_VERSION_HEADER    "X-Powered-By: PHP/" PHP_VERSION

#define SAPI_POST_READER_FUNC(post_reader) void post_reader(TSRMLS_D)
#define SAPI_POST_HANDLER_FUNC(post_handler) void post_handler(char *content_type_dup, void *arg TSRMLS_DC)

#define SAPI_TREAT_DATA_FUNC(treat_data) void treat_data(int arg, char *str, zval* destArray TSRMLS_DC)
#define SAPI_INPUT_FILTER_FUNC(input_filter) unsigned int input_filter(int arg, char *var, char **val, unsigned int val_len, unsigned int *new_val_len TSRMLS_DC)

BEGIN_EXTERN_C()
SAPI_API SAPI_POST_READER_FUNC(sapi_read_standard_form_data);
SAPI_API SAPI_POST_READER_FUNC(php_default_post_reader);
SAPI_API SAPI_TREAT_DATA_FUNC(php_default_treat_data);
SAPI_API SAPI_INPUT_FILTER_FUNC(php_default_input_filter);
END_EXTERN_C()

#define STANDARD_SAPI_MODULE_PROPERTIES

#endif /* SAPI_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
