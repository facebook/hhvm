/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef OPT_EXPLAIN_FORMAT_TRADITIONAL_INCLUDED
#define OPT_EXPLAIN_FORMAT_TRADITIONAL_INCLUDED

#include <stddef.h>

#include "sql/opt_explain_format.h"
#include "sql/parse_tree_node_base.h"

class Item;
class Query_result;
class SELECT_LEX_UNIT;
template <class T>
class List;

/**
  Formatter for the traditional EXPLAIN output
*/

class Explain_format_traditional : public Explain_format {
  class Item_null *nil;
  qep_row column_buffer;  ///< buffer for the current output row

 public:
  Explain_format_traditional() : nil(nullptr) {}

  virtual bool is_hierarchical() const { return false; }
  virtual bool send_headers(Query_result *result);
  virtual bool begin_context(enum_parsing_context, SELECT_LEX_UNIT *,
                             const Explain_format_flags *) {
    return false;
  }
  virtual bool end_context(enum_parsing_context) { return false; }
  virtual bool flush_entry();
  virtual qep_row *entry() { return &column_buffer; }

 private:
  bool push_select_type(List<Item> *items);
};

class Explain_format_tree : public Explain_format {
 public:
  Explain_format_tree() {}

  bool is_hierarchical() const override { return false; }
  bool send_headers(Query_result *) override {
    DBUG_ASSERT(false);
    return true;
  }
  bool begin_context(enum_parsing_context, SELECT_LEX_UNIT *,
                     const Explain_format_flags *) override {
    DBUG_ASSERT(false);
    return true;
  }
  bool end_context(enum_parsing_context) override {
    DBUG_ASSERT(false);
    return true;
  }
  bool flush_entry() override {
    DBUG_ASSERT(false);
    return true;
  }
  qep_row *entry() override {
    DBUG_ASSERT(false);
    return nullptr;
  }
  bool is_tree() const override { return true; }

 private:
  bool push_select_type(List<Item> *items);
};

#endif  // OPT_EXPLAIN_FORMAT_TRADITIONAL_INCLUDED
