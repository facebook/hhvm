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

#include <sstream>

#include "gcs_base_test.h"
#include "my_io.h"

namespace gcs_logging_unittest {

class Mock_gcs_log_sink : public Sink_interface {
 public:
  MOCK_METHOD1(log_event, void(const std::string &message));
  MOCK_METHOD2(log_event, void(const char *message, size_t message_size));
  MOCK_METHOD0(initialize, enum_gcs_error());
  MOCK_METHOD0(finalize, enum_gcs_error());
  /* purecov: begin deadcode */
  MOCK_CONST_METHOD0(get_information, const std::string());
  /* purecov: end */
};

class LoggingDebuggingSystemTest : public GcsBaseTestNoLogging {
 protected:
  LoggingDebuggingSystemTest()
      : common_sink(nullptr), logger(nullptr), debugger(nullptr) {}

  virtual void SetUp() {
    common_sink = new Gcs_async_buffer(new Mock_gcs_log_sink());
    logger = new Gcs_default_logger(common_sink);
    debugger = new Gcs_default_debugger(common_sink);
  }

  virtual void TearDown() {
    Gcs_log_manager::finalize();

    delete logger;
    logger = nullptr;

    Gcs_debug_manager::finalize();

    delete debugger;
    debugger = nullptr;

    delete common_sink;
    common_sink = nullptr;
  }

  Gcs_async_buffer *common_sink;
  Gcs_default_logger *logger;
  Gcs_default_debugger *debugger;
};

TEST_F(LoggingDebuggingSystemTest, DefaultLifecycle) {
  Mock_gcs_log_sink *mock_sink = static_cast<Mock_gcs_log_sink *>(
      static_cast<Gcs_async_buffer *>(common_sink)->get_sink());

  ON_CALL(*mock_sink, initialize()).WillByDefault(Return(GCS_OK));
  ON_CALL(*mock_sink, finalize()).WillByDefault(Return(GCS_OK));

  // on some machines an info message will be displayed stating
  // that a network interface was not successfully probed
  // we cannot predict how many network interfaces are in the
  // machine that cannot be probed
  EXPECT_CALL(
      *mock_sink,
      log_event(ContainsRegex("\\[GCS\\] Unable to probe network interface .*"),
                _))
      .Times(AnyNumber());

  /*
    The following message is expected.
  */
  EXPECT_CALL(*mock_sink,
              log_event(ContainsRegex("\\[GCS\\] SSL was not enabled.*"), _))
      .Times(1);

  /*
    3 messages of this form, corresponding to the 3 logging levels, are
    expected.
  */
  EXPECT_CALL(
      *mock_sink,
      log_event(ContainsRegex("This message belongs to logging level .*"), _))
      .Times(3);

  ASSERT_EQ(true, Gcs_log_manager::get_logger() == nullptr);
  ASSERT_EQ(true, Gcs_debug_manager::get_debugger() == nullptr);

  Gcs_log_manager::initialize(logger);
  Gcs_debug_manager::initialize(debugger);

  Gcs_group_identifier *group_id = new Gcs_group_identifier("only_group");
  Gcs_interface_parameters if_params;

  if_params.add_parameter("group_name", group_id->get_group_id());
  if_params.add_parameter("peer_nodes", "127.0.0.1:12345");
  if_params.add_parameter("local_node", "127.0.0.1:12345");
  if_params.add_parameter("bootstrap_group", "true");
  if_params.add_parameter("poll_spin_loops", "100");

  // just to make the log entries count below deterministic, otherwise,
  // there would be additional info messages due to automatically adding
  // addresses to the whitelist
  if_params.add_parameter("ip_whitelist", Gcs_ip_whitelist::DEFAULT_WHITELIST);

  Gcs_interface *xcom_if = Gcs_xcom_interface::get_interface();
  enum_gcs_error initialized = xcom_if->initialize(if_params);

  ASSERT_EQ(GCS_OK, initialized);

  ASSERT_EQ(true, Gcs_log_manager::get_logger() != nullptr);
  ASSERT_EQ(true, Gcs_debug_manager::get_debugger() != nullptr);

  gcs_log_level_t level;

  for (int i = GCS_ERROR; i <= GCS_INFO; i++) {
    level = static_cast<gcs_log_level_t>(i);
    std::string msg("This message belongs to logging level ");
    msg += gcs_log_levels[level];

    const char *c_msg = msg.c_str();

    Gcs_log_manager::get_logger()->log_event(level, c_msg);
  }

  enum_gcs_error finalize_error = xcom_if->finalize();
  ASSERT_EQ(GCS_OK, finalize_error);

  Gcs_xcom_interface::cleanup();

  delete group_id;

  ASSERT_EQ(true, Gcs_log_manager::get_logger() == nullptr);
  ASSERT_EQ(true, Gcs_debug_manager::get_debugger() == nullptr);
}

#ifndef XCOM_STANDALONE
class Wrapper_file_sink : public Sink_interface {
 public:
  Wrapper_file_sink(Gcs_file_sink *sink) : m_sink(sink) {
    ON_CALL(*this, log_event(_))
        .WillByDefault(Invoke(
            m_sink, static_cast<void (Gcs_file_sink::*)(const std::string &)>(
                        &Gcs_file_sink::log_event)));
    ON_CALL(*this, log_event(_, _))
        .WillByDefault(Invoke(
            m_sink, static_cast<void (Gcs_file_sink::*)(const char *, size_t)>(
                        &Gcs_file_sink::log_event)));
    ON_CALL(*this, initialize())
        .WillByDefault(Invoke(m_sink, &Gcs_file_sink::initialize));
    ON_CALL(*this, finalize())
        .WillByDefault(Invoke(m_sink, &Gcs_file_sink::finalize));
    ON_CALL(*this, get_file_name(_))
        .WillByDefault(Invoke(m_sink, &Gcs_file_sink::get_file_name));
    ON_CALL(*this, get_information())
        .WillByDefault(Invoke(m_sink, &Gcs_file_sink::get_information));
  }

