/*  Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*/

#include "mysql/components/services/clone_protocol_service.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/log_builtins.h"

#include "my_byteorder.h"
#include "mysql.h"
#include "sql/mysqld.h"
#include "sql/protocol_classic.h"
#include "sql/set_var.h"
#include "sql/sql_class.h"
#include "sql/sql_show.h"
#include "sql/sql_thd_internal_api.h"
#include "sql/ssl_acceptor_context.h"
#include "sql/sys_vars_shared.h"
#include "sql_common.h"

#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dictionary.h"

DEFINE_METHOD(void, mysql_clone_start_statement,
              (THD * &thd, PSI_thread_key thread_key,
               PSI_statement_key statement_key)) {
#ifdef HAVE_PSI_THREAD_INTERFACE
  bool thd_created = false;
#endif

  if (thd == nullptr) {
    /* Initialize Session */
    my_thread_init();

    /* Create thread with input key for PFS */
    thd = create_thd(true, true, true, thread_key);
#ifdef HAVE_PSI_THREAD_INTERFACE
    thd_created = true;
#endif
  }

#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Create and set PFS thread key */
  if (thread_key != PSI_NOT_INSTRUMENTED) {
    DBUG_ASSERT(thd_created);
    if (thd_created) {
      PSI_THREAD_CALL(set_thread)(thd->get_psi());
    }
  }
#endif
  /* Create and set PFS statement key */
  if (statement_key != PSI_NOT_INSTRUMENTED) {
    if (thd->m_statement_psi == nullptr) {
      thd->m_statement_psi = MYSQL_START_STATEMENT(
          &thd->m_statement_state, statement_key, thd->db().str,
          thd->db().length, thd->charset(), nullptr);
    } else {
      thd->m_statement_psi =
          MYSQL_REFINE_STATEMENT(thd->m_statement_psi, statement_key);
    }
  }
}

DEFINE_METHOD(void, mysql_clone_finish_statement, (THD * thd)) {
  DBUG_ASSERT(thd->m_statement_psi == nullptr);

  my_thread_end();
  thd->set_psi(nullptr);
  destroy_thd(thd);
}

template <typename T>
using DD_Objs = std::vector<const T *>;

using Releaser = dd::cache::Dictionary_client::Auto_releaser;

DEFINE_METHOD(int, mysql_clone_get_charsets,
              (THD * thd, Mysql_Clone_Values &char_sets)) {
  auto dc = dd::get_dd_client(thd);
  Releaser releaser(dc);

  /* Character set with collation */
  DD_Objs<dd::Collation> dd_charsets;

  if (dc->fetch_global_components(&dd_charsets)) {
    my_error(ER_INTERNAL_ERROR, MYF(0),
             "Clone failed to get all character sets from DD");
    return (ER_INTERNAL_ERROR);
  }

  for (auto dd_charset : dd_charsets) {
    std::string charset;
    charset.assign(dd_charset->name().c_str());
    char_sets.push_back(charset);
  }
  return (0);
}

DEFINE_METHOD(int, mysql_clone_validate_charsets,
              (THD * thd, Mysql_Clone_Values &char_sets)) {
  if (thd == nullptr) {
    return (0);
  }
  for (auto &char_set : char_sets) {
    auto charset_obj = get_charset_by_name(char_set.c_str(), MYF(0));

    /* Check if character set collation is available. */
    if (charset_obj == nullptr) {
      my_error(ER_CLONE_CHARSET, MYF(0), char_set.c_str());
      return (ER_CLONE_CHARSET);
    }
  }
  return (0);
}

