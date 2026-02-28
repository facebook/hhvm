/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PARSE_TREE_COL_ATTRS_INCLUDED
#define PARSE_TREE_COL_ATTRS_INCLUDED

#include <type_traits>

#include "my_dbug.h"
#include "mysql/mysql_lex_string.h"
#include "mysql_com.h"
#include "nullable.h"
#include "sql/gis/srid.h"
#include "sql/item_timefunc.h"
#include "sql/parse_tree_node_base.h"
#include "sql/sql_alter.h"
#include "sql/sql_check_constraint.h"  // Sql_check_constraint_spec
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"

using Mysql::Nullable;

/**
  Parse context for column type attribyte specific parse tree nodes.

  For internal use in the contextualization code.

  @ingroup ptn_column_attrs ptn_gcol_attrs
*/
struct Column_parse_context : public Parse_context {
  const bool is_generated;  ///< Owner column is a generated one.

  Column_parse_context(THD *thd_arg, SELECT_LEX *select_arg, bool is_generated)
      : Parse_context(thd_arg, select_arg), is_generated(is_generated) {}
};

/**
  Base class for all column attributes in @SQL{CREATE/ALTER TABLE}

  @ingroup ptn_column_attrs ptn_gcol_attrs
*/
class PT_column_attr_base : public Parse_tree_node_tmpl<Column_parse_context> {
 protected:
  PT_column_attr_base() {}

 public:
  typedef decltype(Alter_info::flags) alter_info_flags_t;

  virtual void apply_type_flags(ulong *) const {}
  virtual void apply_alter_info_flags(ulonglong *) const {}
  virtual void apply_comment(LEX_CSTRING *) const {}
  virtual void apply_default_value(Item **) const {}
  virtual void apply_gen_default_value(Value_generator **) {}
  virtual void apply_on_update_value(Item **) const {}
  virtual void apply_srid_modifier(Nullable<gis::srid_t> *) const {}
  virtual bool apply_collation(
      Column_parse_context *, const CHARSET_INFO **to MY_ATTRIBUTE((unused)),
      bool *has_explicit_collation MY_ATTRIBUTE((unused))) const {
    return false;
  }
  virtual bool add_check_constraints(
      Sql_check_constraint_spec_list *check_const_list MY_ATTRIBUTE((unused))) {
    return false;
  }

  /**
    Check for the [NOT] ENFORCED characteristic.

    @returns true  if the [NOT] ENFORCED follows the CHECK(...) clause,
             false otherwise.
  */
  virtual bool has_constraint_enforcement() const { return false; }

  /**
    Check if constraint is enforced.
    Method must be called only when has_constraint_enforcement() is true (i.e
    when [NOT] ENFORCED follows the CHECK(...) clause).

    @returns true  if constraint is enforced.
             false otherwise.
  */
  virtual bool is_constraint_enforced() const { return false; }

  /**
    Update the ENFORCED/NOT ENFORCED state of the CHECK constraint.

    @param   enforced     true if ENFORCED, false if NOT ENFORCED.

    @returns false if success, true if error (e.g. if [NOT] ENFORCED follows
             something other than the CHECK clause.)
  */
  virtual bool set_constraint_enforcement(
      bool enforced MY_ATTRIBUTE((unused))) {
    return true;  // error
  }
};

/**
  Node for the @SQL{NULL} column attribute

  @ingroup ptn_column_attrs
*/
class PT_null_column_attr : public PT_column_attr_base {
 public:
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags &= ~NOT_NULL_FLAG;
    *type_flags |= EXPLICIT_NULL_FLAG;
  }
};

/**
  Node for the @SQL{NOT NULL} column attribute

  @ingroup ptn_column_attrs
*/
class PT_not_null_column_attr : public PT_column_attr_base {
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags |= NOT_NULL_FLAG;
  }
};

/**
  Node for the @SQL{NOT SECONDARY} column attribute

  @ingroup ptn_column_attrs
*/
class PT_secondary_column_attr : public PT_column_attr_base {
 public:
  void apply_type_flags(unsigned long *type_flags) const override {
    *type_flags |= NOT_SECONDARY_FLAG;
  }
};

