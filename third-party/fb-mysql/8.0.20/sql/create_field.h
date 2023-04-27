#ifndef SQL_CREATE_FIELD_INCLUDED
#define SQL_CREATE_FIELD_INCLUDED

/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "field_types.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_base.h"
#include "nullable.h"
#include "sql/dd/types/column.h"
#include "sql/field.h"
#include "sql/gis/srid.h"
#include "sql/sql_list.h"
#include "typelib.h"

class Item;
class String;
class Value_generator;

/// Create_field is a description a field/column that may or may not exists in
/// a table.
///
/// The main usage of Create_field is to contain the description of a column
/// given by the user (usually given with CREATE TABLE). It is also used to
/// describe changes to be carried out on a column (usually given with ALTER
/// TABLE ... CHANGE COLUMN).
class Create_field {
 public:
  /// @returns the maximum display width of this column, in number of.
  ///          code points. See m_max_display_width_in_codepoints for an
  ///          explanation of "display width" and "code point".
  size_t max_display_width_in_codepoints() const;

  /// @returns the maximum display width of this column, in number of bytes. For
  ///          numeric types, temporal types, YEAR and BIT, this method returns
  ///          the same as max_display_width_in_codepoints(). For character
  ///          types (CHAR, VARCHAR, TEXT etc) the returned value depends on
  ///          max_display_width_in_codepoints() and the character set of this
  ///          column.
  size_t max_display_width_in_bytes() const;

  /// @returns the pack length for this column, which is the number of bytes
  ///          needed to store this column in memory. Note that blob returns
  ///          a length variable + the size of a pointer to an external memory
  ///          location where the actual data is stored. So LONGBLOB would
  ///          return 4 bytes for the length variable + 8 bytes for the pointer
  ///          to the data (12 bytes in total).
  ///  @param dont_override  Don't use pack_length_override even if non-zero
  ///                        Used by multi-valued index, where pack_length
  ///                        and key_length aren't the same.
  size_t pack_length(bool dont_override = false) const;

  /// @returns the key length for this column.
  size_t key_length() const;

  /// @retval true if the maximum column length was given explicitly by the
  ///         user.
  /// @retval false if the user didn't specify any maximum length.
  bool explicit_display_width() const { return m_explicit_display_width; }

  /// Set the maximum display width based on another Create_field.
  void set_max_display_width_from_create_field(
      const Create_field &create_field) {
    m_max_display_width_in_codepoints =
        create_field.max_display_width_in_codepoints();
  }

  dd::Column::enum_hidden_type hidden;

  const char *field_name;
  /**
    Name of column modified by ALTER TABLE's CHANGE/MODIFY COLUMN clauses,
    NULL for columns added.
  */
  const char *change;
  const char *after;    // Put column after this one
  LEX_CSTRING comment;  // Comment for field

  /**
     The declared default value, if any, otherwise NULL. Note that this member
     is NULL if the default is a function. If the column definition has a
     function declared as the default, the information is found in
     Create_field::auto_flags.

     @see Create_field::auto_flags
  */
  Item *constant_default;
  enum_field_types sql_type;
  uint decimals;
  uint flags{0};
  /**
    Bitmap of flags indicating if field value should be auto-generated
    by default and/or on update, and in which way.

    @sa Field::enum_auto_flags for possible options.
  */
  uchar auto_flags{Field::NONE};
  TYPELIB *interval;  // Which interval to use
                      // Used only for UCS2 intervals
  List<String> interval_list;
  const CHARSET_INFO *charset;
  bool is_explicit_collation;  // User exeplicitly provided charset ?
  Field::geometry_type geom_type;
  Field *field;  // For alter table

  uint offset;

  /**
    Indicate whether column is nullable, zerofill or unsigned.

    Initialized based on flags and other members at prepare_create_field()/
    init_for_tmp_table() stage.
  */
  bool is_nullable;
  bool is_zerofill;
  bool is_unsigned;

  /**
    Indicates that storage engine doesn't support optimized BIT field
    storage.

    @note We also use safe/non-optimized version of BIT field for
          special cases like virtual temporary tables.

    Initialized at mysql_prepare_create_table()/sp_prepare_create_field()/
    init_for_tmp_table() stage.
  */
  bool treat_bit_as_char;

  /**
    Row based replication code sometimes needs to create ENUM and SET
    fields with pack length which doesn't correspond to number of
    elements in interval TYPELIB.

    When this member is non-zero ENUM/SET field to be created will use
    its value as pack length instead of one calculated from number
    elements in its interval.

    Initialized at prepare_create_field()/init_for_tmp_table() stage.
  */
  uint pack_length_override{0};

