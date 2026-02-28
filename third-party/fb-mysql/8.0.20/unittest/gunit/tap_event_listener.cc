/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <stdio.h>
#include <sstream>
#include <string>

#include "gtest/gtest-test-part.h"

using testing::TestCase;
using testing::TestEventListener;
using testing::TestEventListeners;
using testing::TestInfo;
using testing::TestPartResult;
using testing::UnitTest;

/**
   Receives events from googletest, and outputs interesting events
   in TAP compliant format.
   Implementation is inspired by PrettyUnitTestResultPrinter.
   See documentation for base class.
 */
class TapEventListener : public TestEventListener {
 public:
  TapEventListener() : m_test_number(0) {}
  virtual ~TapEventListener() {}

  virtual void OnTestProgramStart(const UnitTest & /*unit_test*/) {}
  virtual void OnTestIterationStart(const UnitTest &unit_test, int iteration);
  virtual void OnEnvironmentsSetUpStart(const UnitTest &unit_test);
  virtual void OnEnvironmentsSetUpEnd(const UnitTest & /*unit_test*/) {}
  virtual void OnTestCaseStart(const TestCase &test_case);
  virtual void OnTestStart(const TestInfo &test_info);
  virtual void OnTestPartResult(const TestPartResult &test_part_result);
  virtual void OnTestEnd(const TestInfo &test_info);
  virtual void OnTestCaseEnd(const TestCase & /*test_case*/) {}
  virtual void OnEnvironmentsTearDownStart(const UnitTest &unit_test);
  virtual void OnEnvironmentsTearDownEnd(const UnitTest & /*unit_test*/) {}
  virtual void OnTestIterationEnd(const UnitTest &unit_test, int iteration);
  virtual void OnTestProgramEnd(const UnitTest & /*unit_test*/) {}

 private:
  int m_test_number;
  std::string m_test_case_name;
};

/**
   Prints argument to stdout, prefixing all lines with "# ".
 */
static void tap_diagnostic_printf(const std::stringstream &str_stream) {
  std::string message = str_stream.str();
  size_t pos = 0;
  while ((pos = message.find("\n", pos)) != std::string::npos) {
    message.replace(pos, 1, "\n# ");
    pos += 1;
  }
  printf("# %s\n", message.c_str());
  fflush(stdout);
}

// Convenience wrapper function.
static void tap_diagnostic_printf(const std::string &txt) {
  std::stringstream str_str;
  str_str << txt;
  tap_diagnostic_printf(str_str);
}

// Convenience wrapper function.
static void tap_diagnostic_printf(const char *txt) {
  tap_diagnostic_printf(std::string(txt));
}

namespace {
// Helper struct to simplify output of "1 test" or "n tests".
struct num_tests {
  num_tests(int num) : m_num(num) {}
  int m_num;
};

std::ostream &operator<<(std::ostream &s, const num_tests &num) {
  return s << num.m_num << (num.m_num == 1 ? " test" : " tests");
}

// Helper struct to simplify output of "1 test case" or "n test cases".
struct num_test_cases {
  num_test_cases(int num) : m_num(num) {}
  int m_num;
};

std::ostream &operator<<(std::ostream &s, const num_test_cases &num) {
  return s << num.m_num << (num.m_num == 1 ? " test case" : " test cases");
}
}  // namespace

/**
   Converts a TestPartResult::Type enum to human-friendly string
   representation.
*/
static std::string test_part_result_type_tostring(TestPartResult::Type type) {
  switch (type) {
    case TestPartResult::kSuccess:
      return "Success";

    case TestPartResult::kNonFatalFailure:
    case TestPartResult::kFatalFailure:
      return "Failure";

    default:
      break;
  }
  return "";
}

/**
   Formats a source file path and a line number as they would appear
   in a compiler error message.
*/
static std::string format_file_location(
    const TestPartResult &test_part_result) {
  const char *const file = test_part_result.file_name();
  const char *const file_name = file == nullptr ? "unknown file" : file;
  const int line = test_part_result.line_number();
  std::stringstream str_stream;
  str_stream << file_name << ":";
  if (line >= 0) str_stream << line << ":";
  return str_stream.str();
}

/**
   Formats a TestPartResult as a string.
 */
static std::string test_part_result_tostring(
    const TestPartResult &test_part_result) {
  return format_file_location(test_part_result) + " " +
         test_part_result_type_tostring(test_part_result.type()) +
         test_part_result.message();
}

void TapEventListener::OnTestIterationStart(const UnitTest &unit_test, int) {
  std::stringstream str_stream;
  str_stream << "Running " << num_tests(unit_test.test_to_run_count())
             << " from " << num_test_cases(unit_test.test_case_to_run_count());
  tap_diagnostic_printf(str_stream);
  printf("%d..%d\n", 1, unit_test.test_to_run_count());
  fflush(stdout);
}

void TapEventListener::OnEnvironmentsSetUpStart(const UnitTest &) {
  tap_diagnostic_printf("Global test environment set-up");
}

void TapEventListener::OnTestCaseStart(const TestCase &test_case) {
  m_test_case_name = test_case.name();
}

void TapEventListener::OnTestStart(const TestInfo &test_info) {
  ++m_test_number;
  std::stringstream str_stream;
  str_stream << "Run " << m_test_number << " " << m_test_case_name << "."
             << test_info.name();
  tap_diagnostic_printf(str_stream);
}

void TapEventListener::OnTestPartResult(
    const TestPartResult &test_part_result) {
  if (test_part_result.passed()) return;
  // Don't prefix error messages with #, as the tap harness will hide them!
  fprintf(stderr, "%s\n", test_part_result_tostring(test_part_result).c_str());
}

void TapEventListener::OnTestEnd(const TestInfo &test_info) {
  if (test_info.result()->Passed())
    printf("ok %d\n", m_test_number);
  else
    printf("not ok %d\n", m_test_number);
  fflush(stdout);
}

void TapEventListener::OnEnvironmentsTearDownStart(const UnitTest &) {
  tap_diagnostic_printf("Global test environment tear-down");
}

void TapEventListener::OnTestIterationEnd(const UnitTest &unit_test, int) {
  std::stringstream str_stream;
  str_stream << "Ran " << num_tests(unit_test.test_to_run_count()) << " from "
             << num_test_cases(unit_test.test_case_to_run_count()) << "\n"
             << "Passed " << num_tests(unit_test.successful_test_count());

  if (!unit_test.Passed())
    str_stream << "\n"
               << "Failed " << num_tests(unit_test.failed_test_count());

  const int num_disabled = unit_test.disabled_test_count();
  if (num_disabled && !testing::GTEST_FLAG(also_run_disabled_tests))
    str_stream << "\n"
               << "YOU HAVE " << num_disabled << " DISABLED "
               << (num_disabled == 1 ? "TEST" : "TESTS");

  tap_diagnostic_printf(str_stream);
}

/**
   Removes the default googletest listener (a PrettyUnitTestResultPrinter),
   and installs our own TAP compliant pretty printer instead.
 */
void install_tap_listener() {
  TestEventListeners &listeners = UnitTest::GetInstance()->listeners();
  delete listeners.Release(listeners.default_result_printer());
  listeners.Append(new TapEventListener);
}
