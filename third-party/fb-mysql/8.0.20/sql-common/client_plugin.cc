/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file

  Support code for the client side (libmysql) plugins

  Client plugins are somewhat different from server plugins, they are simpler.

  They do not need to be installed or in any way explicitly loaded on the
  client, they are loaded automatically on demand.
  One client plugin per shared object, soname *must* match the plugin name.

  There is no reference counting and no unloading either.
*/

#include "my_config.h"

#include <mysql/client_plugin.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>

#include "errmsg.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "sql_common.h"
#include "template_utils.h"

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#if defined(CLIENT_PROTOCOL_TRACING)
#include <mysql/plugin_trace.h>
#endif

PSI_memory_key key_memory_root;
PSI_memory_key key_memory_load_env_plugins;

PSI_mutex_key key_mutex_LOCK_load_client_plugin;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_info all_client_plugin_mutexes[] = {
    {&key_mutex_LOCK_load_client_plugin, "LOCK_load_client_plugin",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_memory_info all_client_plugin_memory[] = {
    {&key_memory_root, "root", PSI_FLAG_ONLY_GLOBAL_STAT, 0, PSI_DOCUMENT_ME},
    {&key_memory_load_env_plugins, "load_env_plugins",
     PSI_FLAG_ONLY_GLOBAL_STAT, 0, PSI_DOCUMENT_ME}};

static void init_client_plugin_psi_keys() {
  const char *category = "sql";
  int count;

  count = array_elements(all_client_plugin_mutexes);
  mysql_mutex_register(category, all_client_plugin_mutexes, count);

  count = array_elements(all_client_plugin_memory);
  mysql_memory_register(category, all_client_plugin_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

struct st_client_plugin_int {
  struct st_client_plugin_int *next;
  void *dlhandle;
  struct st_mysql_client_plugin *plugin;
};

static bool initialized = false;
static MEM_ROOT mem_root;

static const char *plugin_declarations_sym =
    "_mysql_client_plugin_declaration_";
static uint plugin_version[MYSQL_CLIENT_MAX_PLUGINS] = {
    0, /* these two are taken by Connector/C */
    0, /* these two are taken by Connector/C */
    MYSQL_CLIENT_AUTHENTICATION_PLUGIN_INTERFACE_VERSION,
    MYSQL_CLIENT_TRACE_PLUGIN_INTERFACE_VERSION,
};

/*
  Loaded plugins are stored in a linked list.
  The list is append-only, the elements are added to the head (like in a stack).
  The elements are added under a mutex, but the list can be read and traversed
  without any mutex because once an element is added to the list, it stays
  there. The main purpose of a mutex is to prevent two threads from
  loading the same plugin twice in parallel.
*/
struct st_client_plugin_int *plugin_list[MYSQL_CLIENT_MAX_PLUGINS];
static mysql_mutex_t LOCK_load_client_plugin;

static int is_not_initialized(MYSQL *mysql, const char *name) {
  if (initialized) return 0;

  set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, unknown_sqlstate,
                           ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD), name,
                           "not initialized");
  return 1;
}

/**
  finds a plugin in the list

  @param name   plugin name to search for
  @param type   plugin type

  @note this does NOT necessarily need a mutex, take care!

  @retval a pointer to a found plugin or 0
*/
static struct st_mysql_client_plugin *find_plugin(const char *name, int type) {
  struct st_client_plugin_int *p;

  DBUG_ASSERT(initialized);
  DBUG_ASSERT(type >= 0 && type < MYSQL_CLIENT_MAX_PLUGINS);
  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS) return nullptr;

  for (p = plugin_list[type]; p; p = p->next) {
    if (strcmp(p->plugin->name, name) == 0) return p->plugin;
  }
  return nullptr;
}

/**
  verifies the plugin and adds it to the list

  @param mysql          MYSQL structure (for error reporting)
  @param plugin         plugin to install
  @param dlhandle       a handle to the shared object (returned by dlopen)
                        or 0 if the plugin was not dynamically loaded
  @param argc           number of arguments in the 'va_list args'
  @param args           arguments passed to the plugin initialization function

  @retval a pointer to an installed plugin or 0
*/
static struct st_mysql_client_plugin *do_add_plugin(
    MYSQL *mysql, struct st_mysql_client_plugin *plugin, void *dlhandle,
    int argc, va_list args) {
  const char *errmsg;
  struct st_client_plugin_int plugin_int, *p;
  char errbuf[1024];

  DBUG_ASSERT(initialized);

  plugin_int.plugin = plugin;
  plugin_int.dlhandle = dlhandle;

  if (plugin->type >= MYSQL_CLIENT_MAX_PLUGINS) {
    errmsg = "Unknown client plugin type";
    goto err1;
  }

  if (plugin->interface_version < plugin_version[plugin->type] ||
      (plugin->interface_version >> 8) > (plugin_version[plugin->type] >> 8)) {
    errmsg = "Incompatible client plugin interface";
    goto err1;
  }

#if defined(CLIENT_PROTOCOL_TRACING) && !defined(MYSQL_SERVER)
  /*
    If we try to load a protocol trace plugin but one is already
    loaded (global trace_plugin pointer is not NULL) then we ignore
    the new trace plugin and give error. This is done before the
    new plugin gets initialized.
  */
  if (plugin->type == MYSQL_CLIENT_TRACE_PLUGIN && nullptr != trace_plugin) {
    errmsg = "Can not load another trace plugin while one is already loaded";
    goto err1;
  }
#endif

  /* Call the plugin initialization function, if any */
  if (plugin->init && plugin->init(errbuf, sizeof(errbuf), argc, args)) {
    errmsg = errbuf;
    goto err1;
  }

  p = (struct st_client_plugin_int *)memdup_root(&mem_root, &plugin_int,
                                                 sizeof(plugin_int));

  if (!p) {
    errmsg = "Out of memory";
    goto err2;
  }

  mysql_mutex_assert_owner(&LOCK_load_client_plugin);

  p->next = plugin_list[plugin->type];
  plugin_list[plugin->type] = p;
  net_clear_error(&mysql->net);

#if defined(CLIENT_PROTOCOL_TRACING) && !defined(MYSQL_SERVER)
  /*
    If loaded plugin is a protocol trace one, then set the global
    trace_plugin pointer to point at it. When trace_plugin is not NULL,
    each new connection will be traced using the plugin pointed by it
    (see MYSQL_TRACE_STAGE() macro in libmysql/mysql_trace.h).
  */
  if (plugin->type == MYSQL_CLIENT_TRACE_PLUGIN) {
    trace_plugin = (struct st_mysql_client_plugin_TRACE *)plugin;
  }
#endif

  return plugin;

err2:
  if (plugin->deinit) plugin->deinit();
err1:
  set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, unknown_sqlstate,
                           ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD), plugin->name,
                           errmsg);
  if (dlhandle) dlclose(dlhandle);
  return nullptr;
}

