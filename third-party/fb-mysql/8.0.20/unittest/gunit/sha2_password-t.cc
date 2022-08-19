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

#include "my_config.h"

#include <gtest/gtest.h>

#include "crypt_genhash_impl.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/auth/i_sha2_password.h"
#include "sql/auth/sha2_password_common.h"
#include "unittest/gunit/test_utils.h"

namespace sha2_password_unittest {
using namespace sha2_password;
using my_testing::Server_initializer;

class SHA256_digestTest : public ::testing::Test {
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

template <typename T>
void print_hex(T digest) {
  std::cout << std::endl << "Generated digest:";
  for (uint i = 0; i < CACHING_SHA2_DIGEST_LENGTH; i++)
    printf("0x%02x ", digest.str[i]);
  std::cout << std::endl;
}

static void make_hash_key(const char *username, const char *hostname,
                          std::string &key) {
  key.clear();
  key.append(username ? username : "");
  key.push_back('\0');
  key.append(hostname ? hostname : "");
  key.push_back('\0');
}

TEST_F(SHA256_digestTest, InitDigestContext) {
  SHA256_digest sha256_digest;
  ASSERT_TRUE(sha256_digest.all_ok() == true);
}

TEST_F(SHA256_digestTest, DigestSingleStage) {
  SHA256_digest sha256_digest;
  const char *plaintext1 = "quick brown fox jumped over the lazy dog";
  size_t plaintext1_length = strlen(plaintext1);
  unsigned char expected_digest1[] = {
      0x5c, 0xa1, 0xa2, 0x08, 0xc4, 0x61, 0x95, 0x3f, 0x7c, 0x12, 0xd2,
      0x79, 0xc7, 0xa8, 0x62, 0x0f, 0x3f, 0x83, 0x87, 0xf4, 0x09, 0x8c,
      0xe8, 0x53, 0x1b, 0x17, 0x51, 0x03, 0x37, 0xbd, 0x63, 0x78};

  unsigned char digest_output[CACHING_SHA2_DIGEST_LENGTH];

  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext1, plaintext1_length) ==
              false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_output,
                                            sizeof(digest_output)) == false);
  ASSERT_TRUE(
      memcmp(expected_digest1, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);
  sha256_digest.scrub();

  const char *plaintext2 = "ABCD1234%^&*";
  size_t plaintext2_length = strlen(plaintext2);

  unsigned char expected_digest2[] = {
      0xdd, 0xaf, 0x2b, 0xa0, 0x31, 0x73, 0x45, 0x3d, 0x72, 0x34, 0x51,
      0x27, 0xeb, 0x32, 0xb5, 0x24, 0x00, 0xc8, 0xf0, 0x89, 0xcb, 0x8a,
      0xd6, 0xce, 0x3c, 0x67, 0x37, 0xac, 0x01, 0x45, 0x84, 0x09};
  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext2, plaintext2_length) ==
              false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_output,
                                            sizeof(digest_output)) == false);
  ASSERT_TRUE(
      memcmp(expected_digest2, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);

  sha256_digest.scrub();
}

TEST_F(SHA256_digestTest, DigestMultiStage) {
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

  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input1,
                                          plaintext_input1_length) == false);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input2,
                                          plaintext_input2_length) == false);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input3,
                                          plaintext_input3_length) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_output,
                                            sizeof(digest_output)) == false);
  ASSERT_TRUE(
      memcmp(expected_digest1, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);
  sha256_digest.scrub();

  unsigned char expected_digest2[] = {
      0xd0, 0xcf, 0xa1, 0xd2, 0x9f, 0xb6, 0xfe, 0x8f, 0x3c, 0xff, 0x2c,
      0xa8, 0x2a, 0xe2, 0xc6, 0x6b, 0x8b, 0x2b, 0x33, 0xf9, 0x38, 0x35,
      0xc7, 0xae, 0x6a, 0xfc, 0x28, 0x85, 0x5d, 0xd3, 0xe5, 0xc5};
  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input1,
                                          plaintext_input1_length) == false);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input3,
                                          plaintext_input3_length) == false);
  ASSERT_TRUE(sha256_digest.update_digest(plaintext_input2,
                                          plaintext_input2_length) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_output,
                                            sizeof(digest_output)) == false);
  ASSERT_TRUE(
      memcmp(expected_digest2, digest_output, CACHING_SHA2_DIGEST_LENGTH) == 0);

  sha256_digest.scrub();
}