/**
  Node for the @SQL{UNIQUE [KEY]} column attribute

  @ingroup ptn_column_attrs
*/
class PT_unique_key_column_attr : public PT_column_attr_base {
 public:
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags |= UNIQUE_FLAG;
  }

  void apply_alter_info_flags(ulonglong *flags) const override {
    *flags |= Alter_info::ALTER_ADD_INDEX;
  }
};

/**
  Node for the @SQL{PRIMARY [KEY]} column attribute

  @ingroup ptn_column_attrs
*/
class PT_primary_key_column_attr : public PT_column_attr_base {
 public:
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags |= PRI_KEY_FLAG | NOT_NULL_FLAG;
  }

  void apply_alter_info_flags(ulonglong *flags) const override {
    *flags |= Alter_info::ALTER_ADD_INDEX;
  }
};

/**
  Node for the @SQL{[CONSTRAINT [symbol]] CHECK '(' expr ')'} column attribute.

  @ingroup ptn_column_attrs
*/
class PT_check_constraint_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;
  Sql_check_constraint_spec col_cc_spec;

 public:
  explicit PT_check_constraint_column_attr(LEX_STRING &name, Item *expr) {
    col_cc_spec.name = name;
    col_cc_spec.check_expr = expr;
  }

  bool set_constraint_enforcement(bool enforced) override {
    col_cc_spec.is_enforced = enforced;
    return false;
  }

  void apply_alter_info_flags(ulonglong *flags) const override {
    *flags |= Alter_info::ADD_CHECK_CONSTRAINT;
  }

  bool add_check_constraints(
      Sql_check_constraint_spec_list *check_const_list) override {
    DBUG_ASSERT(check_const_list != nullptr);
    return (check_const_list->push_back(&col_cc_spec));
  }

  bool contextualize(Column_parse_context *pc) override {
    return (super::contextualize(pc) ||
            col_cc_spec.check_expr->itemize(pc, &col_cc_spec.check_expr));
  }
};

/**
  Node for the @SQL{[NOT] ENFORCED} column attribute.

  @ingroup ptn_column_attrs
*/
class PT_constraint_enforcement_attr : public PT_column_attr_base {
 public:
  explicit PT_constraint_enforcement_attr(bool enforced)
      : m_enforced(enforced) {}

  bool has_constraint_enforcement() const override { return true; }

  bool is_constraint_enforced() const override { return m_enforced; }

 private:
  const bool m_enforced;
};

/**
  Node for the @SQL{COMMENT @<comment@>} column attribute

  @ingroup ptn_column_attrs
*/
class PT_comment_column_attr : public PT_column_attr_base {
  const LEX_CSTRING comment;

 public:
  explicit PT_comment_column_attr(const LEX_CSTRING &comment)
      : comment(comment) {}

  void apply_comment(LEX_CSTRING *to) const override { *to = comment; }
};

/**
  Node for the @SQL{COLLATE @<collation@>} column attribute

  @ingroup ptn_column_attrs
*/
class PT_collate_column_attr : public PT_column_attr_base {
 public:
  explicit PT_collate_column_attr(const POS &pos, const CHARSET_INFO *collation)
      : m_pos(pos), m_collation(collation) {
    DBUG_ASSERT(m_collation != nullptr);
  }

  bool apply_collation(Column_parse_context *pc, const CHARSET_INFO **to,
                       bool *has_explicit_collation) const override {
    if (*has_explicit_collation) {
      pc->thd->syntax_error_at(m_pos, ER_INVALID_MULTIPLE_CLAUSES, "COLLATE");
      return true;
    }
    *has_explicit_collation = true;
    return merge_charset_and_collation(*to, m_collation, to);
  }

 private:
  const POS m_pos;
  const CHARSET_INFO *const m_collation;
};

// Specific to non-generated columns only:

