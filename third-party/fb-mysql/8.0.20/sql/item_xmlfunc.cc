/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/item_xmlfunc.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <algorithm>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/check_stack.h"
#include "sql/current_thd.h"
#include "sql/derror.h"  // ER_THD
#include "sql/item.h"
#include "sql/item_cmpfunc.h"  // Item_bool_func
#include "sql/item_func.h"
#include "sql/sp_pcontext.h"  // sp_variable
#include "sql/sql_class.h"    // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"

/*
  TODO: future development directions:
  1. add real constants for XPATH_NODESET_CMP and XPATH_NODESET
     into enum Type in item.h.
  2. add nodeset_to_nodeset_comparator
  3. add lacking functions:
       - name()
       - lang()
       - string()
       - id()
       - translate()
       - local-name()
       - starts-with()
       - namespace-uri()
       - substring-after()
       - normalize-space()
       - substring-before()
  4. add lacking axis:
       - following-sibling
       - following,
       - preceding-sibling
       - preceding
*/

/* Lexical analizer token */
struct MY_XPATH_LEX {
  int term;        /* token type, see MY_XPATH_LEX_XXXXX below */
  const char *beg; /* beginnign of the token                   */
  const char *end; /* end of the token                         */
};

/* Structure to store nodesets */
struct MY_XPATH_FLT {
  uint num;  /* absolute position in MY_XML_NODE array */
  uint pos;  /* relative position in context           */
  uint size; /* context size                           */
};

struct MY_XPATH;

/* XPath function creator */
struct MY_XPATH_FUNC {
  const char *name; /* function name           */
  size_t length;    /* function name length    */
  size_t minargs;   /* min number of arguments */
  size_t maxargs;   /* max number of arguments */
  Item *(*create)(MY_XPATH *xpath, Item **args, uint nargs);
};

class Item_nodeset_func;

/* XPath query parser */
struct MY_XPATH {
  int debug;
  MY_XPATH_LEX query;         /* Whole query                               */
  MY_XPATH_LEX lasttok;       /* last scanned token                        */
  MY_XPATH_LEX prevtok;       /* previous scanned token                    */
  int axis;                   /* last scanned axis                         */
  int extra;                  /* last scanned "extra", context dependent   */
  MY_XPATH_FUNC *func;        /* last scanned function creator             */
  Item *item;                 /* current expression                        */
  Item_nodeset_func *context; /* last scanned context                      */
  Item_nodeset_func *rootelement; /* The root element */
  ParsedXML *pxml;        /* Parsed XML, an array of MY_XML_NODE       */
  const CHARSET_INFO *cs; /* character set/collation string comparison */
  int error;
};

using XPathFilter = std::vector<MY_XPATH_FLT>;
// Using std::vector<bool> as a dynamic bitset
using ActiveNodes = std::vector<bool>;

/*
  Common features of the functions returning a node set.
*/
class Item_nodeset_func : public Item_str_func {
 protected:
  const ParsedXML &pxml;
  Item_nodeset_func(const ParsedXML &pxml_arg, const CHARSET_INFO *cs)
      : Item_str_func(), pxml(pxml_arg) {
    collation.collation = cs;
  }
  Item_nodeset_func(Item *a, const ParsedXML &pxml_arg, const CHARSET_INFO *cs)
      : Item_str_func(a), pxml(pxml_arg) {
    collation.collation = cs;
  }
  Item_nodeset_func(Item *a, Item *b, const ParsedXML &pxml_arg,
                    const CHARSET_INFO *cs)
      : Item_str_func(a, b), pxml(pxml_arg) {
    collation.collation = cs;
  }

 public:
  /**
   Evaluate an XPath function.

   @details
   The SQL interface consists of extractvalue() and updatexml().
   Both of them ensure that none of the arguments are NULL, before
   invoking parse_xml(). So, we should never see any NULL SQL input
   and never return NULL from the overridden member functions in
   item_xmlfunc.cc.

   @see Item_xml_str_func::parse_xpath()
   @see Item_func_xml_update::val_str()

   @param nodeset   the nodeset to be modified
  */
  virtual void val_nodeset(XPathFilter *nodeset) const = 0;
  enum Type type() const override { return XPATH_NODESET; }
  String *val_str(String *str) override {
    XPathFilter res;
    val_nodeset(&res);
    ActiveNodes active(pxml.size(), false);
    for (auto &flt : res) {
      for (uint j = 0; j < pxml.size(); j++) {
        const MY_XML_NODE *node = &pxml[j];
        if (node->type == MY_XML_NODE_TEXT && node->parent == flt.num)
          active[j] = true;
      }
    }

    str->length(0);
    str->set_charset(collation.collation);
    for (uint i = 0; i < pxml.size(); i++) {
      if (active[i]) {
        const MY_XML_NODE *node = &pxml[i];
        if (str->length()) str->append(" ", 1, &my_charset_latin1);
        str->append(node->beg, node->end - node->beg);
      }
    }
    return str;
  }
  enum Item_result result_type() const override { return STRING_RESULT; }
  bool resolve_type(THD *) override {
    max_length = MAX_BLOB_WIDTH;
    // To avoid premature evaluation, mark all nodeset functions as non-const.
    used_tables_cache = RAND_TABLE_BIT;
    return false;
  }
  const char *func_name() const override { return "nodeset"; }
  bool check_function_as_value_generator(uchar *checker_args) override {
    auto *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return true;
  }
};

