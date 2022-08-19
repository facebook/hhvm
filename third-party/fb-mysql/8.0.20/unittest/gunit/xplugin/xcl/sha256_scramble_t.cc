/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "plugin/x/client/authentication/sha256_scramble_generator.h"

namespace xcl {
namespace sha256_scramble_unittest {

using namespace xcl::sha256_password;

TEST(SHA256_digest_test, init_digest_context) {
  SHA256_digest sha256_digest;
  ASSERT_TRUE(sha256_digest.all_ok());
}

TEST(SHA256_digest_test, digest_single_stage) {
  SHA256_digest sha256_digest;
  const char *plaintext1 = "quick brown fox jumped over the lazy dog";
  size_t plaintext1_length = strlen(plaintext1);
  unsigned char expected_digest1[] = {
      0x5c, 0xa1, 0xa2, 0x08, 0xc4, 0x61, 0x95, 0x3f, 0x7c, 0x12, 0xd2,
      0x79, 0xc7, 0xa8, 0x62, 0x0f, 0x3f, 0x83, 0x87, 0xf4, 0x09, 0x8c,
      0xe8, 0x53, 0x1b, 0x17, 0x51, 0x03, 0x37, 0xbd, 0x63, 0x78};

  unsigned char digest_output[CACHING_SHA2_DIGEST_LENGTH];

  ASSERT_TRUE(sha256_digest.all_ok());
  ASSERT_FALSE(sha256_digest.update_digest(plaintext1, plaintext1_length));
  ASSERT_FALSE(
      sha256_digest.retrieve_digest(digest_output, sizeof(digest_output)));
  ASSERT_TRUE(
      memcmp(expected_digest1, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);
  sha256_digest.scrub();

  const char *plaintext2 = "ABCD1234%^&*";
  size_t plaintext2_length = strlen(plaintext2);

  unsigned char expected_digest2[] = {
      0xdd, 0xaf, 0x2b, 0xa0, 0x31, 0x73, 0x45, 0x3d, 0x72, 0x34, 0x51,
      0x27, 0xeb, 0x32, 0xb5, 0x24, 0x00, 0xc8, 0xf0, 0x89, 0xcb, 0x8a,
      0xd6, 0xce, 0x3c, 0x67, 0x37, 0xac, 0x01, 0x45, 0x84, 0x09};
  ASSERT_TRUE(sha256_digest.all_ok());
  ASSERT_FALSE(sha256_digest.update_digest(plaintext2, plaintext2_length));
  ASSERT_FALSE(
      sha256_digest.retrieve_digest(digest_output, sizeof(digest_output)));
  ASSERT_TRUE(
      memcmp(expected_digest2, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);

  sha256_digest.scrub();
}

TEST(SHA256_digest_test, digest_multi_stage) {
  SHA256_digest sha256_digest;
  const char *plaintext_input1 = "quick brown fox jumped over the lazy dog";
  size_t plaintext_input1_length = strlen(plaintext_input1);
  const char *plaintext_input2 = "ABCD1234%^&*";
  size_t plaintext_input2_length = strlen(plaintext_input2);
  const char *plaintext_input3 = "Hello World";
  size_t plaintext_input3_length = strlen(plaintext_input3);

  unsigned char digest_output[CACHING_SHA2_DIGEST_LENGTH];

  unsigned char expected_digest1[] = {
      0xd5, 0x66, 0x19, 0xff, 0x7c, 0xdb, 0x13, 0x00, 0x0b, 0x57, 0x1c,
      0x33, 0x8f, 0xe3, 0x33, 0xaf, 0x41, 0x87, 0xa6, 0x83, 0x03, 0x31,
      0x1b, 0xb6, 0x64, 0x7b, 0x6f, 0xbe, 0x6e, 0xf0, 0x99, 0xd9};

  ASSERT_TRUE(sha256_digest.all_ok());
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input1, plaintext_input1_length));
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input2, plaintext_input2_length));
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input3, plaintext_input3_length));
  ASSERT_FALSE(
      sha256_digest.retrieve_digest(digest_output, sizeof(digest_output)));
  ASSERT_TRUE(
      memcmp(expected_digest1, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);
  sha256_digest.scrub();

  unsigned char expected_digest2[] = {
      0xd0, 0xcf, 0xa1, 0xd2, 0x9f, 0xb6, 0xfe, 0x8f, 0x3c, 0xff, 0x2c,
      0xa8, 0x2a, 0xe2, 0xc6, 0x6b, 0x8b, 0x2b, 0x33, 0xf9, 0x38, 0x35,
      0xc7, 0xae, 0x6a, 0xfc, 0x28, 0x85, 0x5d, 0xd3, 0xe5, 0xc5};
  ASSERT_TRUE(sha256_digest.all_ok());
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input1, plaintext_input1_length));
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input3, plaintext_input3_length));
  ASSERT_FALSE(
      sha256_digest.update_digest(plaintext_input2, plaintext_input2_length));
  ASSERT_FALSE(
      sha256_digest.retrieve_digest(digest_output, sizeof(digest_output)));
  ASSERT_TRUE(
      memcmp(expected_digest2, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);

  sha256_digest.scrub();
}

TEST(SHA256_digest_test, generate_scramble) {
  std::string source = "Ab12#$Cd56&*";
  std::string rnd = "eF!@34gH%^78";
  unsigned char scramble[CACHING_SHA2_DIGEST_LENGTH];

  Generate_scramble generate_scramble(source, rnd);

  unsigned char expected_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  ASSERT_FALSE(generate_scramble.scramble(scramble, sizeof(scramble)));
  ASSERT_TRUE(
      memcmp(expected_scramble1, scramble, CACHING_SHA2_DIGEST_LENGTH) == 0);
}

TEST(SHA256_digest_test, generate_sha256_scramble) {
  std::string source = "Ab12#$Cd56&*";
  std::string rnd = "eF!@34gH%^78";

  unsigned char scramble[CACHING_SHA2_DIGEST_LENGTH];

  unsigned char expected_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  ASSERT_FALSE(generate_sha256_scramble(scramble, CACHING_SHA2_DIGEST_LENGTH,
                                        source.c_str(), source.length(),
                                        rnd.c_str(), rnd.length()));

  ASSERT_TRUE(
      memcmp(expected_scramble1, scramble, CACHING_SHA2_DIGEST_LENGTH) == 0);
}

}  // namespace sha256_scramble_unittest
}  // namespace xcl