/**
  Node for the @SQL{DEFAULT @<expression@>} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_default_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

  Item *item;

 public:
  explicit PT_default_column_attr(Item *item) : item(item) {}
  void apply_default_value(Item **value) const override { *value = item; }

  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "DEFAULT", "generated column");
      return true;
    }
    return super::contextualize(pc) || item->itemize(pc, &item);
  }
  void apply_type_flags(ulong *type_flags) const override {
    if (item->type() == Item::NULL_ITEM) *type_flags |= EXPLICIT_NULL_FLAG;
  }
};

/**
  Node for the @SQL{UPDATE NOW[([@<precision@>])]} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_on_update_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

  const uint8 precision;
  Item *item;

 public:
  explicit PT_on_update_column_attr(uint8 precision) : precision(precision) {}
  void apply_on_update_value(Item **value) const override { *value = item; }

  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "ON UPDATE", "generated column");
      return true;
    }
    if (super::contextualize(pc)) return true;

    item = new (pc->thd->mem_root) Item_func_now_local(precision);
    return item == nullptr;
  }
};

/**
  Node for the @SQL{AUTO_INCREMENT} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_auto_increment_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

 public:
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags |= AUTO_INCREMENT_FLAG | NOT_NULL_FLAG;
  }
  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "AUTO_INCREMENT", "generated column");
      return true;
    }
    return super::contextualize(pc);
  }
};

/**
  Node for the @SQL{SERIAL DEFAULT VALUE} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_serial_default_value_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

 public:
  void apply_type_flags(ulong *type_flags) const override {
    *type_flags |= AUTO_INCREMENT_FLAG | NOT_NULL_FLAG | UNIQUE_FLAG;
  }
  void apply_alter_info_flags(ulonglong *flags) const override {
    *flags |= Alter_info::ALTER_ADD_INDEX;
  }
  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "SERIAL DEFAULT VALUE",
               "generated column");
      return true;
    }
    return super::contextualize(pc);
  }
};

/**
  Node for the @SQL{COLUMN_FORMAT @<DEFAULT|FIXED|DYNAMIC@>} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_column_format_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

  column_format_type format;

 public:
  explicit PT_column_format_column_attr(column_format_type format)
      : format(format) {}

  void apply_type_flags(ulong *type_flags) const override {
    *type_flags &= ~(FIELD_FLAGS_COLUMN_FORMAT_MASK);
    *type_flags |= format << FIELD_FLAGS_COLUMN_FORMAT;
  }
  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "COLUMN_FORMAT", "generated column");
      return true;
    }
    return super::contextualize(pc);
  }
};

/**
  Node for the @SQL{STORAGE @<DEFAULT|DISK|MEMORY@>} column attribute

  @ingroup ptn_not_gcol_attr
*/
class PT_storage_media_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

  ha_storage_media media;

 public:
  explicit PT_storage_media_column_attr(ha_storage_media media)
      : media(media) {}

  void apply_type_flags(ulong *type_flags) const override {
    *type_flags &= ~(FIELD_FLAGS_STORAGE_MEDIA_MASK);
    *type_flags |= media << FIELD_FLAGS_STORAGE_MEDIA;
  }
  bool contextualize(Column_parse_context *pc) override {
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "STORAGE", "generated column");
      return true;
    }
    return super::contextualize(pc);
  }
};

/// Node for the SRID column attribute
class PT_srid_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

  gis::srid_t m_srid;

 public:
  explicit PT_srid_column_attr(gis::srid_t srid) : m_srid(srid) {}

  void apply_srid_modifier(Nullable<gis::srid_t> *srid) const override {
    *srid = m_srid;
  }
};

/// Node for the generated default value, column attribute
class PT_generated_default_val_column_attr : public PT_column_attr_base {
  typedef PT_column_attr_base super;

 public:
  PT_generated_default_val_column_attr(Item *expr) {
    m_default_value_expression.expr_item = expr;
    m_default_value_expression.set_field_stored(true);
  }

  void apply_gen_default_value(
      Value_generator **default_value_expression) override {
    *default_value_expression = &m_default_value_expression;
  }

