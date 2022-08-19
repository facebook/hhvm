/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "gcs_base_test.h"

#include <cstring>
#include <string>
#include <vector>
#include "app_data.h"
#include "get_synode_app_data.h"
#include "pax_msg.h"
#include "xcom_cache.h"
#include "xcom_memory.h"
#include "xcom_transport.h"

namespace xcom_transport_unittest {

class XcomTransport : public GcsBaseTest {
 protected:
  XcomTransport() {}
  ~XcomTransport() {}
};

TEST_F(XcomTransport, SerializeTooManySynodes) {
  u_int constexpr nr_synodes = MAX_SYNODE_ARRAY + 1;

  app_data_ptr a = new_app_data();
  a->body.c_t = get_synode_app_data_type;
  a->body.app_u_u.synodes.synode_no_array_len = nr_synodes;
  a->body.app_u_u.synodes.synode_no_array_val =
      static_cast<synode_no *>(std::calloc(nr_synodes, sizeof(synode_no)));

  pax_msg *p = pax_msg_new(null_synode, nullptr);
  p->a = a;
  p->to = VOID_NODE_NO;
  p->op = client_msg;

  uint32_t buflen = 0;
  char *buf = nullptr;
  ASSERT_EQ(serialize_msg(p, x_1_6, &buflen, &buf), 0);

  p->refcnt = 1;
  unchecked_replace_pax_msg(&p, nullptr);

  std::free(buf);
}

}  // namespace xcom_transport_unittest
