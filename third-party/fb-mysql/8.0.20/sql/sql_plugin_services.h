/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/* support for Services */
#include "mysql/services.h"
#include "service_versions.h"

/**
  @page page_ext_plugin_svc_new_service_howto How to add a new service

  A "plugin service" is in its core a C struct containing one or more function
  pointers.

  If you want to export C++ class you need to provide an
  extern "C" function that will create a new instance of your class,
  and put it in a service. But be careful to also provide a destructor
  method since the heaps of the server and the plugin may be different.

  Data structures are not part of the service structure, but they are part
  of the API you create and usually need to be declared in the same
  service_*.h file.

  To turn a **pre-existing** set of functions (foo_func1, foo_func2)
  into a service "foo" you need to:

  <ol>
  <li>
  Create a new file include/mysql/service_foo.h

  The template is:

  @include service_foo.h

  The service_foo.h file should be self-contained, if it needs system headers -
  include them in it, e.g. if you use `size_t`

  ~~~~
  #include <stdlib.h>
  ~~~~

  It should also declare all the accompanying data structures, as necessary
  (e.g. ::thd_alloc_service declares ::MYSQL_LEX_STRING).
</li><li>
  Add the new file to include/mysql/services.h
</li><li>
  Increase the minor plugin ABI version in include/mysql/plugin.h:
     ::MYSQL_PLUGIN_INTERFACE_VERSION = ::MYSQL_PLUGIN_INTERFACE_VERSION + 1
</li><li>
  Add the version of your service to include/service_versions.h:
  ~~~~
      #define VERSION_foo 0x0100
  ~~~~
</li><li>
  Create a new file `libservices/foo_service.c` using the following template:
  @include service_foo.cc
</li><li>
  Add the new file to libservices/CMakeLists.txt (MYSQLSERVICES_SOURCES)
</li><li>
  And finally, register your service for dynamic linking in
      sql/sql_plugin_services.h
  <ul><li>
  Fill in the service structure:
  ~~~
    static struct foo_service_st foo_handler = {
      foo_func1,
      foo_func2
    }
  ~~~
  </li><li>
  Add it to the ::list_of_services

  ~~~
      { "foo_service", VERSION_foo, &foo_handler }
  ~~~
  </li></ul></li></ol>
*/

/**
  @page page_ext_plugin_api_goodservices What defines a "good" plugin service ?

  The following is an attempt to explain what is a good plugin service.
  It is also an attempt to mention some bad practices that should be
  avoided. The text is in no way conclusive and is a constant work in progress
  to keep it updated.


  Avoid exposing the definitions of complex server structure
  ----------------------------------------------------------

  @ref page_ext_plugin_svc_new_service_howto states that the service header
  should be self-contained, i.e. all data structures used by the API need to be
  defined by the service header.

  The service headers are considered "public" and are packaged into the binary
  packages for users of these binary packages to use when developing plugins
  that are using the service.

  When you combine the two together it becomes evident that complex server data
  structures should definitely be avoided as parts of the API.

  The main inconvenience is that a complex structure's definition should be
  copied into the service's header. And if that is a frequently changed
  structure all plugins using it will need to be recompiled every time it
  changes.

  That is the reason why it is always better to pass individual members of
  these structures that are simpler. Even if this means passing more than just
  a single argument.

  If you absolutely must pass references to complex structure instances it is
  better to do it as a "handle", i.e. a `void *`. For convenience, you can have
  a named class for that, e.g.
  ~~~~
  typedef void *my_new_handle_t;
  ~~~~

  This isolates the layout of the server structure from the plugin code while
  still allowing the plugins to operate on it via accessor and mutator methods.

  Strive to make reusable services
  --------------------------------

  Services have some extra processing associated with them. So they should be
  used only for functionality that more than one plugin will probably use.

  How do we know if a service is reusable ?

  If a service deals with complex structures related to a specific plugin API
  or does not provide a logically complete set of operations chances are that
  there will not be much re-use of this service.

  Keep the identifier namespace clean
  -----------------------------------

  As discussed in @ref page_ext_plugin_svc_anathomy each function of each
  service API is added to the global C/C++ namespace for all plugins via a set
  of preprocessor defines.

  So if you name your plugin service functions after single common words, e.g.
  `get` or `put` etc you will strongly pollute the namespace that the plugin
  authors will use. They will either have to refrain from using such methods or
  go to the extra trouble of using either preprocessor or C++ namespace tricks.

  Thus, prefix your APIs. We typically use the `my_` prefix for all MySQL
  exported functions (note that the `mysql_` prefix is reserved for the C API
  functions, e.g. ::mysql_real_connect()). We also add a subsystem prefix, e.g.
  ::my_plugin_log_message() is the service API name for plugins to log messages
  to the server's error log.

  Use services for "published" APIs only
  --------------------------------------

  Plugin services are an element of the binary APIs that must be kept stable
  to keep existing plugins operating.
  Plugin service APIs are thus versioned to allow the server to check if the
  plugins are using the right version of the particular service API.
  The server's build scripts also package the service API headers in a
  designated directory and are documented in a specific service API document.

  This stability comes at a price. All of the above should be observed and the
  versions should be increased accoringly.

  Major should be bumped (and the minor reset) when there are incompatible
  changes: signature change, meaning change etc.

  Minor should be bumped when there are backward compatible changes: adding
  new functions at the end of the API, adding new named constants to be used
  by the API etc.

  Use services to make plugins "clean"
  ------------------------------------

  A clean plugin is one that does not need to reference the symbols of the
  server binary that loads it. Such plugin can be loaded by any binary and
  it is guaranteed to operate in a predictable way as long as the right
  plugin APIs are called and the relevant plugin service APIs are defined
  by the loading binary.

  Always make fully-formed plugin service APIs
  --------------------------------------------

  Follow strictly and completely the steps defined in
  @ref page_ext_plugin_svc_new_service_howto.

  Don't skip steps or you'll be decresing the usability of the service API.
*/