static struct st_mysql_client_plugin *add_plugin_noargs(
    MYSQL *mysql, struct st_mysql_client_plugin *plugin, void *dlhandle,
    int argc, ...) {
  struct st_mysql_client_plugin *retval = nullptr;
  va_list ap;
  va_start(ap, argc);
  retval = do_add_plugin(mysql, plugin, dlhandle, argc, ap);
  va_end(ap);
  return retval;
}

static struct st_mysql_client_plugin *add_plugin_withargs(
    MYSQL *mysql, struct st_mysql_client_plugin *plugin, void *dlhandle,
    int argc, va_list args) {
  return do_add_plugin(mysql, plugin, dlhandle, argc, args);
}

/**
  Loads plugins which are specified in the environment variable
  LIBMYSQL_PLUGINS.

  Multiple plugins must be separated by semicolon. This function doesn't
  return or log an error.

  The function is be called by mysql_client_plugin_init

  @todo
  Support extended syntax, passing parameters to plugins, for example
  LIBMYSQL_PLUGINS="plugin1(param1,param2);plugin2;..."
  or
  LIBMYSQL_PLUGINS="plugin1=int:param1,str:param2;plugin2;..."
*/
static void load_env_plugins(MYSQL *mysql) {
  char *plugs, *free_env, *s = getenv("LIBMYSQL_PLUGINS");
  char *enable_cleartext_plugin = getenv("LIBMYSQL_ENABLE_CLEARTEXT_PLUGIN");

  if (enable_cleartext_plugin && strchr("1Yy", enable_cleartext_plugin[0]))
    libmysql_cleartext_plugin_enabled = true;

  /* no plugins to load */
  if (!s) return;

  free_env = plugs = my_strdup(key_memory_load_env_plugins, s, MYF(MY_WME));

  do {
    if ((s = strchr(plugs, ';'))) *s = '\0';
    mysql_load_plugin(mysql, plugs, -1, 0);
    plugs = s + 1;
  } while (s);

  my_free(free_env);
}

