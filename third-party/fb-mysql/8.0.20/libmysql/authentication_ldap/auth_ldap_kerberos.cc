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

#include "auth_ldap_kerberos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <krb5/krb5.h>
#include <profile.h>
#include <sasl/sasl.h>
#endif

Ldap_logger *g_logger_client = NULL;

namespace auth_ldap_client_kerberos_context {
Kerberos::Kerberos(const char *user, const char *password)
    : m_initialized(false),
      m_user(user),
      m_password(password),
      m_destroy_tgt(false),
      m_context(nullptr),
      m_krb_credentials_cache(nullptr),
      m_credentials_created(false) {
  if (g_logger_client == NULL) {
    g_logger_client = new Ldap_logger();
  }
  setup();
}

Kerberos::~Kerberos() { cleanup(); }

void Kerberos::get_ldap_host(std::string &host) { host = m_ldap_server_host; }

bool Kerberos::setup() {
  krb5_error_code res_kerberos = 0;
  bool ret_val = false;

  if (m_initialized) {
    ret_val = true;
    goto EXIT;
  }
  log_dbg("Kerberos setup starting.");
  if ((res_kerberos = krb5_init_context(&m_context)) != 0) {
    log_info("SASL kerberos set up: failed to initialize context.");
    goto EXIT;
  }
  if ((res_kerberos = get_kerberos_config()) != 0) {
    log_info(
        "SASL kerberos set up: failed to get required details from "
        "configuration file.");
    goto EXIT;
  }
  m_initialized = true;
  ret_val = true;

EXIT:
  if (res_kerberos) {
    log(res_kerberos);
    cleanup();
  }
  return ret_val;
}

void Kerberos::cleanup() {
  if (m_destroy_tgt && m_credentials_created) {
    destroy_credentials();
  }

  if (m_krb_credentials_cache) {
    krb5_cc_close(m_context, m_krb_credentials_cache);
    m_krb_credentials_cache = nullptr;
  }

  if (m_context) {
    krb5_free_context(m_context);
    m_context = nullptr;
  }

  m_initialized = false;
}

krb5_error_code Kerberos::store_credentials() {
  krb5_error_code res_kerberos = 0;
  log_dbg("Store credenatial starting.");
  res_kerberos =
      krb5_cc_store_cred(m_context, m_krb_credentials_cache, &m_credentials);
  if (res_kerberos) {
    log_info("SASL kerberos store credentials: failed to store credentials. ");
  }
  return res_kerberos;
}

krb5_error_code Kerberos::obtain_credentials() {
  krb5_error_code res_kerberos = 0;
  krb5_get_init_creds_opt *options = NULL;
  char *password = const_cast<char *>(m_password.c_str());
  krb5_principal principal = nullptr;

  log_dbg("Obtain credentials starting.");

  if (m_credentials_created) {
    log_info("SASL kerberos obtain credentials: already obtained credential.");
    goto EXIT;
  }

  memset(&principal, 0, sizeof(krb5_principal));

  /*
    Full user name with domain name like, yashwant.sahu@oracle.com.
    Parsed principal will be used for authentication and to check if user is
    already authenticated.
  */
  if (!m_user.empty()) {
    res_kerberos = krb5_parse_name(m_context, m_user.c_str(), &principal);
  } else {
    goto EXIT;
  }
  if (res_kerberos) {
    log_info("SASL kerberos obtain credentials: failed to parse user name. ");
    goto EXIT;
  }
  if (m_krb_credentials_cache == nullptr) {
    res_kerberos = krb5_cc_default(m_context, &m_krb_credentials_cache);
  }
  if (res_kerberos) {
    log_info(
        "SASL kerberos obtain credentials: failed to get default credentials "
        "cache.");
    goto EXIT;
  }
  memset(&m_credentials, 0, sizeof(m_credentials));
  krb5_get_init_creds_opt_alloc(m_context, &options);
  /*
    Getting TGT from TGT server.
  */
  res_kerberos =
      krb5_get_init_creds_password(m_context, &m_credentials, principal,
                                   password, NULL, NULL, 0, NULL, options);

  if (res_kerberos) {
    log_info(
        "SASL kerberos obtain credentials: failed to obtained credential.");
    goto EXIT;
  }
  m_credentials_created = true;
  /*
    Verifying TGT.
  */
  res_kerberos =
      krb5_verify_init_creds(m_context, &m_credentials, NULL, NULL, NULL, NULL);
  if (res_kerberos) {
    log_info("SASL kerberos obtain credentials: failed to verify credential.");
    goto EXIT;
  }
  log_dbg("Obtain credential successful");
  if (principal) {
    res_kerberos =
        krb5_cc_initialize(m_context, m_krb_credentials_cache, principal);
    if (res_kerberos) {
      log_info(
          "SASL kerberos store credentials: failed to initialize credentials "
          "cache.");
      goto EXIT;
    }
  }

EXIT:
  if (options) {
    krb5_get_init_creds_opt_free(m_context, options);
    options = NULL;
  }
  if (principal) {
    krb5_free_principal(m_context, principal);
    principal = nullptr;
  }
  if (m_credentials_created && res_kerberos) {
    krb5_free_cred_contents(m_context, &m_credentials);
    m_credentials_created = false;
  }
  return res_kerberos;
}

bool Kerberos::obtain_store_credentials() {
  bool ret_val = false;
  krb5_error_code res_kerberos = 0;
  if (!m_initialized) {
    log_dbg("Kerberos object is not initialized.");
    goto EXIT;
  }
  /*
    Not attempting authentication as there are few security concern of active
    directory allowing users with empty password. End user may question this
    behaviors as security issue with MySQL. primary purpose of this kind of user
    is to search user DN's. User DN search using empty password users is allowed
    in server side plug-in.
  */
  if (m_user.empty() || m_password.empty()) {
    log_info(
        "SASL kerberos obtain and store TGT: empty user name or password. ");
    goto EXIT;
  }
  /*
    If valid credential exist, no need to obtain it again.
    This is done for the performance reason. Default expiry of TGT is 24 hours
    and this can be configured.
  */
  if ((ret_val = credential_valid())) {
    log_info("SASL kerberos obtain and store TGT: Valid TGT exist. ");
    goto EXIT;
  }
  if ((res_kerberos = obtain_credentials()) != 0) {
    log_info(
        "SASL kerberos obtain and store TGT: failed to obtain "
        "TGT/credential. ");
    goto EXIT;
  }
  /*
    Store the credentials in the default cache. Types can be file, memory,
    keyring etc. Administrator should change default cache based on there
    preference.
  */
  if ((res_kerberos = store_credentials()) != 0) {
    log_info("SASL kerberos obtain and store TGT: failed to store credential.");
    goto EXIT;
  }

  ret_val = true;

EXIT:
  if (res_kerberos) {
    ret_val = false;
    log(res_kerberos);
  }
  /*
    Storing the credentials.
    We need to close the context to save the credentials successfully.
   */
  if (m_credentials_created && !m_destroy_tgt) {
    krb5_free_cred_contents(m_context, &m_credentials);
    m_credentials_created = false;
    if (m_krb_credentials_cache) {
      log_info("Storing credentials into cache, closing krb5 cc.");
      krb5_cc_close(m_context, m_krb_credentials_cache);
      m_krb_credentials_cache = nullptr;
    }
  }
  return ret_val;
}

/**
  This method gets kerberos profile settings from krb5.conf file.
  Sample krb5.conf file format may be like this:

  [realms]
  MEM.LOCAL = {
    kdc = VIKING67.MEM.LOCAL
    admin_server = VIKING67.MEM.LOCAL
    default_domain = MEM.LOCAL
    }

  # This portion is optional
  [appdefaults]
  mysql = {
    ldap_server_host = ldap_host.oracle.com
    ldap_destroy_tgt = true
  }
*/
bool Kerberos::get_kerberos_config() {
  log_dbg("Getting kerberos configuration.");
  /*
    Kerberos profile category/sub-category names.
  */
  const char realms_heading[] = "realms";
  const char host_default[] = "";
  const char apps_heading[] = "appdefaults";
  const char mysql_apps[] = "mysql";
  const char ldap_host_option[] = "ldap_server_host";
  const char ldap_destroy_option[] = "ldap_destroy_tgt";

  krb5_error_code res_kerberos = 0;
  _profile_t *profile = NULL;
  char *host_value = NULL;
  char *default_realm = NULL;

  /*
    Get default realm.
  */
  res_kerberos = krb5_get_default_realm(m_context, &default_realm);
  if (res_kerberos) {
    log_error("get_kerberos_config: failed to get default realm.");
    goto EXIT;
  }

  res_kerberos = krb5_get_profile(m_context, &profile);
  if (res_kerberos) {
    log_error("get_kerberos_config: failed to kerberos configurations.");
    goto EXIT;
  }

  /*
    1. Getting ldap server host from mysql app section.
    2. If failed to get from mysql app section, get from realm section.
    realm section should have kdc server info as without kdc info, kerberos
    authentication will not work. Authentication process will stop and consider
    failed if failed to get LDAP server host.
  */
  res_kerberos =
      profile_get_string(profile, apps_heading, mysql_apps, ldap_host_option,
                         host_default, &host_value);
  if (res_kerberos || !strcmp(host_value, "")) {
    if (host_value) {
      profile_release_string(host_value);
      host_value = NULL;
    }
    res_kerberos = profile_get_string(profile, realms_heading, default_realm,
                                      "kdc", host_default, &host_value);
    if (res_kerberos) {
      if (host_value) {
        profile_release_string(host_value);
        host_value = NULL;
      }
      log_error("get_kerberos_config: failed to get ldap server host.");
      goto EXIT;
    }
  }
  m_ldap_server_host = host_value;
  log_info(host_value);

  /*
    Get the LDAP destroy TGT from MySQL app section.
    If failed to get destroy TGT option, default option value will be false.
    This value is consistent with kerberos authentication usage as TGT was
    supposed to be used till it expires.
  */
  res_kerberos = profile_get_boolean(profile, realms_heading, default_realm,
                                     ldap_destroy_option, m_destroy_tgt,
                                     (int *)&m_destroy_tgt);
  if (res_kerberos) {
    log_info(
        "get_kerberos_config: failed to get destroy TGT flag, default is set.");
  }

EXIT:
  profile_release(profile);
  if (host_value) {
    profile_release_string(host_value);
    host_value = NULL;
  }
  if (default_realm) {
    krb5_free_default_realm(m_context, default_realm);
    default_realm = NULL;
  }
  return res_kerberos;
}

bool Kerberos::credential_valid() {
  bool ret_val = false;
  krb5_error_code res_kerberos = 0;
  krb5_creds credentials;
  krb5_timestamp krb_current_time;
  bool credentials_retrieve = false;
  krb5_creds matching_credential;
  std::stringstream info_stream;
  char *realm = NULL;

  memset(&matching_credential, 0, sizeof(matching_credential));
  memset(&credentials, 0, sizeof(credentials));

  if (m_krb_credentials_cache == nullptr) {
    res_kerberos = krb5_cc_default(m_context, &m_krb_credentials_cache);
    if (res_kerberos) {
      log_info(
          "SASL kerberos set up: failed to get default credentials cache.");
      goto EXIT;
    }
  }
  /*
    Example credentials client principal: test3@MYSQL.LOCAL
    Example credentials server principal: krbtgt/MYSQL.LOCAL@MYSQL.LOCAL
  */
  res_kerberos =
      krb5_parse_name(m_context, m_user.c_str(), &matching_credential.client);
  if (res_kerberos) {
    log_info(
        "SASL kerberos credentials valid: failed to parsed client principal.");
    goto EXIT;
  }
  res_kerberos = krb5_get_default_realm(m_context, &realm);
  if (res_kerberos) {
    log_info("SASL kerberos credentials valid: failed to get default realm.");
    goto EXIT;
  }
  log_info(realm);
  res_kerberos =
      krb5_build_principal(m_context, &matching_credential.server,
                           strlen(realm), realm, "krbtgt", realm, NULL);
  if (res_kerberos) {
    log_info(
        "SASL kerberos credentials valid: failed to build krbtgt principal.");
    goto EXIT;
  }

  /*
    Retrieving credentials using input parameters.
  */
  res_kerberos = krb5_cc_retrieve_cred(m_context, m_krb_credentials_cache, 0,
                                       &matching_credential, &credentials);
  if (res_kerberos) {
    log_info(
        "SASL kerberos credentials valid: failed to retrieve credentials. ");
    goto EXIT;
  }
  credentials_retrieve = true;
  /*
    Getting current time from kerberos libs.
  */
  res_kerberos = krb5_timeofday(m_context, &krb_current_time);
  if (res_kerberos) {
    log_info(
        "SASL kerberos credentials valid: failed to retrieve current time. ");
    goto EXIT;
  }
  /*
    Checking validity of credentials if it is still valid.
  */
  if (credentials.times.endtime < krb_current_time) {
    log_info("SASL kerberos credentials valid: credentials are expired. ");
    goto EXIT;
  } else {
    ret_val = true;
    log_info(
        "SASL kerberos credentials valid: credentials are valid. New TGT will "
        "not be obtained. ");
  }

EXIT:
  if (res_kerberos) {
    ret_val = false;
    log(res_kerberos);
  }
  if (realm) {
    krb5_free_default_realm(m_context, realm);
    realm = NULL;
  }
  if (matching_credential.server) {
    krb5_free_principal(m_context, matching_credential.server);
  }
  if (matching_credential.client) {
    krb5_free_principal(m_context, matching_credential.client);
  }
  if (credentials_retrieve) {
    krb5_free_cred_contents(m_context, &credentials);
  }
  if (m_krb_credentials_cache) {
    krb5_cc_close(m_context, m_krb_credentials_cache);
    m_krb_credentials_cache = nullptr;
  }
  return ret_val;
}

void Kerberos::destroy_credentials() {
  log_dbg("SASL kerberos destroy credentials");
  if (!m_destroy_tgt) {
    log_dbg("SASL kerberos destroy credentials: destroy flag is false.");
    return;
  }
  krb5_error_code res_kerberos = 0;
  if (m_credentials_created) {
    res_kerberos = krb5_cc_remove_cred(m_context, m_krb_credentials_cache, 0,
                                       &m_credentials);
    krb5_free_cred_contents(m_context, &m_credentials);
    m_credentials_created = false;
  }

  if (res_kerberos) {
    log(res_kerberos);
  }
}

bool Kerberos::get_user_name(std::string *name) {
  krb5_error_code res_kerberos = 0;
  krb5_principal principal = nullptr;
  krb5_context context = nullptr;
  char *user_name = nullptr;
  std::stringstream log_stream;

  if (!m_initialized) {
    log_dbg("Kerberos object is not initialized.");
    goto EXIT;
  }
  if (!name) {
    log_dbg("Failed to get Kerberos user name");
    goto EXIT;
  }
  *name = "";
  if (m_krb_credentials_cache == nullptr) {
    res_kerberos = krb5_cc_default(m_context, &m_krb_credentials_cache);
    if (res_kerberos) {
      log_info(
          "SASL kerberos set up: failed to get default credentials cache.");
      goto EXIT;
    }
  }
  /*
    Getting default principal in the kerberos.
  */
  res_kerberos =
      krb5_cc_get_principal(m_context, m_krb_credentials_cache, &principal);
  if (res_kerberos) {
    log_info("SASL get user name: failed to get principal.");
    goto EXIT;
  }
  /*
    Parsing user name from principal.
  */
  res_kerberos = krb5_unparse_name(m_context, principal, &user_name);
  if (res_kerberos) {
    log_info("SASL get user name: failed to parse principal name ");
    goto EXIT;
  } else {
    log_stream << "SASL get user name: ";
    log_stream << user_name;
    log_info(log_stream.str());
    *name = user_name;
  }

EXIT:
  if (user_name) {
    free(user_name);
  }
  if (principal) {
    krb5_free_principal(context, principal);
    principal = nullptr;
  }
  if (m_krb_credentials_cache) {
    krb5_cc_close(m_context, m_krb_credentials_cache);
    m_krb_credentials_cache = nullptr;
  }
  if (res_kerberos) {
    log(res_kerberos);
    return false;
  } else {
    return true;
  }
}

void Kerberos::log(int error_code) {
  const char *err_message = nullptr;
  std::stringstream error_stream;
  if (m_context) {
    err_message = krb5_get_error_message(m_context, error_code);
  }
  if (err_message) {
    error_stream << "LDAP SASL kerberos operation failed with error: "
                 << err_message;
  }
  log_error(error_stream.str());
  if (err_message) {
    krb5_free_error_message(m_context, err_message);
  }
  return;
}
}  // namespace auth_ldap_client_kerberos_context