TEST_F(SHA256_digestTest, GenerateScramble) {
  std::string source = "Ab12#$Cd56&*";
  std::string rnd = "eF!@34gH%^78";
  unsigned char scramble[CACHING_SHA2_DIGEST_LENGTH];

  Generate_scramble generate_scramble(source, rnd);

  unsigned char expected_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  ASSERT_TRUE(generate_scramble.scramble(scramble, sizeof(scramble)) == false);
  ASSERT_TRUE(
      memcmp(expected_scramble1, scramble, CACHING_SHA2_DIGEST_LENGTH) == 0);
}

TEST_F(SHA256_digestTest, ValidateScramble) {
  SHA256_digest sha256_digest;

  unsigned char received_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  const char *source = "Ab12#$Cd56&*";
  size_t source_length = strlen(source);
  std::string rnd = "eF!@34gH%^78";

  unsigned char digest_stage2[CACHING_SHA2_DIGEST_LENGTH];
  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(source, source_length) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_stage2,
                                            sizeof(digest_stage2)) == false);
  sha256_digest.scrub();
  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(digest_stage2,
                                          sizeof(digest_stage2)) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(digest_stage2,
                                            sizeof(digest_stage2)) == false);
  sha256_digest.scrub();

  Validate_scramble validate_scramble(
      received_scramble1, digest_stage2,
      reinterpret_cast<const unsigned char *>(rnd.c_str()), rnd.length());

  ASSERT_TRUE(validate_scramble.validate() == false);
}

TEST_F(SHA256_digestTest, generate_sha256_scramble) {
  std::string source = "Ab12#$Cd56&*";
  std::string rnd = "eF!@34gH%^78";

  unsigned char scramble[CACHING_SHA2_DIGEST_LENGTH];

  unsigned char expected_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  ASSERT_TRUE(generate_sha256_scramble(scramble, CACHING_SHA2_DIGEST_LENGTH,
                                       source.c_str(), source.length(),
                                       rnd.c_str(), rnd.length()) == false);

  ASSERT_TRUE(
      memcmp(expected_scramble1, scramble, CACHING_SHA2_DIGEST_LENGTH) == 0);
}

TEST_F(SHA256_digestTest, validate_sha256_scramble) {
  SHA256_digest sha256_digest;

  unsigned char received_scramble1[] = {
      0x6a, 0x45, 0x37, 0x96, 0x6b, 0x29, 0x63, 0x59, 0x24, 0x8d, 0x64,
      0x86, 0x0a, 0xd6, 0xcc, 0x2a, 0x06, 0x47, 0x8c, 0x26, 0xea, 0xaa,
      0x3b, 0x02, 0x69, 0x4c, 0x85, 0x02, 0xf5, 0x5b, 0xc8, 0xdc};

  const char *source = "Ab12#$Cd56&*";
  size_t source_length = strlen(source);
  const unsigned char *rnd = (const unsigned char *)"eF!@34gH%^78";
  size_t rnd_length = strlen(pointer_cast<const char *>(rnd));

  unsigned char digest_stage2[CACHING_SHA2_DIGEST_LENGTH];

  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(source, source_length) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(
                  digest_stage2, CACHING_SHA2_DIGEST_LENGTH) == false);
  sha256_digest.scrub();
  ASSERT_TRUE(sha256_digest.all_ok() == true);
  ASSERT_TRUE(sha256_digest.update_digest(digest_stage2,
                                          CACHING_SHA2_DIGEST_LENGTH) == false);
  ASSERT_TRUE(sha256_digest.retrieve_digest(
                  digest_stage2, CACHING_SHA2_DIGEST_LENGTH) == false);
  sha256_digest.scrub();

  ASSERT_TRUE(validate_sha256_scramble(
                  received_scramble1, sizeof(received_scramble1), digest_stage2,
                  sizeof(digest_stage2), rnd, rnd_length) == false);
}

