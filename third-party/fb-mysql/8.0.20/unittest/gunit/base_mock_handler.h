/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BASE_MOCK_HANDLER_INCLUDED
#define BASE_MOCK_HANDLER_INCLUDED

#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "sql/handler.h"

/**
  A base mock handler which declares all the pure virtuals. Create subclasses
  mocking additional virtual functions depending on what you want to test.
 */
class Base_mock_HANDLER : public handler {
 public:
  // Declare all the pure-virtuals.
  // Note: Sun Studio needs a little help in resolving uchar.
  MOCK_METHOD0(close, int());
  MOCK_METHOD4(create, int(const char *name, TABLE *form, HA_CREATE_INFO *,
                           dd::Table *table_def));
  MOCK_METHOD1(info, int(unsigned ha_status_bitmap));
  MOCK_METHOD4(open, int(const char *name, int mode, uint test_if_locked,
                         const dd::Table *table_def));
  MOCK_METHOD1(position, void(const ::uchar *record));
  MOCK_METHOD1(rnd_init, int(bool scan));
  MOCK_METHOD1(rnd_next, int(::uchar *buf));
  MOCK_METHOD2(rnd_pos, int(::uchar *buf, ::uchar *pos));
  MOCK_METHOD3(store_lock,
               THR_LOCK_DATA **(THD *, THR_LOCK_DATA **, thr_lock_type));

  MOCK_CONST_METHOD3(index_flags, ulong(::uint idx, ::uint part, bool));
  MOCK_CONST_METHOD0(table_flags, Table_flags());
  MOCK_CONST_METHOD0(table_type, const char *());
  MOCK_METHOD2(print_error, void(int error, myf errflag));

  Base_mock_HANDLER(handlerton *ht_arg, TABLE_SHARE *share_arg)
      : handler(ht_arg, share_arg) {}
};

#endif  // BASE_MOCK_HANDLER_INCLUDED