/********** extern functions to be used by libmysql *********************/

/**
  Initializes the client plugin layer.

  This function must be called before any other client plugin function.

  @retval 0    successful
  @retval != 0 error occurred
*/
int mysql_client_plugin_init() {
  MYSQL mysql;
  struct st_mysql_client_plugin **builtin;

  if (initialized) return 0;

#ifdef HAVE_PSI_INTERFACE
  init_client_plugin_psi_keys();
#endif /* HAVE_PSI_INTERFACE */

  memset(&mysql, 0,
         sizeof(mysql)); /* dummy mysql for set_mysql_extended_error */

  mysql_mutex_init(key_mutex_LOCK_load_client_plugin, &LOCK_load_client_plugin,
                   MY_MUTEX_INIT_SLOW);
  init_alloc_root(key_memory_root, &mem_root, 128, 128);

  memset(&plugin_list, 0, sizeof(plugin_list));

  initialized = true;

  mysql_mutex_lock(&LOCK_load_client_plugin);

  for (builtin = mysql_client_builtins; *builtin; builtin++)
    add_plugin_noargs(&mysql, *builtin, nullptr, 0);

  mysql_mutex_unlock(&LOCK_load_client_plugin);

  load_env_plugins(&mysql);

  mysql_close_free(&mysql);

  return 0;
}

/**
  Deinitializes the client plugin layer.

  Unloades all client plugins and frees any associated resources.
*/
void mysql_client_plugin_deinit() {
  int i;
  struct st_client_plugin_int *p;

  if (!initialized) return;

  for (i = 0; i < MYSQL_CLIENT_MAX_PLUGINS; i++)
    for (p = plugin_list[i]; p; p = p->next) {
      if (p->plugin->deinit) p->plugin->deinit();
      if (p->dlhandle) dlclose(p->dlhandle);
    }

  memset(&plugin_list, 0, sizeof(plugin_list));
  initialized = false;
  free_root(&mem_root, MYF(0));
  mysql_mutex_destroy(&LOCK_load_client_plugin);
}

/************* public facing functions, for client consumption *********/

/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin *mysql_client_register_plugin(
    MYSQL *mysql, struct st_mysql_client_plugin *plugin) {
  if (is_not_initialized(mysql, plugin->name)) return nullptr;

  mysql_mutex_lock(&LOCK_load_client_plugin);

  /* make sure the plugin wasn't loaded meanwhile */
  if (find_plugin(plugin->name, plugin->type)) {
    set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD,
                             unknown_sqlstate,
                             ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD),
                             plugin->name, "it is already loaded");
    plugin = nullptr;
  } else
    plugin = add_plugin_noargs(mysql, plugin, nullptr, 0);

  mysql_mutex_unlock(&LOCK_load_client_plugin);
  return plugin;
}

/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin *mysql_load_plugin_v(MYSQL *mysql,
                                                   const char *name, int type,
                                                   int argc, va_list args) {
  const char *errmsg;
  char dlpath[FN_REFLEN + 1];
  void *sym, *dlhandle;
  struct st_mysql_client_plugin *plugin;
  const char *plugindir;
  const CHARSET_INFO *cs = nullptr;
  size_t len = (name ? strlen(name) : 0);
  int well_formed_error;
  size_t res = 0;
#ifdef _WIN32
  char win_errormsg[2048];
#endif

  DBUG_TRACE;
  DBUG_PRINT("entry", ("name=%s type=%d int argc=%d", name, type, argc));
  if (is_not_initialized(mysql, name)) {
    DBUG_PRINT("leave", ("mysql not initialized"));
    return nullptr;
  }

  mysql_mutex_lock(&LOCK_load_client_plugin);

  /* make sure the plugin wasn't loaded meanwhile */
  if (type >= 0 && find_plugin(name, type)) {
    errmsg = "it is already loaded";
    goto err;
  }

  if (mysql->options.extension && mysql->options.extension->plugin_dir) {
    plugindir = mysql->options.extension->plugin_dir;
  } else {
    plugindir = getenv("LIBMYSQL_PLUGIN_DIR");
    if (!plugindir) {
      plugindir = PLUGINDIR;
    }
  }
  if (mysql && mysql->charset)
    cs = mysql->charset;
  else
    cs = &my_charset_utf8mb4_bin;
  /* check if plugin name does not have any directory separator character */
  if ((my_strcspn(cs, name, name + len, FN_DIRSEP, strlen(FN_DIRSEP))) < len) {
    errmsg = "No paths allowed for shared library";
    goto err;
  }
  /* check if plugin name does not exceed its maximum length */
  res = cs->cset->well_formed_len(cs, name, name + len, NAME_CHAR_LEN,
                                  &well_formed_error);

  if (well_formed_error || len != res) {
    errmsg = "Invalid plugin name";
    goto err;
  }
  /*
    check if length of(plugin_dir + plugin name) does not exceed its maximum
    length
  */
  if ((strlen(plugindir) + len + 1) >= FN_REFLEN) {
    errmsg = "Invalid path";
    goto err;
  }

  /* Compile dll path */
  strxnmov(dlpath, sizeof(dlpath) - 1, plugindir, "/", name, SO_EXT, NullS);

  DBUG_PRINT("info", ("dlopeninig %s", dlpath));
  /* Open new dll handle */
  if (!(dlhandle = dlopen(dlpath, RTLD_NOW))) {
#if defined(__APPLE__)
    /* Apple supports plugins with .so also, so try this as well */
    strxnmov(dlpath, sizeof(dlpath) - 1, plugindir, "/", name, ".so", NullS);
    if ((dlhandle = dlopen(dlpath, RTLD_NOW))) goto have_plugin;
#endif

#ifdef _WIN32
    /* There should be no win32 calls between failed dlopen() and GetLastError()
     */
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0,
                      win_errormsg, 2048, NULL))
      errmsg = win_errormsg;
    else
      errmsg = "";
