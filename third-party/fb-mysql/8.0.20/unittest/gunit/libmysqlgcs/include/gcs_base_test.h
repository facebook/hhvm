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

#ifndef GCS_BASE_TEST_INCLUDED
#define GCS_BASE_TEST_INCLUDED

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_basic_logging.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::ByRef;
using ::testing::ContainsRegex;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::WithArgs;

/**
  Class that defines basic common testing infra-structure to be used
  in all test cases and should be the default choice whenever a new
  testing class is created.

  Note that any global change to the test classes should be made
  here. Currently, it only defines a simple logging object.
*/
class GcsBaseTest : public ::testing::Test {
 public:
  GcsBaseTest() {}

  virtual ~GcsBaseTest() {}

  /**
    Simple logging object that can be used in the test case.
  */
  Gcs_basic_logging logging;
};

/**
  Class that defines basic common testing infra-structure to be used
  in al test cases whenever they need to create its own logging
  objects.
*/
class GcsBaseTestNoLogging : public ::testing::Test {
 public:
  GcsBaseTestNoLogging() {}

  virtual ~GcsBaseTestNoLogging() {}
};

#endif  // GCS_BASE_TEST_INCLUDED
