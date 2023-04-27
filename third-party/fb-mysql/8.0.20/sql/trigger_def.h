/*
   Copyright (c) 2013, 2016, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TRIGGER_DEF_H_INCLUDED
#define TRIGGER_DEF_H_INCLUDED

///////////////////////////////////////////////////////////////////////////

/**
  @file

  @brief
  This file defines all base public constants related to triggers in MySQL.
*/

///////////////////////////////////////////////////////////////////////////

/**
  Constants to enumerate possible event types on which triggers can be fired.
*/
enum enum_trigger_event_type {
  TRG_EVENT_INSERT = 0,
  TRG_EVENT_UPDATE = 1,
  TRG_EVENT_DELETE = 2,
  TRG_EVENT_MAX
};

/**
  Constants to enumerate possible timings when triggers can be fired.
*/
enum enum_trigger_action_time_type {
  TRG_ACTION_BEFORE = 0,
  TRG_ACTION_AFTER = 1,
  TRG_ACTION_MAX
};

/**
  Possible trigger ordering clause values:
    - TRG_ORDER_NONE     -- trigger ordering clause is not specified
    - TRG_ORDER_FOLLOWS  -- FOLLOWS clause
    - TRG_ORDER_PRECEDES -- PRECEDES clause
*/
enum enum_trigger_order_type {
  TRG_ORDER_NONE = 0,
  TRG_ORDER_FOLLOWS = 1,
  TRG_ORDER_PRECEDES = 2
};

/**
  Enum constants to designate NEW and OLD trigger pseudo-variables.
*/
enum enum_trigger_variable_type { TRG_OLD_ROW, TRG_NEW_ROW };

#endif  // TRIGGER_DEF_H_INCLUDED