/**
  Get configuration parameter value in utf8
  @param[in]   thd   server session THD
  @param[in]   config_name  parameter name
  @param[out]  utf8_val     parameter value in utf8 string
  @return error code.
*/
static int get_utf8_config(THD *thd, std::string config_name,
                           String &utf8_val) {
  char val_buf[1024];
  SHOW_VAR show;
  show.type = SHOW_SYS;

  /* Get system configuration parameter. */
  mysql_rwlock_rdlock(&LOCK_system_variables_hash);
  auto var = intern_find_sys_var(config_name.c_str(), config_name.length());
  mysql_rwlock_unlock(&LOCK_system_variables_hash);

  if (var == nullptr) {
    my_error(ER_INTERNAL_ERROR, MYF(0),
             "Clone failed to get system configuration parameter.");
    return (ER_INTERNAL_ERROR);
  }

  show.value = reinterpret_cast<char *>(var);
  show.name = var->name.str;

  mysql_mutex_lock(&LOCK_global_system_variables);
  size_t val_length;
  const CHARSET_INFO *fromcs;

  auto value = get_one_variable(thd, &show, OPT_GLOBAL, SHOW_SYS, nullptr,
                                &fromcs, val_buf, &val_length);

  mysql_mutex_unlock(&LOCK_global_system_variables);

  uint dummy_err;
  const CHARSET_INFO *tocs = &my_charset_utf8mb4_bin;
  utf8_val.copy(value, val_length, fromcs, tocs, &dummy_err);
  return (0);
}

DEFINE_METHOD(int, mysql_clone_get_configs,
              (THD * thd, Mysql_Clone_Key_Values &configs)) {
  int err = 0;

  for (auto &key_val : configs) {
    String utf8_str;
    auto &config_name = key_val.first;
    err = get_utf8_config(thd, config_name, utf8_str);

    if (err != 0) {
      break;
    }

    auto &config_val = key_val.second;
    config_val.assign(utf8_str.c_ptr_quick());
  }
  return (err);
}

DEFINE_METHOD(int, mysql_clone_validate_configs,
              (THD * thd, Mysql_Clone_Key_Values &configs)) {
  int err = 0;

  for (auto &key_val : configs) {
    String utf8_str;
    auto &config_name = key_val.first;
    err = get_utf8_config(thd, config_name, utf8_str);
    if (err != 0) {
      break;
    }

    auto &donor_val = key_val.second;
    std::string config_val;
    config_val.assign(utf8_str.c_ptr_quick());

    /* Check if the parameter value matches. */
    if (config_val == donor_val) {
      continue;
    }

    /* Throw specific error for some configurations. */
    if (config_name.compare("version_compile_os") == 0) {
      err = ER_CLONE_OS;
    } else if (config_name.compare("version") == 0) {
      err = ER_CLONE_DONOR_VERSION;
    } else if (config_name.compare("version_compile_machine") == 0) {
      err = ER_CLONE_PLATFORM;
    }

    if (err != 0) {
      my_error(err, MYF(0), donor_val.c_str(), config_val.c_str());
    } else {
      err = ER_CLONE_CONFIG;
      my_error(ER_CLONE_CONFIG, MYF(0), config_name.c_str(), donor_val.c_str(),
               config_val.c_str());
    }
    break;
  }
  return (err);
}

