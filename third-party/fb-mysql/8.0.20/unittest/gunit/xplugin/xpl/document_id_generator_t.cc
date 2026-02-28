/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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
#include <limits>
#include <memory>

#include "plugin/x/ngs/include/ngs/document_id_generator.h"

namespace xpl {
namespace test {

class Document_id_generator_test : public ::testing::Test {
 public:
  iface::Document_id_generator &generator(const uint64_t timestamp,
                                          const uint64_t serial) {
    gen.reset(new ngs::Document_id_generator(timestamp, serial));
    return *gen;
  }

  std::unique_ptr<iface::Document_id_generator> gen;
};

TEST_F(Document_id_generator_test, generate_id_sequence_1) {
  iface::Document_id_generator::Variables vars{0, 1, 1};
  generator(0, 0);

  EXPECT_STREQ("0000000000000000000000000001", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000002", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000003", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000004", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000005", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000006", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000007", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000008", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000009", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000a", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000b", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000c", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000d", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000e", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000f", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000010", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_5) {
  iface::Document_id_generator::Variables vars{0, 1, 5};
  generator(0, 0);

  EXPECT_STREQ("0000000000000000000000000001", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000006", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000000000000000000b", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000010", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000015", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_16) {
  iface::Document_id_generator::Variables vars{0, 1, 16};
  generator(0, 0);

  EXPECT_STREQ("0000000000000000000000000001", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000011", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000021", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000000000000000000031", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_1_1_serial_limit) {
  iface::Document_id_generator::Variables vars{0, 1, 1};
  generator(0, std::numeric_limits<uint64_t>::max() - 2);
  EXPECT_STREQ("000000000000fffffffffffffffe", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000ffffffffffffffff", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000001", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000002", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_0_1_serial_limit) {
  iface::Document_id_generator::Variables vars{0, 0, 1};
  generator(0, std::numeric_limits<uint64_t>::max() - 2);
  EXPECT_STREQ("000000000000fffffffffffffffe", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000ffffffffffffffff", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000000", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000001", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_1_5_serial_limit) {
  iface::Document_id_generator::Variables vars{0, 1, 5};
  generator(0, std::numeric_limits<uint64_t>::max() - 2 * 5);
  EXPECT_STREQ("000000000000fffffffffffffff6", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000fffffffffffffffb", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000001", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000006", gen->generate(vars).c_str());
}

TEST_F(Document_id_generator_test, generate_id_sequence_0_5_serial_limit) {
  iface::Document_id_generator::Variables vars{0, 0, 5};
  generator(0, std::numeric_limits<uint64_t>::max() - 2 * 5);
  EXPECT_STREQ("000000000000fffffffffffffffa", gen->generate(vars).c_str());
  EXPECT_STREQ("000000000000ffffffffffffffff", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000000", gen->generate(vars).c_str());
  EXPECT_STREQ("0000000000010000000000000005", gen->generate(vars).c_str());
}

struct Param_document_id {
  std::string expect_id;
  uint64_t timestamp, serial;
  uint16_t prefix, offset, increment;
};

class Document_id_generator_param_test
    : public Document_id_generator_test,
      public ::testing::WithParamInterface<Param_document_id> {};

TEST_P(Document_id_generator_param_test, generate_id) {
  using Variables = iface::Document_id_generator::Variables;
  const Param_document_id &param = GetParam();
  std::string result;
  ASSERT_NO_THROW(result = generator(param.timestamp, param.serial)
                               .generate(Variables{param.prefix, param.offset,
                                                   param.increment}));
  EXPECT_EQ(param.expect_id, result);
}

Param_document_id document_id_param[] = {
    {"0000000000000000000000000001", 0, 0, 0, 0, 0},
    {"0001000000000000000000000001", 0, 0, 1, 0, 0},
    {"0000000000010000000000000001", 1, 0, 0, 0, 0},
    {"0000000000000000000000000002", 0, 1, 0, 0, 0},
    {"0000000000000000000000000002", 0, 1, 0, 0, 1},
    {"0000000000000000000000000001", 0, 0, 0, 1, 1},
    {"0000000000000000000000000002", 0, 1, 0, 1, 1},
    {"0001000000010000000000000002", 1, 1, 1, 1, 1},
    {"0000000000000000000000000001", 0, 0, 0, 1, 10},
    {"000000000000000000000000000b", 0, 1, 0, 1, 10},
    {"000000000000000000000000000b", 0, 2, 0, 1, 10},
    {"0000000000000000000000000001", 0, 0, 0, 5, 1},  // offset to big: ignored
    {"0000000000000000000000000005", 0, 1, 0, 5, 10},
    {"000000000000000000000000000f", 0, 5, 0, 5, 10}};

INSTANTIATE_TEST_CASE_P(document_id_generation,
                        Document_id_generator_param_test,
                        testing::ValuesIn(document_id_param));

}  // namespace test
}  // namespace xpl
