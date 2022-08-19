/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TEST_LOGGER_INCLUDED
#define TEST_LOGGER_INCLUDED
#include "gcs_xcom_interface.h"
#include "mysql/gcs/gcs_logging_system.h"

/*
  It is a logger utility helps unit test to verify functions which will log
  errors correctly.

  Usage:
  #include "test_logger.h"

  test_logger.clear_event(); // clear all logged events before a test.
  ...
  test_logger.assert_error("Expected error message");
*/
class Test_logger : public Logger_interface {
 private:
  std::stringstream m_log_stream;

  std::string get_event() { return m_log_stream.str(); }

  void assert_event(gcs_log_level_t level, const std::string &expected) {
    std::string complete_log(gcs_log_levels[level]);
    complete_log += GCS_PREFIX;
    complete_log += expected;

    ASSERT_EQ(complete_log, get_event());
  }

 public:
  Test_logger() { Gcs_log_manager::initialize(this); }

  ~Test_logger() {}

  enum_gcs_error initialize() { return GCS_OK; }

  enum_gcs_error finalize() { return GCS_OK; }

  void log_event(const gcs_log_level_t level, const std::string &message) {
    m_log_stream << gcs_log_levels[level] << message;
  }

  void clear_event() { m_log_stream.str(""); }

  void assert_error(const std::string &expected) {
    assert_event(GCS_ERROR, expected);
  }

  void assert_error(const std::stringstream &expected) {
    assert_error(expected.str());
  }
};

Test_logger test_logger;

#endif  // TEST_LOGGER_INCLUDED
