/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @addtogroup Replication
  @{

  @file rows_event.h

  @brief Contains the classes representing events which are used for row based
  replication. In row-based replication, the master writes events to the binary
  log that indicate how individual table rows are changed.
*/

#ifndef ROWS_EVENT_INCLUDED
#define ROWS_EVENT_INCLUDED

#include <sstream>
#include <vector>
#include "control_events.h"
#include "my_dbug.h"
#include "table_id.h"

/**
   1 byte length, 1 byte format
   Length is total length in bytes, including 2 byte header
   Length values 0 and 1 are currently invalid and reserved.
*/
#define EXTRA_ROW_INFO_LEN_OFFSET 0
#define EXTRA_ROW_INFO_FORMAT_OFFSET 1
#define EXTRA_ROW_INFO_HEADER_LENGTH 2
#define EXTRA_ROW_INFO_MAX_PAYLOAD (255 - EXTRA_ROW_INFO_HEADER_LENGTH)

#define ROWS_MAPID_OFFSET 0
#define ROWS_FLAGS_OFFSET 6
#define ROWS_VHLEN_OFFSET 8
#define EXTRA_ROW_INFO_TYPECODE_LENGTH 1
#define EXTRA_ROW_PART_INFO_VALUE_LENGTH 2

/**
  This is the typecode defined for the different elements present in
  the container Extra_row_info, this is different from the format information
  stored inside extra_row_ndb_info at EXTRA_ROW_INFO_FORMAT_OFFSET.
*/
enum class enum_extra_row_info_typecode { NDB = 0, PART = 1 };

namespace binary_log {
/**
  @class Table_map_event

  In row-based mode, every row operation event is preceded by a
  Table_map_event which maps a table definition to a number.  The
  table definition consists of database name, table name, and column
  definitions.

  @section Table_map_event_binary_format Binary Format

  The Post-Header has the following components:

  <table>
  <caption>Post-Header for Table_map_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>table_id</td>
    <td>6 bytes unsigned integer</td>
    <td>The number that identifies the table.</td>
  </tr>

  <tr>
    <td>flags</td>
    <td>2 byte bitfield</td>
    <td>Reserved for future use; currently always 0.</td>
  </tr>

  </table>

  The Body has the following components:

  <table>
  <caption>Body for Table_map_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>database_name</td>
    <td>one byte string length, followed by null-terminated string</td>
    <td>The name of the database in which the table resides.  The name
    is represented as a one byte unsigned integer representing the
    number of bytes in the name, followed by length bytes containing
    the database name, followed by a terminating 0 byte.  (Note the
    redundancy in the representation of the length.)  </td>
  </tr>

  <tr>
    <td>table_name</td>
    <td>one byte string length, followed by null-terminated string</td>
    <td>The name of the table, encoded the same way as the database
    name above.</td>
  </tr>

  <tr>
    <td>column_count</td>
    <td>@ref packed_integer "Packed Integer"</td>
    <td>The number of columns in the table, represented as a packed
    variable-length integer.</td>
  </tr>

  <tr>
    <td>column_type</td>
    <td>List of column_count 1 byte enumeration values</td>
    <td>The type of each column in the table, listed from left to
    right.  Each byte is mapped to a column type according to the
    enumeration type enum_field_types defined in mysql_com.h.  The
    mapping of types to numbers is listed in the table @ref
    Table_table_map_event_column_types "below" (along with
    description of the associated metadata field).  </td>
  </tr>

  <tr>
    <td>metadata_length</td>
    <td>@ref packed_integer "Packed Integer"</td>
    <td>The length of the following metadata block</td>
  </tr>

  <tr>
    <td>metadata</td>
    <td>list of metadata for each column</td>
    <td>For each column from left to right, a chunk of data who's
    length and semantics depends on the type of the column.  The
    length and semantics for the metadata for each column are listed
    in the table @ref Table_table_map_event_column_types
    "below".</td>
  </tr>

  <tr>
    <td>null_bits</td>
    <td>column_count bits, rounded up to nearest byte</td>
    <td>For each column, a bit indicating whether data in the column
    can be NULL or not.  The number of bytes needed for this is
    int((column_count + 7) / 8).  The flag for the first column from the
    left is in the least-significant bit of the first byte, the second
    is in the second least significant bit of the first byte, the
    ninth is in the least significant bit of the second byte, and so
    on.  </td>
  </tr>

  <tr>
    <td>optional metadata fields</td>
    <td>optional metadata fields are stored in Type, Length, Value(TLV) format.
    Type takes 1 byte. Length is a packed integer value. Values takes
    Length bytes.
    </td>
    <td>There are some optional metadata defined. They are listed in the table
    @ref Table_table_map_event_optional_metadata. Optional metadata fields
    follow null_bits. Whether binlogging an optional metadata is decided by the
    server. The order is not defined, so they can be binlogged in any order.
    </td>
  </tr>
  </table>

  The table below lists all column types, along with the numerical
  identifier for it and the size and interpretation of meta-data used
  to describe the type.

  @anchor Table_table_map_event_column_types
  <table>
  <caption>Table_map_event column types: numerical identifier and
  metadata</caption>
  <tr>
    <th>Name</th>
    <th>Identifier</th>
    <th>Size of metadata in bytes</th>
    <th>Description of metadata</th>
  </tr>

  <tr>
    <td>MYSQL_TYPE_DECIMAL</td><td>0</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_TINY</td><td>1</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_SHORT</td><td>2</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_LONG</td><td>3</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_FLOAT</td><td>4</td>
    <td>1 byte</td>
    <td>1 byte unsigned integer, representing the "pack_length", which
    is equal to sizeof(float) on the server from which the event
    originates.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_DOUBLE</td><td>5</td>
    <td>1 byte</td>
    <td>1 byte unsigned integer, representing the "pack_length", which
    is equal to sizeof(double) on the server from which the event
    originates.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_NULL</td><td>6</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_TIMESTAMP</td><td>7</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_LONGLONG</td><td>8</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_INT24</td><td>9</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_DATE</td><td>10</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_TIME</td><td>11</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_DATETIME</td><td>12</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_YEAR</td><td>13</td>
    <td>0</td>
    <td>No column metadata.</td>
  </tr>

  <tr>
    <td><i>MYSQL_TYPE_NEWDATE</i></td><td><i>14</i></td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_VARCHAR</td><td>15</td>
    <td>2 bytes</td>
    <td>2 byte unsigned integer representing the maximum length of
    the string.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_BIT</td><td>16</td>
    <td>2 bytes</td>
    <td>A 1 byte unsigned int representing the length in bits of the
    bitfield (0 to 64), followed by a 1 byte unsigned int
    representing the number of bytes occupied by the bitfield.  The
    number of bytes is either int((length + 7) / 8) or int(length / 8).
    </td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_NEWDECIMAL</td><td>246</td>
    <td>2 bytes</td>
    <td>A 1 byte unsigned int representing the precision, followed
    by a 1 byte unsigned int representing the number of decimals.</td>
  </tr>

  <tr>
    <td><i>MYSQL_TYPE_ENUM</i></td><td><i>247</i></td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td><i>MYSQL_TYPE_SET</i></td><td><i>248</i></td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_TINY_BLOB</td><td>249</td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td><i>MYSQL_TYPE_MEDIUM_BLOB</i></td><td><i>250</i></td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td><i>MYSQL_TYPE_LONG_BLOB</i></td><td><i>251</i></td>
    <td>&ndash;</td>
    <td><i>This enumeration value is only used internally and cannot
    exist in a binlog.</i></td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_BLOB</td><td>252</td>
    <td>1 byte</td>
    <td>The pack length, i.e., the number of bytes needed to represent
    the length of the blob: 1, 2, 3, or 4.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_VAR_STRING</td><td>253</td>
    <td>2 bytes</td>
    <td>This is used to store both strings and enumeration values.
    The first byte is a enumeration value storing the <i>real
    type</i>, which may be either MYSQL_TYPE_VAR_STRING or
    MYSQL_TYPE_ENUM.  The second byte is a 1 byte unsigned integer
    representing the field size, i.e., the number of bytes needed to
    store the length of the string.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_STRING</td><td>254</td>
    <td>2 bytes</td>
    <td>The first byte is always MYSQL_TYPE_VAR_STRING (i.e., 253).
    The second byte is the field size, i.e., the number of bytes in
    the representation of size of the string: 3 or 4.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_GEOMETRY</td><td>255</td>
    <td>1 byte</td>
    <td>The pack length, i.e., the number of bytes needed to represent
    the length of the geometry: 1, 2, 3, or 4.</td>
  </tr>

  <tr>
    <td>MYSQL_TYPE_TYPED_ARRAY</td><td>15</td>
    <td>up to 4 bytes</td>
    <td>
      - The first byte holds the MySQL type for the elements.
      - The following 0, 1, 2, or 3 bytes holds the metadata for the MySQL
        type for the elements. The contents of these bytes depends on the
        element type, as described in the other rows of this table.
    </td>
  </tr>

  </table>

  The table below lists all optional metadata types, along with the numerical
  identifier for it and the size and interpretation of meta-data used
  to describe the type.

  @anchor Table_table_map_event_optional_metadata
  <table>
  <caption>Table_map_event optional metadata types: numerical identifier and
  metadata. Optional metadata fields are stored in TLV fields.
  Format of values are described in this table. </caption>
  <tr>
    <th>Type</th>
    <th>Description</th>
    <th>Format</th>
  </tr>
  <tr>
    <td>SIGNEDNESS</td>
    <td>signedness of numeric colums. This is included for all values of
    binlog_row_metadata.</td>
    <td>For each numeric column, a bit indicates whether the numeric
    colunm has unsigned flag. 1 means it is unsigned. The number of
    bytes needed for this is int((column_count + 7) / 8). The order is
    the same as the order of column_type field.</td>
  </tr>
  <tr>
    <td>DEFAULT_CHARSET</td>
    <td>Charsets of character columns. It has a default charset for
    the case that most of character columns have same charset and the
    most used charset is binlogged as default charset.Collation
    numbers are binlogged for identifying charsets. They are stored in
    packed length format.  Either DEFAULT_CHARSET or COLUMN_CHARSET is
    included for all values of binlog_row_metadata.</td>
    <td>Default charset's collation is logged first. The charsets which are not
    same to default charset are logged following default charset. They are
    logged as column index and charset collation number pair sequence. The
    column index is counted only in all character columns. The order is same to
    the order of column_type
    field. </td>
  </tr>
  <tr>
    <td>COLUMN_CHARSET</td>
    <td>Charsets of character columns. For the case that most of columns have
    different charsets, this field is logged. It is never logged with
    DEFAULT_CHARSET together.  Either DEFAULT_CHARSET or COLUMN_CHARSET is
    included for all values of binlog_row_metadata.</td>
    <td>It is a collation number sequence for all character columns.</td>
  </tr>
  <tr>
    <td>COLUMN_NAME</td>
    <td>Names of columns. This is only included if
    binlog_row_metadata=FULL.</td>
    <td>A sequence of column names. For each column name, 1 byte for
    the string length in bytes is followed by a string without null
    terminator.</td>
  </tr>
  <tr>
    <td>SET_STR_VALUE</td>
    <td>The string values of SET columns. This is only included if
    binlog_row_metadata=FULL.</td>
    <td>For each SET column, a pack_length representing the value
    count is followed by a sequence of length and string pairs. length
    is the byte count in pack_length format. The string has no null
    terminator.</td>
  </tr>
  <tr>
    <td>ENUM_STR_VALUE</td>
    <td>The string values is ENUM columns. This is only included
    if binlog_row_metadata=FULL.</td>
    <td>The format is the same as SET_STR_VALUE.</td>
  </tr>
  <tr>
    <td>GEOMETRY_TYPE</td>
    <td>The real type of geometry columns. This is only included
    if binlog_row_metadata=FULL.</td>
    <td>A sequence of real type of geometry columns are stored in pack_length
    format. </td>
  </tr>
  <tr>
    <td>SIMPLE_PRIMARY_KEY</td>
    <td>The primary key without any prefix. This is only included
    if binlog_row_metadata=FULL and there is a primary key where every
    key part covers an entire column.</td>
    <td>A sequence of column indexes. The indexes are stored in pack_length
    format.</td>
  </tr>
  <tr>
    <td>PRIMARY_KEY_WITH_PREFIX</td>
    <td>The primary key with some prefix. It doesn't appear together with
    SIMPLE_PRIMARY_KEY. This is only included if
    binlog_row_metadata=FULL and there is a primary key where some key
    part covers a prefix of the column.</td>
    <td>A sequence of column index and prefix length pairs. Both
    column index and prefix length are in pack_length format. Prefix length
    0 means that the whole column value is used.</td>
  </tr>
  <tr>
    <td>ENUM_AND_SET_DEFAULT_CHARSET</td>
    <td>Charsets of ENUM and SET columns. It has the same layout as
    DEFAULT_CHARSET.  If there are SET or ENUM columns and
    binlog_row_metadata=FULL, exactly one of
    ENUM_AND_SET_DEFAULT_CHARSET and ENUM_AND_SET_COLUMN_CHARSET
    appears (the encoder chooses the representation that uses the
    least amount of space).  Otherwise, none of them appears.</td>
    <td>The same format as for DEFAULT_CHARSET, except it counts ENUM
    and SET columns rather than character columns.</td>
  </tr>
  <tr>
    <td>ENUM_AND_SET_COLUMN_CHARSET</td>
    <td>Charsets of ENUM and SET columns. It has the same layout as
    COLUMN_CHARSET.  If there are SET or ENUM columns and
    binlog_row_metadata=FULL, exactly one of
    ENUM_AND_SET_DEFAULT_CHARSET and ENUM_AND_SET_COLUMN_CHARSET
    appears (the encoder chooses the representation that uses the
    least amount of space).  Otherwise, none of them appears.</td>
    <td>The same format as for COLUMN_CHARSET, except it counts ENUM
    and SET columns rather than character columns.</td>
  </tr>
  </table>
*/
class Table_map_event : public Binary_log_event {
 public:
  /** Constants representing offsets */
  enum Table_map_event_offset {
    /** TM = "Table Map" */
    TM_MAPID_OFFSET = 0,
    TM_FLAGS_OFFSET = 6
  };

  typedef uint16_t flag_set;

  /**
    DEFAULT_CHARSET and COLUMN_CHARSET don't appear together, and
    ENUM_AND_SET_DEFAULT_CHARSET and ENUM_AND_SET_COLUMN_CHARSET don't
    appear together. They are just alternative ways to pack character
    set information. When binlogging, it logs character sets in the
    way that occupies least storage.

    SIMPLE_PRIMARY_KEY and PRIMARY_KEY_WITH_PREFIX don't appear together.
    SIMPLE_PRIMARY_KEY is for the primary keys which only use whole values of
    pk columns. PRIMARY_KEY_WITH_PREFIX is
    for the primary keys which just use part value of pk columns.
   */
  enum Optional_metadata_field_type {
    SIGNEDNESS = 1,  // UNSIGNED flag of numeric columns
    DEFAULT_CHARSET, /* Character set of string columns, optimized to
                        minimize space when many columns have the
                        same charset. */
    COLUMN_CHARSET,  /* Character set of string columns, optimized to
                        minimize space when columns have many
                        different charsets. */
    COLUMN_NAME,
    SET_STR_VALUE,                // String value of SET columns
    ENUM_STR_VALUE,               // String value of ENUM columns
    GEOMETRY_TYPE,                // Real type of geometry columns
    SIMPLE_PRIMARY_KEY,           // Primary key without prefix
    PRIMARY_KEY_WITH_PREFIX,      // Primary key with prefix
    ENUM_AND_SET_DEFAULT_CHARSET, /* Character set of enum and set
                                     columns, optimized to minimize
                                     space when many columns have the
                                     same charset. */
    ENUM_AND_SET_COLUMN_CHARSET,  /* Character set of enum and set
                                     columns, optimized to minimize
                                     space when many columns have the
                                     same charset. */
  };

  /**
    Metadata_fields organizes m_optional_metadata into a structured format which
    is easy to access.
  */
  struct Optional_metadata_fields {
    typedef std::pair<unsigned int, unsigned int> uint_pair;
    typedef std::vector<std::string> str_vector;

    struct Default_charset {
      Default_charset() : default_charset(0) {}
      bool empty() const { return default_charset == 0; }

      // Default charset for the columns which are not in charset_pairs.
      unsigned int default_charset;

      /* The uint_pair means <column index, column charset number>. */
      std::vector<uint_pair> charset_pairs;
    };

    // Contents of DEFAULT_CHARSET field are converted into Default_charset.
    Default_charset m_default_charset;
    // Contents of ENUM_AND_SET_DEFAULT_CHARSET are converted into
    // Default_charset.
    Default_charset m_enum_and_set_default_charset;
    std::vector<bool> m_signedness;
    // Character set number of every string column
    std::vector<unsigned int> m_column_charset;
    // Character set number of every ENUM or SET column.
    std::vector<unsigned int> m_enum_and_set_column_charset;
    std::vector<std::string> m_column_name;
    // each str_vector stores values of one enum/set column
    std::vector<str_vector> m_enum_str_value;
    std::vector<str_vector> m_set_str_value;
    std::vector<unsigned int> m_geometry_type;
    /*
      The uint_pair means <column index, prefix length>.  Prefix length is 0 if
      whole column value is used.
    */
    std::vector<uint_pair> m_primary_key;

    /*
      It parses m_optional_metadata and populates into above variables.

      @param[in] optional_metadata points to the begin of optional metadata
                                   fields in table_map_event.
      @param[in] optional_metadata_len length of optional_metadata field.
     */
    Optional_metadata_fields(unsigned char *optional_metadata,
                             unsigned int optional_metadata_len);
    // It is used to specify the validity of the deserialized structure
    bool is_valid;
  };

  /**
    <pre>
    The buffer layout for fixed data part is as follows:
    +-----------------------------------+
    | table_id | Reserved for future use|
    +-----------------------------------+
    </pre>

    <pre>
    The buffer layout for variable data part is as follows:
    +--------------------------------------------------------------------------+
    | db len| db name | table len| table name | no of cols | array of col types|
    +--------------------------------------------------------------------------+
    +---------------------------------------------+
    | metadata len | metadata block | m_null_bits |
    +---------------------------------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Table_map_event(const char *buf, const Format_description_event *fde);

  Table_map_event(const Table_id &tid, unsigned long colcnt, const char *dbnam,
                  size_t dblen, const char *tblnam, size_t tbllen)
      : Binary_log_event(TABLE_MAP_EVENT),
        m_table_id(tid),
        m_fb_format(true),
        m_data_size(0),
        m_dbnam(""),
        m_dblen(dblen),
        m_tblnam(""),
        m_tbllen(tbllen),
        m_colcnt(colcnt),
        m_field_metadata_size(0),
        m_field_metadata(nullptr),
        m_null_bits(nullptr),
        m_optional_metadata_len(0),
        m_optional_metadata(nullptr),
        m_primary_key_fields_size(0),
        m_primary_key_fields(nullptr),
        m_sign_bits_size(0),
        m_sign_bits(nullptr),
        m_column_names_size(0),
        m_column_names(nullptr) {
    if (dbnam) m_dbnam = std::string(dbnam, m_dblen);
    if (tblnam) m_tblnam = std::string(tblnam, m_tbllen);
  }

  virtual ~Table_map_event();

  /** Event post header contents */
  Table_id m_table_id;
  flag_set m_flags;
  bool m_fb_format;

  size_t m_data_size; /** event data size */

  /** Event body contents */
  std::string m_dbnam;
  size_t m_dblen;
  std::string m_tblnam;
  size_t m_tbllen;
  unsigned long m_colcnt;
  unsigned char *m_coltype;

  /**
    The size of field metadata buffer set by calling save_field_metadata()
  */
  unsigned long m_field_metadata_size;
  unsigned char *m_field_metadata; /** field metadata */
  unsigned char *m_null_bits;
  unsigned int m_optional_metadata_len;
  unsigned char *m_optional_metadata;
  unsigned int m_primary_key_fields_size;
  unsigned char *m_primary_key_fields;

  /**
    Bitmap storing the unsigned flag for all the columns in this table. This
    is necessary for external application reading mysql binary logs and get the
    correct value of integer types.
  */
  unsigned int m_sign_bits_size;
  unsigned char *m_sign_bits;

  /**
    Since m_column_names buffer contains terminating '\0' in the middle,
    using strlen() will not give correct length, so track the actual length
    in the size variable.

    Table column names are added to the Table_map_log_event at the end
    in the following format:
    a) Length of the column name including the terminating '\0' is added in
    one byte (strlen(column_name) + 1). One byte is enough since maximum
    column name length is 64.
    b) column_name is appended to the buffer including the terminating '\0'.
  */
  unsigned long m_column_names_size;
  unsigned char *m_column_names;

  Table_map_event()
      : Binary_log_event(TABLE_MAP_EVENT),
        m_coltype(nullptr),
        m_field_metadata_size(0),
        m_field_metadata(nullptr),
        m_null_bits(nullptr),
        m_optional_metadata_len(0),
        m_optional_metadata(nullptr),
        m_primary_key_fields_size(0),
        m_primary_key_fields(nullptr),
        m_sign_bits_size(0),
        m_sign_bits(nullptr),
        m_column_names_size(0),
        m_column_names(nullptr) {}

  unsigned long long get_table_id() { return m_table_id.id(); }
  std::string get_table_name() { return m_tblnam; }
  std::string get_db_name() { return m_dbnam; }

#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif
};

/**
  @class Rows_event

 Common base class for all row-containing binary log events.

 RESPONSIBILITIES

   - Provide an interface for adding an individual row to the event.

  @section Rows_event_binary_format Binary Format

  The Post-Header has the following components:

  <table>
  <caption>Post-Header for Rows_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>table_id</td>
    <td>6 bytes unsigned integer</td>
    <td>The number that identifies the table</td>
  </tr>

  <tr>
    <td>flags</td>
    <td>2 byte bitfield</td>
    <td>Reserved for future use; currently always 0.</td>
  </tr>

  </table>

  The Body has the following components:

  <table>
  <caption>Body for Rows_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>


  <tr>
    <td>width</td>
    <td>packed integer</td>
    <td>Represents the number of columns in the table</td>
  </tr>

  <tr>
    <td>cols</td>
    <td>Bitfield, variable sized</td>
    <td>Indicates whether each column is used, one bit per column.
        For this field, the amount of storage required is
        INT((width + 7) / 8) bytes. </td>
  </tr>

  <tr>
    <td>extra_row_info</td>
    <td>An object of class Extra_row_info</td>
    <td>The class Extra_row_info will be storing the information related
        to m_extra_row_ndb_info and partition info (partition_id and
        source_partition_id). At any given time a Rows_event can have both, one
        or none of ndb_info and partition_info present as part of Rows_event.
        In case both ndb_info and partition_info are present then below will
        be the order in which they will be stored.

        @verbatim
        +----------+--------------------------------------+
        |type_code |        extra_row_ndb_info            |
        +--- ------+--------------------------------------+
        | NDB      |Len of ndb_info |Format |ndb_data     |
        | 1 byte   |1 byte          |1 byte |len - 2 byte |
        +----------+----------------+-------+-------------+

        In case of INSERT/DELETE
        +-----------+----------------+
        | type_code | partition_info |
        +-----------+----------------+
        |   PART    |  partition_id  |
        | (1 byte)  |     2 byte     |
        +-----------+----------------+

        In case of UPDATE
        +-----------+------------------------------------+
        | type_code |        partition_info              |
        +-----------+--------------+---------------------+
        |   PART    | partition_id | source_partition_id |
        | (1 byte)  |    2 byte    |       2 byte        |
        +-----------+--------------+---------------------+

        source_partition_id is used only in the case of Update_event
        to log the partition_id of the source partition.

        @endverbatim
        This is the format for any information stored as extra_row_info.
        type_code is not a part of the class Extra_row_info as it is a constant
        values used at the time of serializing and decoding the event.
   </td>
  </tr>

  <tr>
    <td>columns_before_image</td>
    <td>vector of elements of type uint8_t</td>
    <td>For DELETE and UPDATE only.
        Bit-field indicating whether each column is used
        one bit per column. For this field, the amount of storage
        required for N columns is INT((N + 7) / 8) bytes.</td>
  </tr>

  <tr>
    <td>columns_after_image</td>
    <td>vector of elements of type uint8_t</td>
    <td>For WRITE and UPDATE only.
        Bit-field indicating whether each column is used in the
        UPDATE_ROWS_EVENT and WRITE_ROWS_EVENT after-image; one bit per column.
        For this field, the amount of storage required for N columns
        is INT((N + 7) / 8) bytes.

        @verbatim
          +-------------------------------------------------------+
          | Event Type | Cols_before_image | Cols_after_image     |
          +-------------------------------------------------------+
          |  DELETE    |   Deleted row     |    NULL              |
          |  INSERT    |   NULL            |    Inserted row      |
          |  UPDATE    |   Old     row     |    Updated row       |
          +-------------------------------------------------------+
        @endverbatim
    </td>
  </tr>

  <tr>
    <td>row</td>
    <td>vector of elements of type uint8_t</td>
    <td> A sequence of zero or more rows. The end is determined by the size
         of the event. Each row has the following format:
           - A Bit-field indicating whether each field in the row is NULL.
             Only columns that are "used" according to the second field in
             the variable data part are listed here. If the second field in
             the variable data part has N one-bits, the amount of storage
             required for this field is INT((N + 7) / 8) bytes.
           - The row-image, containing values of all table fields. This only
             lists table fields that are used (according to the second field
             of the variable data part) and non-NULL (according to the
             previous field). In other words, the number of values listed here
             is equal to the number of zero bits in the previous field.
             (not counting padding bits in the last byte).
             @verbatim
                For example, if a INSERT statement inserts into 4 columns of a
                table, N= 4 (in the formula above).
                length of bitmask= (4 + 7) / 8 = 1
                Number of fields in the row= 4.

                        +------------------------------------------------+
                        |Null_bit_mask(4)|field-1|field-2|field-3|field 4|
                        +------------------------------------------------+
             @endverbatim
    </td>
  </tr>
  </table>
*/
class Rows_event : public Binary_log_event {
 public:
  /**
    These definitions allow to combine the flags into an
    appropriate flag set using the normal bitwise operators.  The
    implicit conversion from an enum-constant to an integer is
    accepted by the compiler, which is then used to set the real set
    of flags.
  */
  enum enum_flag {
    /** Last event of a statement */
    STMT_END_F = (1U << 0),
    /** Value of the OPTION_NO_FOREIGN_KEY_CHECKS flag in thd->options */
    NO_FOREIGN_KEY_CHECKS_F = (1U << 1),
    /** Value of the OPTION_RELAXED_UNIQUE_CHECKS flag in thd->options */
    RELAXED_UNIQUE_CHECKS_F = (1U << 2),
    /**
      Indicates that rows in this event are complete, that is contain
      values for all columns of the table.
    */
    COMPLETE_ROWS_F = (1U << 3),
    /**
      Indicates that this event is for writing a row as part of a blind
      'replace into' statement optimization where pk constraints are ignored.
      Note that we are using the MSB to make this forward compatible
    */
    BLIND_REPLACE_INTO_F = (1U << 15),
    /**
      Flags for everything. Please update when you add new flags.
     */
    ALL_FLAGS = STMT_END_F | NO_FOREIGN_KEY_CHECKS_F | COMPLETE_ROWS_F |
                BLIND_REPLACE_INTO_F
  };

  /**
    Constructs an event directly. The members are assigned default values.

    @param type_arg          Type of ROW_EVENT. Expected types are:
                             - WRITE_ROWS_EVENT, WRITE_ROWS_EVENT_V1
                             - UPDATE_ROWS_EVENT, UPDATE_ROWS_EVENT_V1,
                               PARTIAL_UPDATE_ROWS_EVENT
                             - DELETE_ROWS_EVENT, DELETE_ROWS_EVENT_V1
  */
  explicit Rows_event(Log_event_type type_arg)
      : Binary_log_event(type_arg),
        m_table_id(0),
        m_width(0),
        columns_before_image(0),
        columns_after_image(0),
        row(0) {}
  /**
    The constructor is responsible for decoding the event contained in
    the buffer.

    <pre>
    The buffer layout for fixed data part is as follows
    +------------------------------------+
    | table_id | reserved for future use |
    +------------------------------------+
    </pre>

    <pre>
    The buffer layout for variable data part is as follows
    +------------------------------------------------------------------+
    | var_header_len | column_before_image | columns_after_image | row |
    +------------------------------------------------------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Rows_event(const char *buf, const Format_description_event *fde);

  virtual ~Rows_event();

 protected:
  Log_event_type m_type; /** Actual event type */

  /** Post header content */
  Table_id m_table_id;
  uint16_t m_flags; /** Flags for row-level events */

  /* Body of the event */
  unsigned long m_width; /** The width of the columns bitmap */
  uint32_t n_bits_len;   /** value determined by (m_width + 7) / 8 */
  uint16_t var_header_len;

  std::vector<uint8_t> columns_before_image;
  std::vector<uint8_t> columns_after_image;
  std::vector<uint8_t> row;

 public:
  class Extra_row_info {
   private:
    /** partition_id for a row in a partitioned table */
    int m_partition_id;
    /**
      It is the partition_id of the source partition in case
      of Update_event, the target's partition_id is m_partition_id.
      This variable is used only in case of Update_event.
    */
    int m_source_partition_id;
    /** The extra row info provided by NDB */
    unsigned char *m_extra_row_ndb_info;

   public:
    Extra_row_info()
        : m_partition_id(UNDEFINED),
          m_source_partition_id(UNDEFINED),
          m_extra_row_ndb_info(nullptr) {}

    Extra_row_info(const Extra_row_info &) = delete;

    int get_partition_id() const { return m_partition_id; }
    void set_partition_id(int partition_id) {
      BAPI_ASSERT(partition_id <= 65535);
      m_partition_id = partition_id;
    }

    int get_source_partition_id() const { return m_source_partition_id; }
    void set_source_partition_id(int source_partition_id) {
      BAPI_ASSERT(source_partition_id <= 65535);
      m_source_partition_id = source_partition_id;
    }

    unsigned char *get_ndb_info() const { return m_extra_row_ndb_info; }
    void set_ndb_info(const unsigned char *ndb_info, size_t len) {
      BAPI_ASSERT(!have_ndb_info());
      m_extra_row_ndb_info =
          static_cast<unsigned char *>(bapi_malloc(len, 16 /* flags */));
      std::copy(ndb_info, ndb_info + len, m_extra_row_ndb_info);
    }
    /**
      Compares the extra_row_info in a Row event, it checks three things
      1. The m_extra_row_ndb_info pointers. It compares their significant bytes.
      2. Partition_id
      3. source_partition_id

      @return
       true   all the above variables are same in the event and the one passed
              in parameter.
       false  Any of the above variable has a different value.
    */
    bool compare_extra_row_info(const unsigned char *ndb_info_arg,
                                int part_id_arg, int source_part_id);

    bool have_part() const { return m_partition_id != UNDEFINED; }

    bool have_ndb_info() const { return m_extra_row_ndb_info != nullptr; }
    size_t get_ndb_length();
    size_t get_part_length();
    ~Extra_row_info();

    static const int UNDEFINED{INT_MAX};
  };
  Extra_row_info m_extra_row_info;

  unsigned long long get_table_id() const { return m_table_id.id(); }

  enum_flag get_flags() const { return static_cast<enum_flag>(m_flags); }

  uint32_t get_null_bits_len() const { return n_bits_len; }

  unsigned long get_width() const { return m_width; }

  std::string get_enum_flag_string() const {
    std::stringstream flag_str;

#define PARSE_FLAG(__str__, __flag__) \
  if (m_flags & __flag__) {           \
    __str__ << " " #__flag__;         \
  }

    flag_str << " flags:";
    PARSE_FLAG(flag_str, STMT_END_F);
    PARSE_FLAG(flag_str, NO_FOREIGN_KEY_CHECKS_F);
    PARSE_FLAG(flag_str, RELAXED_UNIQUE_CHECKS_F);
    PARSE_FLAG(flag_str, COMPLETE_ROWS_F);
    PARSE_FLAG(flag_str, BLIND_REPLACE_INTO_F);
    auto unknown_flags = (m_flags & ~ALL_FLAGS);
    if (unknown_flags) {
      DBUG_ASSERT(false);
      flag_str << " UNKNOWN_FLAG(";
      flag_str << std::hex << "0x" << unknown_flags << ")";
    }
    return flag_str.str();
  }

  static std::string get_flag_string(enum_flag flag) {
    std::string str = "";
    if (flag & STMT_END_F) str.append(" Last event of the statement");
    if (flag & NO_FOREIGN_KEY_CHECKS_F) str.append(" No foreign Key checks");
    if (flag & RELAXED_UNIQUE_CHECKS_F) str.append(" No unique key checks");
    if (flag & COMPLETE_ROWS_F) str.append(" Complete Rows");
    if (flag & BLIND_REPLACE_INTO_F) str.append(" Blind Replace Into");
    if (flag & ~ALL_FLAGS) str.append("Unknown Flag");
    return str;
  }
#ifndef HAVE_MYSYS
  void print_event_info(std::ostream &info);
  void print_long_info(std::ostream &info);
#endif

  template <class Iterator_value_type>
  friend class Row_event_iterator;
};

/**
  @class Write_rows_event

  Log row insertions. The event contain several  insert/update rows
  for a table. Note that each event contains only  rows for one table.

  @section Write_rows_event_binary_format Binary Format
*/
class Write_rows_event : public virtual Rows_event {
 public:
  Write_rows_event(const char *buf, const Format_description_event *fde);
  Write_rows_event() : Rows_event(WRITE_ROWS_EVENT) {}
};

/**
  @class Update_rows_event

  Log row updates with a before image. The event contain several
  update rows for a table. Note that each event contains only rows for
  one table.

  Also note that the row data consists of pairs of row data: one row
  for the old data and one row for the new data.

  @section Update_rows_event_binary_format Binary Format
*/
class Update_rows_event : public virtual Rows_event {
 public:
  Update_rows_event(const char *buf, const Format_description_event *fde);
  Update_rows_event(Log_event_type event_type) : Rows_event(event_type) {}
};

/**
  @class Delete_rows_event

  Log row deletions. The event contain several delete rows for a
  table. Note that each event contains only rows for one table.

  RESPONSIBILITIES

    - Act as a container for rows that has been deleted on the master
      and should be deleted on the slave.

   @section Delete_rows_event_binary_format Binary Format
*/
class Delete_rows_event : public virtual Rows_event {
 public:
  Delete_rows_event(const char *buf, const Format_description_event *fde);
  Delete_rows_event() : Rows_event(DELETE_ROWS_EVENT) {}
};

/**
  @class Rows_query_event

  Rows query event type, which is a subclass
  of the Ignorable_event, to record the original query for the rows
  events in RBR. This event can be used to display the original query as
  comments by SHOW BINLOG EVENTS query, or mysqlbinlog client when the
  --verbose option is given twice

  @section Rows_query_var_event_binary_format Binary Format

  The Post-Header for this event type is empty. The Body has one
  component:

  <table>
  <caption>Body for Rows_query_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>m_rows_query</td>
    <td>char array</td>
    <td>Records the original query executed in RBR </td>
  </tr>
  </table>
*/
class Rows_query_event : public virtual Ignorable_event {
 public:
  /**
    It is used to write the original query in the binlog file in case of RBR
    when the session flag binlog_rows_query_log_events is set.

    <pre>
    The buffer layout is as follows:
    +------------------------------------+
    | The original query executed in RBR |
    +------------------------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */

  Rows_query_event(const char *buf, const Format_description_event *fde);
  /**
    It is the minimal constructor, and all it will do is set the type_code as
    ROWS_QUERY_LOG_EVENT in the header object in Binary_log_event.
  */
  Rows_query_event()
      : Ignorable_event(ROWS_QUERY_LOG_EVENT), m_rows_query(nullptr) {}

  virtual ~Rows_query_event();

 protected:
  char *m_rows_query;
};
}  // namespace binary_log

/**
  @} (end of group Replication)
*/
#endif /* ROWS_EVENT_INCLUDED */
