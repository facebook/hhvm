/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mysql/plugin_keyring.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "mysqld_error.h"
#include "plugin/keyring/buffered_file_io.h"
#include "plugin/keyring/common/keyring_key.h"
#include "sql/sql_plugin_ref.h"
#include "unittest/gunit/keyring/mock_logger.h"

#if defined(HAVE_PSI_INTERFACE)
namespace keyring {
extern PSI_file_key keyring_file_data_key;
extern PSI_file_key keyring_backup_file_data_key;
}  // namespace keyring
#endif

namespace keyring_buffered_file_io_unittest {
using namespace keyring;
using ::testing::StrEq;

class Buffered_file_io_test : public ::testing::Test {
 protected:
  virtual void SetUp() {
    keyring_file_data_key = PSI_NOT_INSTRUMENTED;
    keyring_backup_file_data_key = PSI_NOT_INSTRUMENTED;
    logger = new Mock_logger;
  }

  virtual void TearDown() {
    fake_mysql_plugin.name.str = const_cast<char *>("FakeKeyringPlugin");
    fake_mysql_plugin.name.length = strlen("FakeKeyringPlugin");
    delete logger;
  }

 protected:
  st_plugin_int fake_mysql_plugin;
  Mock_logger *logger;
};

TEST_F(Buffered_file_io_test, InitWithNotExisitingKeyringFile) {
  std::string file_name("./some_funny_name");
  Buffered_file_io buffered_io(logger);
  remove(file_name.c_str());
  EXPECT_EQ(buffered_io.init(&file_name), 0);
  ISerialized_object *serialized_object = nullptr;

  EXPECT_EQ(buffered_io.get_serialized_object(&serialized_object), 0);
  // The keyring file is new so no keys should be available
  ASSERT_TRUE(serialized_object == nullptr);

  remove(file_name.c_str());
}

TEST_F(Buffered_file_io_test, WriteAndFetchKey) {
  std::string file_name("./write_key");
  Buffered_file_io *buffered_io = new Buffered_file_io(logger);
  remove(file_name.c_str());
  EXPECT_EQ(buffered_io->init(&file_name), 0);
  std::string sample_key_data;

  Key key_to_add("Robert_add_key", "AES", "Roberts_add_key_type",
                 sample_key_data.c_str(), sample_key_data.length() + 1);

  Buffer *empty_serialized_object = new Buffer;
  empty_serialized_object->set_key_operation(NONE);
  Buffer *serialized_object_with_key_to_add =
      new Buffer(key_to_add.get_key_pod_size());
  key_to_add.store_in_buffer(serialized_object_with_key_to_add->data,
                             &(serialized_object_with_key_to_add->position));
  serialized_object_with_key_to_add->position = 0;  // rewind buffer
  serialized_object_with_key_to_add->set_key_operation(STORE_KEY);

  EXPECT_EQ(buffered_io->flush_to_backup(empty_serialized_object), 0);
  // flush to keyring expects backup file to exist
  EXPECT_EQ(buffered_io->flush_to_storage(serialized_object_with_key_to_add),
            0);
  delete buffered_io;
  delete empty_serialized_object;
  delete serialized_object_with_key_to_add;

  Buffered_file_io *buffered_io_2 = new Buffered_file_io(logger);
  buffered_io_2->init(&file_name);
  IKey *retrieved_key = nullptr;
  IKey *retrieved_key2 = nullptr;
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_EQ(buffered_io_2->get_serialized_object(&serialized_keys), 0);
  ASSERT_TRUE(serialized_keys != nullptr);

  EXPECT_EQ(serialized_keys->has_next_key(), true);
  EXPECT_EQ(serialized_keys->get_next_key(&retrieved_key), false);
  EXPECT_EQ(serialized_keys->has_next_key(), false);
  EXPECT_EQ(serialized_keys->get_next_key(&retrieved_key2), true);
  ASSERT_TRUE(retrieved_key2 == nullptr);

  EXPECT_STREQ("Robert_add_keyRoberts_add_key_type",
               retrieved_key->get_key_signature()->c_str());
  EXPECT_EQ(strlen("Robert_add_keyRoberts_add_key_type"),
            retrieved_key->get_key_signature()->length());

  uchar *retrieved_key_data = retrieved_key->get_key_data();
  size_t retrieved_key_data_size = retrieved_key->get_key_data_size();
  EXPECT_EQ(retrieved_key_data_size, sample_key_data.length() + 1);
  EXPECT_EQ(memcmp(retrieved_key_data, sample_key_data.c_str(),
                   retrieved_key_data_size),
            0);

  delete retrieved_key;
  delete serialized_keys;
  delete buffered_io_2;
  remove(file_name.c_str());
}

TEST_F(Buffered_file_io_test, Write2KeysAndFetchKeys) {
  std::string file_name("./write_key");
  Buffered_file_io *buffered_io = new Buffered_file_io(logger);
  remove(file_name.c_str());
  EXPECT_EQ(buffered_io->init(&file_name), 0);
  std::string sample_key_data1("Robi1");
  std::string sample_key_data2("Robi2");

  Key key_to_add1("Robert_add_key1", "AES", "Roberts_add_key1_type",
                  sample_key_data1.c_str(), sample_key_data1.length() + 1);
  Key key_to_add2("Robert_add_key2", "AES", "Roberts_add_key2_type",
                  sample_key_data2.c_str(), sample_key_data2.length() + 1);

  Buffer *empty_serialized_object = new Buffer;
  empty_serialized_object->set_key_operation(NONE);
  Buffer *serialized_object_with_keys_to_add = new Buffer(
      key_to_add1.get_key_pod_size() + key_to_add2.get_key_pod_size());
  key_to_add1.store_in_buffer(serialized_object_with_keys_to_add->data,
                              &(serialized_object_with_keys_to_add->position));
  key_to_add2.store_in_buffer(serialized_object_with_keys_to_add->data,
                              &(serialized_object_with_keys_to_add->position));
  serialized_object_with_keys_to_add->position = 0;  // rewind buffer
  serialized_object_with_keys_to_add->set_key_operation(STORE_KEY);

  EXPECT_EQ(buffered_io->flush_to_backup(empty_serialized_object), 0);
  // flush to keyring expects backup file to exist
  EXPECT_EQ(buffered_io->flush_to_storage(serialized_object_with_keys_to_add),
            0);
  delete buffered_io;
  delete empty_serialized_object;
  delete serialized_object_with_keys_to_add;

  Buffered_file_io *buffered_io_2 = new Buffered_file_io(logger);
  buffered_io_2->init(&file_name);
  IKey *retrieved_key1 = nullptr;
  IKey *retrieved_key2 = nullptr;
  IKey *retrieved_key3 = nullptr;
  ISerialized_object *serialized_keys = nullptr;
  EXPECT_EQ(buffered_io_2->get_serialized_object(&serialized_keys), 0);
  ASSERT_TRUE(serialized_keys != nullptr);

  EXPECT_EQ(serialized_keys->has_next_key(), true);
  EXPECT_EQ(serialized_keys->get_next_key(&retrieved_key1), false);
  ASSERT_TRUE(retrieved_key1 != nullptr);
  EXPECT_EQ(serialized_keys->has_next_key(), true);
  EXPECT_EQ(serialized_keys->get_next_key(&retrieved_key2), false);
  ASSERT_TRUE(retrieved_key2 != nullptr);
  EXPECT_EQ(serialized_keys->has_next_key(), false);
  EXPECT_EQ(serialized_keys->get_next_key(&retrieved_key3), true);
  ASSERT_TRUE(retrieved_key3 == nullptr);

  EXPECT_STREQ("Robert_add_key1Roberts_add_key1_type",
               retrieved_key1->get_key_signature()->c_str());
  EXPECT_EQ(strlen("Robert_add_key1Roberts_add_key1_type"),
            retrieved_key1->get_key_signature()->length());
  uchar *retrieved_key1_data = retrieved_key1->get_key_data();
  size_t retrieved_key1_data_size = retrieved_key1->get_key_data_size();
  EXPECT_EQ(retrieved_key1_data_size, sample_key_data1.length() + 1);
  EXPECT_EQ(memcmp(retrieved_key1_data, sample_key_data1.c_str(),
                   retrieved_key1_data_size),
            0);

  EXPECT_STREQ("Robert_add_key2Roberts_add_key2_type",
               retrieved_key2->get_key_signature()->c_str());
  EXPECT_EQ(strlen("Robert_add_key2Roberts_add_key2_type"),
            retrieved_key2->get_key_signature()->length());
  uchar *retrieved_key2_data = retrieved_key2->get_key_data();
  size_t retrieved_key2_data_size = retrieved_key2->get_key_data_size();
  EXPECT_EQ(retrieved_key2_data_size, sample_key_data2.length() + 1);
  EXPECT_EQ(memcmp(retrieved_key2_data, sample_key_data2.c_str(),
                   retrieved_key2_data_size),
            0);

  delete retrieved_key1;
  delete retrieved_key2;
  delete serialized_keys;
  delete buffered_io_2;
  remove(file_name.c_str());
}

TEST_F(Buffered_file_io_test,
       RemoveKeyringFileBetweenStoreToBackupAndStoreToKeyring) {
  std::string file_name("./sample_keyring");
  Buffered_file_io *buffered_io = new Buffered_file_io(logger);
  remove(file_name.c_str());
  EXPECT_EQ(buffered_io->init(&file_name), 0);
  std::string sample_key_data1("Robi1");
  std::string sample_key_data2("Robi2");

  Key key_to_add1("Robert_add_key1", "AES", "Roberts_add_key1_type",
                  sample_key_data1.c_str(), sample_key_data1.length() + 1);
  Key key_to_add2("Robert_add_key2", "AES", "Roberts_add_key2_type",
                  sample_key_data2.c_str(), sample_key_data2.length() + 1);

  Buffer *empty_serialized_object = new Buffer;
  empty_serialized_object->set_key_operation(NONE);
  Buffer *serialized_object_with_keys_to_add = new Buffer(
      key_to_add1.get_key_pod_size() + key_to_add2.get_key_pod_size());
  key_to_add1.store_in_buffer(serialized_object_with_keys_to_add->data,
                              &(serialized_object_with_keys_to_add->position));
  key_to_add2.store_in_buffer(serialized_object_with_keys_to_add->data,
                              &(serialized_object_with_keys_to_add->position));
  serialized_object_with_keys_to_add->position = 0;  // rewind buffer
  serialized_object_with_keys_to_add->set_key_operation(STORE_KEY);

  EXPECT_EQ(buffered_io->flush_to_backup(empty_serialized_object), 0);
  // flush to keyring expects backup file to exist
  EXPECT_EQ(buffered_io->flush_to_storage(serialized_object_with_keys_to_add),
            0);
  delete empty_serialized_object;

  std::string sample_key_data3("Robi3");
  Key key_to_add3("Robert_add_key3", "AES", "Roberts_add_key3_type",
                  sample_key_data3.c_str(), sample_key_data3.length() + 1);
  Buffer *serialized_object_with_key3_to_add =
      new Buffer(key_to_add3.get_key_pod_size());
  serialized_object_with_key3_to_add->position = 0;  // rewind buffer
  serialized_object_with_key3_to_add->set_key_operation(STORE_KEY);

  EXPECT_EQ(buffered_io->flush_to_backup(serialized_object_with_keys_to_add),
            0);
  remove("./sample_keyring");
  EXPECT_CALL(*logger, log(ERROR_LEVEL, StrEq("Incorrect Keyring file")));

  // keyring was removed between flush to backup and flush to storage operations
  // thus flush to storage should fail
  EXPECT_EQ(buffered_io->flush_to_storage(serialized_object_with_key3_to_add),
            1);

  delete serialized_object_with_keys_to_add;
  delete serialized_object_with_key3_to_add;
  delete buffered_io;
}
}  // namespace keyring_buffered_file_io_unittest
