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
#include <algorithm>
#include <cstdlib>
#include <cstring>  // std::{memset,strncmp}
#include <ctime>
#include <sstream>

#include "gcs_base_test.h"

#include "gcs_internal_message.h"
#include "gcs_message_stage_lz4.h"
#include "gcs_message_stage_split.h"
#include "gcs_message_stages.h"
#include "gcs_xcom_statistics_interface.h"
#include "mysql/gcs/xplatform/byteorder.h"

namespace gcs_xcom_stages_unittest {

class XcomStagesTest : public GcsBaseTest {
 protected:
  XcomStagesTest() {}

 public:
  static const unsigned long long LARGE_PAYLOAD_LEN;
  static const unsigned long long SMALL_PAYLOAD_LEN;
  Gcs_message_pipeline pipeline;
};

constexpr unsigned long long XcomStagesTest::LARGE_PAYLOAD_LEN =
    Gcs_message_stage_lz4::DEFAULT_THRESHOLD +
    Gcs_internal_message_header::WIRE_TOTAL_FIXED_HEADER_SIZE +
    WIRE_HEADER_LEN_SIZE + WIRE_PAYLOAD_LEN_SIZE;
constexpr unsigned long long XcomStagesTest::SMALL_PAYLOAD_LEN =
    Gcs_message_stage_lz4::DEFAULT_THRESHOLD -
    Gcs_internal_message_header::WIRE_TOTAL_FIXED_HEADER_SIZE -
    WIRE_HEADER_LEN_SIZE - WIRE_PAYLOAD_LEN_SIZE;

/*
 Create a message with a content whose size is smaller than the compression
 threshold and make it go through the compression stage and check that the
 content is not really compressed.
 */
TEST_F(XcomStagesTest, DoNotCompressMessage) {
  // Configure the pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4>(
      true, Gcs_message_stage_lz4::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V1, {Stage_code::ST_LZ4_V1}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V1);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size = SMALL_PAYLOAD_LEN;

  unsigned char control[payload_size];
  std::memset(&control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(reinterpret_cast<unsigned char *>(&control),
                             payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().size(), 0);

  Gcs_message_data msg_data_2(packet.get_payload_length());
  ASSERT_FALSE(msg_data_2.decode(packet.get_payload_pointer(),
                                 packet.get_payload_length()));
  ASSERT_EQ(
      std::strncmp(reinterpret_cast<char const *>(&control),
                   reinterpret_cast<char const *>(msg_data_2.get_payload()),
                   payload_size),
      0);
}

/**
 Create a message with a content whose size is greater than the compression
 threshold and make it go through the compression stage and check that
 the content is really compressed.
*/
TEST_F(XcomStagesTest, CompressDecompressMessage) {
  // Configure the pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4>(
      true, Gcs_message_stage_lz4::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V1, {Stage_code::ST_LZ4_V1}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V1);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size = LARGE_PAYLOAD_LEN;

  unsigned char control[payload_size];
  std::memset(control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(control, payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().size(), 1);
  ASSERT_EQ(packet.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V1);

  // Goes through the network, in imagination land...

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_size;
  std::tie(buffer, buffer_size) = packet.serialize();
  auto packet_from_network = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_size, null_synode, pipeline);

  Gcs_pipeline_incoming_result error_code;
  Gcs_packet packet_in;
  std::tie(error_code, packet_in) =
      pipeline.process_incoming(std::move(packet_from_network));
  ASSERT_EQ(error_code, Gcs_pipeline_incoming_result::OK_PACKET);

  ASSERT_EQ(packet_in.get_dynamic_headers().size(), 1);
  ASSERT_EQ(packet_in.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V1);

  Gcs_message_data msg_data_3(packet_in.get_payload_length());
  ASSERT_FALSE(msg_data_3.decode(packet_in.get_payload_pointer(),
                                 packet_in.get_payload_length()));
  ASSERT_EQ(
      std::strncmp(reinterpret_cast<char const *>(control),
                   reinterpret_cast<char const *>(msg_data_3.get_payload()),
                   payload_size),
      0);
}

