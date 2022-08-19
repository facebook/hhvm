/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <string>
#include <vector>

#include "gcs_base_test.h"

using std::vector;

namespace gcs_xcom_networking_unittest {

void clean_sock_probe(sock_probe *s) { free(s); }

class mock_gcs_sock_probe_interface : public Gcs_sock_probe_interface {
 public:
  MOCK_METHOD1(init_sock_probe, int(sock_probe *s));
  MOCK_METHOD1(number_of_interfaces, int(sock_probe *s));
  MOCK_METHOD3(get_sockaddr_address,
               void(sock_probe *s, int count, struct sockaddr **out));
  MOCK_METHOD3(get_sockaddr_netmask,
               void(sock_probe *s, int count, struct sockaddr **out));
  MOCK_METHOD2(get_if_name, char *(sock_probe *s, int count));
  MOCK_METHOD2(is_if_running, bool_t(sock_probe *s, int count));
  MOCK_METHOD1(close_sock_probe, void(sock_probe *s));

  void mock_gcs_sock_probe_interface_default() {
    ON_CALL(*this, close_sock_probe(_)).WillByDefault(Invoke(clean_sock_probe));
  }
};

class GcsXComNetworking : public GcsBaseTest {
 protected:
  GcsXComNetworking() : m_sock_probe_mock() {}
  ~GcsXComNetworking() {}

  void SetUp() { m_sock_probe_mock.mock_gcs_sock_probe_interface_default(); }

  mock_gcs_sock_probe_interface m_sock_probe_mock;
};

TEST_F(GcsXComNetworking, SockProbeInvalid) {
  EXPECT_CALL(m_sock_probe_mock, init_sock_probe(_))
      .Times(1)
      .WillOnce(Return(-1));

  std::map<std::string, int> out_value;
  bool result = get_local_addresses(m_sock_probe_mock, out_value);

  ASSERT_TRUE(result);
}

TEST_F(GcsXComNetworking, NoInterfaces) {
  EXPECT_CALL(m_sock_probe_mock, init_sock_probe(_))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(m_sock_probe_mock, close_sock_probe(_)).Times(1);

  EXPECT_CALL(m_sock_probe_mock, number_of_interfaces(_))
      .Times(1)
      .WillOnce(Return(0));

  std::map<std::string, int> out_value;
  bool result = get_local_addresses(m_sock_probe_mock, out_value);

  ASSERT_TRUE(result);
}

TEST_F(GcsXComNetworking, ErrorRetrievingSockaddr) {
  std::string if_name("interface");

  EXPECT_CALL(m_sock_probe_mock, init_sock_probe(_))
      .Times(1)
      .WillOnce(Return(0));

  EXPECT_CALL(m_sock_probe_mock, close_sock_probe(_)).Times(1);

  EXPECT_CALL(m_sock_probe_mock, number_of_interfaces(_))
      .Times(3)
      .WillRepeatedly(Return(1));

  struct sockaddr *null_sockaddr = nullptr;
  EXPECT_CALL(m_sock_probe_mock, get_sockaddr_netmask(_, _, _))
      .Times(1)
      .WillOnce(testing::SetArgPointee<2>(null_sockaddr));

  EXPECT_CALL(m_sock_probe_mock, get_sockaddr_address(_, _, _))
      .Times(1)
      .WillOnce(testing::SetArgPointee<2>(null_sockaddr));

  EXPECT_CALL(m_sock_probe_mock, get_if_name(_, _))
      .Times(1)
      .WillOnce(Return(const_cast<char *>(if_name.c_str())));

  std::map<std::string, int> out_value;
  bool result = get_local_addresses(m_sock_probe_mock, out_value);

  ASSERT_TRUE(result);
}

TEST_F(GcsXComNetworking, ResolveAllIPV6) {
  std::vector<std::pair<sa_family_t, std::string>> out_value;
  bool retval = resolve_all_ip_addr_from_hostname("::1", out_value);

  ASSERT_FALSE(retval);
}

}  // namespace gcs_xcom_networking_unittest