/* Returns an XML root */
class Item_nodeset_func_rootelement : public Item_nodeset_func {
 public:
  Item_nodeset_func_rootelement(const ParsedXML &pxml_arg,
                                const CHARSET_INFO *cs)
      : Item_nodeset_func(pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_rootelement"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns a Union of two node sets */
class Item_nodeset_func_union : public Item_nodeset_func {
 public:
  Item_nodeset_func_union(Item *a, Item *b, const ParsedXML &pxml_arg,
                          const CHARSET_INFO *cs)
      : Item_nodeset_func(a, b, pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_union"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Makes one step towards the given axis */
class Item_nodeset_func_axisbyname : public Item_nodeset_func {
  const char *node_name;
  const uint node_namelen;

 public:
  Item_nodeset_func_axisbyname(Item *a, const char *n_arg, uint l_arg,
                               const ParsedXML &pxml_arg,
                               const CHARSET_INFO *cs)
      : Item_nodeset_func(a, pxml_arg, cs),
        node_name(n_arg),
        node_namelen(l_arg) {}
  const char *func_name() const override { return "xpath_axisbyname"; }
  bool validname(const MY_XML_NODE &n) const {
    if (node_name[0] == '*') return true;
    return (node_namelen == static_cast<uint>(n.end - n.beg)) &&
           !memcmp(node_name, n.beg, node_namelen);
  }
};

/* Returns self */
class Item_nodeset_func_selfbyname : public Item_nodeset_func_axisbyname {
 public:
  Item_nodeset_func_selfbyname(Item *a, const char *n_arg, uint l_arg,
                               const ParsedXML &pxml_arg,
                               const CHARSET_INFO *cs)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_selfbyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns children */
class Item_nodeset_func_childbyname : public Item_nodeset_func_axisbyname {
 public:
  Item_nodeset_func_childbyname(Item *a, const char *n_arg, uint l_arg,
                                const ParsedXML &pxml_arg,
                                const CHARSET_INFO *cs)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_childbyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns descendants */
class Item_nodeset_func_descendantbyname : public Item_nodeset_func_axisbyname {
  const bool need_self;

 public:
  Item_nodeset_func_descendantbyname(Item *a, const char *n_arg, uint l_arg,
                                     const ParsedXML &pxml_arg,
                                     const CHARSET_INFO *cs, bool need_self_arg)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs),
        need_self(need_self_arg) {}
  const char *func_name() const override { return "xpath_descendantbyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns ancestors */
class Item_nodeset_func_ancestorbyname : public Item_nodeset_func_axisbyname {
  bool need_self;

 public:
  Item_nodeset_func_ancestorbyname(Item *a, const char *n_arg, uint l_arg,
                                   const ParsedXML &pxml_arg,
                                   const CHARSET_INFO *cs, bool need_self_arg)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs),
        need_self(need_self_arg) {}
  const char *func_name() const override { return "xpath_ancestorbyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns parents */
class Item_nodeset_func_parentbyname : public Item_nodeset_func_axisbyname {
 public:
  Item_nodeset_func_parentbyname(Item *a, const char *n_arg, uint l_arg,
                                 const ParsedXML &pxml_arg,
                                 const CHARSET_INFO *cs)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_parentbyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Returns attributes */
class Item_nodeset_func_attributebyname : public Item_nodeset_func_axisbyname {
 public:
  Item_nodeset_func_attributebyname(Item *a, const char *n_arg, uint l_arg,
                                    const ParsedXML &pxml_arg,
                                    const CHARSET_INFO *cs)
      : Item_nodeset_func_axisbyname(a, n_arg, l_arg, pxml_arg, cs) {}
  const char *func_name() const override { return "xpath_attributebyname"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

class Item_nodeset_context_cache;

/*
  Condition iterator: goes through all nodes in the current
  context and checks a condition, returning those nodes
  giving true condition result.
*/
class Item_nodeset_func_predicate : public Item_nodeset_func {
  Item_nodeset_context_cache *m_context_cache;

 public:
  Item_nodeset_func_predicate(Item *a, Item *b, const ParsedXML &pxml_arg,
                              const CHARSET_INFO *cs,
                              Item_nodeset_context_cache *context_cache)
      : Item_nodeset_func(a, b, pxml_arg, cs), m_context_cache(context_cache) {}
  const char *func_name() const override { return "xpath_predicate"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/* Selects nodes with a given position in context */
class Item_nodeset_func_elementbyindex : public Item_nodeset_func {
  Item_nodeset_context_cache *m_context_cache;

 public:
  Item_nodeset_func_elementbyindex(Item *a, Item *b, const ParsedXML &pxml_arg,
                                   const CHARSET_INFO *cs,
                                   Item_nodeset_context_cache *context_cache)
      : Item_nodeset_func(a, b, pxml_arg, cs), m_context_cache(context_cache) {}
  const char *func_name() const override { return "xpath_elementbyindex"; }
  void val_nodeset(XPathFilter *nodeset) const override;
};

/*
  We need to distinguish a number from a boolean:
  a[1] and a[true] are different things in XPath.
*/
class Item_bool final : public Item_int {
 public:
  explicit Item_bool(int32 i) : Item_int(i) {}
  bool is_bool_func() const override { return true; }
};

/*
  Converts its argument into a boolean value.
  * a number is true if it is non-zero
  * a node-set is true if and only if it is non-empty
  * a string is true if and only if its length is non-zero
*/
class Item_xpath_cast_bool final : public Item_int_func {
 public:
  explicit Item_xpath_cast_bool(Item *a) : Item_int_func(a) {}
  const char *func_name() const override { return "xpath_cast_bool"; }
  bool is_bool_func() const override { return true; }
  longlong val_int() override {
    if (args[0]->type() == XPATH_NODESET) {
      auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
      XPathFilter flt;
      nodeset_func->val_nodeset(&flt);
      return flt.size() == 1 ? 1 : 0;
    }
    return args[0]->val_real() != 0 ? 1 : 0;
  }
};

/*
  Converts its argument into a number
*/
class Item_xpath_cast_number : public Item_real_func {
 public:
  explicit Item_xpath_cast_number(Item *a) : Item_real_func(a) {}
  const char *func_name() const override { return "xpath_cast_number"; }
  double val_real() override { return args[0]->val_real(); }
};

/*
  Context cache, for predicate
*/
class Item_nodeset_context_cache : public Item_nodeset_func {
  bool m_is_empty;
  uint32 m_num;
  uint32 m_pos;
  size_t m_size;

 public:
  Item_nodeset_context_cache(const ParsedXML &pxml_arg, const CHARSET_INFO *cs)
      : Item_nodeset_func(pxml_arg, cs),
        m_is_empty(true),
        m_num(0),
        m_pos(0),
        m_size(0) {}
  void val_nodeset(XPathFilter *nodeset) const override {
    nodeset->clear();
    if (!m_is_empty)
      nodeset->push_back({m_num, m_pos, static_cast<uint>(m_size)});
  }
  bool resolve_type(THD *) override {
    max_length = MAX_BLOB_WIDTH;
    return false;
  }
  void set_element(uint32 num, uint32 pos, size_t size) {
    m_num = num;
    m_pos = pos;
    m_size = size;
    m_is_empty = false;
  }
};

class Item_func_xpath_position : public Item_int_func {
 public:
  explicit Item_func_xpath_position(Item *a) : Item_int_func(a) {}
  const char *func_name() const override { return "xpath_position"; }
  bool resolve_type(THD *) override {
    max_length = 10;
    return false;
  }
  longlong val_int() override {
    auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
    XPathFilter flt;
    nodeset_func->val_nodeset(&flt);
    if (flt.size() == 1) return flt.at(0).pos + 1;
    return 0;
  }
};

class Item_func_xpath_count : public Item_int_func {
 public:
  explicit Item_func_xpath_count(Item *a) : Item_int_func(a) {}
  const char *func_name() const override { return "xpath_count"; }
  bool resolve_type(THD *) override {
    max_length = 10;
    return false;
  }
  longlong val_int() override {
    uint predicate_supplied_context_size;
    auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
    XPathFilter res;
    nodeset_func->val_nodeset(&res);
    if (res.size() == 1 && (predicate_supplied_context_size = res.at(0).size))
      return predicate_supplied_context_size;
    return static_cast<longlong>(res.size());
  }
};

class Item_func_xpath_sum : public Item_real_func {
  const ParsedXML *pxml;

 public:
  Item_func_xpath_sum(Item *a, const ParsedXML *p)
      : Item_real_func(a), pxml(p) {}

  const char *func_name() const override { return "xpath_sum"; }
  double val_real() override {
    double sum = 0;
    auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
    XPathFilter res;
    nodeset_func->val_nodeset(&res);

    for (auto &flt : res) {
      const MY_XML_NODE *self = &pxml->at(flt.num);
      for (uint j = flt.num + 1; j < pxml->size(); j++) {
        const MY_XML_NODE *node = &pxml->at(j);
        if (node->level <= self->level) break;
        if ((node->parent == flt.num) && (node->type == MY_XML_NODE_TEXT)) {
          const char *end;
          int err;
          double add = my_strntod(collation.collation, node->beg,
                                  node->end - node->beg, &end, &err);
          if (!err) sum += add;
        }
      }
    }
    return sum;
  }
};

class Item_nodeset_to_const_comparator final : public Item_bool_func {
  const ParsedXML *pxml;

 public:
  Item_nodeset_to_const_comparator(Item_nodeset_func *nodeset,
                                   Item_bool_func *cmpfunc, const ParsedXML *p)
      : Item_bool_func(nodeset, cmpfunc), pxml(p) {}
  enum Type type() const override { return XPATH_NODESET_CMP; }
  const char *func_name() const override {
    return "xpath_nodeset_to_const_comparator";
  }
  bool is_bool_func() const override { return true; }

  longlong val_int() override {
    auto comp = down_cast<Item_bool_func *>(args[1]);
    auto fake = down_cast<Item_string *>(comp->arguments()[0]);
    fake->collation.collation = collation.collation;
    auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
    XPathFilter res;
    nodeset_func->val_nodeset(&res);

    for (auto &flt : res) {
      const MY_XML_NODE *self = &pxml->at(flt.num);
      for (uint j = flt.num + 1; j < pxml->size(); j++) {
        const MY_XML_NODE *node = &pxml->at(j);
        if (node->level <= self->level) break;
        if ((node->parent == flt.num) && (node->type == MY_XML_NODE_TEXT)) {
          fake->set_str_with_copy(node->beg,
                                  static_cast<uint>(node->end - node->beg));
          if (args[1]->val_int()) return 1;
        }
      }
    }
    return 0;
  }

  bool check_function_as_value_generator(uchar *checker_args) override {
    auto *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(
            checker_args);
    func_arg->banned_function_name = func_name();
    return true;
  }
};

void Item_nodeset_func_rootelement::val_nodeset(XPathFilter *nodeset) const {
  nodeset->clear();
  nodeset->push_back({0, 0, 0});
}

void Item_nodeset_func_union::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func1 = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter set0;
  nodeset_func1->val_nodeset(&set0);
  auto *nodeset_func2 = down_cast<const Item_nodeset_func *>(args[1]);
  XPathFilter set1;
  nodeset_func2->val_nodeset(&set1);
  ActiveNodes both(pxml.size(), false);

  for (auto &i : set0) both[i.num] = true;

  for (auto &i : set1) both[i.num] = true;

  nodeset->clear();
  for (uint i = 0, pos = 0; i < pxml.size(); i++) {
    if (both[i]) nodeset->push_back({i, pos++, 0});
  }
}

void Item_nodeset_func_selfbyname::val_nodeset(XPathFilter *nodeset) const {
  XPathFilter res;
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    if (validname(pxml[flt.num])) nodeset->push_back({flt.num, 0, 0});
  }
}

void Item_nodeset_func_childbyname::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    const MY_XML_NODE *self = &pxml[flt.num];
    for (uint pos = 0, j = flt.num + 1; j < pxml.size(); j++) {
      const MY_XML_NODE *node = &pxml[j];
      if (node->level <= self->level) break;
      if ((node->parent == flt.num) && (node->type == MY_XML_NODE_TAG) &&
          validname(*node))
        nodeset->push_back({j, pos++, 0});
    }
  }
}

void Item_nodeset_func_descendantbyname::val_nodeset(
    XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    uint pos = 0;
    const MY_XML_NODE *self = &pxml[flt.num];
    if (need_self && validname(*self)) nodeset->push_back({flt.num, pos++, 0});
    for (uint j = flt.num + 1; j < pxml.size(); j++) {
      const MY_XML_NODE *node = &pxml[j];
      if (node->level <= self->level) break;
      if ((node->type == MY_XML_NODE_TAG) && validname(*node))
        nodeset->push_back({j, pos++, 0});
    }
  }
}

void Item_nodeset_func_ancestorbyname::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  ActiveNodes active(pxml.size(), false);
  uint pos = 0;

  for (auto &flt : res) {
    /*
       Go to the root and add all nodes on the way.
       Don't add the root if context is the root itelf
    */
    const MY_XML_NODE *self = &pxml[flt.num];
    if (need_self && validname(*self)) {
      active[flt.num] = true;
      pos++;
    }

    for (uint j = self->parent; pxml[j].parent != j; j = pxml[j].parent) {
      if (flt.num && validname(pxml[j])) {
        active[j] = true;
        pos++;
      }
    }
  }

  for (uint j = 0; j < pxml.size(); j++) {
    if (active[j]) nodeset->push_back({j, --pos, 0});
  }
}

void Item_nodeset_func_parentbyname::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  ActiveNodes active(pxml.size(), false);
  for (auto &flt : res) {
    uint j = pxml[flt.num].parent;
    if (flt.num && validname(pxml[j])) active[j] = true;
  }
  for (uint j = 0, pos = 0; j < pxml.size(); j++) {
    if (active[j]) nodeset->push_back({j, pos++, 0});
  }
}

void Item_nodeset_func_attributebyname::val_nodeset(
    XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<const Item_nodeset_func *>(args[0]);
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    const MY_XML_NODE *self = &pxml[flt.num];
    for (uint pos = 0, j = flt.num + 1; j < pxml.size(); j++) {
      const MY_XML_NODE *node = &pxml[j];
      if (node->level <= self->level) break;
      if ((node->parent == flt.num) && (node->type == MY_XML_NODE_ATTR) &&
          validname(*node))
        nodeset->push_back({j, pos++, 0});
    }
  }
}

void Item_nodeset_func_predicate::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<Item_nodeset_func *>(args[0]);
  // comp_func may actually be Item_bool rather than Item_func
  Item *comp_func = args[1];
  uint pos = 0;
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    m_context_cache->set_element(flt.num, flt.pos,
                                 res.size());  // Not thread safe
    if (comp_func->val_int()) nodeset->push_back({flt.num, pos++, 0});
  }
}

void Item_nodeset_func_elementbyindex::val_nodeset(XPathFilter *nodeset) const {
  auto *nodeset_func = down_cast<Item_nodeset_func *>(args[0]);
  uint pos = 0;
  XPathFilter res;
  nodeset_func->val_nodeset(&res);
  nodeset->clear();
  for (auto &flt : res) {
    m_context_cache->set_element(flt.num, flt.pos,
                                 res.size());  // Not thread safe
    int index = static_cast<int>(args[1]->val_int()) - 1;
    if (index >= 0 &&
        (flt.pos == static_cast<uint>(index) || args[1]->is_bool_func()))
      nodeset->push_back({flt.num, pos++, 0});
  }
}

/*
  If item is a node set, then casts it to boolean,
  otherwise returns the item itself.
*/
static Item *nodeset2bool(Item *item) {
  if (item->type() == Item::XPATH_NODESET)
    return new Item_xpath_cast_bool(item);
  return item;
}

/*
  XPath lexical tokens
*/
#define MY_XPATH_LEX_DIGITS 'd'
#define MY_XPATH_LEX_IDENT 'i'
#define MY_XPATH_LEX_STRING 's'
#define MY_XPATH_LEX_SLASH '/'
#define MY_XPATH_LEX_LB '['
#define MY_XPATH_LEX_RB ']'
#define MY_XPATH_LEX_LP '('
#define MY_XPATH_LEX_RP ')'
#define MY_XPATH_LEX_EQ '='
#define MY_XPATH_LEX_LESS '<'
#define MY_XPATH_LEX_GREATER '>'
#define MY_XPATH_LEX_AT '@'
#define MY_XPATH_LEX_COLON ':'
#define MY_XPATH_LEX_ASTERISK '*'
#define MY_XPATH_LEX_DOT '.'
#define MY_XPATH_LEX_VLINE '|'
#define MY_XPATH_LEX_MINUS '-'
#define MY_XPATH_LEX_PLUS '+'
#define MY_XPATH_LEX_EXCL '!'
#define MY_XPATH_LEX_COMMA ','
#define MY_XPATH_LEX_DOLLAR '$'
#define MY_XPATH_LEX_ERROR 'A'
#define MY_XPATH_LEX_EOF 'B'
#define MY_XPATH_LEX_AND 'C'
#define MY_XPATH_LEX_OR 'D'
#define MY_XPATH_LEX_DIV 'E'
#define MY_XPATH_LEX_MOD 'F'
#define MY_XPATH_LEX_FUNC 'G'
#define MY_XPATH_LEX_NODETYPE 'H'
#define MY_XPATH_LEX_AXIS 'I'
#define MY_XPATH_LEX_LE 'J'
#define MY_XPATH_LEX_GE 'K'

/*
  XPath axis type
*/
#define MY_XPATH_AXIS_ANCESTOR 0
#define MY_XPATH_AXIS_ANCESTOR_OR_SELF 1
#define MY_XPATH_AXIS_ATTRIBUTE 2
#define MY_XPATH_AXIS_CHILD 3
#define MY_XPATH_AXIS_DESCENDANT 4
#define MY_XPATH_AXIS_DESCENDANT_OR_SELF 5
#define MY_XPATH_AXIS_FOLLOWING 6
#define MY_XPATH_AXIS_FOLLOWING_SIBLING 7
#define MY_XPATH_AXIS_NAMESPACE 8
#define MY_XPATH_AXIS_PARENT 9
#define MY_XPATH_AXIS_PRECEDING 10
#define MY_XPATH_AXIS_PRECEDING_SIBLING 11
#define MY_XPATH_AXIS_SELF 12

/*
  Create scalar comparator

  SYNOPSYS
    Create a comparator function for scalar arguments,
    for the given arguments and operation.

  RETURN
    The newly created item.
*/
static Item_bool_func *eq_func(int oper, Item *a, Item *b) {
  switch (oper) {
    case '=':
      return new Item_func_eq(a, b);
    case '!':
      return new Item_func_ne(a, b);
    case MY_XPATH_LEX_GE:
      return new Item_func_ge(a, b);
    case MY_XPATH_LEX_LE:
      return new Item_func_le(a, b);
    case MY_XPATH_LEX_GREATER:
      return new Item_func_gt(a, b);
    case MY_XPATH_LEX_LESS:
      return new Item_func_lt(a, b);
  }
  return nullptr;
}

/*
  Create scalar comparator

  SYNOPSYS
    Create a comparator function for scalar arguments,
    for the given arguments and reverse operation, e.g.

    A > B  is converted into  B < A

  RETURN
    The newly created item.
*/
static Item_bool_func *eq_func_reverse(int oper, Item *a, Item *b) {
  switch (oper) {
    case '=':
      return new Item_func_eq(a, b);
    case '!':
      return new Item_func_ne(a, b);
    case MY_XPATH_LEX_GE:
      return new Item_func_le(a, b);
    case MY_XPATH_LEX_LE:
      return new Item_func_ge(a, b);
    case MY_XPATH_LEX_GREATER:
      return new Item_func_lt(a, b);
    case MY_XPATH_LEX_LESS:
      return new Item_func_gt(a, b);
  }
  return nullptr;
}

/*
  Create a comparator

  SYNOPSYS
    Create a comparator for scalar or non-scalar arguments,
    for the given arguments and operation.

  RETURN
    The newly created item.
*/
static Item *create_comparator(MY_XPATH *xpath, int oper, MY_XPATH_LEX *context,
                               Item *a, Item *b) {
  if (a->type() != Item::XPATH_NODESET && b->type() != Item::XPATH_NODESET) {
    return eq_func(oper, a, b);  // two scalar arguments
  } else if (a->type() == Item::XPATH_NODESET &&
             b->type() == Item::XPATH_NODESET) {
    size_t len = xpath->query.end - context->beg;
    len = std::min(len, size_t(32));
    my_printf_error(ER_UNKNOWN_ERROR,
                    "XPATH error: "
                    "comparison of two nodesets is not supported: '%.*s'",
                    MYF(0), static_cast<int>(len), context->beg);

    return nullptr;  // TODO: Comparison of two nodesets
  } else {
    /*
     Compare a node set to a scalar value.
     We just create a fake Item_string() argument,
     which will be filled to the partular value
     in a loop through all of the nodes in the node set.
    */

    Item_string *fake = new Item_string("", 0, xpath->cs);
    /* Don't cache fake because its value will be changed during comparison.*/
    fake->set_used_tables(RAND_TABLE_BIT);
    Item_nodeset_func *nodeset;
    Item *scalar;
    Item_bool_func *comp;
    if (a->type() == Item::XPATH_NODESET) {
      nodeset = down_cast<Item_nodeset_func *>(a);
      scalar = b;
      comp = eq_func(oper, fake, scalar);
    } else {
      nodeset = down_cast<Item_nodeset_func *>(b);
      scalar = a;
      comp = eq_func_reverse(oper, fake, scalar);
    }
    return new Item_nodeset_to_const_comparator(nodeset, comp, xpath->pxml);
  }
}

/*
  Create a step

  SYNOPSYS
    Create a step function for the given argument and axis.

  RETURN
    The newly created item.
*/
static Item_nodeset_func *nametestfunc(MY_XPATH *xpath, int type, Item *arg,
                                       const char *beg, size_t len) {
  DBUG_ASSERT(arg != nullptr);
  DBUG_ASSERT(arg->type() == Item::XPATH_NODESET);
  DBUG_ASSERT(beg != nullptr);
  DBUG_ASSERT(len > 0);

  Item_nodeset_func *res;
  switch (type) {
    case MY_XPATH_AXIS_ANCESTOR:
      res = new Item_nodeset_func_ancestorbyname(arg, beg, len, *xpath->pxml,
                                                 xpath->cs, false);
      break;
    case MY_XPATH_AXIS_ANCESTOR_OR_SELF:
      res = new Item_nodeset_func_ancestorbyname(arg, beg, len, *xpath->pxml,
                                                 xpath->cs, true);
      break;
    case MY_XPATH_AXIS_PARENT:
      res = new Item_nodeset_func_parentbyname(arg, beg, len, *xpath->pxml,
                                               xpath->cs);
      break;
    case MY_XPATH_AXIS_DESCENDANT:
      res = new Item_nodeset_func_descendantbyname(arg, beg, len, *xpath->pxml,
                                                   xpath->cs, false);
      break;
    case MY_XPATH_AXIS_DESCENDANT_OR_SELF:
      res = new Item_nodeset_func_descendantbyname(arg, beg, len, *xpath->pxml,
                                                   xpath->cs, true);
      break;
    case MY_XPATH_AXIS_ATTRIBUTE:
      res = new Item_nodeset_func_attributebyname(arg, beg, len, *xpath->pxml,
                                                  xpath->cs);
      break;
    case MY_XPATH_AXIS_SELF:
      res = new Item_nodeset_func_selfbyname(arg, beg, len, *xpath->pxml,
                                             xpath->cs);
      break;
    default:
      res = new Item_nodeset_func_childbyname(arg, beg, len, *xpath->pxml,
                                              xpath->cs);
  }
  return res;
}

/*
  Tokens consisting of one character, for faster lexical analizer.
*/
static char simpletok[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    /*
        ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ?
      @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _
      ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~ \80
    */
    0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};

/*
  XPath keywords
*/
struct my_xpath_keyword_names_st {
  int tok;
  const char *name;
  size_t length;
  int extra;
};

static struct my_xpath_keyword_names_st my_keyword_names[] = {
    {MY_XPATH_LEX_AND, "and", 3, 0},
    {MY_XPATH_LEX_OR, "or", 2, 0},
    {MY_XPATH_LEX_DIV, "div", 3, 0},
    {MY_XPATH_LEX_MOD, "mod", 3, 0},
    {0, nullptr, 0, 0}};

static struct my_xpath_keyword_names_st my_axis_names[] = {
    {MY_XPATH_LEX_AXIS, "ancestor", 8, MY_XPATH_AXIS_ANCESTOR},
    {MY_XPATH_LEX_AXIS, "ancestor-or-self", 16, MY_XPATH_AXIS_ANCESTOR_OR_SELF},
    {MY_XPATH_LEX_AXIS, "attribute", 9, MY_XPATH_AXIS_ATTRIBUTE},
    {MY_XPATH_LEX_AXIS, "child", 5, MY_XPATH_AXIS_CHILD},
    {MY_XPATH_LEX_AXIS, "descendant", 10, MY_XPATH_AXIS_DESCENDANT},
    {MY_XPATH_LEX_AXIS, "descendant-or-self", 18,
     MY_XPATH_AXIS_DESCENDANT_OR_SELF},
    {MY_XPATH_LEX_AXIS, "following", 9, MY_XPATH_AXIS_FOLLOWING},
    {MY_XPATH_LEX_AXIS, "following-sibling", 17,
     MY_XPATH_AXIS_FOLLOWING_SIBLING},
    {MY_XPATH_LEX_AXIS, "namespace", 9, MY_XPATH_AXIS_NAMESPACE},
    {MY_XPATH_LEX_AXIS, "parent", 6, MY_XPATH_AXIS_PARENT},
    {MY_XPATH_LEX_AXIS, "preceding", 9, MY_XPATH_AXIS_PRECEDING},
    {MY_XPATH_LEX_AXIS, "preceding-sibling", 17,
     MY_XPATH_AXIS_PRECEDING_SIBLING},
    {MY_XPATH_LEX_AXIS, "self", 4, MY_XPATH_AXIS_SELF},
    {0, nullptr, 0, 0}};

static struct my_xpath_keyword_names_st my_nodetype_names[] = {
    {MY_XPATH_LEX_NODETYPE, "comment", 7, 0},
    {MY_XPATH_LEX_NODETYPE, "text", 4, 0},
    {MY_XPATH_LEX_NODETYPE, "processing-instruction", 22, 0},
    {MY_XPATH_LEX_NODETYPE, "node", 4, 0},
    {0, nullptr, 0, 0}};

/*
  Lookup a keyword

  SYNOPSYS
    Check that the last scanned identifier is a keyword.

  RETURN
    - Token type, on lookup success.
    - MY_XPATH_LEX_IDENT, on lookup failure.
*/
static int my_xpath_keyword(MY_XPATH *x,
                            struct my_xpath_keyword_names_st *keyword_names,
                            const char *beg, const char *end) {
  struct my_xpath_keyword_names_st *k;
  size_t length = end - beg;
  for (k = keyword_names; k->name; k++) {
    if (length == k->length && !native_strncasecmp(beg, k->name, length)) {
      x->extra = k->extra;
      return k->tok;
    }
  }
  return MY_XPATH_LEX_IDENT;
}

/*
  Functions to create an item, a-la those in item_create.cc
*/

static Item *create_func_true(MY_XPATH *, Item **, uint) {
  return new Item_bool(1);
}

static Item *create_func_false(MY_XPATH *, Item **, uint) {
  return new Item_bool(0);
}

static Item *create_func_not(MY_XPATH *, Item **args, uint) {
  return new Item_func_not(nodeset2bool(args[0]));
}

static Item *create_func_ceiling(MY_XPATH *, Item **args, uint) {
  return new Item_func_ceiling(args[0]);
}

static Item *create_func_floor(MY_XPATH *, Item **args, uint) {
  return new Item_func_floor(args[0]);
}

static Item *create_func_bool(MY_XPATH *, Item **args, uint) {
  return new Item_xpath_cast_bool(args[0]);
}

static Item *create_func_number(MY_XPATH *xpath, Item **args, uint nargs) {
  Item *arg;

  if (nargs > 0) {
    arg = args[0];
  } else {
    arg = xpath->context != nullptr ? xpath->context : xpath->rootelement;
  }
  return new Item_xpath_cast_number(arg);
}

static Item *create_func_string_length(MY_XPATH *xpath, Item **args,
                                       uint nargs) {
  Item *arg = nargs ? args[0] : xpath->context;
  return arg ? new Item_func_char_length(arg) : nullptr;
}

static Item *create_func_round(MY_XPATH *, Item **args, uint) {
  return new Item_func_round(args[0], new Item_int_0(), false);
}

static Item *create_func_last(MY_XPATH *xpath, Item **, uint) {
  return xpath->context ? new Item_func_xpath_count(xpath->context) : nullptr;
}

static Item *create_func_position(MY_XPATH *xpath, Item **, uint) {
  return xpath->context ? new Item_func_xpath_position(xpath->context)
                        : nullptr;
}

static Item *create_func_contains(MY_XPATH *, Item **args, uint) {
  return new Item_xpath_cast_bool(new Item_func_locate(args[0], args[1]));
}

static Item *create_func_concat(MY_XPATH *, Item **args, uint) {
  return new Item_func_concat(args[0], args[1]);
}

static Item *create_func_substr(MY_XPATH *, Item **args, uint nargs) {
  if (nargs == 2)
    return new Item_func_substr(args[0], args[1]);
  else
    return new Item_func_substr(args[0], args[1], args[2]);
}

static Item *create_func_count(MY_XPATH *, Item **args, uint) {
  if (args[0]->type() != Item::XPATH_NODESET) return nullptr;
  return new Item_func_xpath_count(args[0]);
}

static Item *create_func_sum(MY_XPATH *xpath, Item **args, uint) {
  if (args[0]->type() != Item::XPATH_NODESET) return nullptr;
  return new Item_func_xpath_sum(args[0], xpath->pxml);
}

/*
  Functions names. Separate lists for names with
  lengths 3,4,5 and 6 for faster lookups.
*/
static MY_XPATH_FUNC my_func_names3[] = {{"sum", 3, 1, 1, create_func_sum},
                                         {"not", 3, 1, 1, create_func_not},
                                         {nullptr, 0, 0, 0, nullptr}};

static MY_XPATH_FUNC my_func_names4[] = {{"last", 4, 0, 0, create_func_last},
                                         {"true", 4, 0, 0, create_func_true},
                                         {"name", 4, 0, 1, nullptr},
                                         {"lang", 4, 1, 1, nullptr},
                                         {nullptr, 0, 0, 0, nullptr}};

static MY_XPATH_FUNC my_func_names5[] = {{"count", 5, 1, 1, create_func_count},
                                         {"false", 5, 0, 0, create_func_false},
                                         {"floor", 5, 1, 1, create_func_floor},
                                         {"round", 5, 1, 1, create_func_round},
                                         {nullptr, 0, 0, 0, nullptr}};

static MY_XPATH_FUNC my_func_names6[] = {
    {"concat", 6, 2, 255, create_func_concat},
    {"number", 6, 0, 1, create_func_number},
    {"string", 6, 0, 1, nullptr},
    {nullptr, 0, 0, 0, nullptr}};

/* Other functions, with name longer than 6, all together */
static MY_XPATH_FUNC my_func_names[] = {
    {"id", 2, 1, 1, nullptr},
    {"boolean", 7, 1, 1, create_func_bool},
    {"ceiling", 7, 1, 1, create_func_ceiling},
    {"position", 8, 0, 0, create_func_position},
    {"contains", 8, 2, 2, create_func_contains},
    {"substring", 9, 2, 3, create_func_substr},
    {"translate", 9, 3, 3, nullptr},

    {"local-name", 10, 0, 1, nullptr},
    {"starts-with", 11, 2, 2, nullptr},
    {"namespace-uri", 13, 0, 1, nullptr},
    {"string-length", 13, 0, 1, create_func_string_length},
    {"substring-after", 15, 2, 2, nullptr},
    {"normalize-space", 15, 0, 1, nullptr},
    {"substring-before", 16, 2, 2, nullptr},

    {nullptr, 0, 0, 0, nullptr}};

/*
  Lookup a function by name

  SYNOPSYS
    Lookup a function by its name.

  RETURN
    Pointer to a MY_XPATH_FUNC variable on success.
    0 - on failure.

*/
static MY_XPATH_FUNC *my_xpath_function(const char *beg, const char *end) {
  MY_XPATH_FUNC *k, *function_names;
  size_t length = end - beg;
  switch (length) {
    case 1:
      return nullptr;
    case 3:
      function_names = my_func_names3;
      break;
    case 4:
      function_names = my_func_names4;
      break;
    case 5:
      function_names = my_func_names5;
      break;
    case 6:
      function_names = my_func_names6;
      break;
    default:
      function_names = my_func_names;
  }
  for (k = function_names; k->name; k++)
    if (k->create && length == k->length &&
        !native_strncasecmp(beg, k->name, length))
      return k;
  return nullptr;
}

/* Initialize a lex analizer token */
static void my_xpath_lex_init(MY_XPATH_LEX *lex, const char *str,
                              const char *strend) {
  lex->beg = str;
  lex->end = strend;
}

/* Initialize an XPath query parser */
static void my_xpath_init(MY_XPATH *xpath) {
  memset(xpath, 0, sizeof(xpath[0]));
}

static int my_xdigit(int c) { return ((c) >= '0' && (c) <= '9'); }

/*
  Scan the next token

  SYNOPSYS
    Scan the next token from the input.
    lex->term is set to the scanned token type.
    lex->beg and lex->end are set to the beginnig
    and to the end of the token.
  RETURN
    N/A
*/
static void my_xpath_lex_scan(MY_XPATH *xpath, MY_XPATH_LEX *lex,
                              const char *beg, const char *end) {
  int ch, ctype, length;
  for (; beg < end && *beg == ' '; beg++)
    ;  // skip leading spaces
  lex->beg = beg;

  if (beg >= end) {
    lex->end = beg;
    lex->term = MY_XPATH_LEX_EOF;  // end of line reached
    return;
  }

  // Check ident, or a function call, or a keyword
  if ((length = xpath->cs->cset->ctype(
           xpath->cs, &ctype, reinterpret_cast<const uchar *>(beg),
           reinterpret_cast<const uchar *>(end))) > 0 &&
      ((ctype & (_MY_L | _MY_U)) || *beg == '_')) {
    // scan untill the end of the idenfitier
    for (beg += length;
         (length = xpath->cs->cset->ctype(
              xpath->cs, &ctype, reinterpret_cast<const uchar *>(beg),
              reinterpret_cast<const uchar *>(end))) > 0 &&
         ((ctype & (_MY_L | _MY_U | _MY_NMR)) || *beg == '_' || *beg == '-' ||
          *beg == '.');
         beg += length) /* no op */
      ;
    lex->end = beg;

    if (beg < end) {
      if (*beg == '(') {
        /*
         check if a function call, e.g.: count(/a/b)
         or a nodetype test,       e.g.: /a/b/text()
        */
        if ((xpath->func = my_xpath_function(lex->beg, beg)))
          lex->term = MY_XPATH_LEX_FUNC;
        else
          lex->term = my_xpath_keyword(xpath, my_nodetype_names, lex->beg, beg);
        return;
      }
      // check if an axis specifier, e.g.: /a/b/child::*
      else if (*beg == ':' && beg + 1 < end && beg[1] == ':') {
        lex->term = my_xpath_keyword(xpath, my_axis_names, lex->beg, beg);
        return;
      }
    }
    // check if a keyword
    lex->term = my_xpath_keyword(xpath, my_keyword_names, lex->beg, beg);
    return;
  }

  ch = *beg++;

  if (ch > 0 && ch < 128 && simpletok[ch]) {
    // a token consisting of one character found
    lex->end = beg;
    lex->term = ch;
    return;
  }

  if (my_xdigit(ch))  // a sequence of digits
  {
    for (; beg < end && my_xdigit(*beg); beg++)
      ;
    lex->end = beg;
    lex->term = MY_XPATH_LEX_DIGITS;
    return;
  }

  if (ch == '"' || ch == '\'')  // a string: either '...' or "..."
  {
    for (; beg < end && *beg != ch; beg++)
      ;
    if (beg < end) {
      lex->end = beg + 1;
      lex->term = MY_XPATH_LEX_STRING;
      return;
    } else {
      // unexpected end-of-line, without closing quot sign
      lex->end = end;
      lex->term = MY_XPATH_LEX_ERROR;
      return;
    }
  }

  lex->end = beg;
  lex->term = MY_XPATH_LEX_ERROR;  // unknown character
  return;
}

/*
  Scan the given token

  SYNOPSYS
    Scan the given token and rotate lasttok to prevtok on success.

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_term(MY_XPATH *xpath, int term) {
  if (xpath->lasttok.term == term && !xpath->error) {
    xpath->prevtok = xpath->lasttok;
    my_xpath_lex_scan(xpath, &xpath->lasttok, xpath->lasttok.end,
                      xpath->query.end);
    return 1;
  }
  return 0;
}

/*
  Scan AxisName

  SYNOPSYS
    Scan an axis name and store the scanned axis type into xpath->axis.

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AxisName(MY_XPATH *xpath) {
  int rc = my_xpath_parse_term(xpath, MY_XPATH_LEX_AXIS);
  xpath->axis = xpath->extra;
  return rc;
}

/*********************************************
** Grammar rules, according to http://www.w3.org/TR/xpath
** Implemented using recursive descendant method.
** All the following grammar processing functions accept
** a signle "xpath" argument and return 1 on success and 0 on error.
** They also modify "xpath" argument by creating new items.
*/

/* [9]  PredicateExpr ::= Expr */
#define my_xpath_parse_PredicateExpr(x) my_xpath_parse_Expr((x))

/* [14] Expr ::= OrExpr */
#define my_xpath_parse_Expr(x) my_xpath_parse_OrExpr((x))

static int my_xpath_parse_LocationPath(MY_XPATH *xpath);
static int my_xpath_parse_AbsoluteLocationPath(MY_XPATH *xpath);
static int my_xpath_parse_RelativeLocationPath(MY_XPATH *xpath);
static int my_xpath_parse_AbbreviatedStep(MY_XPATH *xpath);
static int my_xpath_parse_Step(MY_XPATH *xpath);
static int my_xpath_parse_AxisSpecifier(MY_XPATH *xpath);
static int my_xpath_parse_NodeTest(MY_XPATH *xpath);
static int my_xpath_parse_AbbreviatedAxisSpecifier(MY_XPATH *xpath);
static int my_xpath_parse_NameTest(MY_XPATH *xpath);
static int my_xpath_parse_FunctionCall(MY_XPATH *xpath);
static int my_xpath_parse_Number(MY_XPATH *xpath);
static int my_xpath_parse_FilterExpr(MY_XPATH *xpath);
static int my_xpath_parse_PathExpr(MY_XPATH *xpath);
static int my_xpath_parse_OrExpr(MY_XPATH *xpath);
static int my_xpath_parse_UnaryExpr(MY_XPATH *xpath);
static int my_xpath_parse_MultiplicativeExpr(MY_XPATH *xpath);
static int my_xpath_parse_AdditiveExpr(MY_XPATH *xpath);
static int my_xpath_parse_RelationalExpr(MY_XPATH *xpath);
static int my_xpath_parse_AndExpr(MY_XPATH *xpath);
static int my_xpath_parse_EqualityExpr(MY_XPATH *xpath);
static int my_xpath_parse_VariableReference(MY_XPATH *xpath);

/*
  Scan LocationPath

  SYNOPSYS

    [1] LocationPath ::=   RelativeLocationPath
                         | AbsoluteLocationPath
    [3] RelativeLocationPath ::= RelativeLocationPath '/' Step

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_LocationPath(MY_XPATH *xpath) {
  Item_nodeset_func *context = xpath->context;

  if (!xpath->context) xpath->context = xpath->rootelement;
  int rc = my_xpath_parse_RelativeLocationPath(xpath) ||
           my_xpath_parse_AbsoluteLocationPath(xpath);

  xpath->item = xpath->context;
  xpath->context = context;
  return rc;
}

/*
  Scan Absolute Location Path

  SYNOPSYS

    [2]     AbsoluteLocationPath ::=   '/' RelativeLocationPath?
                                     | AbbreviatedAbsoluteLocationPath
    [10]    AbbreviatedAbsoluteLocationPath ::=  '//' RelativeLocationPath

    We combine these two rules into one rule for better performance:

    [2,10]  AbsoluteLocationPath ::=  '/'   RelativeLocationPath?
                                     | '//' RelativeLocationPath

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AbsoluteLocationPath(MY_XPATH *xpath) {
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH)) return 0;

  xpath->context = xpath->rootelement;

  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH)) {
    xpath->context = new Item_nodeset_func_descendantbyname(
        xpath->context, "*", 1, *xpath->pxml, xpath->cs, true);
    return my_xpath_parse_RelativeLocationPath(xpath);
  }

  my_xpath_parse_RelativeLocationPath(xpath);

  return (xpath->error == 0);
}

/*
  Scan Relative Location Path

  SYNOPSYS

    For better performance we combine these two rules

    [3] RelativeLocationPath ::=   Step
                                 | RelativeLocationPath '/' Step
                                 | AbbreviatedRelativeLocationPath
    [11] AbbreviatedRelativeLocationPath ::=  RelativeLocationPath '//' Step


    Into this one:

    [3-11] RelativeLocationPath ::=   Step
                                    | RelativeLocationPath '/'  Step
                                    | RelativeLocationPath '//' Step
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_RelativeLocationPath(MY_XPATH *xpath) {
  if (!my_xpath_parse_Step(xpath)) return 0;
  while (my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH)) {
    if (my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH))
      xpath->context = new Item_nodeset_func_descendantbyname(
          xpath->context, "*", 1, *xpath->pxml, xpath->cs, true);
    if (!my_xpath_parse_Step(xpath)) {
      xpath->error = 1;
      return 0;
    }
  }
  return 1;
}

/*
  Scan non-abbreviated or abbreviated Step

  SYNOPSYS

  [4] Step ::=   AxisSpecifier NodeTest Predicate*
               | AbbreviatedStep
  [8] Predicate ::= '[' PredicateExpr ']'
  [9] PredicateExpr ::= Expr (RECURSIVE)
  [14] Expr ::= OrExpr

  reduced to:

  [8b] Predicate ::= '[' OrExpr ']' (RECURSIVE)

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AxisSpecifier_NodeTest_opt_Predicate_list(
    MY_XPATH *xpath) {
  if (!my_xpath_parse_AxisSpecifier(xpath)) return 0;

  if (!my_xpath_parse_NodeTest(xpath)) return 0;

  while (my_xpath_parse_term(xpath, MY_XPATH_LEX_LB)) {
    Item *prev_context = xpath->context;
    auto *cache = new Item_nodeset_context_cache(*xpath->pxml, xpath->cs);
    xpath->context = cache;

    if (!my_xpath_parse_PredicateExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }

    if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_RB)) {
      xpath->error = 1;
      return 0;
    }

    xpath->item = nodeset2bool(xpath->item);

    if (xpath->item->is_bool_func()) {
      xpath->context = new Item_nodeset_func_predicate(
          prev_context, xpath->item, *xpath->pxml, xpath->cs, cache);
    } else {
      xpath->context = new Item_nodeset_func_elementbyindex(
          prev_context, xpath->item, *xpath->pxml, xpath->cs, cache);
    }
  }
  return 1;
}

static int my_xpath_parse_Step(MY_XPATH *xpath) {
  return my_xpath_parse_AxisSpecifier_NodeTest_opt_Predicate_list(xpath) ||
         my_xpath_parse_AbbreviatedStep(xpath);
}

/*
  Scan Abbreviated Axis Specifier

  SYNOPSYS
  [5] AxisSpecifier ::=  AxisName '::'
                         | AbbreviatedAxisSpecifier

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AbbreviatedAxisSpecifier(MY_XPATH *xpath) {
  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_AT))
    xpath->axis = MY_XPATH_AXIS_ATTRIBUTE;
  else
    xpath->axis = MY_XPATH_AXIS_CHILD;
  return 1;
}

/*
  Scan non-abbreviated axis specifier

  SYNOPSYS

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AxisName_colon_colon(MY_XPATH *xpath) {
  return my_xpath_parse_AxisName(xpath) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_COLON) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_COLON);
}

/*
  Scan Abbreviated AxisSpecifier

  SYNOPSYS
    [13] AbbreviatedAxisSpecifier  ::=  '@'?

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AxisSpecifier(MY_XPATH *xpath) {
  return my_xpath_parse_AxisName_colon_colon(xpath) ||
         my_xpath_parse_AbbreviatedAxisSpecifier(xpath);
}

/*
  Scan NodeType followed by parens

  SYNOPSYS

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_NodeTest_lp_rp(MY_XPATH *xpath) {
  return my_xpath_parse_term(xpath, MY_XPATH_LEX_NODETYPE) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_LP) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_RP);
}

/*
  Scan NodeTest

  SYNOPSYS

  [7] NodeTest ::=   NameTest
                   | NodeType '(' ')'
                   | 'processing-instruction' '(' Literal ')'
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_NodeTest(MY_XPATH *xpath) {
  return my_xpath_parse_NameTest(xpath) || my_xpath_parse_NodeTest_lp_rp(xpath);
}

/*
  Scan Abbreviated Step

  SYNOPSYS

  [12] AbbreviatedStep  ::= '.'	| '..'

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AbbreviatedStep(MY_XPATH *xpath) {
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_DOT)) return 0;
  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_DOT))
    xpath->context = new Item_nodeset_func_parentbyname(
        xpath->context, "*", 1, *xpath->pxml, xpath->cs);
  return 1;
}

/*
  Scan Primary Expression

  SYNOPSYS

  [15] PrimaryExpr ::= VariableReference
                       | '(' Expr ')'   (RECURSIVE)
                       | Literal
                       | Number
                       | FunctionCall
  [14] Expr ::= OrExpr

  reduced to:

  [15b] PrimaryExpr ::= '(' OrExpr ')'  (RECURSIVE)

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_lp_Expr_rp(MY_XPATH *xpath) {
  return my_xpath_parse_term(xpath, MY_XPATH_LEX_LP) &&
         my_xpath_parse_Expr(xpath) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_RP);
}
static int my_xpath_parse_PrimaryExpr_literal(MY_XPATH *xpath) {
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_STRING)) return 0;
  xpath->item =
      new Item_string(xpath->prevtok.beg + 1,
                      xpath->prevtok.end - xpath->prevtok.beg - 2, xpath->cs);
  return 1;
}
static int my_xpath_parse_PrimaryExpr(MY_XPATH *xpath) {
  return my_xpath_parse_lp_Expr_rp(xpath) ||
         my_xpath_parse_VariableReference(xpath) ||
         my_xpath_parse_PrimaryExpr_literal(xpath) ||
         my_xpath_parse_Number(xpath) || my_xpath_parse_FunctionCall(xpath);
}

/*
  Scan Function Call

  SYNOPSYS
    [16] FunctionCall ::= FunctionName '(' ( Argument ( ',' Argument )* )? ')'
    [17] Argument      ::= Expr (RECURSIVE)
    [14] Expr ::= OrExpr

    reduced to:

    [16b] FunctionCall ::= FunctionName '(' ( OrExpr ( ',' OrExpr )* )? ')'
  (RECURSIVE)

  RETURN
    1 - success
    0 - failure

*/
static int my_xpath_parse_FunctionCall(MY_XPATH *xpath) {
  Item *args[256];
  uint nargs;

  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_FUNC)) return 0;

  MY_XPATH_FUNC *func = xpath->func;

  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_LP)) return 0;

  for (nargs = 0; nargs < func->maxargs;) {
    if (!my_xpath_parse_Expr(xpath)) {
      if (nargs < func->minargs) return 0;
      goto right_paren;
    }
    args[nargs++] = xpath->item;
    if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_COMMA)) {
      if (nargs < func->minargs)
        return 0;
      else
        break;
    }
  }