DEFINE_METHOD(MYSQL *, mysql_clone_connect,
              (THD * thd, const char *host, uint32_t port, const char *user,
               const char *passwd, mysql_clone_ssl_context *ssl_ctx,
               MYSQL_SOCKET *socket)) {
  DBUG_TRACE;

  /* Set default as 5 seconds */
  timeout_t net_read_timeout = timeout_from_seconds(5);
  timeout_t net_write_timeout = timeout_from_seconds(5);

  /* Clean any previous Error and Warnings in THD */
  if (thd != nullptr) {
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    net_read_timeout = timeout_from_seconds(thd->variables.net_read_timeout);
    net_write_timeout = timeout_from_seconds(thd->variables.net_write_timeout);
  }

  MYSQL *mysql;
  MYSQL *ret_mysql;

  /* Connect using classic protocol */
  mysql = mysql_init(nullptr);

  auto client_ssl_mode = static_cast<enum mysql_ssl_mode>(ssl_ctx->m_ssl_mode);

  /* Get server public key for RSA key pair-based password exchange.*/
  bool get_key = true;
  mysql_options(mysql, MYSQL_OPT_GET_SERVER_PUBLIC_KEY, &get_key);

  if (client_ssl_mode != SSL_MODE_DISABLED) {
    /* Verify server's certificate */
    if (ssl_ctx->m_ssl_ca != nullptr) {
      client_ssl_mode = SSL_MODE_VERIFY_CA;
    }

    OptionalString capath, cipher, ciphersuites, crl, crlpath, version;

    SslAcceptorContext::read_parameters(nullptr, &capath, &version, nullptr,
                                        &cipher, &ciphersuites, nullptr, &crl,
                                        &crlpath);

    mysql_ssl_set(mysql, ssl_ctx->m_ssl_key, ssl_ctx->m_ssl_cert,
                  ssl_ctx->m_ssl_ca, capath.c_str(), cipher.c_str());

    mysql_options(mysql, MYSQL_OPT_SSL_CRL, crl.c_str());
    mysql_options(mysql, MYSQL_OPT_SSL_CRLPATH, crlpath.c_str());
    mysql_options(mysql, MYSQL_OPT_TLS_VERSION, version.c_str());
    mysql_options(mysql, MYSQL_OPT_TLS_CIPHERSUITES, ciphersuites.c_str());
  }

  mysql_options(mysql, MYSQL_OPT_SSL_MODE, &client_ssl_mode);

  auto timeout = static_cast<uint>(connect_timeout);
  mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                reinterpret_cast<char *>(&timeout));

  /* Enable compression. */
  if (ssl_ctx->m_enable_compression) {
    mysql_options(mysql, MYSQL_OPT_COMPRESS, nullptr);
    mysql_extension_set_server_extn(mysql, ssl_ctx->m_server_extn);
  }

  ret_mysql =
      mysql_real_connect(mysql, host, user, passwd, nullptr, port, nullptr, 0);

  if (ret_mysql == nullptr) {
    char err_buf[MYSYS_ERRMSG_SIZE + 64];
    snprintf(err_buf, sizeof(err_buf), "Connect failed: %u : %s",
             mysql_errno(mysql), mysql_error(mysql));

    my_error(ER_CLONE_DONOR, MYF(0), err_buf);
    LogErr(INFORMATION_LEVEL, ER_CLONE_CLIENT_TRACE, err_buf);

    mysql_close(mysql);
    return nullptr;
  }

  NET *net = &mysql->net;
  Vio *vio = net->vio;

  *socket = vio->mysql_socket;

  net_clear_error(net);
  net_clear(net, true);

  /* Set network read/write timeout */
  my_net_set_read_timeout(net, net_read_timeout);
  my_net_set_write_timeout(net, net_write_timeout);

  if (thd != nullptr) {
    /* Set current active vio so that shutdown and KILL
       signals can wake up current thread. */
    thd->set_clone_vio(net->vio);
  }

  /* Load clone plugin in remote */
  auto result = simple_command(mysql, COM_CLONE, nullptr, 0, 0);

  if (result) {
    if (thd != nullptr) {
      thd->clear_clone_vio();
    }
    char err_buf[MYSYS_ERRMSG_SIZE + 64];
    snprintf(err_buf, sizeof(err_buf), "%d : %s", net->last_errno,
             net->last_error);

    my_error(ER_CLONE_DONOR, MYF(0), err_buf);

    snprintf(err_buf, sizeof(err_buf), "COM_CLONE failed: %d : %s",
             net->last_errno, net->last_error);
    LogErr(INFORMATION_LEVEL, ER_CLONE_CLIENT_TRACE, err_buf);

    mysql_close(mysql);
    mysql = nullptr;
  }
  return mysql;
}

DEFINE_METHOD(int, mysql_clone_send_command,
              (THD * thd, MYSQL *connection, bool set_active, uchar command,
               uchar *com_buffer, size_t buffer_length)) {
  DBUG_TRACE;
  NET *net = &connection->net;

  if (net->last_errno != 0) {
    return static_cast<int>(net->last_errno);
  }

  net_clear_error(net);
  net_clear(net, true);

  if (set_active && thd->killed != THD::NOT_KILLED) {
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    return ER_QUERY_INTERRUPTED;
  }

  auto result =
      net_write_command(net, command, nullptr, 0, com_buffer, buffer_length);
  if (!result) {
    return 0;
  }

  int err = static_cast<int>(net->last_errno);

  /* Check if query is interrupted */
  if (set_active && thd->killed != THD::NOT_KILLED) {
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    err = ER_QUERY_INTERRUPTED;
  }

  DBUG_ASSERT(err != 0);
  return err;
}

