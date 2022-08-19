#ifndef ITEM_XMLFUNC_INCLUDED
#define ITEM_XMLFUNC_INCLUDED

/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_inttypes.h"
#include "my_xml.h"            // my_xml_node_type
#include "sql/item_strfunc.h"  // Item_str_func
#include "sql/parse_tree_node_base.h"
#include "sql_string.h"

class Item;
class THD;

/* This file defines all XML functions */

/* Structure to store a parsed XML tree */
struct MY_XML_NODE {
  uint level;                 /* level in XML tree, 0 means root node   */
  enum my_xml_node_type type; /* node type: node, or attribute, or text */
  uint parent;                /* link to the parent                     */
  const char *beg;            /* beginning of the name or text          */
  const char *end;            /* end of the name or text                */
  const char *tagend;         /* where this tag ends                    */
};

using ParsedXML = std::vector<MY_XML_NODE>;

class Item_xml_str_func : public Item_str_func {
 protected:
  ParsedXML pxml;
  Item *nodeset_func;
  String xpath_tmp_value;

 public:
  Item_xml_str_func(const POS &pos, Item *a, Item *b)
      : Item_str_func(pos, a, b), nodeset_func(nullptr) {
    maybe_null = true;
  }
  Item_xml_str_func(const POS &pos, Item *a, Item *b, Item *c)
      : Item_str_func(pos, a, b, c), nodeset_func(nullptr) {
    maybe_null = true;
  }
  bool resolve_type(THD *thd) override;
  bool check_function_as_value_generator(uchar *) override { return false; }

 protected:
  /**
    Parse the specified XPATH expression and initialize @c nodeset_func.

    @note This is normally called in resolve phase since we only support
          constant XPATH expressions, but it may be called upon execution when
          const value is not yet known at resolve time.

    @param xpath_expr XPATH expression to be parsed

    @returns false on success, true on error
   */
  bool parse_xpath(Item *xpath_expr);
};

class Item_func_xml_extractvalue final : public Item_xml_str_func {
  String tmp_value;

 public:
  Item_func_xml_extractvalue(const POS &pos, Item *a, Item *b)
      : Item_xml_str_func(pos, a, b) {}
  const char *func_name() const override { return "extractvalue"; }
  String *val_str(String *) override;
};

class Item_func_xml_update final : public Item_xml_str_func {
  String tmp_value;

 public:
  Item_func_xml_update(const POS &pos, Item *a, Item *b, Item *c)
      : Item_xml_str_func(pos, a, b, c) {}
  const char *func_name() const override { return "updatexml"; }
  String *val_str(String *) override;
  bool check_function_as_value_generator(uchar *checker_args) override {
    auto *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return true;
  }
};

#endif /* ITEM_XMLFUNC_INCLUDED */