right_paren:
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_RP)) return 0;

  return ((xpath->item = func->create(xpath, args, nargs))) ? 1 : 0;
}

/*
  Scan Union Expression

  SYNOPSYS
    [18] UnionExpr ::=   PathExpr
                       | UnionExpr '|' PathExpr

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_UnionExpr(MY_XPATH *xpath) {
  if (!my_xpath_parse_PathExpr(xpath)) return 0;

  while (my_xpath_parse_term(xpath, MY_XPATH_LEX_VLINE)) {
    Item *prev = xpath->item;
    if (prev->type() != Item::XPATH_NODESET) return 0;

    if (!my_xpath_parse_PathExpr(xpath) ||
        xpath->item->type() != Item::XPATH_NODESET) {
      xpath->error = 1;
      return 0;
    }
    xpath->item =
        new Item_nodeset_func_union(prev, xpath->item, *xpath->pxml, xpath->cs);
  }
  return 1;
}

/*
  Scan Path Expression

  SYNOPSYS

  [19] PathExpr ::=   LocationPath
                    | FilterExpr
                    | FilterExpr '/' RelativeLocationPath
                    | FilterExpr '//' RelativeLocationPath
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_FilterExpr_opt_slashes_RelativeLocationPath(
    MY_XPATH *xpath) {
  Item_nodeset_func *context = xpath->context;
  int rc;

  if (!my_xpath_parse_FilterExpr(xpath)) return 0;

  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH)) return 1;

  if (xpath->item->type() != Item::XPATH_NODESET) {
    xpath->lasttok = xpath->prevtok;
    xpath->error = 1;
    return 0;
  }

  /*
    The context for the next relative path is the nodeset
    returned by FilterExpr
  */
  xpath->context = down_cast<Item_nodeset_func *>(xpath->item);

  /* treat double slash (//) as /descendant-or-self::node()/ */
  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_SLASH))
    xpath->context = new Item_nodeset_func_descendantbyname(
        xpath->context, "*", 1, *xpath->pxml, xpath->cs, true);
  rc = my_xpath_parse_RelativeLocationPath(xpath);

  /* push back the context and restore the item */
  xpath->item = xpath->context;
  xpath->context = context;
  return rc;
}
static int my_xpath_parse_PathExpr(MY_XPATH *xpath) {
  return my_xpath_parse_LocationPath(xpath) ||
         my_xpath_parse_FilterExpr_opt_slashes_RelativeLocationPath(xpath);
}