  bool contextualize(Column_parse_context *pc) override {
    // GC and default value expressions are mutually exclusive and thus only
    // one is allowed to be present on the same column definition.
    if (pc->is_generated) {
      my_error(ER_WRONG_USAGE, MYF(0), "DEFAULT", "generated column");
      return true;
    }
    Parse_context expr_pc(pc->thd, pc->select);
    return super::contextualize(pc) ||
           m_default_value_expression.expr_item->itemize(
               &expr_pc, &m_default_value_expression.expr_item);
  }

 private:
  Value_generator m_default_value_expression;
};

// Type nodes:

/**
  Base class for all column type nodes

  @ingroup ptn_column_types
*/
class PT_type : public Parse_tree_node {
 public:
  const enum_field_types type;

 protected:
  explicit PT_type(enum_field_types type) : type(type) {}

 public:
  virtual ulong get_type_flags() const { return 0; }
  virtual const char *get_length() const { return nullptr; }
  virtual const char *get_dec() const { return nullptr; }
  virtual const CHARSET_INFO *get_charset() const { return nullptr; }
  virtual uint get_uint_geom_type() const { return 0; }
  virtual List<String> *get_interval_list() const { return nullptr; }
};

/**
  Node for numeric types

  Type list:
  * NUMERIC, REAL, DOUBLE, DECIMAL and FIXED,
  * INTEGER, INT, INT1, INT2, INT3, INT4, TINYINT, SMALLINT, MEDIUMINT and
    BIGINT.

  @ingroup ptn_column_types
*/
class PT_numeric_type : public PT_type {
  const char *length;
  const char *dec;
  ulong options;

  using Parent_type = std::remove_const<decltype(PT_type::type)>::type;

 public:
  PT_numeric_type(THD *thd, Numeric_type type_arg, const char *length,
                  const char *dec, ulong options)
      : PT_type(static_cast<Parent_type>(type_arg)),
        length(length),
        dec(dec),
        options(options) {
    DBUG_ASSERT((options & ~(UNSIGNED_FLAG | ZEROFILL_FLAG)) == 0);

    if (type_arg != Numeric_type::DECIMAL && dec != nullptr) {
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT,
                   ER_THD(thd, ER_WARN_DEPRECATED_FLOAT_DIGITS));
    }
    if (options & UNSIGNED_FLAG) {
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT,
                   ER_THD(thd, ER_WARN_DEPRECATED_FLOAT_UNSIGNED));
    }
  }
  PT_numeric_type(THD *thd, Int_type type_arg, const char *length,
                  ulong options)
      : PT_type(static_cast<enum_field_types>(type_arg)),
        length(length),
        dec(nullptr),
        options(options) {
    DBUG_ASSERT((options & ~(UNSIGNED_FLAG | ZEROFILL_FLAG)) == 0);

    if (length != nullptr && enable_deprecation_warning) {
      push_warning(thd, Sql_condition::SL_WARNING,
                   ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT,
                   ER_THD(thd, ER_WARN_DEPRECATED_INTEGER_DISPLAY_WIDTH));
    }
  }

  ulong get_type_flags() const override {
    return (options & ZEROFILL_FLAG) ? (options | UNSIGNED_FLAG) : options;
  }
  const char *get_length() const override { return length; }
  const char *get_dec() const override { return dec; }
};

/**
  Node for the BIT type

  @ingroup ptn_column_types
*/
class PT_bit_type : public PT_type {
  const char *length;

 public:
  PT_bit_type() : PT_type(MYSQL_TYPE_BIT), length("1") {}
  explicit PT_bit_type(const char *length)
      : PT_type(MYSQL_TYPE_BIT), length(length) {}

  const char *get_length() const override { return length; }
};

/**
  Node for the BOOL/BOOLEAN type

  @ingroup ptn_column_types
*/
class PT_boolean_type : public PT_type {
 public:
  PT_boolean_type() : PT_type(MYSQL_TYPE_TINY) {}
  const char *get_length() const override { return "1"; }
};