TEST_F(SHA256_digestTest, SHA2_password_cache) {
  SHA2_password_cache sha2_password_cache;

  std::string auth_id1;
  std::string auth_id1_u;
  std::string auth_id2;
  std::string auth_id3;
  std::string auth_no_user;
  std::string auth_no_host;
  std::string auth_empty;
  std::string auth_id1_v1;
  std::string auth_id1_v2;

  make_hash_key("arthur", "dent.com", auth_id1);
  make_hash_key("Arthur", "Dent.com", auth_id1_u);
  make_hash_key("marvin", "theparanoidandroid.com", auth_id2);
  make_hash_key("zaphod", "beeblebrox", auth_id3);
  make_hash_key("", "arthurdent.com", auth_no_user);
  make_hash_key("arthurdent.com", "", auth_no_host);
  make_hash_key("", "", auth_empty);

  sha2_cache_entry stored_hash;
  sha2_cache_entry retrieved_password;
  {
    SHA256_digest sha256_digest;

    std::string source("HahaH0hO1234#$@#%");

    ASSERT_TRUE(sha256_digest.all_ok() == true);
    ASSERT_TRUE(sha256_digest.update_digest(source.c_str(), source.length()) ==
                false);
    ASSERT_TRUE(sha256_digest.retrieve_digest(stored_hash.digest_buffer[0],
                                              CACHING_SHA2_DIGEST_LENGTH) ==
                false);
    sha256_digest.scrub();
    ASSERT_TRUE(sha256_digest.update_digest(stored_hash.digest_buffer[0],
                                            CACHING_SHA2_DIGEST_LENGTH) ==
                false);
    ASSERT_TRUE(sha256_digest.retrieve_digest(stored_hash.digest_buffer[0],
                                              CACHING_SHA2_DIGEST_LENGTH) ==
                false);
    sha256_digest.scrub();
    ASSERT_TRUE(sha256_digest.all_ok() == true);
  }

  ASSERT_TRUE(sha2_password_cache.size() == 0);
  ASSERT_TRUE(sha2_password_cache.add(auth_id1, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.search(auth_id1, retrieved_password) ==
              false);
  ASSERT_TRUE(memcmp((void *)stored_hash.digest_buffer[0],
                     (void *)retrieved_password.digest_buffer[0],
                     CACHING_SHA2_DIGEST_LENGTH) == 0);
  ASSERT_TRUE(sha2_password_cache.remove(auth_id1) == false);
  ASSERT_TRUE(sha2_password_cache.add(auth_id1, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.size() == 1);

  ASSERT_TRUE(sha2_password_cache.search(auth_no_user, retrieved_password) ==
              true);
  ASSERT_TRUE(sha2_password_cache.search(auth_no_host, retrieved_password) ==
              true);
  ASSERT_TRUE(sha2_password_cache.search(auth_empty, retrieved_password) ==
              true);

  ASSERT_TRUE(sha2_password_cache.add(auth_id1, stored_hash) == true);
  ASSERT_TRUE(sha2_password_cache.size() == 1);

  ASSERT_TRUE(sha2_password_cache.add(auth_id2, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.add(auth_id3, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.size() == 3);

  ASSERT_TRUE(sha2_password_cache.remove(auth_id2) == false);
  ASSERT_TRUE(sha2_password_cache.size() == 2);

  ASSERT_TRUE(sha2_password_cache.add(auth_id1_u, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.size() == 3);

  ASSERT_TRUE(sha2_password_cache.search(auth_id1, retrieved_password) ==
              false);
  ASSERT_TRUE(memcmp((void *)stored_hash.digest_buffer[0],
                     (void *)retrieved_password.digest_buffer[0],
                     CACHING_SHA2_DIGEST_LENGTH) == 0);

  ASSERT_TRUE(sha2_password_cache.search(auth_id3, retrieved_password) ==
              false);
  ASSERT_TRUE(memcmp((void *)stored_hash.digest_buffer[0],
                     (void *)retrieved_password.digest_buffer[0],
                     CACHING_SHA2_DIGEST_LENGTH) == 0);

  ASSERT_TRUE(sha2_password_cache.search(auth_id2, retrieved_password) == true);
  ASSERT_TRUE(sha2_password_cache.remove(auth_id2) == true);

  ASSERT_TRUE(sha2_password_cache.add(auth_empty, stored_hash) == false);
  ASSERT_TRUE(sha2_password_cache.search(auth_empty, retrieved_password) ==
              false);
  ASSERT_TRUE(memcmp((void *)stored_hash.digest_buffer[0],
                     (void *)retrieved_password.digest_buffer[0],
                     CACHING_SHA2_DIGEST_LENGTH) == 0);
  ASSERT_TRUE(sha2_password_cache.remove(auth_empty) == false);
}

TEST_F(SHA256_digestTest, Caching_sha2_password_Serialize_Deserialize) {
  Caching_sha2_password caching_sha2_password(nullptr);

  Digest_info digest_type;

  std::string sha256_digest;
  std::string salt;

  size_t iterations;

  /* Case 1 : valid string */
  std::string sha256_valid_auth_string1(
      "$A$005$ABCDEFGHIJKLMNOPQRSTabcdefgh01234567ijklmnop89012345ABCDEFGH678");
  std::string sha256_valid_auth_string1_serialized;

  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_valid_auth_string1, digest_type, salt, sha256_digest,
                  iterations) == false);
  ASSERT_TRUE(digest_type == Digest_info::SHA256_DIGEST);
  ASSERT_TRUE(memcmp(salt.c_str(), "ABCDEFGHIJKLMNOPQRST", SALT_LENGTH) == 0);
  ASSERT_TRUE(memcmp(sha256_digest.c_str(),
                     "abcdefgh01234567ijklmnop89012345ABCDEFGH678",
                     STORED_SHA256_DIGEST_LENGTH) == 0);
  ASSERT_TRUE(iterations == 5000);

  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string1_serialized, digest_type, salt,
                  sha256_digest, iterations) == false);
  ASSERT_TRUE(sha256_valid_auth_string1 ==
              sha256_valid_auth_string1_serialized);

  /* Valid string with special characters */
  std::string sha256_valid_auth_string2(
      "$A$005$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*;[-=,+]']+*");
  std::string sha256_valid_auth_string2_serialized;

  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_valid_auth_string2, digest_type, salt, sha256_digest,
                  iterations) == false);
  ASSERT_TRUE(digest_type == Digest_info::SHA256_DIGEST);
  ASSERT_TRUE(memcmp(salt.c_str(), "01234567ABCDEFGH!@#$", SALT_LENGTH) == 0);
  ASSERT_TRUE(memcmp(sha256_digest.c_str(),
                     "abcdefgh01234567ijklm890!@#$%^&*;[-=,+]']+*",
                     STORED_SHA256_DIGEST_LENGTH) == 0);
  ASSERT_TRUE(iterations == 5000);

  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == false);
  ASSERT_TRUE(sha256_valid_auth_string2 ==
              sha256_valid_auth_string2_serialized);

  /* Invalid string with incorrect digest type/length */
  std::string sha256_invalid_digest_type(
      "$B$005$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_digest_type, digest_type, salt, sha256_digest,
                  iterations) == true);
  sha256_invalid_digest_type.assign(
      "$HOHO$005$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-"
      "4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_digest_type, digest_type, salt, sha256_digest,
                  iterations) == true);

  digest_type = Digest_info::DIGEST_LAST;
  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == true);
  digest_type = Digest_info::SHA256_DIGEST;

  /* Invalid string with incorrect iteration count */
  std::string sha256_invalid_iteration(
      "$A$0$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_iteration, digest_type, salt, sha256_digest,
                  iterations) == true);
  sha256_invalid_iteration.assign(
      "$A$000$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_iteration, digest_type, salt, sha256_digest,
                  iterations) == true);
  sha256_invalid_iteration.assign(
      "$A$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_iteration, digest_type, salt, sha256_digest,
                  iterations) == true);
  sha256_invalid_iteration.assign(
      "$A$$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&*a;3-6;2[6-4");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_iteration, digest_type, salt, sha256_digest,
                  iterations) == true);
  iterations = 0;
  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == true);
  iterations = MAX_ITERATIONS + 1;
  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == true);
  iterations = 5000;
  /* Invalid string with incorrect salt length */
  std::string sha256_invalid_salt("$A$0005$01234567ABCDE");
  ASSERT_TRUE(
      caching_sha2_password.deserialize(sha256_invalid_salt, digest_type, salt,
                                        sha256_digest, iterations) == true);
  salt.assign("$A$0005$01234567ABCDEW$% CF#$BBF");
  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == true);
  salt.assign("");
  ASSERT_TRUE(caching_sha2_password.serialize(
                  sha256_valid_auth_string2_serialized, digest_type, salt,
                  sha256_digest, iterations) == true);
  salt.assign("ABCDEFGHIJKLMNOPQRST");
  /* Invalid string with incorrect digest length */
  std::string sha256_invalid_digest(
      "$A$0005$01234567ABCDEFGH!@#$abcdefgh01234567ijklm890!@#$%^&");
  ASSERT_TRUE(caching_sha2_password.deserialize(
                  sha256_invalid_digest, digest_type, salt, sha256_digest,
                  iterations) == true);
}