/*
  Scan Filter Expression

  SYNOPSYS
    [20]  FilterExpr ::=   PrimaryExpr
                         | FilterExpr Predicate

    or in other words:

    [20]  FilterExpr ::=   PrimaryExpr Predicate*

  RETURN
    1 - success
    0 - failure

*/
static int my_xpath_parse_FilterExpr(MY_XPATH *xpath) {
  return my_xpath_parse_PrimaryExpr(xpath);
}

/*
  Scan Or Expression

  SYNOPSYS
    [21] OrExpr ::=   AndExpr
                    | OrExpr 'or' AndExpr

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_OrExpr(MY_XPATH *xpath) {
  THD *thd = current_thd;
  uchar stack_top;

  if (check_stack_overrun(thd, STACK_MIN_SIZE, &stack_top)) return 1;

  if (!my_xpath_parse_AndExpr(xpath)) return 0;

  while (my_xpath_parse_term(xpath, MY_XPATH_LEX_OR)) {
    Item *prev = xpath->item;
    if (!my_xpath_parse_AndExpr(xpath)) {
      return 0;
      xpath->error = 1;
    }
    xpath->item =
        new Item_cond_or(nodeset2bool(prev), nodeset2bool(xpath->item));
  }
  return 1;
}

/*
  Scan And Expression

  SYNOPSYS
    [22] AndExpr ::=   EqualityExpr
                     | AndExpr 'and' EqualityExpr

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AndExpr(MY_XPATH *xpath) {
  if (!my_xpath_parse_EqualityExpr(xpath)) return 0;

  while (my_xpath_parse_term(xpath, MY_XPATH_LEX_AND)) {
    Item *prev = xpath->item;
    if (!my_xpath_parse_EqualityExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }

    xpath->item =
        new Item_cond_and(nodeset2bool(prev), nodeset2bool(xpath->item));
  }
  return 1;
}

/*
  Scan Equality Expression

  SYNOPSYS
    [23] EqualityExpr ::=   RelationalExpr
                          | EqualityExpr '=' RelationalExpr
                          | EqualityExpr '!=' RelationalExpr
    or in other words:

    [23] EqualityExpr ::= RelationalExpr ( EqualityOperator EqualityExpr )*

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_ne(MY_XPATH *xpath) {
  MY_XPATH_LEX prevtok = xpath->prevtok;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_EXCL)) return 0;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_EQ)) {
    /* Unget the exclamation mark */
    xpath->lasttok = xpath->prevtok;
    xpath->prevtok = prevtok;
    return 0;
  }
  return 1;
}
static int my_xpath_parse_EqualityOperator(MY_XPATH *xpath) {
  if (my_xpath_parse_ne(xpath)) {
    xpath->extra = '!';
    return 1;
  }
  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_EQ)) {
    xpath->extra = '=';
    return 1;
  }
  return 0;
}
static int my_xpath_parse_EqualityExpr(MY_XPATH *xpath) {
  MY_XPATH_LEX operator_context;
  if (!my_xpath_parse_RelationalExpr(xpath)) return 0;

  operator_context = xpath->lasttok;
  while (my_xpath_parse_EqualityOperator(xpath)) {
    Item *prev = xpath->item;
    int oper = xpath->extra;
    if (!my_xpath_parse_RelationalExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }

    if (!(xpath->item = create_comparator(xpath, oper, &operator_context, prev,
                                          xpath->item)))
      return 0;

    operator_context = xpath->lasttok;
  }
  return 1;
}