enum class Char_type : ulong {
  CHAR = MYSQL_TYPE_STRING,
  VARCHAR = MYSQL_TYPE_VARCHAR,
  TEXT = MYSQL_TYPE_BLOB,
};

class PT_char_type : public PT_type {
  const char *length;
  const CHARSET_INFO *charset;
  const bool force_binary;

  using Parent_type = std::remove_const<decltype(PT_type::type)>::type;

 public:
  PT_char_type(Char_type char_type, const char *length,
               const CHARSET_INFO *charset, bool force_binary = false)
      : PT_type(static_cast<Parent_type>(char_type)),
        length(length),
        charset(charset),
        force_binary(force_binary) {
    DBUG_ASSERT(charset == nullptr || !force_binary);
  }
  PT_char_type(Char_type char_type, const CHARSET_INFO *charset,
               bool force_binary = false)
      : PT_char_type(char_type, "1", charset, force_binary) {}
  ulong get_type_flags() const override {
    return force_binary ? BINCMP_FLAG : 0;
  }
  const char *get_length() const override { return length; }
  const CHARSET_INFO *get_charset() const override { return charset; }
};

enum class Blob_type {
  TINY = MYSQL_TYPE_TINY_BLOB,
  MEDIUM = MYSQL_TYPE_MEDIUM_BLOB,
  LONG = MYSQL_TYPE_LONG_BLOB,
};

/**
  Node for BLOB types

  Types: BLOB, TINYBLOB, MEDIUMBLOB, LONGBLOB, LONG, LONG VARBINARY,
         LONG VARCHAR, TEXT, TINYTEXT, MEDIUMTEXT, LONGTEXT.

  @ingroup ptn_column_types
*/
class PT_blob_type : public PT_type {
  const char *length;
  const CHARSET_INFO *charset;
  const bool force_binary;

  using Parent_type = std::remove_const<decltype(PT_type::type)>::type;

 public:
  PT_blob_type(Blob_type blob_type, const CHARSET_INFO *charset,
               bool force_binary = false)
      : PT_type(static_cast<Parent_type>(blob_type)),
        length(nullptr),
        charset(charset),
        force_binary(force_binary) {
    DBUG_ASSERT(charset == nullptr || !force_binary);
  }
  explicit PT_blob_type(const char *length)
      : PT_type(MYSQL_TYPE_BLOB),
        length(length),
        charset(&my_charset_bin),
        force_binary(false) {}

  ulong get_type_flags() const override {
    return force_binary ? BINCMP_FLAG : 0;
  }
  const CHARSET_INFO *get_charset() const override { return charset; }
  const char *get_length() const override { return length; }
};

/**
  Node for the YEAR type

  @ingroup ptn_column_types
*/
class PT_year_type : public PT_type {
 public:
  PT_year_type() : PT_type(MYSQL_TYPE_YEAR) {}
};

/**
  Node for the DATE type

  @ingroup ptn_column_types
*/
class PT_date_type : public PT_type {
 public:
  PT_date_type() : PT_type(MYSQL_TYPE_DATE) {}
};

enum class Time_type : ulong {
  TIME = MYSQL_TYPE_TIME2,
  DATETIME = MYSQL_TYPE_DATETIME2,
};

/**
  Node for the TIME, TIMESTAMP and DATETIME types

  @ingroup ptn_column_types
*/
class PT_time_type : public PT_type {
  const char *dec;

  using Parent_type = std::remove_const<decltype(PT_type::type)>::type;

 public:
  PT_time_type(Time_type time_type, const char *dec)
      : PT_type(static_cast<Parent_type>(time_type)), dec(dec) {}

  const char *get_dec() const override { return dec; }
};

/**
  Node for the TIMESTAMP type

  @ingroup ptn_column_types
*/
class PT_timestamp_type : public PT_type {
  typedef PT_type super;

  const char *dec;
  ulong type_flags;

 public:
  explicit PT_timestamp_type(const char *dec)
      : super(MYSQL_TYPE_TIMESTAMP2), dec(dec), type_flags(0) {}

  const char *get_dec() const override { return dec; }
  ulong get_type_flags() const override { return type_flags; }

