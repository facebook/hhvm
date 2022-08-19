/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "plugin/x/client/context/xssl_config.h"

namespace xcl {
namespace test {

using ::testing::Test;
using ::testing::Values;
using ::testing::WithParamInterface;

using Ssl_string_field = std::string Ssl_config::*;
using Ssl_mode_field = Ssl_config::Mode Ssl_config::*;

class Xcl_ssl_config_tests : public Test {
 public:
  bool assert_mode_requires_ssl(const Ssl_config::Mode m) {
    m_sut.reset(new Ssl_config());
    m_sut->m_mode = m;

    return m_sut->does_mode_requires_ssl();
  }

  template <typename Value_type, typename Method, typename Field>
  bool assert_value(Field field, Method method, const Value_type value,
                    const bool reset = true) {
    if (reset) m_sut.reset(new Ssl_config());
    auto sut = m_sut.get();
    sut->*field = value;

    return (sut->*method)();
  }

  std::unique_ptr<Ssl_config> m_sut{new Ssl_config()};

  Ssl_mode_field mode = &Ssl_config::m_mode;
  Ssl_string_field tls_version = &Ssl_config::m_tls_version;
  Ssl_string_field key = &Ssl_config::m_key;
  Ssl_string_field ca = &Ssl_config::m_ca;
  Ssl_string_field ca_path = &Ssl_config::m_ca_path;
  Ssl_string_field cert = &Ssl_config::m_cert;
  Ssl_string_field cipher = &Ssl_config::m_cipher;
  Ssl_string_field crl = &Ssl_config::m_crl;
  Ssl_string_field crl_path = &Ssl_config::m_crl_path;
};

TEST_F(Xcl_ssl_config_tests, is_configured) {
  auto is_conf = &Ssl_config::is_configured;

  ASSERT_TRUE(m_sut->is_configured());
  ASSERT_FALSE(assert_value(mode, is_conf, Ssl_config::Mode::Ssl_disabled));
  ASSERT_TRUE(assert_value(mode, is_conf, Ssl_config::Mode::Ssl_preferred));
  ASSERT_TRUE(assert_value(mode, is_conf, Ssl_config::Mode::Ssl_required));
  ASSERT_TRUE(assert_value(mode, is_conf, Ssl_config::Mode::Ssl_verify_ca));
  ASSERT_TRUE(
      assert_value(mode, is_conf, Ssl_config::Mode::Ssl_verify_identity));
}

TEST_F(Xcl_ssl_config_tests, is_ca_configured) {
  auto is_ca_conf = &Ssl_config::is_ca_configured;

  ASSERT_TRUE(m_sut->is_configured());
  ASSERT_FALSE(assert_value(mode, is_ca_conf, Ssl_config::Mode::Ssl_required));
  ASSERT_FALSE(assert_value(tls_version, is_ca_conf, "text value"));
  ASSERT_FALSE(assert_value(key, is_ca_conf, "text value"));
  ASSERT_TRUE(assert_value(ca, is_ca_conf, "text value"));
  ASSERT_TRUE(assert_value(ca_path, is_ca_conf, "text value"));
  ASSERT_FALSE(assert_value(cert, is_ca_conf, "text value"));
  ASSERT_FALSE(assert_value(cipher, is_ca_conf, "text value"));
  ASSERT_FALSE(assert_value(crl, is_ca_conf, "text value"));
  ASSERT_FALSE(assert_value(crl_path, is_ca_conf, "text value"));
}

TEST_F(Xcl_ssl_config_tests, requires_ssl) {
  ASSERT_FALSE(assert_mode_requires_ssl(Ssl_config::Mode::Ssl_disabled));
  ASSERT_FALSE(assert_mode_requires_ssl(Ssl_config::Mode::Ssl_preferred));
  ASSERT_TRUE(assert_mode_requires_ssl(Ssl_config::Mode::Ssl_required));
  ASSERT_TRUE(assert_mode_requires_ssl(Ssl_config::Mode::Ssl_verify_ca));
  ASSERT_TRUE(assert_mode_requires_ssl(Ssl_config::Mode::Ssl_verify_identity));
}

TEST_F(Xcl_ssl_config_tests, requires_ca) {
  auto requires_ca = &Ssl_config::does_mode_requires_ca;

  ASSERT_FALSE(assert_value(mode, requires_ca, Ssl_config::Mode::Ssl_disabled));
  ASSERT_FALSE(
      assert_value(mode, requires_ca, Ssl_config::Mode::Ssl_preferred));
  ASSERT_FALSE(assert_value(mode, requires_ca, Ssl_config::Mode::Ssl_required));
  ASSERT_TRUE(assert_value(mode, requires_ca, Ssl_config::Mode::Ssl_verify_ca));
  ASSERT_TRUE(
      assert_value(mode, requires_ca, Ssl_config::Mode::Ssl_verify_identity));
}

}  // namespace test
}  // namespace xcl
