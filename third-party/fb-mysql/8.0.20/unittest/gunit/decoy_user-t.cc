/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "lex_string.h"
#include "m_string.h"
#include "my_inttypes.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_utility.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_authentication.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"

namespace decoy_user_unittest {
using my_testing::Server_initializer;
using std::max;
using std::rand;

class Decoy_user_Test : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *get_thd() { return initializer.thd(); }

  Server_initializer initializer;
};

std::set<Auth_id> unknown_authids;
const size_t unknown_authids_size = 1000;

void generate_random() {
  unknown_authids.clear();

  /* Random user and host of length size */
  auto get_random_account = [](size_t length) -> Auth_id {
    const char chars[] =
        "QWERTYUIOPASDFGHJKLZXCVBNM"
        "qwertyuiopasdfghjklzxcvbnm";
    const size_t limit = (sizeof(chars) - 1);

    std::string user, host;
    for (size_t i = 0; i < length; ++i) user.push_back(chars[rand() % limit]);
    for (size_t i = 0; i < length; ++i) host.push_back(chars[rand() % limit]);

    return (Auth_id(user, host));
  };

  while (unknown_authids.size() < unknown_authids_size) {
    unknown_authids.insert(get_random_account(max(5, rand() % 16)));
  }
}

TEST_F(Decoy_user_Test, DecoyUserConsistency) {
  THD *thd = get_thd();
  ACL_USER *decoy = nullptr;

  unknown_accounts = new Map_with_rw_lock<Auth_id, uint>(0);

  const LEX_CSTRING users[] = {{STRING_WITH_LEN("foo")},
                               {STRING_WITH_LEN("baz")},
                               {STRING_WITH_LEN("quux")}};
  const LEX_CSTRING hosts[] = {{STRING_WITH_LEN("bar")},
                               {STRING_WITH_LEN("qux")},
                               {STRING_WITH_LEN("quuz")}};

  for (auto i = 0; i < 3; ++i) {
    for (auto j = 0; j < 3; ++j) {
      decoy = nullptr;
      decoy = decoy_user(users[i], hosts[j], thd->mem_root, &thd->rand, true);
      std::string first(decoy->plugin.str, decoy->plugin.length);
      decoy = decoy_user(users[i], hosts[j], thd->mem_root, &thd->rand, true);
      std::string second(decoy->plugin.str, decoy->plugin.length);
      EXPECT_FALSE(first.compare(second));
    }
  }

  EXPECT_TRUE(unknown_accounts->size() == 9);

  delete unknown_accounts;
}

TEST_F(Decoy_user_Test, DecoyUserRandomness) {
  THD *thd = get_thd();
  std::map<std::string, uint> plugin_hits;
  uint hit_roof = 4;
  uint trials = 0;
  uint threshold = 500;
  ACL_USER *decoy = nullptr;

  unknown_accounts = new Map_with_rw_lock<Auth_id, uint>(0);

  /* initialize the map */
  for (uint a = 0; a < (uint)PLUGIN_LAST; ++a) {
    std::string plugin(
        Cached_authentication_plugins::cached_plugins_names[a].str,
        Cached_authentication_plugins::cached_plugins_names[a].length);
    plugin_hits[plugin]; /* operator [] initializes entry with 0 if missing */
  }

  generate_random();

  while (trials < hit_roof) {
    for (auto &element : unknown_authids) {
      decoy = nullptr;
      LEX_CSTRING user;
      LEX_CSTRING host;
      user.str = element.user().c_str();
      user.length = element.user().length();
      host.str = element.host().c_str();
      host.length = element.host().length();

      decoy = decoy_user(user, host, thd->mem_root, &thd->rand, true);
      std::string hit(decoy->plugin.str, decoy->plugin.length);
      plugin_hits[hit]++;
    }

    ++trials;
  }

  for (auto &entry : plugin_hits) {
    EXPECT_TRUE(entry.second >= threshold);
  }

  delete unknown_accounts;
}

}  // namespace decoy_user_unittest
