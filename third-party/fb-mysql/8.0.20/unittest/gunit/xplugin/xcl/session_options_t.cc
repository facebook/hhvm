/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "unittest/gunit/xplugin/xcl/session_t.h"

namespace xcl {
namespace test {

class Xcl_session_impl_tests_param_bool_option
    : public Xcl_session_impl_tests,
      public WithParamInterface<XSession::Mysqlx_option> {};

TEST_P(Xcl_session_impl_tests_param_bool_option,
       set_not_supported_combination_of_option_with_bool) {
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), false).error());

  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), true).error());
}

INSTANTIATE_TEST_CASE_P(InstantiationNotSupportedBoolOptions,
                        Xcl_session_impl_tests_param_bool_option,
                        Values(XSession::Mysqlx_option::Read_timeout,
                               XSession::Mysqlx_option::Write_timeout,
                               XSession::Mysqlx_option::Connect_timeout,
                               XSession::Mysqlx_option::Allowed_tls,
                               XSession::Mysqlx_option::Ssl_mode,
                               XSession::Mysqlx_option::Hostname_resolve_to,
                               XSession::Mysqlx_option::Ssl_key,
                               XSession::Mysqlx_option::Ssl_ca,
                               XSession::Mysqlx_option::Ssl_ca_path,
                               XSession::Mysqlx_option::Ssl_cert,
                               XSession::Mysqlx_option::Ssl_cipher,
                               XSession::Mysqlx_option::Ssl_crl,
                               XSession::Mysqlx_option::Ssl_crl_path,
                               XSession::Mysqlx_option::Authentication_method));

class Xcl_session_impl_tests_param_int_option
    : public Xcl_session_impl_tests_param_bool_option {};

TEST_P(Xcl_session_impl_tests_param_int_option, set_not_supported_combination) {
  const bool expected_value_bool = false;
  const char *expected_value_pchar = "";
  const std::string expected_value_string = "";

  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), expected_value_bool).error());
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), expected_value_pchar).error());
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), expected_value_string).error());
}

INSTANTIATE_TEST_CASE_P(InstantiationNotSupportedIntOptions,
                        Xcl_session_impl_tests_param_bool_option,
                        Values(XSession::Mysqlx_option::Read_timeout,
                               XSession::Mysqlx_option::Write_timeout,
                               XSession::Mysqlx_option::Connect_timeout));

class Xcl_session_impl_tests_param_text_option
    : public Xcl_session_impl_tests_param_bool_option {};

TEST_P(Xcl_session_impl_tests_param_text_option,
       set_not_supported_combination) {
  const bool expected_value_bool = false;
  const int64_t expected_value_int = 0;

  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), expected_value_bool).error());
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION,
            m_sut->set_mysql_option(GetParam(), expected_value_int).error());
}

INSTANTIATE_TEST_CASE_P(InstantiationNotSupportedTextOptions,
                        Xcl_session_impl_tests_param_text_option,
                        Values(XSession::Mysqlx_option::Hostname_resolve_to,
                               XSession::Mysqlx_option::Allowed_tls,
                               XSession::Mysqlx_option::Ssl_mode,
                               XSession::Mysqlx_option::Ssl_key,
                               XSession::Mysqlx_option::Ssl_ca,
                               XSession::Mysqlx_option::Ssl_ca_path,
                               XSession::Mysqlx_option::Ssl_cert,
                               XSession::Mysqlx_option::Ssl_cipher,
                               XSession::Mysqlx_option::Ssl_crl,
                               XSession::Mysqlx_option::Ssl_crl_path));

TEST_F(Xcl_session_impl_tests, xsession_option_allowed_tls) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Allowed_tls,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_tls_version);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_mode) {
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION_VALUE,
            assert_ssl_mode("NOT_VALID_STRING").error());

  ASSERT_FALSE(assert_ssl_mode("PREFERRED"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_preferred, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("DISABLED"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_disabled, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("REQUIRED"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_required, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("VERIFY_CA"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_verify_ca, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("VERIFY_IDENTITY"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_verify_identity, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("preferred"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_preferred, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("disabled"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_disabled, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("required"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_required, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("verify_ca"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_verify_ca, m_out_ssl_config->m_mode);

  ASSERT_FALSE(assert_ssl_mode("verify_identity"));
  ASSERT_EQ(Ssl_config::Mode::Ssl_verify_identity, m_out_ssl_config->m_mode);
}

TEST_F(Xcl_session_impl_tests, xsession_option_Hostname_resolve_to) {
  ASSERT_EQ(CR_X_UNSUPPORTED_OPTION_VALUE,
            assert_resolve_to("NOT_VALID_STRING").error());

  ASSERT_FALSE(assert_resolve_to("ANY"));
  ASSERT_FALSE(assert_resolve_to("IP4"));
  ASSERT_FALSE(assert_resolve_to("IP6"));
  ASSERT_FALSE(assert_resolve_to("any"));
  ASSERT_FALSE(assert_resolve_to("ip4"));
  ASSERT_FALSE(assert_resolve_to("ip6"));
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_key) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_key,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_key);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_ca) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_ca,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_ca);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_ca_path) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_ca_path,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_ca_path);
}

TEST_F(Xcl_session_impl_tests, session_option_ssl_cert) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_cert,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_cert);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_cipher) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_cipher,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_cipher);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_crl) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_crl,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_crl);
}

TEST_F(Xcl_session_impl_tests, xsession_option_ssl_crl_path) {
  const std::string expected_str_value = "expected value";
  auto error = m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_crl_path,
                                       expected_str_value);
  ASSERT_FALSE(error);
  ASSERT_EQ(expected_str_value, m_out_ssl_config->m_crl_path);
}

TEST_F(Xcl_session_impl_tests, set_valid_timeout_value) {
  const int64_t expected_value_read = 1201;
  const int64_t expected_value_write = 1202;
  const int64_t expected_value_connect = 1203;

  ASSERT_FALSE(m_sut->set_mysql_option(XSession::Mysqlx_option::Read_timeout,
                                       expected_value_read));
  ASSERT_EQ(expected_value_read, m_out_connection_config->m_timeout_read);

  ASSERT_FALSE(m_sut->set_mysql_option(XSession::Mysqlx_option::Write_timeout,
                                       expected_value_write));
  ASSERT_EQ(expected_value_write, m_out_connection_config->m_timeout_write);

  ASSERT_FALSE(m_sut->set_mysql_option(XSession::Mysqlx_option::Connect_timeout,
                                       expected_value_connect));
  ASSERT_EQ(expected_value_connect, m_out_connection_config->m_timeout_connect);
}

class Xcl_session_impl_tests_connected : public Xcl_session_impl_tests {
 public:
  void SetUp() override {
    const bool connected = true;
    m_sut = make_sut(connected);
    expect_connection_close();
  }
};

TEST_F(Xcl_session_impl_tests_connected, xsession_option_set_fails) {
  ASSERT_EQ(
      CR_ALREADY_CONNECTED,
      m_sut
          ->set_mysql_option(XSession::Mysqlx_option::Read_timeout, int64_t{10})
          .error());
  ASSERT_EQ(CR_ALREADY_CONNECTED,
            m_sut->set_mysql_option(XSession::Mysqlx_option::Read_timeout, true)
                .error());
  ASSERT_EQ(
      CR_ALREADY_CONNECTED,
      m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_ca, "").error());
}

}  // namespace test
}  // namespace xcl
