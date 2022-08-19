/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <functional>
#include <vector>

#include "client/base/mysql_connection_options.h"
#include "client/client_priv.h"
#include "my_compiler.h"
#include "sslopt-vars.h"
#include "typelib.h"

using namespace Mysql::Tools::Base::Options;
using std::placeholders::_1;

void Mysql_connection_options::Ssl_options::create_options() {
  std::function<void(char *)> callback(std::bind(
      &Mysql_connection_options::Ssl_options::mode_option_callback, this, _1));

  this->create_new_option(&this->m_ssl_mode_string, "ssl-mode",
                          "SSL connection mode.")
      ->add_callback(new std::function<void(char *)>(std::bind(
          &Mysql_connection_options::Ssl_options::mode_option_callback, this,
          _1)));
  this->create_new_option(&::opt_ssl_ca, "ssl-ca", "CA file in PEM format.")
      ->add_callback(new std::function<void(char *)>(
          std::bind(&Mysql_connection_options::Ssl_options::ca_option_callback,
                    this, _1)));
  this->create_new_option(&::opt_ssl_capath, "ssl-capath", "CA directory.")
      ->add_callback(new std::function<void(char *)>(
          std::bind(&Mysql_connection_options::Ssl_options::ca_option_callback,
                    this, _1)));
  this->create_new_option(&::opt_ssl_cert, "ssl-cert",
                          "X509 cert in PEM format.");
  this->create_new_option(&::opt_ssl_cipher, "ssl-cipher",
                          "SSL cipher to use.");
  this->create_new_option(&::opt_tls_ciphersuites, "tls-ciphersuites",
                          "TLS v1.3 cipher to use.");
  this->create_new_option(&::opt_ssl_key, "ssl-key", "X509 key in PEM format.");
  this->create_new_option(&::opt_ssl_crl, "ssl-crl",
                          "Certificate revocation list.");
  this->create_new_option(&::opt_ssl_crlpath, "ssl-crlpath",
                          "Certificate revocation list path.");
  this->create_new_option(&::opt_tls_version, "tls-version",
                          "TLS version to use.");
  this->create_new_option(&this->m_ssl_fips_mode_string, "ssl-fips-mode",
                          "SSL fips mode to use.")
      ->add_callback(new std::function<void(char *)>(std::bind(
          &Mysql_connection_options::Ssl_options::fips_mode_option_callback,
          this, _1)));
}

void Mysql_connection_options::Ssl_options::ca_option_callback(
    char *argument MY_ATTRIBUTE((unused))) {
  if (!ssl_mode_set_explicitly) ::opt_ssl_mode = SSL_MODE_VERIFY_CA;
}

void Mysql_connection_options::Ssl_options::mode_option_callback(
    char *argument) {
  ::opt_ssl_mode = find_type_or_exit(argument, &ssl_mode_typelib, "ssl-mode");
  ssl_mode_set_explicitly = true;
}

void Mysql_connection_options::Ssl_options::fips_mode_option_callback(
    char *argument) {
  ::opt_ssl_fips_mode =
      find_type_or_exit(argument, &ssl_fips_mode_typelib, "ssl-fips-mode") - 1;
}

bool Mysql_connection_options::Ssl_options::apply_for_connection(
    MYSQL *connection) {
  if (SSL_SET_OPTIONS(connection)) {
    std::cerr << SSL_SET_OPTIONS_ERROR;
    return true;
  }
  return false;
}