  bool contextualize(Parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    /*
      TIMESTAMP fields are NOT NULL by default, unless the variable
      explicit_defaults_for_timestamp is true.
    */
    if (!pc->thd->variables.explicit_defaults_for_timestamp)
      type_flags = NOT_NULL_FLAG;
    /*
      To flag the current statement as dependent for binary
      logging on the session var. Extra copying to Lex is
      done in case prepared stmt.
    */
    pc->thd->lex->binlog_need_explicit_defaults_ts =
        pc->thd->binlog_need_explicit_defaults_ts = true;

    return false;
  }
};

/**
  Node for spatial types

  Types: GEOMETRY, GEOMCOLLECTION/GEOMETRYCOLLECTION, POINT, MULTIPOINT,
         LINESTRING, MULTILINESTRING, POLYGON, MULTIPOLYGON

  @ingroup ptn_column_types
*/
class PT_spacial_type : public PT_type {
  Field::geometry_type geo_type;

 public:
  explicit PT_spacial_type(Field::geometry_type geo_type)
      : PT_type(MYSQL_TYPE_GEOMETRY), geo_type(geo_type) {}

  const CHARSET_INFO *get_charset() const override { return &my_charset_bin; }
  uint get_uint_geom_type() const override { return geo_type; }
  const char *get_length() const override { return nullptr; }
};

enum class Enum_type { ENUM = MYSQL_TYPE_ENUM, SET = MYSQL_TYPE_SET };

template <Enum_type enum_type>
class PT_enum_type_tmpl : public PT_type {
  List<String> *const interval_list;
  const CHARSET_INFO *charset;
  const bool force_binary;

  using Parent_type = std::remove_const<decltype(PT_type::type)>::type;

 public:
  PT_enum_type_tmpl(List<String> *interval_list, const CHARSET_INFO *charset,
                    bool force_binary)
      : PT_type(static_cast<Parent_type>(enum_type)),
        interval_list(interval_list),
        charset(charset),
        force_binary(force_binary) {
    DBUG_ASSERT(charset == nullptr || !force_binary);
  }

  const CHARSET_INFO *get_charset() const override { return charset; }
  ulong get_type_flags() const override {
    return force_binary ? BINCMP_FLAG : 0;
  }
  List<String> *get_interval_list() const override { return interval_list; }
};

/**
  Node for the ENUM type

  @ingroup ptn_column_types
*/
typedef PT_enum_type_tmpl<Enum_type::ENUM> PT_enum_type;

/**
  Node for the SET type

  @ingroup ptn_column_types
*/
typedef PT_enum_type_tmpl<Enum_type::SET> PT_set_type;

class PT_serial_type : public PT_type {
 public:
  PT_serial_type() : PT_type(MYSQL_TYPE_LONGLONG) {}

  ulong get_type_flags() const override {
    return AUTO_INCREMENT_FLAG | NOT_NULL_FLAG | UNSIGNED_FLAG | UNIQUE_FLAG;
  }
};

/**
  Node for the JSON type

  @ingroup ptn_column_types
*/
class PT_json_type : public PT_type {
 public:
  PT_json_type() : PT_type(MYSQL_TYPE_JSON) {}
  const CHARSET_INFO *get_charset() const override { return &my_charset_bin; }
};

/**
  Base class for both generated and regular column definitions

  @ingroup ptn_create_table
*/
class PT_field_def_base : public Parse_tree_node {
  typedef Parse_tree_node super;
  typedef decltype(Alter_info::flags) alter_info_flags_t;

 public:
  enum_field_types type;
  ulong type_flags;
  const char *length;
  const char *dec;
  const CHARSET_INFO *charset;
  bool has_explicit_collation;
  uint uint_geom_type;
  List<String> *interval_list;
  alter_info_flags_t alter_info_flags;
  LEX_CSTRING comment;
  Item *default_value;
  Item *on_update_value;
  Value_generator *gcol_info;
  /// Holds the expression to generate default values
  Value_generator *default_val_info;
  Nullable<gis::srid_t> m_srid;
  // List of column check constraint's specification.
  Sql_check_constraint_spec_list *check_const_spec_list{nullptr};

