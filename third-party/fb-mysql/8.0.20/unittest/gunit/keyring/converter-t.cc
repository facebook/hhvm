/*
  Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <plugin/keyring/converter.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace keyring_converter_unittest {

using namespace keyring;
using ::testing::StrEq;
using Arch = Converter::Arch;

class Converter_test : public ::testing::Test {};

class KeyData {
 public:
  KeyData(const char *id, const char *type, const char *user_id,
          const char *data)
      : id(id), type(type), user_id(user_id), data(data) {}

  std::string serialize(Arch arch) const {
    std::string output;

    size_t width = Converter::get_width(arch);

    // key data lengths
    std::vector<size_t> lengths = {id.length(), type.length(), user_id.length(),
                                   data.length()};

    // append lengths
    char src[8] = {0};
    char dst[8] = {0};
    for (auto const &length : lengths) {
      memcpy(src, &length, sizeof(size_t));
      Converter::convert(src, dst, Converter::get_native_arch(), arch);
      output.append(dst, width);
    }

    // append  data
    output += id + type + user_id + data;

    // calculate total length and append padding
    size_t total = width + output.length();
    size_t padding = (width - total % width) % width;
    output.append(padding, '\0');
    total += padding;

    // convert total
    memcpy(src, &total, sizeof(size_t));
    Converter::convert(src, dst, Converter::get_native_arch(), arch);
    output = std::string(dst, width) + output;

    return output;
  }

 private:
  std::string id, type, user_id, data;
};

/**
  tests conversion between various width/endianess binary representations
*/
TEST_F(Converter_test, LengthConversion) {
  struct Conversion {
    Arch src, dst;
    const std::string source, expected;
  };

  // list of valid conversions and expected results
  Conversion valid_tasks[] = {

      // no conversion
      {Arch::LE_32, Arch::LE_32, "\x12\x34\x56\x78", "\x12\x34\x56\x78"},
      {Arch::BE_32, Arch::BE_32, "\x12\x34\x56\x78", "\x12\x34\x56\x78"},
      {
          Arch::LE_64,
          Arch::LE_64,
          "\x12\x34\x56\x78\x12\x34\x56\x78",
          "\x12\x34\x56\x78\x12\x34\x56\x78",
      },
      {
          Arch::BE_64,
          Arch::BE_64,
          "\x12\x34\x56\x78\x12\x34\x56\x78",
          "\x12\x34\x56\x78\x12\x34\x56\x78",
      }

      // 32 bit endianess conversions
      ,
      {Arch::LE_32, Arch::BE_32, "\x12\x34\x56\x78", "\x78\x56\x34\x12"},
      {Arch::BE_32, Arch::LE_32, "\x12\x34\x56\x78", "\x78\x56\x34\x12"}

      // 64 bit endianess conversions
      ,
      {Arch::LE_64, Arch::BE_64, "\x12\x34\x56\x78\x12\x34\x56\x78",
       "\x78\x56\x34\x12\x78\x56\x34\x12"},
      {Arch::BE_64, Arch::LE_64, "\x12\x34\x56\x78\x12\x34\x56\x78",
       "\x78\x56\x34\x12\x78\x56\x34\x12"}

      // growth conversions
      ,
      {Arch::LE_32,
       Arch::LE_64,
       "\x12\x34\x56\x78",
       {"\x12\x34\x56\x78\x00\x00\x00\x00", 8}},
      {Arch::BE_32, Arch::LE_64, "\x12\x34\x56\x78",
       "\x00\x00\x00\x00\x78\x56\x34\x12"},
      {Arch::LE_32, Arch::BE_64, "\x12\x34\x56\x78",
       "\x00\x00\x00\x00\x78\x56\x34\x12"},
      {Arch::BE_32, Arch::BE_64, "\x12\x34\x56\x78",
       "\x00\x00\x00\x00\x12\x34\x56\x78"}

      // shrink conversions
      ,
      {Arch::LE_64,
       Arch::LE_32,
       {"\x12\x34\x56\x78\x00\x00\x00\x00", 8},
       "\x12\x34\x56\x78"},
      {Arch::LE_64,
       Arch::BE_32,
       {"\x12\x34\x56\x78\x00\x00\x00\x00", 8},
       "\x78\x56\x34\x12"},
      {Arch::BE_64,
       Arch::LE_32,
       {"\x00\x00\x00\x00\x12\x34\x56\x78", 8},
       "\x78\x56\x34\x12"},
      {Arch::BE_64,
       Arch::BE_32,
       {"\x00\x00\x00\x00\x12\x34\x56\x78", 8},
       "\x12\x34\x56\x78"}};

  char result[8] = {0};
  for (auto &task : valid_tasks) {
    // convert task
    auto length =
        Converter::convert(task.source.c_str(), result, task.src, task.dst);

    // there should be no errors
    EXPECT_NE(length, 0);

    // result should match expected value
    EXPECT_EQ(strncmp(result, task.expected.c_str(), task.expected.length()),
              0);
  }

  // list of invalid conversions
  Conversion invalid_tasks[] = {
      // shrink conversions with values > 32bit
      {Arch::LE_64, Arch::LE_32, {"\x12\x34\x56\x78\x90\x00\x00\x00", 8}, ""},
      {Arch::LE_64, Arch::BE_32, {"\x12\x34\x56\x78\x00\x90\x00\x00", 8}, ""},
      {Arch::BE_64, Arch::LE_32, {"\x00\x90\x00\x00\x12\x34\x56\x78", 8}, ""},
      {Arch::BE_64, Arch::BE_32, {"\x90\x00\x00\x00\x12\x34\x56\x78", 8}, ""}};

  // each invalid task should cause errors
  for (auto &task : invalid_tasks)
    EXPECT_EQ(
        Converter::convert(task.source.c_str(), result, task.src, task.dst), 0);
}

/**
  tests conversion of key buffer examples
*/
TEST_F(Converter_test, KeyDataConversion) {
  const std::vector<KeyData> keys = {
      {"Key1", "AES", "Module1", "Key1DataString"},
      {"Key2", "RSA", "Module2", "Key2DataStringLonger"},
      {"Key3", "DES", "Module3", "Key3DataStringLongest"}};

  const std::vector<Arch> order = {Arch::LE_32, Arch::BE_32, Arch::LE_64,
                                   Arch::BE_64};

  // generate keyring file representations
  std::map<Arch, std::string> files;
  for (auto const &arch : order)
    for (auto const &key : keys) files[arch] += key.serialize(arch);

  // verify conversions
  std::string result;
  for (auto const &arch1 : order)
    for (auto const &arch2 : order) {
      // at least one arch has to be native
      if (arch1 != Converter::get_native_arch() &&
          arch2 != Converter::get_native_arch())
        continue;

      // conversion has to be free of errors
      EXPECT_FALSE(Converter::convert_data(
          files[arch1].c_str(), files[arch1].length(), arch1, arch2, result));

      // conversion result has to match destination format
      EXPECT_EQ(result, files[arch2]);
    }
}

}  // namespace keyring_converter_unittest