DEFINE_METHOD(int, mysql_clone_get_response,
              (THD * thd, MYSQL *connection, bool set_active, uint32_t timeout,
               uchar **packet, size_t *length, size_t *net_length)) {
  DBUG_TRACE;
  NET *net = &connection->net;

  if (net->last_errno != 0) {
    return static_cast<int>(net->last_errno);
  }

  if (set_active && thd->killed != THD::NOT_KILLED) {
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    return ER_QUERY_INTERRUPTED;
  }

  net_new_transaction(net);

  /* Adjust read timeout if specified. */
  if (timeout != 0) {
    my_net_set_read_timeout(net, timeout_from_seconds(timeout));
  }

  /* Dummy function callback invoked before getting header. */
  auto func_before = [](NET *, void *, size_t) {};

  /* Callback function called after receiving header. */
  auto func_after = [](NET *net_arg, void *ctx, size_t, bool) {
    auto net_bytes = static_cast<size_t *>(ctx);
    *net_bytes +=
        static_cast<size_t>(uint3korr(net_arg->buff + net_arg->where_b));
  };

  /* Use server extension callback to capture network byte information. */
  NET_SERVER server_extn;
  server_extn.m_user_data = static_cast<void *>(net_length);
  server_extn.m_before_header = func_before;
  server_extn.m_after_header = func_after;
  auto saved_extn = net->extension;
  if (saved_extn != nullptr && net->compress)
    server_extn.compress_ctx =
        (static_cast<NET_SERVER *>(saved_extn))->compress_ctx;
  else
    server_extn.compress_ctx.algorithm = MYSQL_UNCOMPRESSED;
  net->extension = &server_extn;

  *net_length = 0;
  *length = my_net_read(net);

  net->extension = saved_extn;
  server_extn.compress_ctx.algorithm = MYSQL_UNCOMPRESSED;

  /* Reset timeout back to default value. */
  my_net_set_read_timeout(
      net, timeout_from_seconds(thd->variables.net_read_timeout));

  *packet = net->read_pos;

  if (*length != packet_error && *length != 0) {
    return 0;
  }

  int err = static_cast<int>(net->last_errno);
  /* Check if query is interrupted */
  if (set_active && thd->killed != THD::NOT_KILLED) {
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    err = ER_QUERY_INTERRUPTED;
  }

  if (err == 0) {
    net->last_errno = ER_NET_PACKETS_OUT_OF_ORDER;
    err = ER_NET_PACKETS_OUT_OF_ORDER;
    my_error(err, MYF(0));
  }
  return err;
}

DEFINE_METHOD(int, mysql_clone_kill,
              (MYSQL * connection, MYSQL *kill_connection)) {
  DBUG_TRACE;

  auto kill_conn_id = kill_connection->thread_id;

  char kill_buffer[64];
  snprintf(kill_buffer, 64, "KILL CONNECTION %lu", kill_conn_id);

  auto err = mysql_real_query(connection, kill_buffer,
                              static_cast<ulong>(strlen(kill_buffer)));

  return err;
}

DEFINE_METHOD(void, mysql_clone_disconnect,
              (THD * thd, MYSQL *mysql, bool is_fatal, bool clear_error)) {
  DBUG_TRACE;

  if (thd != nullptr) {
    thd->clear_clone_vio();

    /* clear any session error, if requested */
    if (clear_error) {
      thd->clear_error();
      thd->get_stmt_da()->reset_condition_info(thd);
    }
  }

  /* Make sure that the other end has switched back from clone protocol. */
  if (!is_fatal) {
    is_fatal = simple_command(mysql, COM_RESET_CONNECTION, nullptr, 0, 0);
  }

  if (is_fatal) {
    end_server(mysql);
  }

  /* Disconnect */
  mysql_close(mysql);
}

DEFINE_METHOD(void, mysql_clone_get_error,
              (THD * thd, uint32_t *err_num, const char **err_mesg)) {
  DBUG_TRACE;
  *err_num = 0;
  *err_mesg = nullptr;
  /* Check if THD exists. */
  if (thd == nullptr) {
    return;
  }
  /* Check if DA exists. */
  auto da = thd->get_stmt_da();
  if (da == nullptr || !da->is_error()) {
    return;
  }
  /* Get error from DA. */
  *err_num = da->mysql_errno();
  *err_mesg = da->message_text();
}