/*
  Scan Relational Expression

  SYNOPSYS

    [24] RelationalExpr ::=   AdditiveExpr
                            | RelationalExpr '<' AdditiveExpr
                            | RelationalExpr '>' AdditiveExpr
                            | RelationalExpr '<=' AdditiveExpr
                            | RelationalExpr '>=' AdditiveExpr
  or in other words:

    [24] RelationalExpr ::= AdditiveExpr (RelationalOperator RelationalExpr)*

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_RelationalOperator(MY_XPATH *xpath) {
  if (my_xpath_parse_term(xpath, MY_XPATH_LEX_LESS)) {
    xpath->extra = my_xpath_parse_term(xpath, MY_XPATH_LEX_EQ)
                       ? MY_XPATH_LEX_LE
                       : MY_XPATH_LEX_LESS;
    return 1;
  } else if (my_xpath_parse_term(xpath, MY_XPATH_LEX_GREATER)) {
    xpath->extra = my_xpath_parse_term(xpath, MY_XPATH_LEX_EQ)
                       ? MY_XPATH_LEX_GE
                       : MY_XPATH_LEX_GREATER;
    return 1;
  }
  return 0;
}
static int my_xpath_parse_RelationalExpr(MY_XPATH *xpath) {
  MY_XPATH_LEX operator_context;
  if (!my_xpath_parse_AdditiveExpr(xpath)) return 0;
  operator_context = xpath->lasttok;
  while (my_xpath_parse_RelationalOperator(xpath)) {
    Item *prev = xpath->item;
    int oper = xpath->extra;

    if (!my_xpath_parse_AdditiveExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }

    if (!(xpath->item = create_comparator(xpath, oper, &operator_context, prev,
                                          xpath->item)))
      return 0;
    operator_context = xpath->lasttok;
  }
  return 1;
}

/*
  Scan Additive Expression

  SYNOPSYS

    [25] AdditiveExpr ::=   MultiplicativeExpr
                          | AdditiveExpr '+' MultiplicativeExpr
                          | AdditiveExpr '-' MultiplicativeExpr
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_AdditiveOperator(MY_XPATH *xpath) {
  return my_xpath_parse_term(xpath, MY_XPATH_LEX_PLUS) ||
         my_xpath_parse_term(xpath, MY_XPATH_LEX_MINUS);
}
static int my_xpath_parse_AdditiveExpr(MY_XPATH *xpath) {
  if (!my_xpath_parse_MultiplicativeExpr(xpath)) return 0;

  while (my_xpath_parse_AdditiveOperator(xpath)) {
    int oper = xpath->prevtok.term;
    Item *prev = xpath->item;
    if (!my_xpath_parse_MultiplicativeExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }

    if (oper == MY_XPATH_LEX_PLUS)
      xpath->item = new Item_func_plus(prev, xpath->item);
    else
      xpath->item = new Item_func_minus(prev, xpath->item);
  };
  return 1;
}

/*
  Scan Multiplicative Expression

  SYNOPSYS

    [26] MultiplicativeExpr ::=   UnaryExpr
                                | MultiplicativeExpr MultiplyOperator UnaryExpr
                                | MultiplicativeExpr 'div' UnaryExpr
                                | MultiplicativeExpr 'mod' UnaryExpr
    or in other words:

    [26]  MultiplicativeExpr ::= UnaryExpr (MulOper MultiplicativeExpr)*

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_MultiplicativeOperator(MY_XPATH *xpath) {
  return my_xpath_parse_term(xpath, MY_XPATH_LEX_ASTERISK) ||
         my_xpath_parse_term(xpath, MY_XPATH_LEX_DIV) ||
         my_xpath_parse_term(xpath, MY_XPATH_LEX_MOD);
}
static int my_xpath_parse_MultiplicativeExpr(MY_XPATH *xpath) {
  if (!my_xpath_parse_UnaryExpr(xpath)) return 0;

  while (my_xpath_parse_MultiplicativeOperator(xpath)) {
    int oper = xpath->prevtok.term;
    Item *prev = xpath->item;
    if (!my_xpath_parse_UnaryExpr(xpath)) {
      xpath->error = 1;
      return 0;
    }
    switch (oper) {
      case MY_XPATH_LEX_ASTERISK:
        xpath->item = new Item_func_mul(prev, xpath->item);
        break;
      case MY_XPATH_LEX_DIV:
        xpath->item = new Item_func_int_div(prev, xpath->item);
        break;
      case MY_XPATH_LEX_MOD:
        xpath->item = new Item_func_mod(prev, xpath->item);
        break;
    }
  }
  return 1;
}

/*
  Scan Unary Expression

  SYNOPSYS

    [27] UnaryExpr ::=   UnionExpr
                       | '-' UnaryExpr
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_UnaryExpr(MY_XPATH *xpath) {
  THD *thd = current_thd;
  uchar stack_top;

  if (check_stack_overrun(thd, STACK_MIN_SIZE, &stack_top)) return 0;

  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_MINUS))
    return my_xpath_parse_UnionExpr(xpath);
  if (!my_xpath_parse_UnaryExpr(xpath)) return 0;
  xpath->item = new Item_func_neg(xpath->item);
  return 1;
}

/*
  Scan Number

  SYNOPSYS

    [30] Number ::= Digits ('.' Digits?)? | '.' Digits)

  or in other words:

    [30] Number ::= Digits
                    | Digits '.'
                    | Digits '.' Digits
                    | '.' Digits

  Note: the last rule is not supported yet,
  as it is in conflict with abbreviated step.
  1 + .123    does not work,
  1 + 0.123   does.
  Perhaps it is better to move this code into lex analizer.

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_Number(MY_XPATH *xpath) {
  const char *beg;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_DIGITS)) return 0;
  beg = xpath->prevtok.beg;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_DOT)) {
    xpath->item = new Item_int(xpath->prevtok.beg,
                               xpath->prevtok.end - xpath->prevtok.beg);
    return 1;
  }
  my_xpath_parse_term(xpath, MY_XPATH_LEX_DIGITS);

  xpath->item = new Item_float(beg, xpath->prevtok.end - beg);
  return 1;
}

/*
  Scan NCName.

  SYNOPSYS

    The keywords AND, OR, MOD, DIV are valid identitiers
    when they are in identifier context:

    SELECT
    ExtractValue('<and><or><mod><div>VALUE</div></mod></or></and>',
                 '/and/or/mod/div')
    ->  VALUE

  RETURN
    1 - success
    0 - failure
*/

