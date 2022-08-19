/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef AUTH_LDAP_SASL_CLIENT_H_
#define AUTH_LDAP_SASL_CLIENT_H_

#include "my_config.h"

#include "auth_ldap_sasl_mechanism.h"

#include <mysql.h>
#include <mysql/client_plugin.h>
#include <mysql/plugin.h>
#include <mysql/plugin_auth_common.h>
#include <sasl/sasl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_client.h"

#define SASL_MAX_STR_SIZE 1024
#define SASL_BUFFER_SIZE 9000
#define SASL_SERVICE_NAME "ldap"

static const sasl_callback_t callbacks[] = {
#ifdef SASL_CB_GETREALM
    {SASL_CB_GETREALM, nullptr, nullptr},
#endif
    {SASL_CB_USER, nullptr, nullptr},
    {SASL_CB_AUTHNAME, nullptr, nullptr},
    {SASL_CB_PASS, nullptr, nullptr},
    {SASL_CB_ECHOPROMPT, nullptr, nullptr},
    {SASL_CB_NOECHOPROMPT, nullptr, nullptr},
    {SASL_CB_LIST_END, nullptr, nullptr}};

/*
  MAX SSF - The maximum Security Strength Factor supported by the mechanism
  (roughly the number of bits of encryption provided, but may have other
  meanings, for example an SSF of 1 indicates integrity protection only, no
  encryption). SECURITY PROPERTIES are: NOPLAIN, NOACTIVE, NODICT, FORWARD,
  NOANON, CRED, MUTUAL. More details are in:
  https://www.sendmail.org/~ca/email/cyrus2/mechanisms.html
*/
sasl_security_properties_t security_properties = {
    /** Minimum acceptable final level. (min_ssf) */
    56,
    /** Maximum acceptable final level. (max_ssf) */
    0,
    /** Maximum security layer receive buffer size. */
    0,
    /** security flags (security_flags) */
    0,
    /** Property names. (property_names) */
    nullptr,
    /** Property values. (property_values)*/
    nullptr,
};

class Sasl_client {
 public:
  Sasl_client();
  ~Sasl_client();
  int initilize();
  void set_plugin_info(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql);
  void interact(sasl_interact_t *ilist);
  int read_method_name_from_server();
  int sasl_start(char **client_output, int *client_output_length);
  int sasl_step(char *server_in, int server_in_length, char **client_out,
                int *client_out_length);
  int send_sasl_request_to_server(const unsigned char *request, int request_len,
                                  unsigned char **reponse, int *response_len);
  void set_user_info(std::string name, std::string pwd);
  static void sasl_client_done_wrapper();
  std::string get_method();
#if defined(KERBEROS_LIB_CONFIGURED)
  void read_kerberos_user_name();
#endif

 protected:
  char m_user_name[SASL_MAX_STR_SIZE];
  char m_user_pwd[SASL_MAX_STR_SIZE];
  char m_mechanism[SASL_MAX_STR_SIZE];
  char m_service_name[SASL_MAX_STR_SIZE];
  std::string m_ldap_server_host;
  sasl_conn_t *m_connection;
  MYSQL_PLUGIN_VIO *m_vio;
  MYSQL *m_mysql;
  Sasl_mechanism *m_sasl_mechanism;
};

#endif  // AUTH_LDAP_SASL_CLIENT_H_