DEFINE_METHOD(int, mysql_clone_get_command,
              (THD * thd, uchar *command, uchar **com_buffer,
               size_t *buffer_length)) {
  DBUG_TRACE;

  NET *net = thd->get_protocol_classic()->get_net();

  if (net->last_errno != 0) {
    return static_cast<int>(net->last_errno);
  }

  /* flush any data in write buffer */
  if (!net_flush(net)) {
    net_new_transaction(net);

    /* Use idle timeout while waiting for commands */
    my_net_set_read_timeout(
        net, timeout_from_seconds(thd->variables.net_wait_timeout));

    *buffer_length = my_net_read(net);

    my_net_set_read_timeout(
        net, timeout_from_seconds(thd->variables.net_read_timeout));

    if (*buffer_length != packet_error && *buffer_length != 0) {
      *com_buffer = net->read_pos;
      *command = **com_buffer;

      ++(*com_buffer);
      --(*buffer_length);

      return 0;
    }
  }

  int err = static_cast<int>(net->last_errno);

  if (err == 0) {
    net->last_errno = ER_NET_PACKETS_OUT_OF_ORDER;
    err = ER_NET_PACKETS_OUT_OF_ORDER;
    my_error(err, MYF(0));
  }
  return err;
}

DEFINE_METHOD(int, mysql_clone_send_response,
              (THD * thd, bool secure, uchar *packet, size_t length)) {
  DBUG_TRACE;
  NET *net = thd->get_protocol_classic()->get_net();

  if (net->last_errno != 0) {
    return static_cast<int>(net->last_errno);
  }

  auto conn_type = thd->get_vio_type();

  if (secure && conn_type != VIO_TYPE_SSL) {
    my_error(ER_CLONE_ENCRYPTION, MYF(0));
    return ER_CLONE_ENCRYPTION;
  }

  net_clear(net, true);

  if (!my_net_write(net, packet, length) && !net_flush(net)) {
    return 0;
  }

  int err = static_cast<int>(net->last_errno);

  DBUG_ASSERT(err != 0);
  return err;
}

DEFINE_METHOD(int, mysql_clone_send_error,
              (THD * thd, uchar err_cmd, bool is_fatal)) {
  DBUG_TRACE;

  uchar err_packet[1 + 4 + MYSQL_ERRMSG_SIZE + 1];
  uchar *buf_ptr = &err_packet[0];
  size_t packet_length = 0;

  *buf_ptr = err_cmd;
  ++buf_ptr;
  ++packet_length;

  auto da = thd->get_stmt_da();

  char *bufp;

  if (da->is_error()) {
    int4store(buf_ptr, da->mysql_errno());
    buf_ptr += 4;
    packet_length += 4;

    bufp = reinterpret_cast<char *>(buf_ptr);
    packet_length +=
        snprintf(bufp, MYSQL_ERRMSG_SIZE, "%s", da->message_text());
    if (is_fatal) {
      mysql_mutex_lock(&thd->LOCK_thd_data);
      thd->shutdown_active_vio();
      mysql_mutex_unlock(&thd->LOCK_thd_data);

      return da->mysql_errno();
    }
  } else {
    int4store(buf_ptr, ER_INTERNAL_ERROR);
    buf_ptr += 4;
    packet_length += 4;

    bufp = reinterpret_cast<char *>(buf_ptr);
    packet_length += snprintf(bufp, MYSQL_ERRMSG_SIZE, "%s", "Unknown Error");
  }

  NET *net = thd->get_protocol_classic()->get_net();

  if (net->last_errno != 0) {
    return static_cast<int>(net->last_errno);
  }

  DBUG_ASSERT(!is_fatal);

  /* Clean error in THD */
  thd->clear_error();
  thd->get_stmt_da()->reset_condition_info(thd);
  net_clear(net, true);

  if (my_net_write(net, &err_packet[0], packet_length) || net_flush(net)) {
    int err = static_cast<int>(net->last_errno);

    if (err == 0) {
      net->last_errno = ER_NET_PACKETS_OUT_OF_ORDER;
      err = ER_NET_PACKETS_OUT_OF_ORDER;
      my_error(err, MYF(0));
    }
    return err;
  }
  return 0;
}