static int my_xpath_parse_NCName(MY_XPATH *xpath) {
  return my_xpath_parse_term(xpath, MY_XPATH_LEX_IDENT) ||
                 my_xpath_parse_term(xpath, MY_XPATH_LEX_AND) ||
                 my_xpath_parse_term(xpath, MY_XPATH_LEX_OR) ||
                 my_xpath_parse_term(xpath, MY_XPATH_LEX_MOD) ||
                 my_xpath_parse_term(xpath, MY_XPATH_LEX_DIV)
             ? 1
             : 0;
}

/*
  QName grammar can be found in a separate document
  http://www.w3.org/TR/REC-xml-names/#NT-QName

  [6] 	QName     ::= (Prefix ':')? LocalPart
  [7] 	Prefix    ::= NCName
  [8] 	LocalPart ::= NCName
*/

static int my_xpath_parse_QName(MY_XPATH *xpath) {
  const char *beg;
  if (!my_xpath_parse_NCName(xpath)) return 0;
  beg = xpath->prevtok.beg;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_COLON))
    return 1; /* Non qualified name */
  if (!my_xpath_parse_NCName(xpath)) return 0;
  xpath->prevtok.beg = beg;
  return 1;
}

/**
  Scan Variable reference

  @details Implements parsing of two syntax structures:

    1. Standard XPath syntax [36], for SP variables:

      VariableReference ::= '$' QName

      Finds a SP variable with the given name.
      If outside of a SP context, or variable with
      the given name doesn't exists, then error is returned.

    2. Non-standard syntax - MySQL extension for user variables:

      VariableReference ::= '$' '@' QName

    Item, corresponding to the variable, is returned
    in xpath->item in both cases.

  @param  xpath pointer to XPath structure

  @return Operation status
    @retval 1 Success
    @retval 0 Failure
*/

