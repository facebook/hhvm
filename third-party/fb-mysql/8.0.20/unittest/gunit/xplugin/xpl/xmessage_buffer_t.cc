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

#include "plugin/x/ngs/include/ngs/protocol/protocol_protobuf.h"
#include "plugin/x/protocol/encoders/encoding_xmessages.h"
#include "unittest/gunit/xplugin/xpl/encoder_validator.h"
#include "unittest/gunit/xplugin/xpl/protobuf_message.h"

namespace protocol {

namespace test {

class Encoder_validator_testsuite : public ::testing::Test {
 public:
  protocol::XMessage_encoder_base<Encoder_validator> m_encoder;
};

TEST_F(Encoder_validator_testsuite, encode_full_metadata) {
  ::ngs::Encode_column_info column_info;
  column_info.m_compact = false;

  m_encoder.encode_metadata(&column_info);
}

TEST_F(Encoder_validator_testsuite, encode_compact_metadata) {
  ::ngs::Encode_column_info column_info;

  m_encoder.encode_metadata(&column_info);
}

TEST_F(Encoder_validator_testsuite, encode_compact_metadata_multiple_params) {
  m_encoder.encode_compact_metadata(0, nullptr, nullptr, nullptr, nullptr,
                                    nullptr);
}

TEST_F(Encoder_validator_testsuite, encode_full_metadata_multiple_params) {
  m_encoder.encode_full_metadata("", "", "", "", "", "", 0, nullptr, nullptr,
                                 nullptr, nullptr, nullptr);
}

TEST_F(Encoder_validator_testsuite, encode_notice_row_affected) {
  m_encoder.encode_notice_rows_affected(0);
}

TEST_F(Encoder_validator_testsuite, encode_notice_client_id) {
  m_encoder.encode_notice_client_id(0);
}

TEST_F(Encoder_validator_testsuite, encode_notice_expired) {
  m_encoder.encode_notice_expired();
}

TEST_F(Encoder_validator_testsuite, encode_notice_generated_insert_id) {
  m_encoder.encode_notice_generated_insert_id(0);
}

TEST_F(Encoder_validator_testsuite, encode_notice_text_message) {
  m_encoder.encode_notice_text_message("");
}

TEST_F(Encoder_validator_testsuite, encode_notice) {
  m_encoder.encode_notice(0, 0, "");
}

TEST_F(Encoder_validator_testsuite, encode_global_notice) {
  m_encoder.encode_global_notice(0, "");
}

TEST_F(Encoder_validator_testsuite, encode_fetch_more_resultsets) {
  m_encoder.encode_fetch_more_resultsets();
}

TEST_F(Encoder_validator_testsuite, encode_fetch_out_params) {
  m_encoder.encode_fetch_out_params();
}

TEST_F(Encoder_validator_testsuite, encode_fetch_suspended) {
  m_encoder.encode_fetch_suspended();
}

TEST_F(Encoder_validator_testsuite, encode_fetch_done) {
  m_encoder.encode_fetch_done();
}

TEST_F(Encoder_validator_testsuite, encode_stmt_execute_ok) {
  m_encoder.encode_stmt_execute_ok();
}

TEST_F(Encoder_validator_testsuite, encode_ok) { m_encoder.encode_ok(); }

TEST_F(Encoder_validator_testsuite, encode_ok_with_param) {
  m_encoder.encode_ok("");
}

TEST_F(Encoder_validator_testsuite, encode_error) {
  m_encoder.encode_error(0, 0, "", "");
}

TEST_F(Encoder_validator_testsuite, encode_xmessage) {
  m_encoder.encode_xmessage<1>("");
}

}  // namespace test

}  // namespace protocol