  ~Wrapper_file_sink() { delete m_sink; }

  MOCK_METHOD1(log_event, void(const std::string &message));
  MOCK_METHOD2(log_event, void(const char *message, size_t message_size));
  MOCK_METHOD0(initialize, enum_gcs_error());
  MOCK_METHOD0(finalize, enum_gcs_error());
  MOCK_METHOD1(get_file_name, enum_gcs_error(char *file_name_buffer));
  MOCK_CONST_METHOD0(get_information, const std::string());

 private:
  Gcs_file_sink *m_sink;
};

class FileOutputSystemTest : public GcsBaseTestNoLogging {};

TEST_F(FileOutputSystemTest, SinkFileTest) {
  /*
    Definition of local variables.
  */
  char file_name_buffer[FN_REFLEN];
  std::stringstream buffer;
  std::string repeat("/unknown");
  size_t base = repeat.length();
  size_t size = 0;
  File m_fd;
  uchar read_buffer[10];
  read_buffer[0] = '\0';
  std::string info("testing");

  /*
    Try to create a file that exceeds the normal name's length.
  */
  *file_name_buffer = 0;
  for (size = 0; size < FN_REFLEN; size += base) {
    buffer << repeat;
  }
  buffer << "/";
  Wrapper_file_sink wrapper_big(new Gcs_file_sink("unknown", buffer.str()));
  ASSERT_EQ(wrapper_big.get_file_name(file_name_buffer), GCS_NOK);

  /*
    The sink will fail to initialize due to an invalid file
  */
  Wrapper_file_sink wrapper_unknown(
      new Gcs_file_sink("unknown", "/unknown/unknown/"));
  ASSERT_EQ(wrapper_unknown.initialize(), GCS_NOK);
  ASSERT_EQ(wrapper_unknown.finalize(), GCS_OK);

  /*
     Then sink will succeed to initialize and will create a file name
     gcs-communication.debug
  */
  Wrapper_file_sink wrapper(new Gcs_file_sink("gcs-communication", "."));
  ASSERT_EQ(wrapper.initialize(), GCS_OK);
  ASSERT_EQ(wrapper.get_file_name(file_name_buffer), GCS_OK);
  ASSERT_TRUE((m_fd = my_open(file_name_buffer, 0 /* O_RDONLY */, MYF(0))) > 0);
  wrapper.log_event(info);
  ASSERT_EQ(my_read(m_fd, read_buffer, info.length(), MYF(0)), info.length());
  read_buffer[info.length()] = '\0';
  ASSERT_EQ(info.compare(0, info.length(), (char *)read_buffer), 0);
  ASSERT_EQ(wrapper.finalize(), GCS_OK);
  ASSERT_EQ(my_close(m_fd, MYF(0)), 0);
}
#endif /* XCOM_STANDALONE */

}  // namespace gcs_logging_unittest
