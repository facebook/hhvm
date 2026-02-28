/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef HANDLER_T_INCLUDED
#define HANDLER_T_INCLUDED

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "my_inttypes.h"
#include "unittest/gunit/base_mock_handler.h"

/**
  A mock handler extending Base_mock_HANDLER
 */
class Mock_HANDLER : public Base_mock_HANDLER {
 public:
  // Declare the members we actually want to test.
  MOCK_METHOD2(print_error, void(int error, myf errflag));

  Mock_HANDLER(handlerton *ht_arg, TABLE_SHARE *share_arg)
      : Base_mock_HANDLER(ht_arg, share_arg) {}
};

/**
  A mock handler for testing the sampling handler.
*/
class Mock_SAMPLING_HANDLER : public Base_mock_HANDLER {
 public:
  /*
    Declare the members we actually want to test. These are the members that
    should be called by the "default" sampling implementation.
  */
  MOCK_METHOD1(rnd_init, int(bool scan));
  MOCK_METHOD1(rnd_next, int(::uchar *buf));
  MOCK_METHOD0(rnd_end, int());

  Mock_SAMPLING_HANDLER(handlerton *ht_arg, TABLE *table_arg,
                        TABLE_SHARE *share)
      : Base_mock_HANDLER(ht_arg, share) {
    table = table_arg;
  }
};

/**
  A mock for the handlerton struct
*/
class Fake_handlerton : public handlerton {
 public:
  /// Minimal initialization of the handlerton
  Fake_handlerton() { slot = 0; }
};

#endif  // HANDLER_T_INCLUDED
