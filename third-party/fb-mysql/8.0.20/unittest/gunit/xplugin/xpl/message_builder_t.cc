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

#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>

#include "plugin/x/ngs/include/ngs/protocol/column_info_builder.h"
#include "plugin/x/ngs/include/ngs/protocol/protocol_protobuf.h"
#include "plugin/x/protocol/encoders/encoding_xmessages.h"
#include "unittest/gunit/xplugin/xpl/protobuf_message.h"

namespace xpl {

namespace test {

template <typename T>
class Message_builder_encode_resultset : public ::testing::Test {
 public:
  std::string get_data_from_buffer() const {
    std::string result;

    auto page = m_buffer.m_front;

    while (page && page->get_used_bytes()) {
      const std::string data_on_page(
          reinterpret_cast<char *>(page->m_begin_data), page->get_used_bytes());

      result += data_on_page;
      page = page->m_next_page;
    }

    return result;
  }

  ngs::Memory_block_pool m_memory_block_pool{{10, k_minimum_page_size}};
  protocol::Encoding_pool m_pool{0, &m_memory_block_pool};
  protocol::Encoding_buffer m_buffer{&m_pool};
  protocol::XMessage_encoder m_encoder{&m_buffer};
};

template <typename ResultsetT, Mysqlx::ServerMessages::Type MessageId>
struct Resultset_pair_type {
  using ResultsetType = ResultsetT;
  static constexpr uint32_t message_id = MessageId;
};

using Resultset_types = ::testing::Types<
    Resultset_pair_type<Mysqlx::Sql::StmtExecuteOk, Mysqlx::ServerMessages::OK>,
    Resultset_pair_type<Mysqlx::Resultset::FetchDone,
                        Mysqlx::ServerMessages::RESULTSET_FETCH_DONE>,
    Resultset_pair_type<
        Mysqlx::Resultset::FetchDoneMoreResultsets,
        Mysqlx::ServerMessages::RESULTSET_FETCH_DONE_MORE_RESULTSETS>,
    Resultset_pair_type<
        Mysqlx::Resultset::FetchDoneMoreOutParams,
        Mysqlx::ServerMessages::RESULTSET_FETCH_DONE_MORE_OUT_PARAMS>,
    Resultset_pair_type<Mysqlx::Resultset::FetchSuspended,
                        Mysqlx::ServerMessages::RESULTSET_FETCH_SUSPENDED>>;

TYPED_TEST_CASE(Message_builder_encode_resultset, Resultset_types);

TYPED_TEST(Message_builder_encode_resultset, encode_resultset) {
  this->m_encoder.template empty_xmessage<TypeParam::message_id>();

  std::unique_ptr<typename TypeParam::ResultsetType> msg(
      message_with_header_from_buffer<typename TypeParam::ResultsetType>(
          this->get_data_from_buffer()));

  ASSERT_TRUE(nullptr != msg);
  ASSERT_TRUE(msg->IsInitialized());
}

namespace details {

const char k_first_character = 'a';
const char k_last_character = 'z';
const int k_character_distance = k_first_character - k_last_character;

std::string generate_long_string(const int length) {
  std::string result(length, ' ');
  int gen_state = 0;

  std::generate(result.begin(), result.end(), [&gen_state]() {
    return (gen_state++ % k_character_distance) + k_first_character;
  });

  return result;
}

}  // namespace details

class Message_builder : public Message_builder_encode_resultset<void> {};

TEST_F(Message_builder, encode_compact_metadata) {
  const uint64_t COLLATION = 1u;
  const auto TYPE = Mysqlx::Resultset::ColumnMetaData::SINT;
  const int DECIMALS = 3;
  const uint32_t FLAGS = 0xabcdu;
  const uint32_t LENGTH = 20u;
  const uint32_t CONTENT_TYPE = 7u;

  ::ngs::Column_info_builder column_info;

  column_info.set_collation(COLLATION);
  column_info.set_decimals(DECIMALS);
  column_info.set_flags(FLAGS);
  column_info.set_length(LENGTH);
  column_info.set_type(TYPE);
  column_info.set_content_type(CONTENT_TYPE);
  m_encoder.encode_metadata(column_info.get());

  std::string data = get_data_from_buffer();
  std::unique_ptr<Mysqlx::Resultset::ColumnMetaData> msg(
      message_with_header_from_buffer<Mysqlx::Resultset::ColumnMetaData>(data));

  ASSERT_TRUE(nullptr != msg);

  ASSERT_TRUE(msg->has_collation());
  ASSERT_EQ(COLLATION, msg->collation());
  ASSERT_TRUE(msg->has_type());
  ASSERT_EQ(TYPE, msg->type());
  ASSERT_TRUE(msg->has_fractional_digits());
  ASSERT_EQ(DECIMALS, msg->fractional_digits());
  ASSERT_TRUE(msg->has_flags());
  ASSERT_EQ(FLAGS, msg->flags());
  ASSERT_TRUE(msg->has_length());
  ASSERT_EQ(LENGTH, msg->length());
  ASSERT_TRUE(msg->has_content_type());
  ASSERT_EQ(CONTENT_TYPE, msg->content_type());

  ASSERT_FALSE(msg->has_catalog());
  ASSERT_FALSE(msg->has_name());
  ASSERT_FALSE(msg->has_original_name());
  ASSERT_FALSE(msg->has_original_table());
  ASSERT_FALSE(msg->has_schema());
  ASSERT_FALSE(msg->has_table());
}

TEST_F(Message_builder, encode_full_metadata) {
  const uint64_t COLLATION = 2u;
  const auto TYPE = Mysqlx::Resultset::ColumnMetaData::BYTES;
  const int DECIMALS = 4;
  const uint32_t FLAGS = 0x89abu;
  const uint32_t LENGTH = 0u;
  const uint32_t CONTENT_TYPE = 1u;
  const std::string CATALOG = "CATALOG_NAME";
  const std::string TABLE_NAME = "TABLE_NAME";
  const std::string ORG_TABLE_NAME = "ORG_TABLE_NAME";
  const std::string SCHEMA = "SCHEMA_NAME";
  const std::string COLUM_NAME = "COLUMN_NAME";
  const std::string ORG_COLUM_NAME = "ORG_COLUMN_NAME";

  ::ngs::Column_info_builder column_info;

  column_info.set_non_compact_data(
      CATALOG.c_str(), COLUM_NAME.c_str(), TABLE_NAME.c_str(), SCHEMA.c_str(),
      ORG_COLUM_NAME.c_str(), ORG_TABLE_NAME.c_str());
  column_info.set_collation(COLLATION);
  column_info.set_decimals(DECIMALS);
  column_info.set_flags(FLAGS);
  column_info.set_length(LENGTH);
  column_info.set_type(TYPE);
  column_info.set_content_type(CONTENT_TYPE);

  m_encoder.encode_metadata(column_info.get());

  std::string data = get_data_from_buffer();
  std::unique_ptr<Mysqlx::Resultset::ColumnMetaData> msg(
      message_with_header_from_buffer<Mysqlx::Resultset::ColumnMetaData>(data));

  ASSERT_TRUE(nullptr != msg);

  ASSERT_TRUE(msg->has_collation());
  ASSERT_EQ(COLLATION, msg->collation());
  ASSERT_TRUE(msg->has_type());
  ASSERT_EQ(TYPE, msg->type());
  ASSERT_TRUE(msg->has_fractional_digits());
  ASSERT_EQ(DECIMALS, msg->fractional_digits());
  ASSERT_TRUE(msg->has_flags());
  ASSERT_EQ(FLAGS, msg->flags());
  ASSERT_TRUE(msg->has_length());
  ASSERT_EQ(LENGTH, msg->length());
  ASSERT_TRUE(msg->has_content_type());
  ASSERT_EQ(CONTENT_TYPE, msg->content_type());
  ASSERT_TRUE(msg->has_catalog());
  ASSERT_EQ(CATALOG, msg->catalog());
  ASSERT_TRUE(msg->has_name());
  ASSERT_EQ(COLUM_NAME, msg->name());
  ASSERT_TRUE(msg->has_original_name());
  ASSERT_EQ(ORG_COLUM_NAME, msg->original_name());
  ASSERT_TRUE(msg->has_original_table());
  ASSERT_EQ(ORG_TABLE_NAME, msg->original_table());
  ASSERT_TRUE(msg->has_schema());
  ASSERT_EQ(SCHEMA, msg->schema());
  ASSERT_TRUE(msg->has_table());
  ASSERT_EQ(TABLE_NAME, msg->table());
}

TEST_F(Message_builder, encode_notice_with_text) {
  const std::string k_expected_string = details::generate_long_string(1000);

  m_encoder.encode_notice_text_message(k_expected_string);

  std::string data = get_data_from_buffer();
  std::unique_ptr<Mysqlx::Notice::Frame> msg(
      message_with_header_from_buffer<Mysqlx::Notice::Frame>(data));

  std::unique_ptr<Mysqlx::Notice::SessionStateChanged> state_changed(
      message_from_buffer<Mysqlx::Notice::SessionStateChanged>(msg->payload()));

  ASSERT_EQ(Mysqlx::Notice::Frame_Type_SESSION_STATE_CHANGED, msg->type());
  ASSERT_EQ(Mysqlx::Notice::Frame_Scope_LOCAL, msg->scope());
  ASSERT_EQ(Mysqlx::Notice::SessionStateChanged_Parameter_PRODUCED_MESSAGE,
            state_changed->param());

  ASSERT_EQ(1, state_changed->value_size());
  ASSERT_EQ(Mysqlx::Datatypes::Scalar_Type::Scalar_Type_V_STRING,
            state_changed->value(0).type());
  ASSERT_EQ(k_expected_string, state_changed->value(0).v_string().value());
}

TEST_F(Message_builder, encode_notice_frame) {
  const uint32_t TYPE = 2;
  const int SCOPE = Mysqlx::Notice::Frame_Scope_GLOBAL;
  const std::string DATA = "\0\0\1\12\12aaa\0";

  m_encoder.encode_notice(TYPE, SCOPE, DATA);

  std::unique_ptr<Mysqlx::Notice::Frame> msg(
      message_with_header_from_buffer<Mysqlx::Notice::Frame>(
          get_data_from_buffer()));

  ASSERT_TRUE(nullptr != msg);

  ASSERT_TRUE(msg->has_type());
  ASSERT_EQ(TYPE, msg->type());
  ASSERT_TRUE(msg->has_scope());
  ASSERT_EQ(SCOPE, msg->scope());
  ASSERT_TRUE(msg->has_payload());
  ASSERT_EQ(DATA, msg->payload());
}

TEST_F(Message_builder, encode_notice_rows_affected) {
  const uint64_t ROWS_AFFECTED = 10001u;

  m_encoder.encode_notice_rows_affected(ROWS_AFFECTED);

  std::unique_ptr<Mysqlx::Notice::Frame> msg(
      message_with_header_from_buffer<Mysqlx::Notice::Frame>(
          get_data_from_buffer()));

  ASSERT_TRUE(nullptr != msg);

  ASSERT_TRUE(msg->has_type());
  ASSERT_EQ(3, msg->type()); /*Mysqlx::Notice::SessionStateChanged*/
  ASSERT_TRUE(msg->has_scope());
  ASSERT_EQ(Mysqlx::Notice::Frame_Scope_LOCAL, msg->scope());
  ASSERT_TRUE(msg->has_payload());

  Mysqlx::Notice::SessionStateChanged change;
  change.ParseFromString(msg->payload());

  ASSERT_EQ(Mysqlx::Notice::SessionStateChanged::ROWS_AFFECTED, change.param());
  ASSERT_EQ(Mysqlx::Datatypes::Scalar::V_UINT, change.value(0).type());
  ASSERT_EQ(ROWS_AFFECTED, change.value(0).v_unsigned_int());
}

}  // namespace test

}  // namespace xpl