/**
 Create a message with a content whose size is greater than the compression
 threshold and make it go through the compression stage and check that
 the content is really compressed.
 Then simulate a bit flip on the payload, and verify that the pipeline
 processing fails.
*/
TEST_F(XcomStagesTest, DecompressCorruptedPayload) {
  // Configure the pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4>(
      true, Gcs_message_stage_lz4::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V1, {Stage_code::ST_LZ4_V1}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V1);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size = LARGE_PAYLOAD_LEN;

  unsigned char control[payload_size];
  std::memset(control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(control, payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().size(), 1);
  ASSERT_EQ(packet.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V1);

  // Goes through the network, in imagination land...

  auto payload_length = packet.get_payload_length();
  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_size;
  std::tie(buffer, buffer_size) = packet.serialize();

  // Corrupt the payload by flipping its bits
  auto *raw_buffer = buffer.get();
  for (int offset_from_end = payload_length; offset_from_end > 0;
       offset_from_end--) {
    raw_buffer[buffer_size - offset_from_end] =
        ~raw_buffer[buffer_size - offset_from_end];
  }

  auto packet_from_network = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_size, null_synode, pipeline);

  Gcs_pipeline_incoming_result error_code;
  Gcs_packet packet_in;
  std::tie(error_code, packet_in) =
      pipeline.process_incoming(std::move(packet_from_network));
  ASSERT_EQ(error_code, Gcs_pipeline_incoming_result::ERROR);
}

class Mock_gcs_message_data : public Gcs_message_data {
 public:
  Mock_gcs_message_data(uint64_t const &encode_size)
      : encode_size_(encode_size) {}
  uint64_t get_encode_size() const override { return encode_size_; }

 private:
  uint64_t const encode_size_;
};

/**
 Verify whether the behaviour when we try to compress a packet whose size
 exceeds some limits: max_input_compression() and
 std::numeric_limits<int>::max().
 */
TEST_F(XcomStagesTest, CannotCompressPayloadTooBig) {
  // Configure the pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4>(
      true, Gcs_message_stage_lz4::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V1, {Stage_code::ST_LZ4_V1}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V1);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size =
      Gcs_message_stage_lz4::max_input_compression() + 1;

  Mock_gcs_message_data msg_data(payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_TRUE(error);

  /*
    Set payload's length bigger than uint32. apply() should return true and
    log an error when payload is bigger than uint32. It must be tested because
    length bigger than uint32 is  handled different from length in uint32 range.
    For detail, see the comment in gcs_message_stage_lz4::apply().
  */
  constexpr unsigned long long payload_size_2 = (1ULL << 32) + 1;

  Mock_gcs_message_data msg_data_2(payload_size_2);

  std::vector<Gcs_packet> packets_out_2;
  std::tie(error, packets_out_2) =
      pipeline.process_outgoing(msg_data_2, Cargo_type::CT_USER_DATA);
  ASSERT_TRUE(error);
}

/**
 Verify that we fail when we try to fragment a packet in a number of fragments
 that exceeds the limit: std::numeric_limits<unsigned int>::max().
 */