 protected:
  PT_type *type_node;

  explicit PT_field_def_base(PT_type *type_node)
      : has_explicit_collation(false),
        alter_info_flags(0),
        comment(EMPTY_CSTR),
        default_value(nullptr),
        on_update_value(nullptr),
        gcol_info(nullptr),
        default_val_info(nullptr),
        type_node(type_node) {}

 public:
  bool contextualize(Parse_context *pc) override {
    if (super::contextualize(pc) || type_node->contextualize(pc)) return true;

    type = type_node->type;
    type_flags = type_node->get_type_flags();
    length = type_node->get_length();
    dec = type_node->get_dec();
    charset = type_node->get_charset();
    uint_geom_type = type_node->get_uint_geom_type();
    interval_list = type_node->get_interval_list();
    check_const_spec_list = new (pc->thd->mem_root)
        Sql_check_constraint_spec_list(pc->thd->mem_root);
    if (check_const_spec_list == nullptr) return true;  // OOM
    return false;
  }

 protected:
  template <class T>
  bool contextualize_attrs(Column_parse_context *pc,
                           Mem_root_array<T *> *attrs) {
    if (attrs != nullptr) {
      for (auto attr : *attrs) {
        if (attr->contextualize(pc)) return true;
        attr->apply_type_flags(&type_flags);
        attr->apply_alter_info_flags(&alter_info_flags);
        attr->apply_comment(&comment);
        attr->apply_default_value(&default_value);
        attr->apply_gen_default_value(&default_val_info);
        attr->apply_on_update_value(&on_update_value);
        attr->apply_srid_modifier(&m_srid);
        if (attr->apply_collation(pc, &charset, &has_explicit_collation))
          return true;
        if (attr->add_check_constraints(check_const_spec_list)) return true;
      }
    }
    return false;
  }
};

/**
  Base class for regular (non-generated) column definition nodes

  @ingroup ptn_create_table
*/
class PT_field_def : public PT_field_def_base {
  typedef PT_field_def_base super;

  Mem_root_array<PT_column_attr_base *> *opt_attrs;

 public:
  PT_field_def(PT_type *type_node_arg,
               Mem_root_array<PT_column_attr_base *> *opt_attrs)
      : super(type_node_arg), opt_attrs(opt_attrs) {}

  bool contextualize(Parse_context *pc_arg) override {
    Column_parse_context pc(pc_arg->thd, pc_arg->select, false);
    return super::contextualize(&pc) || contextualize_attrs(&pc, opt_attrs);
  }
};

/**
  Base class for generated column definition nodes

  @ingroup ptn_create_table
*/
class PT_generated_field_def : public PT_field_def_base {
  typedef PT_field_def_base super;

  const Virtual_or_stored virtual_or_stored;
  Item *expr;
  Mem_root_array<PT_column_attr_base *> *opt_attrs;

 public:
  PT_generated_field_def(PT_type *type_node_arg, Item *expr,
                         Virtual_or_stored virtual_or_stored,
                         Mem_root_array<PT_column_attr_base *> *opt_attrs)
      : super(type_node_arg),
        virtual_or_stored(virtual_or_stored),
        expr(expr),
        opt_attrs(opt_attrs) {}

  bool contextualize(Parse_context *pc_arg) override {
    Column_parse_context pc(pc_arg->thd, pc_arg->select, true);
    if (super::contextualize(&pc) || contextualize_attrs(&pc, opt_attrs) ||
        expr->itemize(&pc, &expr))
      return true;

    gcol_info = new (pc.mem_root) Value_generator;
    if (gcol_info == nullptr) return true;  // OOM
    gcol_info->expr_item = expr;
    if (virtual_or_stored == Virtual_or_stored::STORED)
      gcol_info->set_field_stored(true);
    gcol_info->set_field_type(type);

    return false;
  }
};

#endif /* PARSE_TREE_COL_ATTRS_INCLUDED */