  /* Generated column expression information */
  Value_generator *gcol_info{nullptr};
  /*
    Indication that the field is phycically stored in tables
    rather than just generated on SQL queries.
    As of now, false can only be set for virtual generated columns.
  */
  bool stored_in_db;

  /// Holds the expression to be used to generate default values.
  Value_generator *m_default_val_expr{nullptr};
  Nullable<gis::srid_t> m_srid;

  // Whether the field is actually an array of the field's type;
  bool is_array{false};

  Create_field()
      : after(nullptr),
        is_explicit_collation(false),
        geom_type(Field::GEOM_GEOMETRY),
        is_nullable(false),
        is_zerofill(false),
        is_unsigned(false),
        /*
          Initialize treat_bit_as_char for all field types even if
          it is only used for MYSQL_TYPE_BIT. This avoids bogus
          valgrind warnings in optimized builds.
        */
        treat_bit_as_char(false),
        pack_length_override(0),
        stored_in_db(false),
        m_default_val_expr(nullptr) {}
  Create_field(Field *field, Field *orig_field);

  /* Used to make a clone of this object for ALTER/CREATE TABLE */
  Create_field *clone(MEM_ROOT *mem_root) const {
    return new (mem_root) Create_field(*this);
  }
  bool is_gcol() const { return gcol_info; }
  bool is_virtual_gcol() const {
    return gcol_info && !gcol_info->get_field_stored();
  }

  /* Init for a tmp table field. To be extended if need be. */
  void init_for_tmp_table(enum_field_types sql_type_arg, uint32 max_length,
                          uint32 decimals, bool is_nullable, bool is_unsigned,
                          uint pack_length_override,
                          const char *field_name = "");

  bool init(THD *thd, const char *field_name, enum_field_types type,
            const char *length, const char *decimals, uint type_modifier,
            Item *default_value, Item *on_update_value, LEX_CSTRING *comment,
            const char *change, List<String> *interval_list,
            const CHARSET_INFO *cs, bool has_explicit_collation,
            uint uint_geom_type, Value_generator *gcol_info,
            Value_generator *default_val_expr, Nullable<gis::srid_t> srid,
            dd::Column::enum_hidden_type hidden, bool is_array = false);

  ha_storage_media field_storage_type() const {
    return (ha_storage_media)((flags >> FIELD_FLAGS_STORAGE_MEDIA) & 3);
  }

  column_format_type column_format() const {
    return (column_format_type)((flags >> FIELD_FLAGS_COLUMN_FORMAT) & 3);
  }

 private:
  /// The maximum display width of this column.
  ///
  /// The "display width" is the number of code points that is needed to print
  /// out the string represenation of a value. It can be given by the user
  /// both explicitly and implicitly. If a user creates a table with the columns
  /// "a VARCHAR(3), b INT(3)", both columns are given an explicit display width
  /// of 3 code points. But if a user creates a table with the columns
  /// "a INT, b TINYINT UNSIGNED", the first column has an implicit display
  /// width of 11 (-2147483648 is the longest value for a signed int) and the
  /// second column has an implicit display width of 3 (255 is the longest value
  /// for an unsigned tinyint).
  /// This is related to storage size for some types (VARCHAR, BLOB etc), but
  /// not for all types (an INT is four bytes regardless of the display width).
  ///
  /// A "code point" is bascially a numeric value. For instance, ASCII
  /// compromises of 128 code points (0x00 to 0x7F), while unicode contains way
  /// more. In most cases a code point represents a single graphical unit (aka
  /// grapheme), but not always. For instance, É may consists of two code points
  /// where one is the letter E and the other one is the quotation mark above
  /// the letter.
  size_t m_max_display_width_in_codepoints{0};

  /// Whether or not the display width was given explicitly by the user.
  bool m_explicit_display_width{false};

  /// The maximum number of bytes a TINYBLOB can hold.
  static constexpr size_t TINYBLOB_MAX_SIZE_IN_BYTES{255};

  /// The maximum number of bytes a BLOB can hold.
  static constexpr size_t BLOB_MAX_SIZE_IN_BYTES{65535};

  /// The maximum number of bytes a MEDIUMBLOB can hold.
  static constexpr size_t MEDIUMBLOB_MAX_SIZE_IN_BYTES{16777215};

  /// The maximum number of bytes a LONGBLOB can hold.
  static constexpr size_t LONGBLOB_MAX_SIZE_IN_BYTES{4294967295};
};

/// @returns whether or not this field is a hidden column that represents a
///          functional index.
bool is_field_for_functional_index(const Create_field *create_field);

#endif