TEST_F(SHA256_digestTest, Caching_sha2_password_generate_fast_digest) {
  Caching_sha2_password caching_sha2_password(nullptr);
  Caching_sha2_password caching_sha2_password_4(
      nullptr, DEFAULT_STORED_DIGEST_ROUNDS, 4);

  std::string plaintext("HahaH0hO1234#$@#%");
  sha2_cache_entry digest;

  unsigned char expected_digest[] = {
      0x00, 0x3c, 0x20, 0x68, 0xb3, 0x6b, 0xc9, 0xb7, 0x18, 0x75, 0xb3,
      0x38, 0x7d, 0x2f, 0x86, 0xf9, 0xe3, 0x3a, 0xb3, 0x83, 0x11, 0x6b,
      0xd9, 0x57, 0xf1, 0x73, 0x46, 0x9b, 0x1f, 0x38, 0xf2, 0xdf};

  unsigned char expected_digest_4[] = {
      0x5c, 0x28, 0x83, 0xc3, 0x7a, 0xf0, 0xe1, 0x69, 0x2a, 0x03, 0xe2,
      0x91, 0x06, 0x4a, 0xf2, 0xa5, 0x05, 0x8b, 0x4a, 0xcd, 0x7d, 0x9c,
      0x90, 0xa5, 0x4b, 0x44, 0x45, 0xf5, 0xfc, 0x33, 0x53, 0xfa};

  ASSERT_TRUE(caching_sha2_password.generate_fast_digest(plaintext, digest,
                                                         0) == false);
  ASSERT_TRUE(memcmp(digest.digest_buffer[0], expected_digest,
                     CACHING_SHA2_DIGEST_LENGTH) == 0);

  ASSERT_TRUE(caching_sha2_password_4.generate_fast_digest(plaintext, digest,
                                                           0) == false);
  ASSERT_TRUE(memcmp(digest.digest_buffer[0], expected_digest_4,
                     CACHING_SHA2_DIGEST_LENGTH) == 0);

  plaintext.assign(";bCdEF34^i9&*\":({\\56\"");

  unsigned char expected_digest2[] = {
      0x36, 0x87, 0x98, 0x1f, 0xd5, 0x1f, 0xcb, 0x29, 0x66, 0x12, 0x71,
      0x51, 0x2c, 0x84, 0xc5, 0x7e, 0x9f, 0xca, 0x6a, 0xf1, 0xb9, 0x4d,
      0x33, 0x74, 0x8e, 0x38, 0xfe, 0x83, 0x8c, 0x45, 0x2c, 0x51};

  unsigned char expected_digest2_4[] = {
      0x8f, 0xb6, 0xbe, 0x4e, 0x03, 0xae, 0x07, 0xa2, 0x0c, 0x1d, 0x34,
      0x4c, 0x65, 0x8c, 0x98, 0xfb, 0x72, 0x0a, 0x10, 0xd3, 0x27, 0x91,
      0xf8, 0x73, 0x1b, 0xfc, 0x11, 0xb9, 0x28, 0x09, 0x32, 0x7a};

  ASSERT_TRUE(caching_sha2_password.generate_fast_digest(plaintext, digest,
                                                         0) == false);
  ASSERT_TRUE(memcmp(digest.digest_buffer[0], expected_digest2,
                     CACHING_SHA2_DIGEST_LENGTH) == 0);

  ASSERT_TRUE(caching_sha2_password_4.generate_fast_digest(plaintext, digest,
                                                           0) == false);
  ASSERT_TRUE(memcmp(digest.digest_buffer[0], expected_digest2_4,
                     CACHING_SHA2_DIGEST_LENGTH) == 0);
}