static int my_xpath_parse_VariableReference(MY_XPATH *xpath) {
  int user_var;
  const char *dollar_pos;
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_DOLLAR) ||
      (!(dollar_pos = xpath->prevtok.beg)) ||
      (!((user_var = my_xpath_parse_term(xpath, MY_XPATH_LEX_AT) &&
                     my_xpath_parse_term(xpath, MY_XPATH_LEX_IDENT))) &&
       !my_xpath_parse_term(xpath, MY_XPATH_LEX_IDENT)))
    return 0;

  size_t name_length = xpath->prevtok.end - xpath->prevtok.beg;
  const char *name_str = xpath->prevtok.beg;

  if (user_var)
    xpath->item =
        new Item_func_get_user_var(Name_string(name_str, name_length, false));
  else {
    sp_variable *spv;
    sp_pcontext *spc;
    LEX *lex;
    if ((lex = current_thd->lex) && (spc = lex->get_sp_current_parsing_ctx()) &&
        (spv = spc->find_variable(name_str, name_length, false))) {
      Item_splocal *splocal = new Item_splocal(
          Name_string(name_str, name_length, false), spv->offset, spv->type, 0);
#ifndef DBUG_OFF
      if (splocal) splocal->m_sp = lex->sphead;
#endif
      xpath->item = down_cast<Item *>(splocal);
    } else {
      xpath->item = nullptr;
      DBUG_ASSERT(xpath->query.end > dollar_pos);
      size_t len = xpath->query.end - dollar_pos;
      len = std::min(len, size_t(32));
      my_printf_error(ER_UNKNOWN_ERROR, "Unknown XPATH variable at: '%.*s'",
                      MYF(0), static_cast<int>(len), dollar_pos);
    }
  }
  return xpath->item ? 1 : 0;
}