TEST_F(XcomStagesTest, CannotFragmentPayloadFragmentSizeTooSmall) {
  // Configure the pipeline with fragmentation and fragment size = 1.
  pipeline.register_stage<Gcs_message_stage_split_v2>(true, 1);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V2, {Stage_code::ST_SPLIT_V2}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size =
      std::numeric_limits<unsigned int>::max();

  Mock_gcs_message_data msg_data(payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_TRUE(error);
}

/**
 Verify that we fail when we receive a fragment from a sender who is not in the
 group.
 */
TEST_F(XcomStagesTest, ReceiveFragmentFromSenderNotInGroup) {
  /*
   Configure the pipeline with fragmentation.
   Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD is not a bug, it is to force two
   fragments using LARGE_PAYLOAD_LEN.
  */
  pipeline.register_stage<Gcs_message_stage_split_v2>(
      true, Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V2, {Stage_code::ST_SPLIT_V2}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size = LARGE_PAYLOAD_LEN;

  unsigned char control[payload_size];
  std::memset(control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(control, payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 2);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_SPLIT_V2);

  // Goes through the network, in imagination land...

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_size;
  std::tie(buffer, buffer_size) = packet.serialize();

  auto packet_from_network = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_size, null_synode, pipeline);

  Gcs_pipeline_incoming_result error_code;
  Gcs_packet packet_in;
  std::tie(error_code, packet_in) =
      pipeline.process_incoming(std::move(packet_from_network));
  ASSERT_EQ(error_code, Gcs_pipeline_incoming_result::ERROR);
}

/**
 Create a message with a content whose size is greater than the compression
 threshold and make it go through the compression stage and check that
 the content is really compressed.
 Then receive the packet via a pipeline that does not support compression and
 verify that we fail.
*/
TEST_F(XcomStagesTest, ReceivePacketWithUnknownStage) {
  // Configure the sender pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4_v2>(
      true, Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V2, {Stage_code::ST_LZ4_V2}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  constexpr unsigned long long payload_size = LARGE_PAYLOAD_LEN;

  unsigned char control[payload_size];
  std::memset(control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(control, payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().size(), 1);
  ASSERT_EQ(packet.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V2);

  // Goes through the network, in imagination land...

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_size;
  std::tie(buffer, buffer_size) = packet.serialize();

  auto packet_from_network = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_size, null_synode, pipeline);

  // Configure the receiver pipeline without compression.
  Gcs_message_pipeline receiver_pipeline;
  receiver_pipeline.register_stage<Gcs_message_stage_split_v2>(
      true, Gcs_message_stage_split_v2::DEFAULT_THRESHOLD);
  // clang-format off
  error = receiver_pipeline.register_pipeline({
    {Gcs_protocol_version::V2, {Stage_code::ST_SPLIT_V2}}
  });
  // clang-format on
  receiver_pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  Gcs_pipeline_incoming_result error_code;
  Gcs_packet packet_in;
  std::tie(error_code, packet_in) =
      receiver_pipeline.process_incoming(std::move(packet_from_network));
  ASSERT_EQ(error_code, Gcs_pipeline_incoming_result::ERROR);
}

/*
 New class that implements the Gcs_message_stage to test the multiple
 stages.
 */
class Gcs_new_stage_1 : public Gcs_message_stage {
 protected:
  virtual stage_status skip_apply(
      uint64_t const &original_payload_size) const override {
    bool result = (original_payload_size != 0);
    return result ? stage_status::apply : stage_status::abort;
  }

  virtual stage_status skip_revert(const Gcs_packet &packet) const override {
    bool result = (packet.get_payload_length() != 0);
    return result ? stage_status::apply : stage_status::abort;
  }

  virtual std::pair<bool, std::vector<Gcs_packet>> apply_transformation(
      Gcs_packet &&packet) override {
    int64_t id = htole64(get_id());

    auto const old_payload_length = packet.get_payload_length();
    auto *old_payload_pointer = packet.get_payload_pointer();

    auto const new_payload_length =
        packet.get_payload_length() + MESSAGE_ID_SIZE;
    bool packet_ok;
    Gcs_packet new_packet;
    std::tie(packet_ok, new_packet) =
        Gcs_packet::make_from_existing_packet(packet, new_payload_length);
    assert(packet_ok);
    auto *new_payload_pointer = new_packet.get_payload_pointer();

    std::memcpy(new_payload_pointer, &id, MESSAGE_ID_SIZE);

    std::memcpy(new_payload_pointer + MESSAGE_ID_SIZE, old_payload_pointer,
                old_payload_length);

    std::vector<Gcs_packet> packets_out;
    packets_out.push_back(std::move(new_packet));
    return std::make_pair(false, std::move(packets_out));
  }

  virtual std::pair<Gcs_pipeline_incoming_result, Gcs_packet>
  revert_transformation(Gcs_packet &&packet) override {
#ifndef NDEBUG
    auto const old_payload_length = packet.get_payload_length();
#endif
    auto *old_payload_pointer = packet.get_payload_pointer();

    auto const new_payload_length =
        packet.get_current_dynamic_header().get_payload_length();
    assert(new_payload_length == (old_payload_length - MESSAGE_ID_SIZE));
    bool packet_ok;
    Gcs_packet new_packet;
    std::tie(packet_ok, new_packet) =
        Gcs_packet::make_from_existing_packet(packet, new_payload_length);
    assert(packet_ok);
    auto *new_payload_pointer = new_packet.get_payload_pointer();

    int64_t id = 0;
    std::memcpy(&id, old_payload_pointer, MESSAGE_ID_SIZE);
    id = le64toh(id);
    assert(get_id() == id);

    std::memcpy(new_payload_pointer, old_payload_pointer + MESSAGE_ID_SIZE,
                new_payload_length);

    return std::make_pair(Gcs_pipeline_incoming_result::OK_PACKET,
                          std::move(new_packet));
  }

 private:
  int m_id{0};

  static const unsigned short MESSAGE_ID_SIZE{8};

  int64_t get_id() { return m_id; }

 public:
  explicit Gcs_new_stage_1() : m_id(std::rand()) {}

  virtual ~Gcs_new_stage_1() override {}

  virtual Stage_code get_stage_code() const override { return my_stage_code(); }

  static Stage_code my_stage_code() { return static_cast<Stage_code>(10); }

  std::unique_ptr<Gcs_stage_metadata> get_stage_header() override {
    return std::unique_ptr<Gcs_stage_metadata>(new Gcs_empty_stage_metadata());
  }
};

const unsigned short Gcs_new_stage_1::MESSAGE_ID_SIZE;

/*
 New class that implements the Gcs_message_stage to test the multiple
 stages.
 */
class Gcs_new_stage_2 : public Gcs_new_stage_1 {
 public:
  explicit Gcs_new_stage_2() {}

  virtual ~Gcs_new_stage_2() {}

  virtual Stage_code get_stage_code() const { return my_stage_code(); }

  static Stage_code my_stage_code() { return static_cast<Stage_code>(11); }
};

class Gcs_new_stage_3 : public Gcs_new_stage_1 {
 public:
  explicit Gcs_new_stage_3() {}

  virtual ~Gcs_new_stage_3() {}

  virtual Stage_code get_stage_code() const { return my_stage_code(); }

  static Stage_code my_stage_code() { return static_cast<Stage_code>(12); }
};

class Gcs_new_stage_split_4 : public Gcs_message_stage_split_v2 {
 public:
  explicit Gcs_new_stage_split_4(bool enabled, unsigned long long threshold)
      : Gcs_message_stage_split_v2(enabled, threshold) {}

  virtual ~Gcs_new_stage_split_4() {}

  virtual Stage_code get_stage_code() const { return my_stage_code(); }

  static Stage_code my_stage_code() { return static_cast<Stage_code>(13); }
};

class Gcs_new_stage_lz4_5 : public Gcs_message_stage_lz4 {
 public:
  explicit Gcs_new_stage_lz4_5(bool enable, unsigned long long threshold)
      : Gcs_message_stage_lz4(enable, threshold) {}

  virtual ~Gcs_new_stage_lz4_5() {}

  virtual Stage_code get_stage_code() const { return my_stage_code(); }

  static Stage_code my_stage_code() { return static_cast<Stage_code>(14); }
};

class XcomMultipleStagesTest : public GcsBaseTest {
 protected:
  XcomMultipleStagesTest() {}

  virtual void SetUp() {}

  virtual void TearDown() {}

 public:
  Gcs_message_pipeline pipeline{};
};

TEST_F(XcomMultipleStagesTest, MultipleStagesCheckConfigure) {
  /*
   The following configuration is perfectly fine as all stages have
   different type codes but it will fail because none of the stages
   were registered.
   */
  ASSERT_EQ(
      pipeline.register_pipeline(
          {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
           {Gcs_protocol_version::V2, {Gcs_new_stage_2::my_stage_code()}},
           {Gcs_protocol_version::V3, {Gcs_new_stage_3::my_stage_code()}}}),
      true);

  pipeline.register_stage<Gcs_new_stage_1>();

  /*
   The following configuration is perfectly fine as all stages have
   different type codes but it will fail because there are stages
   that were not registered.
   */
  ASSERT_EQ(
      pipeline.register_pipeline(
          {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
           {Gcs_protocol_version::V2, {Gcs_new_stage_2::my_stage_code()}},
           {Gcs_protocol_version::V3, {Gcs_new_stage_3::my_stage_code()}}}),
      true);

  pipeline.register_stage<Gcs_new_stage_1>();
  pipeline.register_stage<Gcs_new_stage_2>();
  pipeline.register_stage<Gcs_new_stage_3>();

  /*
   Handlers were not defined and the configuration should fail.
   */
  ASSERT_EQ(pipeline.register_pipeline({
                {Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
                {Gcs_protocol_version::V3, {Stage_code::ST_UNKNOWN}},
            }),
            true);

  /*
   There are handlers with the same type code in different pipeline
   versions.
   */
  ASSERT_EQ(pipeline.register_pipeline(
                {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
                 {Gcs_protocol_version::V2,
                  {Gcs_new_stage_1::my_stage_code(),
                   Gcs_new_stage_2::my_stage_code()}}}),
            true);

  /*
   The following configuration is perfectly fine as all stages have
   different type codes.
   */
  ASSERT_EQ(
      pipeline.register_pipeline(
          {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
           {Gcs_protocol_version::V2, {Gcs_new_stage_2::my_stage_code()}},
           {Gcs_protocol_version::V3, {Gcs_new_stage_3::my_stage_code()}}}),
      false);

  /*
   If we want to redefine the pipeline, we have to clean it up first.
   */
  pipeline.cleanup();

  pipeline.register_stage<Gcs_new_stage_1>();
  pipeline.register_stage<Gcs_new_stage_2>();
  pipeline.register_stage<Gcs_new_stage_3>();

  ASSERT_EQ(
      pipeline.register_pipeline(
          {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
           {Gcs_protocol_version::V2, {Gcs_new_stage_2::my_stage_code()}},
           {Gcs_protocol_version::V3, {Gcs_new_stage_3::my_stage_code()}}}),
      false);
}

TEST_F(XcomMultipleStagesTest, MultipleStagesCheckVersion) {
  pipeline.register_stage<Gcs_new_stage_1>();
  pipeline.register_stage<Gcs_new_stage_2>();
  pipeline.register_stage<Gcs_message_stage_lz4>();

  /*
   Configure the pipeline with the set of supported versions.
   */
  pipeline.register_pipeline(
      {{Gcs_protocol_version::V1, {Gcs_new_stage_1::my_stage_code()}},
       {Gcs_protocol_version::V2, {Gcs_new_stage_2::my_stage_code()}},
       {Gcs_protocol_version::V3, {Stage_code::ST_LZ4_V1}}});
  /*
   Check properties when the different versions are set up and they are
   increasing.
   */
  std::vector<Gcs_protocol_version> requested_inc_versions = {
      Gcs_protocol_version::UNKNOWN, Gcs_protocol_version::V1,
      Gcs_protocol_version::V2, Gcs_protocol_version::V3,
      Gcs_protocol_version::V5};
  std::vector<Gcs_protocol_version> configured_inc_versions = {
      Gcs_protocol_version::V2, Gcs_protocol_version::V1,
      Gcs_protocol_version::V2, Gcs_protocol_version::V3,
      Gcs_protocol_version::V3};
  std::vector<int> configured_inc_success = {false, true, true, true, false};
  std::vector<Gcs_protocol_version> outcome_inc_versions{};
  std::vector<bool> outcome_inc_success{};
  for (const auto &version : requested_inc_versions) {
    /*
     Setting the protocol version to be used.
     */
    outcome_inc_success.push_back(!pipeline.set_version(version));
    outcome_inc_versions.push_back(pipeline.get_version());
  }
  ASSERT_EQ(
      std::equal(configured_inc_versions.begin(), configured_inc_versions.end(),
                 outcome_inc_versions.begin()),
      true);
  ASSERT_EQ(
      std::equal(configured_inc_success.begin(), configured_inc_success.end(),
                 outcome_inc_success.begin()),
      true);

  /*
   Check properties when the different versions are set up and they are
   increasing.
   */
  std::vector<Gcs_protocol_version> requested_dec_versions = {
      Gcs_protocol_version::V5, Gcs_protocol_version::V3,
      Gcs_protocol_version::V2, Gcs_protocol_version::V1};
  std::vector<Gcs_protocol_version> configured_dec_versions = {
      Gcs_protocol_version::V3, Gcs_protocol_version::V3,
      Gcs_protocol_version::V2, Gcs_protocol_version::V1};
  std::vector<int> configured_dec_success = {false, true, true, true};
  std::vector<Gcs_protocol_version> outcome_dec_versions{};
  std::vector<bool> outcome_dec_success{};
  for (const auto &version : requested_dec_versions) {
    /*
     Setting the protocol version to be used.
     */
    outcome_dec_success.push_back(!pipeline.set_version(version));
    outcome_dec_versions.push_back(pipeline.get_version());
  }
  ASSERT_EQ(
      std::equal(configured_dec_versions.begin(), configured_dec_versions.end(),
                 outcome_dec_versions.begin()),
      true);
  ASSERT_EQ(
      std::equal(configured_dec_success.begin(), configured_dec_success.end(),
                 outcome_dec_success.begin()),
      true);
}

TEST_F(XcomMultipleStagesTest, MultipleStagesCheckData) {
  std::string sent_message("Message in a bottle. Message in a bottle.");

  /*
   Configure the pipeline with the set of supported versions.
   */
  pipeline.register_stage<Gcs_new_stage_1>();
  pipeline.register_stage<Gcs_new_stage_2>();
  pipeline.register_stage<Gcs_new_stage_3>();
  pipeline.register_stage<Gcs_message_stage_lz4>(true, 1);
  pipeline.register_stage<Gcs_message_stage_lz4_v2>(true, 1);
  pipeline.register_stage<Gcs_message_stage_split_v2>(true, 10);

  // clang-format off
  bool pipeline_error = pipeline.register_pipeline({
    {Gcs_protocol_version::V1,
     {Gcs_new_stage_1::my_stage_code(),
      Gcs_new_stage_2::my_stage_code()}
    },
    {Gcs_protocol_version::V3,
     {Gcs_new_stage_3::my_stage_code(),
      Stage_code::ST_LZ4_V1
     }
    },
    {Gcs_protocol_version::V4,
      {Stage_code::ST_LZ4_V2,
       Stage_code::ST_SPLIT_V2
      }
    }
  });
  // clang-format on
  ASSERT_EQ(pipeline_error, false);

  /*
   Define/update the membership for all the stages that need it.
   */
  Gcs_xcom_nodes nodes;
  Gcs_xcom_node_information node("127.0.0.1:8080", Gcs_xcom_uuid::create_uuid(),
                                 0, true);
  nodes.add_node(node);

  Gcs_message_stage &split2 = pipeline.get_stage(Stage_code::ST_SPLIT_V2);
  split2.update_members_information(node.get_member_id(), nodes);

  /*
   Check properties when the different versions are set up.
   */
  std::vector<Gcs_protocol_version> requested_versions = {
      Gcs_protocol_version::V1, Gcs_protocol_version::V3,
      Gcs_protocol_version::V4};
  for (const auto &version : requested_versions) {
    /*
     Calculate sizes of different bits and pieces.
     */
    unsigned long long payload_size = sent_message.size() + 1;

    /*
     Setting the protocol version to be used.
     */
    pipeline.set_version(version);

    /*
     Set up the packet and copy the payload content: "Message in a bottle".
     */
    Gcs_message_data sent_msg_data(0, payload_size);
    sent_msg_data.append_to_payload(
        reinterpret_cast<unsigned char const *>(sent_message.c_str()),
        payload_size);

    /*
     Traverse all the stages and get an updated packet ready to be
     sent through the network.
     */
    bool error;
    std::vector<Gcs_packet> packets_out;
    std::tie(error, packets_out) =
        pipeline.process_outgoing(sent_msg_data, Cargo_type::CT_USER_DATA);
    ASSERT_FALSE(error);

    /*
     Process the outcome set of packets.
     */
    for (auto &out : packets_out) {
      /*
       Traverse all the stages and get an updated packet ready to be consumed by
       the application.
       */
      Gcs_packet::buffer_ptr buffer;
      unsigned long long buffer_size;
      std::tie(buffer, buffer_size) = out.serialize();

      auto in = Gcs_packet::make_incoming_packet(std::move(buffer), buffer_size,
                                                 null_synode, pipeline);
      Gcs_pipeline_incoming_result error_code;
      Gcs_packet packet_in;
      std::tie(error_code, packet_in) =
          pipeline.process_incoming(std::move(in));
      ASSERT_TRUE(error_code == Gcs_pipeline_incoming_result::OK_PACKET ||
                  error_code == Gcs_pipeline_incoming_result::OK_NO_PACKET);

      /*
        Check the payload content.
       */
      if (error_code == Gcs_pipeline_incoming_result::OK_PACKET) {
        Gcs_message_data received_msg_data(packet_in.get_payload_length());
        ASSERT_FALSE(received_msg_data.decode(packet_in.get_payload_pointer(),
                                              packet_in.get_payload_length()));
        std::string received_message{
            reinterpret_cast<char const *>(received_msg_data.get_payload())};
        ASSERT_EQ(sent_message.compare(received_message), 0);
      }
    }
  }
}

/**
 Create a message with a content whose size is greater than the compression
 threshold and make it go through the compression AND the fragmentation stages.
 Check that the content is really compressed, and that the fragmentation stage
 produces a single fragment.
*/
TEST_F(XcomMultipleStagesTest, SingleFragment) {
  // Configure the pipeline with compression.
  pipeline.register_stage<Gcs_message_stage_lz4_v2>(
      true, Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD);
  // Configure the pipeline with fragmentation, with the same threshold of the
  // compression stage.
  pipeline.register_stage<Gcs_message_stage_split_v2>(
      true, Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD);
  // clang-format off
  bool error = pipeline.register_pipeline({
    {Gcs_protocol_version::V2, {Stage_code::ST_LZ4_V2, Stage_code::ST_SPLIT_V2}}
  });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  // Define/update the membership for all the stages that need it.
  Gcs_xcom_nodes nodes;
  Gcs_xcom_node_information node("127.0.0.1:8080", Gcs_xcom_uuid::create_uuid(),
                                 0, true);
  nodes.add_node(node);
  Gcs_message_stage &split2 = pipeline.get_stage(Stage_code::ST_SPLIT_V2);
  split2.update_members_information(node.get_member_id(), nodes);

  constexpr unsigned long long payload_size =
      Gcs_message_stage_lz4_v2::DEFAULT_THRESHOLD + 1;

  unsigned char control[payload_size];
  std::memset(control, 0x61, payload_size);

  Gcs_message_data msg_data(0, payload_size);
  msg_data.append_to_payload(control, payload_size);

  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(msg_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet packet(std::move(packets_out[0]));
  ASSERT_EQ(packet.get_dynamic_headers().size(), 2);
  ASSERT_EQ(packet.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V2);
  ASSERT_EQ(packet.get_dynamic_headers().at(1).get_stage_code(),
            Stage_code::ST_SPLIT_V2);

  // Goes through the network, in imagination land...

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_size;
  std::tie(buffer, buffer_size) = packet.serialize();
  auto packet_from_network = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_size, null_synode, pipeline);

  Gcs_pipeline_incoming_result error_code;
  Gcs_packet packet_in;
  std::tie(error_code, packet_in) =
      pipeline.process_incoming(std::move(packet_from_network));
  ASSERT_EQ(error_code, Gcs_pipeline_incoming_result::OK_PACKET);

  ASSERT_EQ(packet_in.get_dynamic_headers().size(), 2);
  ASSERT_EQ(packet_in.get_dynamic_headers().at(0).get_stage_code(),
            Stage_code::ST_LZ4_V2);
  ASSERT_EQ(packet_in.get_dynamic_headers().at(1).get_stage_code(),
            Stage_code::ST_SPLIT_V2);

  Gcs_message_data msg_data_3(packet_in.get_payload_length());
  ASSERT_FALSE(msg_data_3.decode(packet_in.get_payload_pointer(),
                                 packet_in.get_payload_length()));
  ASSERT_EQ(
      std::strncmp(reinterpret_cast<char const *>(control),
                   reinterpret_cast<char const *>(msg_data_3.get_payload()),
                   payload_size),
      0);
}

TEST_F(XcomMultipleStagesTest, SplitMessages) {
  /*
   Define a message to be sent and that shall be split and compressed.
   */
  std::string base_message("Message in a bottle. Message in a bottle.");
  std::ostringstream os;
  for (int i = 0; i < 1024; i++) os << base_message;
  std::string sent_message(os.str());

  /*
   Configure the pipeline with the set of supported versions.
   */
  pipeline.register_stage<Gcs_message_stage_lz4_v2>(true, 1);
  pipeline.register_stage<Gcs_message_stage_split_v2>(true, 10);
  pipeline.register_stage<Gcs_new_stage_split_4>(true, 10);
  pipeline.register_stage<Gcs_new_stage_lz4_5>(true, 1);

  // clang-format off
    bool pipeline_error = pipeline.register_pipeline({
      {Gcs_protocol_version::V1,
        {Stage_code::ST_SPLIT_V2,
         Stage_code::ST_LZ4_V2
        }
      },
      {Gcs_protocol_version::V2,
        {Gcs_new_stage_lz4_5::my_stage_code(),
         Gcs_new_stage_split_4::my_stage_code()
        }
      }
    });
  // clang-format on
  ASSERT_EQ(pipeline_error, false);

  /*
   Define/update the membership for all the stages that need it.
   */
  Gcs_xcom_nodes nodes;
  Gcs_xcom_node_information node("127.0.0.1:8080", Gcs_xcom_uuid::create_uuid(),
                                 0, true);
  nodes.add_node(node);

  Gcs_message_stage &split2 = pipeline.get_stage(Stage_code::ST_SPLIT_V2);
  split2.update_members_information(node.get_member_id(), nodes);

  Gcs_message_stage &split4 =
      pipeline.get_stage(Gcs_new_stage_split_4::my_stage_code());
  split4.update_members_information(node.get_member_id(), nodes);

  /*
   Check properties when the different versions are set up.
   */
  std::vector<Gcs_protocol_version> requested_versions = {
      Gcs_protocol_version::V1, Gcs_protocol_version::V2};
  for (const auto &version : requested_versions) {
    /*
     Calculate sizes of different bits and pieces.
     */
    unsigned long long payload_size = sent_message.size() + 1;

    /*
     Setting the protocol version to be used.
     */
    pipeline.set_version(version);

    /*
     Set up the packet and copy the payload content: "Message in a bottle".
     */
    Gcs_message_data sent_msg_data(0, payload_size);
    sent_msg_data.append_to_payload(
        reinterpret_cast<unsigned char const *>(sent_message.c_str()),
        payload_size);

    /*
     Traverse all the stages and get an updated packet ready to be
     sent through the network.
     */
    bool error;
    std::vector<Gcs_packet> packets_out;
    std::tie(error, packets_out) =
        pipeline.process_outgoing(sent_msg_data, Cargo_type::CT_USER_DATA);
    ASSERT_FALSE(error);

    /*
     Process the outcome set of packets.
     */
    for (auto &out : packets_out) {
      /*
       Traverse all the stages and get an updated packet ready to be
       consumed by the application
       */
      Gcs_packet::buffer_ptr buffer;
      unsigned long long buffer_size;
      std::tie(buffer, buffer_size) = out.serialize();

      auto in = Gcs_packet::make_incoming_packet(std::move(buffer), buffer_size,
                                                 null_synode, pipeline);
      Gcs_pipeline_incoming_result error_code;
      Gcs_packet packet_in;
      std::tie(error_code, packet_in) =
          pipeline.process_incoming(std::move(in));
      ASSERT_TRUE(error_code == Gcs_pipeline_incoming_result::OK_PACKET ||
                  error_code == Gcs_pipeline_incoming_result::OK_NO_PACKET);

      /*
       Check the payload content.
       */
      if (error_code == Gcs_pipeline_incoming_result::OK_PACKET) {
        Gcs_message_data received_msg_data(packet_in.get_payload_length());
        ASSERT_FALSE(received_msg_data.decode(packet_in.get_payload_pointer(),
                                              packet_in.get_payload_length()));
        std::string received_message{
            reinterpret_cast<char const *>(received_msg_data.get_payload())};
        ASSERT_EQ(sent_message.compare(received_message), 0);
      }
    }
  }
}

}  // namespace gcs_xcom_stages_unittest