TEST_F(SHA256_digestTest, Caching_sha2_password_generate_sha2_multi_hash) {
  std::string auth_id_arthur("'arthur'@'dent.com'");
  std::string plaintext_buffer_arthur("HahaH0hO1234#$@#%");
  std::string salt_buffer_arthur("01234567899876543210");
  char stage2[CRYPT_MAX_PASSWORD_SIZE + 1];
  memset(stage2, 0, CRYPT_MAX_PASSWORD_SIZE + 1);
  my_crypt_genhash(
      stage2, CRYPT_MAX_PASSWORD_SIZE, plaintext_buffer_arthur.c_str(),
      plaintext_buffer_arthur.length(), salt_buffer_arthur.c_str(), nullptr);

  std::string digest_string1(stage2);
  digest_string1 = digest_string1.substr(
      digest_string1.find('$', 3 + CRYPT_SALT_LENGTH) + 1, std::string::npos);

  Caching_sha2_password caching_sha2_password(nullptr);
  std::string digest;
  ASSERT_TRUE(caching_sha2_password.generate_sha2_multi_hash(
                  plaintext_buffer_arthur, salt_buffer_arthur, &digest, 5000) ==
              false);
  ASSERT_TRUE(digest == digest_string1);

  /*Try with random salt */
  char random_salt_buffer[CRYPT_SALT_LENGTH + 1];
  generate_user_salt(random_salt_buffer, CRYPT_SALT_LENGTH + 1);
  salt_buffer_arthur.assign(random_salt_buffer, CRYPT_SALT_LENGTH);

  memset(stage2, 0, CRYPT_MAX_PASSWORD_SIZE + 1);
  my_crypt_genhash(
      stage2, CRYPT_MAX_PASSWORD_SIZE, plaintext_buffer_arthur.c_str(),
      plaintext_buffer_arthur.length(), salt_buffer_arthur.c_str(), nullptr);
  digest_string1.assign(stage2);
  digest_string1 = digest_string1.substr(
      digest_string1.find('$', 3 + CRYPT_SALT_LENGTH) + 1, std::string::npos);

  ASSERT_TRUE(caching_sha2_password.generate_sha2_multi_hash(
                  plaintext_buffer_arthur, salt_buffer_arthur, &digest, 5000) ==
              false);
  ASSERT_TRUE(digest == digest_string1);
}