#else
    errmsg = dlerror();
#endif
    DBUG_PRINT("info", ("failed to dlopen"));
    goto err;
  }

#if defined(__APPLE__)
have_plugin:
#endif
  if (!(sym = dlsym(dlhandle, plugin_declarations_sym))) {
    errmsg = "not a plugin";
    dlclose(dlhandle);
    goto err;
  }

  plugin = (struct st_mysql_client_plugin *)sym;

  if (type >= 0 && type != plugin->type) {
    errmsg = "type mismatch";
    goto err;
  }

  if (strcmp(name, plugin->name)) {
    errmsg = "name mismatch";
    goto err;
  }

  if (type < 0 && find_plugin(name, plugin->type)) {
    errmsg = "it is already loaded";
    goto err;
  }

  plugin = add_plugin_withargs(mysql, plugin, dlhandle, argc, args);

  mysql_mutex_unlock(&LOCK_load_client_plugin);

  DBUG_PRINT("leave", ("plugin loaded ok"));
  return plugin;

err:
  mysql_mutex_unlock(&LOCK_load_client_plugin);
  DBUG_PRINT("leave", ("plugin load error : %s", errmsg));
  set_mysql_extended_error(mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, unknown_sqlstate,
                           ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD), name, errmsg);
  return nullptr;
}

/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin *mysql_load_plugin(MYSQL *mysql, const char *name,
                                                 int type, int argc, ...) {
  struct st_mysql_client_plugin *p;
  va_list args;
  va_start(args, argc);
  p = mysql_load_plugin_v(mysql, name, type, argc, args);
  va_end(args);
  return p;
}

/* see <mysql/client_plugin.h> for a full description */
struct st_mysql_client_plugin *mysql_client_find_plugin(MYSQL *mysql,
                                                        const char *name,
                                                        int type) {
  struct st_mysql_client_plugin *p;

  DBUG_TRACE;
  DBUG_PRINT("entry", ("name=%s, type=%d", name, type));
  if (is_not_initialized(mysql, name)) return nullptr;

  if (type < 0 || type >= MYSQL_CLIENT_MAX_PLUGINS) {
    set_mysql_extended_error(
        mysql, CR_AUTH_PLUGIN_CANNOT_LOAD, unknown_sqlstate,
        ER_CLIENT(CR_AUTH_PLUGIN_CANNOT_LOAD), name, "invalid type");
  }

  if ((p = find_plugin(name, type))) {
    DBUG_PRINT("leave", ("found %p", p));
    return p;
  }

  /* not found, load it */
  p = mysql_load_plugin(mysql, name, type, 0);
  DBUG_PRINT("leave", ("loaded %p", p));
  return p;
}

/* see <mysql/client_plugin.h> for a full description */
int mysql_plugin_options(struct st_mysql_client_plugin *plugin,
                         const char *option, const void *value) {
  DBUG_TRACE;
  /* does the plugin support options call? */
  if (!plugin || !plugin->options) return 1;
  return plugin->options(option, value);
}
