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

#include "gcs_base_test.h"
#include "mysql/gcs/gcs_logging_system.h"

namespace gcs_logging_unittest {
class Mock_Logger : public Logger_interface {
 public:
  Mock_Logger() {
    ON_CALL(*this, initialize()).WillByDefault(Return(GCS_OK));
    ON_CALL(*this, finalize()).WillByDefault(Return(GCS_OK));
  }

  ~Mock_Logger() {}
  MOCK_METHOD0(initialize, enum_gcs_error());
  MOCK_METHOD0(finalize, enum_gcs_error());
  MOCK_METHOD2(log_event, void(const gcs_log_level_t, const std::string &));
};

class LoggingInfrastructureTest : public GcsBaseTestNoLogging {
 protected:
  LoggingInfrastructureTest() : logger(nullptr) {}

  virtual void SetUp() { logger = new Mock_Logger(); }

  virtual void TearDown() {
    Gcs_log_manager::finalize();
    delete logger;
    logger = nullptr;
  }

  Mock_Logger *logger;
};

TEST_F(LoggingInfrastructureTest, InjectedMockLoggerTest) {
  EXPECT_CALL(*logger, initialize()).Times(1);
  EXPECT_CALL(*logger, log_event(_, _)).Times(4);

  Gcs_log_manager::initialize(logger);

  // Logger 1 initialized
  ASSERT_EQ(true, Gcs_log_manager::get_logger() != nullptr);
  ASSERT_EQ(logger, Gcs_log_manager::get_logger());

  // Log some messages on logger
  int l;
  for (l = GCS_FATAL; l <= GCS_INFO; l++) {
    MYSQL_GCS_LOG(
        (gcs_log_level_t)l,
        gcs_log_levels[l] << "This is a logging message with level " << l);
  }

  // Initialize new mock logger
  Mock_Logger *anotherLogger = new Mock_Logger();
  Gcs_log_manager::initialize(anotherLogger);

  // anotherLogger initialized
  ASSERT_EQ(true, Gcs_log_manager::get_logger() != nullptr);
  ASSERT_EQ(anotherLogger, Gcs_log_manager::get_logger());

  Gcs_log_manager::finalize();
  delete anotherLogger;
}

class DebuggingInfrastructureTest : public GcsBaseTestNoLogging {
 protected:
  DebuggingInfrastructureTest()
      : debugger(nullptr), sink(nullptr), saved_options(GCS_DEBUG_NONE) {}

  virtual void SetUp() {
    sink = new Gcs_async_buffer(new Gcs_output_sink());
    debugger = new Gcs_default_debugger(sink);
    saved_options = Gcs_debug_manager::get_current_debug_options();
    Gcs_debug_manager::unset_debug_options(GCS_DEBUG_ALL);
    ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  }

  virtual void TearDown() {
    Gcs_debug_manager::unset_debug_options(GCS_DEBUG_ALL);
    Gcs_debug_manager::set_debug_options(saved_options);
    ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), saved_options);

    Gcs_debug_manager::finalize();
    delete debugger;
    delete sink;
    debugger = nullptr;
  }

  Gcs_default_debugger *debugger;
  Gcs_async_buffer *sink;
  int64_t saved_options;
};

TEST_F(DebuggingInfrastructureTest, DebugManagerTestingSetOfOptions) {
  /*
    Prepare the environement.
  */
  Gcs_debug_manager::initialize(debugger);

  /*
    Checking if there are gaps in the set of valid options.
  */
  int64_t option;
  int64_t options = GCS_DEBUG_NONE;
  for (unsigned int i = 0; i < Gcs_debug_manager::get_number_debug_options();
       i++) {
    option = static_cast<int64_t>(1) << i;
    ASSERT_TRUE(Gcs_debug_manager::is_valid_debug_options(option));
    options = options | option;
  }
  ASSERT_EQ(Gcs_debug_manager::get_valid_debug_options(), options);

  /*
    Check if the set of valid options is different from GCS_DEBUG_NONE and
    GCS_DEBUG_ALL. Although one can pass them as parameter to methods in
    this interface, they are only useful definitions.
  */
  ASSERT_TRUE(Gcs_debug_manager::is_valid_debug_options(GCS_DEBUG_ALL));
  ASSERT_TRUE(Gcs_debug_manager::is_valid_debug_options(GCS_DEBUG_NONE));
  ASSERT_FALSE(GCS_DEBUG_ALL == options);
  ASSERT_FALSE(GCS_DEBUG_NONE == options);
  ASSERT_FALSE(Gcs_debug_manager::is_valid_debug_options(GCS_INVALID_DEBUG));

  /*
    Restore the environment.
  */
  Gcs_debug_manager::finalize();
}