TEST_F(SHA256_digestTest,
       Caching_sha2_password_authenticate_fast_authenticate) {
  Caching_sha2_password caching_sha2_password(nullptr);
  std::string serialized_string[MAX_PASSWORDS];
  Digest_info digest_type = Digest_info::SHA256_DIGEST;
  std::string digest;
  std::string salt;
  std::string plaintext;
  size_t iterations = 5000;

  /* Part 1 : Populate the cache */

  std::string auth_id_arthur("'arthur'@'dent.com'");
  const char plaintext_buffer_arthur[] = "HahaH0hO1234#$@#%";
  const char salt_buffer_arthur[] = "AbCd!@#	EfgH%^&*01@#";
  const char digest_buffer_arthur[] = {
      0x64, 0x4e, 0x46, 0x65, 0x67, 0x45, 0x61, 0x34, 0x4f, 0x47, 0x44,
      0x72, 0x6e, 0x45, 0x75, 0x37, 0x4d, 0x34, 0x54, 0x2e, 0x6e, 0x6b,
      0x6d, 0x6d, 0x37, 0x39, 0x34, 0x78, 0x49, 0x2f, 0x51, 0x6f, 0x43,
      0x52, 0x33, 0x5a, 0x2e, 0x78, 0x70, 0x4c, 0x4f, 0x57, 0x36};
  digest.assign(digest_buffer_arthur, STORED_SHA256_DIGEST_LENGTH);
  salt.assign(salt_buffer_arthur);
  plaintext.assign(plaintext_buffer_arthur);
  ASSERT_TRUE(caching_sha2_password.serialize(serialized_string[0], digest_type,
                                              salt, digest,
                                              iterations) == false);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == false);

  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 1);
  // Attempt again, it should pass
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == false);

  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 1);
  // Attempt with incorrect digest, it should fail
  const char digest_buffer_arthur_ic[] = {
      0x64, 0x4e, 0x46, 0x65, 0x67, 0x45, 0x61, 0x34, 0x4f, 0x47, 0x44,
      0x72, 0x6e, 0x45, 0x75, 0x37, 0x4d, 0x34, 0x54, 0x2e, 0x6e, 0x6b,
      0x6d, 0x6d, 0x37, 0x39, 0x34, 0x78, 0x49, 0x2f, 0x51, 0x6f, 0x43,
      0x52, 0x33, 0x5a, 0x2e, 0x78, 0x70, 0x4c, 0x4f, 0x57, 0x3a};

  digest.assign(digest_buffer_arthur_ic, STORED_SHA256_DIGEST_LENGTH);
  ASSERT_TRUE(caching_sha2_password.serialize(serialized_string[0], digest_type,
                                              salt, digest,
                                              iterations) == false);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == true);

  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 1);
  std::string auth_id_marvin("'marvin'@'theparanoidandroid.com'");
  const char plaintext_buffer_marvin[] = ";bCdEF34^i9&*\":({\\56\"";
  const char salt_buffer_marvin[] = "CVlq))+AC>Q)#4!@# x!";
  const char digest_buffer_marvin[] = {
      0x53, 0x50, 0x4d, 0x4e, 0x2f, 0x4c, 0x38, 0x2e, 0x6e, 0x6f, 0x4b,
      0x71, 0x65, 0x38, 0x49, 0x58, 0x56, 0x5a, 0x32, 0x44, 0x70, 0x43,
      0x45, 0x36, 0x77, 0x32, 0x39, 0x62, 0x31, 0x30, 0x36, 0x76, 0x43,
      0x51, 0x65, 0x63, 0x61, 0x70, 0x39, 0x4a, 0x4b, 0x70, 0x41};

  digest.assign(digest_buffer_marvin, STORED_SHA256_DIGEST_LENGTH);
  salt.assign(salt_buffer_marvin);
  plaintext.assign(plaintext_buffer_marvin);

  ASSERT_TRUE(caching_sha2_password.serialize(serialized_string[0], digest_type,
                                              salt, digest,
                                              iterations) == false);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_marvin, serialized_string, plaintext)
                  .first == false);
  // Attempt again, it should pass
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_marvin, serialized_string, plaintext)
                  .first == false);

  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 2);
  std::string auth_id_zaphod("'zaphod'@'beeblebrox'");
  const char plaintext_buffer_zaphod[] = " CQ#$ML CF%IF$#(<#R ()<_q@#(rq";
  const char salt_buffer_zaphod[] = "X#y)+q x2cx3,qr2##3	";
  const char digest_buffer_zaphod[] = {
      0x72, 0x50, 0x38, 0x4a, 0x6e, 0x38, 0x4d, 0x74, 0x70, 0x36, 0x58,
      0x43, 0x4d, 0x78, 0x31, 0x51, 0x6a, 0x53, 0x44, 0x41, 0x38, 0x4e,
      0x53, 0x61, 0x6c, 0x66, 0x56, 0x4d, 0x53, 0x42, 0x4e, 0x4d, 0x49,
      0x72, 0x32, 0x79, 0x71, 0x63, 0x45, 0x69, 0x36, 0x51, 0x38};

  digest.assign(digest_buffer_zaphod, STORED_SHA256_DIGEST_LENGTH);
  salt.assign(salt_buffer_zaphod);
  plaintext.assign(plaintext_buffer_zaphod);

  ASSERT_TRUE(caching_sha2_password.serialize(serialized_string[0], digest_type,
                                              salt, digest,
                                              iterations) == false);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_zaphod, serialized_string, plaintext)
                  .first == false);
  // Attempt again, it should pass
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_zaphod, serialized_string, plaintext)
                  .first == false);
  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 3);

  /* Part 2 : Perform fast authentication */
  std::string scramble_random_zaphod("CVOS)=M@)=*%!)#_[-(2");
  Generate_scramble generate_scramble(plaintext, scramble_random_zaphod);
  unsigned char scramble_zaphod[CACHING_SHA2_DIGEST_LENGTH];
  generate_scramble.scramble(scramble_zaphod, CACHING_SHA2_DIGEST_LENGTH);

  ASSERT_TRUE(caching_sha2_password
                  .fast_authenticate(auth_id_zaphod,
                                     reinterpret_cast<const unsigned char *>(
                                         scramble_random_zaphod.c_str()),
                                     scramble_random_zaphod.length(),
                                     scramble_zaphod, false)
                  .first == false);
  caching_sha2_password.remove_cached_entry(auth_id_zaphod);
  ASSERT_TRUE(caching_sha2_password
                  .fast_authenticate(auth_id_zaphod,
                                     reinterpret_cast<const unsigned char *>(
                                         scramble_random_zaphod.c_str()),
                                     scramble_random_zaphod.length(),
                                     scramble_zaphod, false)
                  .first == true);

  plaintext.assign(plaintext_buffer_arthur);
  char random_salt_buffer[CRYPT_SALT_LENGTH + 1];
  generate_user_salt(random_salt_buffer, CRYPT_SALT_LENGTH + 1);
  std::string scramble_random_arthur(random_salt_buffer);
  Generate_scramble generate_scramble_arthur(plaintext, scramble_random_arthur);
  unsigned char scramble_arthur[CACHING_SHA2_DIGEST_LENGTH];
  generate_scramble_arthur.scramble(scramble_arthur,
                                    CACHING_SHA2_DIGEST_LENGTH);

  ASSERT_TRUE(caching_sha2_password
                  .fast_authenticate(auth_id_arthur,
                                     reinterpret_cast<const unsigned char *>(
                                         scramble_random_arthur.c_str()),
                                     scramble_random_arthur.length(),
                                     scramble_arthur, false)
                  .first == false);

  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 2);
  caching_sha2_password.clear_cache();
  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 0);
}

TEST_F(SHA256_digestTest, Caching_sha2_password_authenticate_sanity) {
  Caching_sha2_password caching_sha2_password(nullptr);
  std::string serialized_string[MAX_PASSWORDS];
  std::string plaintext;

  std::string auth_id_arthur("'arthur'@'dent.com'");
  const char empty_plaintext_buffer_arthur[] = "";
  const char nonempty_plaintext_buffer_arthur[] = "HahaH0hO1234#$@#%";
  const char invalid_plaintext_buffer_arthur[] =
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

  plaintext.assign(empty_plaintext_buffer_arthur);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == false);
  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 0);

  plaintext.assign(nonempty_plaintext_buffer_arthur);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == true);
  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 0);

  plaintext.assign(invalid_plaintext_buffer_arthur);
  ASSERT_TRUE(caching_sha2_password
                  .authenticate(auth_id_arthur, serialized_string, plaintext)
                  .first == true);
  ASSERT_TRUE(caching_sha2_password.get_cache_count() == 0);
}
}  // namespace sha2_password_unittest