/**
  @defgroup group_ext_plugin_services MySQL Server Plugin Services

  This is a group of all plugin service APIs.

  See @ref page_ext_plugin_services for more details.
*/

/**
  A server-side reference to a plugin service.

  @sa plugin_add, list_of_services
*/
struct st_service_ref {
  /** The name of the service pointer symbol exported by the plugin */
  const char *name;
  /** The service version provided by the server */
  uint version;
  /** The actual server side service structure pointer */
  void *service;
};

static struct srv_session_service_st srv_session_service_handler = {
    srv_session_init_thread, srv_session_deinit_thread,
    srv_session_open,        srv_session_detach,
    srv_session_close,       srv_session_server_is_available,
    srv_session_attach};

static struct command_service_st command_handler = {
    command_service_run_command,
};

static struct srv_session_info_service_st srv_session_info_handler = {
    srv_session_info_get_thd,         srv_session_info_get_session_id,
    srv_session_info_get_current_db,  srv_session_info_get_client_port,
    srv_session_info_set_client_port, srv_session_info_set_connection_type,
    srv_session_info_killed,          srv_session_info_session_count,
    srv_session_info_thread_count};

static struct thd_alloc_service_st thd_alloc_handler = {
    thd_alloc,   thd_calloc, thd_strdup,
    thd_strmake, thd_memdup, thd_make_lex_string};

static struct thd_wait_service_st thd_wait_handler = {thd_wait_begin,
                                                      thd_wait_end};

static struct my_thread_scheduler_service my_thread_scheduler_handler = {
    my_connection_handler_set, my_connection_handler_reset};

static struct my_plugin_log_service my_plugin_log_handler = {
    my_plugin_log_message};

static struct mysql_string_service_st mysql_string_handler = {
    mysql_string_convert_to_char_ptr, mysql_string_get_iterator,
    mysql_string_iterator_next,       mysql_string_iterator_isupper,
    mysql_string_iterator_islower,    mysql_string_iterator_isdigit,
    mysql_string_to_lowercase,        mysql_string_free,
    mysql_string_iterator_free,
};

static struct mysql_malloc_service_st mysql_malloc_handler = {
    my_malloc, my_realloc, my_claim, my_free, my_memdup, my_strdup, my_strndup};

static struct mysql_password_policy_service_st mysql_password_policy_handler = {
    my_validate_password_policy, my_calculate_password_strength};

static struct mysql_parser_service_st parser_handler = {
    mysql_parser_current_session,      mysql_parser_open_session,
    mysql_parser_start_thread,         mysql_parser_join_thread,
    mysql_parser_set_current_database, mysql_parser_parse,
    mysql_parser_get_statement_type,   mysql_parser_get_statement_digest,
    mysql_parser_get_number_params,    mysql_parser_extract_prepared_params,
    mysql_parser_visit_tree,           mysql_parser_item_string,
    mysql_parser_free_string,          mysql_parser_get_query,
    mysql_parser_get_normalized_query};

static struct rpl_transaction_ctx_service_st rpl_transaction_ctx_handler = {
    set_transaction_ctx,
};

static struct transaction_write_set_service_st transaction_write_set_handler = {
    get_transaction_write_set,
};

static struct mysql_locking_service_st locking_service_handler = {
    mysql_acquire_locking_service_locks_nsec,
    mysql_release_locking_service_locks};

static struct security_context_service_st security_context_handler = {
    thd_get_security_context,    thd_set_security_context,
    security_context_create,     security_context_destroy,
    security_context_copy,       security_context_lookup,
    security_context_get_option, security_context_set_option};

static struct mysql_keyring_service_st mysql_keyring_handler = {
    my_key_store, my_key_fetch, my_key_remove, my_key_generate};

static struct plugin_registry_service_st plugin_registry_handler = {
    mysql_plugin_registry_acquire, mysql_plugin_registry_release};

static struct st_service_ref list_of_services[] = {
    {"srv_session_service", VERSION_srv_session_service,
     &srv_session_service_handler},
    {"command_service", VERSION_command, &command_handler},
    {"srv_session_info_service", VERSION_srv_session_info_service,
     &srv_session_info_handler},
    {"thd_alloc_service", VERSION_thd_alloc, &thd_alloc_handler},
    {"thd_wait_service", VERSION_thd_wait, &thd_wait_handler},
    {"my_thread_scheduler_service", VERSION_my_thread_scheduler,
     &my_thread_scheduler_handler},
    {"my_plugin_log_service", VERSION_my_plugin_log, &my_plugin_log_handler},
    {"mysql_string_service", VERSION_mysql_string, &mysql_string_handler},
    {"mysql_malloc_service", VERSION_mysql_malloc, &mysql_malloc_handler},
    {"mysql_password_policy_service", VERSION_mysql_password_policy,
     &mysql_password_policy_handler},
    {"mysql_parser_service", VERSION_parser, &parser_handler},
    {"rpl_transaction_ctx_service", VERSION_rpl_transaction_ctx_service,
     &rpl_transaction_ctx_handler},
    {"transaction_write_set_service", VERSION_transaction_write_set_service,
     &transaction_write_set_handler},
    {"security_context_service", VERSION_security_context_service,
     &security_context_handler},
    {"mysql_locking_service", VERSION_locking_service,
     &locking_service_handler},
    {"mysql_keyring_service", VERSION_mysql_keyring_service,
     &mysql_keyring_handler},
    {"plugin_registry_service", VERSION_plugin_registry_service,
     &plugin_registry_handler},
};