/*
  Scan Name Test

  SYNOPSYS

    [37] NameTest ::=  '*'
                      | NCName ':' '*'
                      | QName
  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse_NodeTest_QName(MY_XPATH *xpath) {
  if (!my_xpath_parse_QName(xpath)) return 0;
  DBUG_ASSERT(xpath->context);
  size_t len = xpath->prevtok.end - xpath->prevtok.beg;
  xpath->context =
      nametestfunc(xpath, xpath->axis, xpath->context, xpath->prevtok.beg, len);
  return 1;
}
static int my_xpath_parse_NodeTest_asterisk(MY_XPATH *xpath) {
  if (!my_xpath_parse_term(xpath, MY_XPATH_LEX_ASTERISK)) return 0;
  DBUG_ASSERT(xpath->context);
  xpath->context = nametestfunc(xpath, xpath->axis, xpath->context, "*", 1);
  return 1;
}
static int my_xpath_parse_NameTest(MY_XPATH *xpath) {
  return my_xpath_parse_NodeTest_asterisk(xpath) ||
         my_xpath_parse_NodeTest_QName(xpath);
}

/*
  Scan an XPath expression

  SYNOPSYS
    Scan xpath expression.
    The expression is returned in xpath->expr.

  RETURN
    1 - success
    0 - failure
*/
static int my_xpath_parse(MY_XPATH *xpath, const char *str,
                          const char *strend) {
  my_xpath_lex_init(&xpath->query, str, strend);
  my_xpath_lex_init(&xpath->prevtok, str, strend);
  my_xpath_lex_scan(xpath, &xpath->lasttok, str, strend);

  xpath->rootelement =
      new Item_nodeset_func_rootelement(*xpath->pxml, xpath->cs);

  return my_xpath_parse_Expr(xpath) &&
         my_xpath_parse_term(xpath, MY_XPATH_LEX_EOF);
}

bool Item_xml_str_func::resolve_type(THD *) {
  nodeset_func = nullptr;

  if (agg_arg_charsets_for_comparison(collation, args, arg_count)) return true;

  set_data_type_string(uint32(MAX_BLOB_WIDTH));

  if (collation.collation->mbminlen > 1) {
    /* UCS2 is not supported */
    my_printf_error(ER_UNKNOWN_ERROR,
                    "Character set '%s' is not supported by XPATH", MYF(0),
                    collation.collation->csname);
    return true;
  }

  if (!args[1]->const_for_execution()) {
    my_printf_error(ER_UNKNOWN_ERROR,
                    "Only constant XPATH queries are supported", MYF(0));
    return true;
  }

  if (args[1]->const_item() && parse_xpath(args[1])) return true;

  return false;
}

bool Item_xml_str_func::parse_xpath(Item *xpath_expr) {
  String *xp;
  MY_XPATH xpath;

  if (!(xp = xpath_expr->val_str(&xpath_tmp_value)))
    return current_thd->is_error();

  my_xpath_init(&xpath);
  xpath.cs = collation.collation;
  xpath.debug = 0;
  xpath.pxml = &pxml;

  int rc = my_xpath_parse(&xpath, xp->ptr(), xp->ptr() + xp->length());

  if (!rc) {
    size_t clen = xpath.query.end - xpath.lasttok.beg;
    clen = std::min(clen, size_t(32));
    my_printf_error(ER_UNKNOWN_ERROR, "XPATH syntax error: '%.*s'", MYF(0),
                    static_cast<int>(clen), xpath.lasttok.beg);
    return true;
  }

  nodeset_func = xpath.item;
  if (nodeset_func && nodeset_func->fix_fields(current_thd, &nodeset_func))
    return true;

  return false;
}

#define MAX_LEVEL 256
typedef struct {
  uint level;
  ParsedXML *pxml;      // parsed XML
  uint pos[MAX_LEVEL];  // Tag position stack
  uint parent;          // Offset of the parent of the current node
} MY_XML_USER_DATA;

/*
  Process tag beginning

  SYNOPSYS

    A call-back function executed when XML parser
    is entering a tag or an attribue.
    Appends the new node into data->pxml.
    Increments data->level.

  RETURN
    Currently only MY_XML_OK
*/
extern "C" int xml_enter(MY_XML_PARSER *st, const char *attr, size_t len);

int xml_enter(MY_XML_PARSER *st, const char *attr, size_t len) {
  auto *data = reinterpret_cast<MY_XML_USER_DATA *>(st->user_data);
  MY_XML_NODE node;

  node.parent = data->parent;  // Set parent for the new node to old parent
  data->parent = data->pxml->size();  // Remember current node as new parent
  DBUG_ASSERT(data->level < MAX_LEVEL);
  data->pos[data->level] = data->pxml->size();
  if (data->level < MAX_LEVEL - 1)
    node.level = data->level++;
  else
    return MY_XML_ERROR;
  node.type = st->current_node_type;  // TAG or ATTR
  node.beg = attr;
  node.end = attr + len;
  data->pxml->push_back(node);
  return MY_XML_OK;
}

/*
  Process text node

  SYNOPSYS

    A call-back function executed when XML parser
    is entering into a tag or an attribue textual value.
    The value is appended into data->pxml.

  RETURN
    Currently only MY_XML_OK
*/
extern "C" int xml_value(MY_XML_PARSER *st, const char *attr, size_t len);

int xml_value(MY_XML_PARSER *st, const char *attr, size_t len) {
  auto *data = reinterpret_cast<MY_XML_USER_DATA *>(st->user_data);
  MY_XML_NODE node;

  node.parent = data->parent;  // Set parent for the new text node to old parent
  node.level = data->level;
  node.type = MY_XML_NODE_TEXT;
  node.beg = attr;
  node.end = attr + len;
  data->pxml->push_back(node);
  return MY_XML_OK;
}

/*
  Leave a tag or an attribute

  SYNOPSYS

    A call-back function executed when XML parser
    is leaving a tag or an attribute.
    Decrements data->level.

  RETURN
    Currently only MY_XML_OK
*/
extern "C" int xml_leave(MY_XML_PARSER *st, const char *attr, size_t len);

int xml_leave(MY_XML_PARSER *st, const char *, size_t) {
  auto *data = reinterpret_cast<MY_XML_USER_DATA *>(st->user_data);
  DBUG_ASSERT(data->level > 0);
  data->level--;

  data->parent = data->pxml->at(data->parent).parent;
  data->pxml->at(data->pos[data->level]).tagend = st->cur;

  return MY_XML_OK;
}

/*
  Parse raw XML

  SYNOPSYS


  RETURN
    Currently pointer to parsed XML on success
    0 on parse error
*/
static bool parse_xml(String *raw_xml, ParsedXML *parsed_xml_buf) {
  MY_XML_PARSER p;
  MY_XML_USER_DATA user_data;
  int rc;

  parsed_xml_buf->clear();

  /* Prepare XML parser */
  my_xml_parser_create(&p);
  p.flags = MY_XML_FLAG_RELATIVE_NAMES | MY_XML_FLAG_SKIP_TEXT_NORMALIZATION;
  user_data.level = 0;
  user_data.pxml = parsed_xml_buf;
  user_data.parent = 0;
  my_xml_set_enter_handler(&p, xml_enter);
  my_xml_set_value_handler(&p, xml_value);
  my_xml_set_leave_handler(&p, xml_leave);
  my_xml_set_user_data(&p, &user_data);

  /* Add root node */
  p.current_node_type = MY_XML_NODE_TAG;
  xml_enter(&p, raw_xml->ptr(), 0);

  /* Execute XML parser */
  if ((rc = my_xml_parse(&p, raw_xml->ptr(), raw_xml->length())) != MY_XML_OK) {
    char buf[128];
    snprintf(buf, sizeof(buf) - 1, "parse error at line %d pos %lu: %s",
             my_xml_error_lineno(&p) + 1,
             static_cast<ulong>(my_xml_error_pos(&p)) + 1,
             my_xml_error_string(&p));
    push_warning_printf(current_thd, Sql_condition::SL_WARNING, ER_WRONG_VALUE,
                        ER_THD(current_thd, ER_WRONG_VALUE), "XML", buf);
  }
  my_xml_parser_free(&p);

  return rc == MY_XML_OK;
}

String *Item_func_xml_extractvalue::val_str(String *str) {
  String *res;
  null_value = false;
  if (!nodeset_func && parse_xpath(args[1])) {
    DBUG_ASSERT(maybe_null);
    null_value = true;
    return nullptr;
  }

  tmp_value.set("", 0, collation.collation);
  if (!nodeset_func || !(res = args[0]->val_str(str)) ||
      !parse_xml(res, &pxml) || !(res = nodeset_func->val_str(&tmp_value))) {
    null_value = true;
    return nullptr;
  }
  return res;
}

String *Item_func_xml_update::val_str(String *str) {
  String *res, *rep;

  null_value = false;
  if (!nodeset_func && parse_xpath(args[1])) {
    DBUG_ASSERT(maybe_null);
    null_value = true;
    return nullptr;
  }

  if (!nodeset_func || !(res = args[0]->val_str(str)) ||
      !(rep = args[2]->val_str(&tmp_value)) || !parse_xml(res, &pxml) ||
      (nodeset_func->type() != XPATH_NODESET)) {
    null_value = true;
    return nullptr;
  }

  XPathFilter nodeset;
  down_cast<const Item_nodeset_func *>(nodeset_func)->val_nodeset(&nodeset);

  /* Allow replacing of one tag only */
  if (nodeset.size() != 1) {
    /* TODO: perhaps add a warning that more than one tag selected */
    return res;
  }

  const MY_XML_NODE *node = &pxml.at(nodeset.at(0).num);

  if (!node->level) {
    /*
      Root element, without NameTest:
      UpdateXML(xml, '/', 'replacement');
      Just return the replacement string.
    */
    return rep;
  }

  tmp_value.length(0);
  tmp_value.set_charset(collation.collation);
  uint offs = node->type == MY_XML_NODE_TAG ? 1 : 0;
  tmp_value.append(res->ptr(), node->beg - res->ptr() - offs);
  tmp_value.append(rep->ptr(), rep->length());
  const char *end = node->tagend + offs;
  tmp_value.append(end, res->ptr() + res->length() - end);
  return &tmp_value;
}
