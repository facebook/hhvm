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