TEST_F(DebuggingInfrastructureTest, DebugManagerTestingSettingIntegerOptions) {
  /*
    Prepare the environement to set the current debug options using integers.
  */
  std::string res_debug_options;
  Gcs_debug_manager::initialize(debugger);

  /*
    Check if the initial value is GCS_DEBUG_NONE.
  */
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  /*
    Check if it is possible to set GCS_DEBUG_BASIC.
  */
  Gcs_debug_manager::set_debug_options(GCS_DEBUG_BASIC);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC"), 0);

  /*
    Check if it is possible to set GCS_DEBUG_TRACE.
  */
  Gcs_debug_manager::set_debug_options(GCS_DEBUG_TRACE);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(),
            GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC,GCS_DEBUG_TRACE"), 0);

  /*
    Check if it is possible to set GCS_DEBUG_ALL.
  */
  Gcs_debug_manager::set_debug_options(GCS_DEBUG_ALL);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  /*
     Check if it is possible to set GCS_DEBUG_NONE.
   */
  Gcs_debug_manager::set_debug_options(GCS_DEBUG_NONE);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::unset_debug_options(GCS_DEBUG_ALL);

  Gcs_debug_manager::set_debug_options(GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(),
            GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC,GCS_DEBUG_TRACE"), 0);

  Gcs_debug_manager::force_debug_options(GCS_DEBUG_BASIC);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC"), 0);

  Gcs_debug_manager::unset_debug_options(GCS_DEBUG_ALL);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  ASSERT_TRUE(Gcs_debug_manager::set_debug_options(GCS_INVALID_DEBUG));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  ASSERT_FALSE(Gcs_debug_manager::set_debug_options(GCS_DEBUG_ALL));
  ASSERT_TRUE(Gcs_debug_manager::unset_debug_options(GCS_INVALID_DEBUG));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::force_debug_options(GCS_INVALID_DEBUG);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::unset_debug_options(GCS_DEBUG_ALL);

  ASSERT_FALSE(Gcs_debug_manager::is_valid_debug_options(GCS_INVALID_DEBUG));
  Gcs_debug_manager::set_debug_options(GCS_DEBUG_BASIC | GCS_INVALID_DEBUG);
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  /*
    Restore the environment.
  */
  Gcs_debug_manager::finalize();
}

TEST_F(DebuggingInfrastructureTest, DebugManagerTestingSettingStringOptions) {
  /*
    Prepare the environement.
  */
  std::string res_debug_options;
  Gcs_debug_manager::initialize(debugger);

  /*
    Setting the current debug options using strings.
  */
  Gcs_debug_manager::set_debug_options("GCS_DEBUG_BASIC");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC"), 0);

  Gcs_debug_manager::set_debug_options("GCS_DEBUG_TRACE");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(),
            GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC,GCS_DEBUG_TRACE"), 0);

  Gcs_debug_manager::set_debug_options("GCS_DEBUG_ALL");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::set_debug_options("GCS_DEBUG_NONE");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::unset_debug_options("gcs_debug_all");
  Gcs_debug_manager::set_debug_options("gcs_debug_basic,gcs_invalid_debug,,");
  ASSERT_FALSE(Gcs_debug_manager::is_valid_debug_options("gcs_invalid_debug"));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  Gcs_debug_manager::set_debug_options(",,,");
  ASSERT_FALSE(Gcs_debug_manager::is_valid_debug_options(",,,"));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  Gcs_debug_manager::set_debug_options(
      ",,gcs_debug_basic ,  gcs_debug_trace ,,");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(),
            GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC,GCS_DEBUG_TRACE"), 0);

  Gcs_debug_manager::set_debug_options(",,,");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(),
            GCS_DEBUG_BASIC | GCS_DEBUG_TRACE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC,GCS_DEBUG_TRACE"), 0);

  Gcs_debug_manager::force_debug_options("GCS_DEBUG_BASIC");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_BASIC"), 0);

  Gcs_debug_manager::unset_debug_options("GCS_DEBUG_ALL");
  Gcs_debug_manager::set_debug_options(
      "gcs_debug_basic,gcs_debug_trace,gcs_debug_all,gcs_debug_none");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::unset_debug_options("GCS_DEBUG_ALL");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  ASSERT_FALSE(Gcs_debug_manager::is_valid_debug_options("GCS_INVALID_DEBUG"));
  ASSERT_TRUE(Gcs_debug_manager::set_debug_options("GCS_INVALID_DEBUG"));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_NONE);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_NONE"), 0);

  ASSERT_FALSE(Gcs_debug_manager::set_debug_options("GCS_DEBUG_ALL"));
  ASSERT_TRUE(Gcs_debug_manager::unset_debug_options("GCS_INVALID_DEBUG"));
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::force_debug_options("GCS_INVALID_DEBUG");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  Gcs_debug_manager::force_debug_options("GCS_DEBUG_BASIC");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::set_debug_options("GCS_DEBUG_ALL,GCS_INVALID_DEBUG");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::set_debug_options("GCS_INVALID_ALL,GCS_DEBUG_ALL");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_BASIC);
  Gcs_debug_manager::set_debug_options("GCS_DEBUG_ALL");
  ASSERT_EQ(Gcs_debug_manager::get_current_debug_options(), GCS_DEBUG_ALL);
  Gcs_debug_manager::get_current_debug_options(res_debug_options);
  ASSERT_EQ(res_debug_options.compare("GCS_DEBUG_ALL"), 0);

  /*
    Restore the environment.
  */
  Gcs_debug_manager::finalize();
}

}  // namespace gcs_logging_unittest
