/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef AUTH_LDAP_SASL_MECHANISM_H_
#define AUTH_LDAP_SASL_MECHANISM_H_

#include "my_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#if defined(KERBEROS_LIB_CONFIGURED)
#include "auth_ldap_kerberos.h"
#endif
#include "log_client.h"

const char SASL_GSSAPI[] = "GSSAPI";

class Sasl_mechanism {
 public:
  Sasl_mechanism();
  virtual ~Sasl_mechanism();
  bool virtual pre_authentication();
  void virtual get_ldap_host(std::string &host);
  void set_user_info(std::string user, std::string password);

 protected:
  std::string m_user;
  std::string m_password;
};

#if defined(KERBEROS_LIB_CONFIGURED)
class Sasl_mechanism_kerberos : public Sasl_mechanism {
 public:
  Sasl_mechanism_kerberos();
  ~Sasl_mechanism_kerberos() override;
  bool virtual pre_authentication() override;
  void virtual get_ldap_host(std::string &host) override;

 private:
  std::unique_ptr<auth_ldap_client_kerberos_context::Kerberos> m_kerberos;
};
#endif
#endif  // AUTH_LDAP_SASL_MECHANISM_H_
