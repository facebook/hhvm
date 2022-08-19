/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef OPT_EXPLAIN_FORMAT_JSON_INCLUDED
#define OPT_EXPLAIN_FORMAT_JSON_INCLUDED

#include <stddef.h>

#include "sql/opt_explain_format.h"
#include "sql/parse_tree_node_base.h"

class Query_result;
class SELECT_LEX_UNIT;

namespace opt_explain_json_namespace {
class context;
}

/**
  Formatter class for EXPLAIN FORMAT=JSON output
*/

class Explain_format_JSON : public Explain_format {
 private:
  opt_explain_json_namespace::context *current_context;  ///< current tree node

 public:
  Explain_format_JSON() : current_context(nullptr) {}

  virtual bool is_hierarchical() const { return true; }
  virtual bool send_headers(Query_result *result);
  virtual bool begin_context(enum_parsing_context context,
                             SELECT_LEX_UNIT *subquery,
                             const Explain_format_flags *flags);
  virtual bool end_context(enum_parsing_context context);
  virtual bool flush_entry() { return false; }
  virtual qep_row *entry();
};

#endif  // OPT_EXPLAIN_FORMAT_JSON_INCLUDED
