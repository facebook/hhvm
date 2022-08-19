/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_PLUGIN_TABLE_SERVICE_H
#define PFS_PLUGIN_TABLE_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/service_implementation.h>

/**
  @page EXAMPLE_PLUGIN_COMPONENT Example plugin/component to use this service.

  Any plugin/component, which exposes tables in performance schema,
  has to provide an implementation of interface PFS_engine_table_proxy.

  As there is no storage engine here to handle table data, plugin/component has
  to:
  - maintain storage for table being exposed,
  - take care of handling any duplicate check (Primary/Unique Key, etc.)

  The following table describes datatypes exposed to plugin/component
  which should be used to implement columns.

  COLUMN TYPE | TO BE USED   | NULL VALUE INDICATION
  ----------- | ------------ | ---------------------
   INTEGER    | PSI_int      |     is_null=true
   TINYINT    | PSI_tinyint  |        -do-
   SMALLINT   | PSI_smallint |        -do-
   BIGINT     | PSI_bigint   |        -do-
   MEDIUMINT  | PSI_mediumint|        -do-
   DECIMAL    | PSI_decimal  |        -do-
   FLOAT      | PSI_float    |        -do-
   DOUBLE     | PSI_double   |        -do-
   ENUM       | PSI_enum     |        -do-
   YEAR       | PSI_year     |        -do-
   DATE       | char array   |      length=0
   TIME       | char array   |        -do-
   DATETIME   | char array   |        -do-
   TIMESTAMP  | char array   |        -do-
   CHAR       | char array   |        -do-
   VARCHAR    | char array   |        -do-
   BLOB       | char array   |        -do-

   @section STEPS How to write a plugin/component exposing tables in Performance
  Schema

   Following are the example implementations of a plugin and a component which
   uses this pfs_plugin_table service.

   @subpage EXAMPLE_PLUGIN

   @subpage EXAMPLE_COMPONENT
*/

/* Define ERRORS */
#define PFS_HA_ERR_WRONG_COMMAND 131
#define PFS_HA_ERR_RECORD_DELETED 134
#define PFS_HA_ERR_END_OF_FILE 137
#define PFS_HA_ERR_NO_REFERENCED_ROW 151
#define PFS_HA_ERR_FOUND_DUPP_KEY 121
#define PFS_HA_ERR_RECORD_FILE_FULL 135

/* Helper macro */
struct PFS_string {
  char *str;
  unsigned int length;
};
typedef struct PFS_string PFS_string;

/**
  This is an opaque structure to denote filed in plugin/component code.
*/
typedef struct PSI_field PSI_field;
/**
  This is an opaque structure to denote table handle in plugin/component code.
*/
typedef struct PSI_table_handle PSI_table_handle;
/**
  This is an opaque structure to denote cursor position in plugin/component
  code.
*/
typedef struct PSI_pos PSI_pos;
/**
  This is an opaque structure to denote Key Reader in plugin/component code.
*/
typedef struct PSI_key_reader PSI_key_reader;
/**
  This is an opaque structure to denote Index Handle in plugin/component code.
*/
typedef struct PSI_index_handle PSI_index_handle;

struct PSI_long {
  long val;     /* Column value */
  bool is_null; /* If Column value is NULL */
};
typedef struct PSI_long PSI_long;

struct PSI_ulong {
  unsigned long val; /* Column value */
  bool is_null;      /* If Column value is NULL */
};
typedef struct PSI_ulong PSI_ulong;

struct PSI_longlong {
  long long val; /* Column value */
  bool is_null /* If Column value is NULL */;
};
typedef struct PSI_longlong PSI_longlong;

struct PSI_ulonglong {
  unsigned long long val; /* Column value */
  bool is_null /* If Column value is NULL */;
};
typedef struct PSI_ulonglong PSI_ulonglong;

struct PSI_double {
  double val; /* Column value */
  bool is_null /* If Column value is NULL */;
};
typedef struct PSI_double PSI_double;

#define PSI_tinyint PSI_long
#define PSI_utinyint PSI_ulong
#define PSI_smallint PSI_long
#define PSI_usmallint PSI_ulong
#define PSI_mediumint PSI_long
#define PSI_umediumint PSI_ulong
#define PSI_int PSI_long
#define PSI_uint PSI_ulong
#define PSI_bigint PSI_longlong
#define PSI_ubigint PSI_ulonglong
#define PSI_year PSI_ulong
#define PSI_enum PSI_ulonglong
#define PSI_decimal PSI_double
#define PSI_float PSI_double

/**
  A structure to denote a key of type long in an index.
*/
struct PSI_plugin_key_integer {
  /* name of the key column */
  const char *m_name;
  /* find flags */
  int m_find_flags;
  /* is column NULL */
  bool m_is_null;
  /* value of the key column */
  long m_value;
};
typedef struct PSI_plugin_key_integer PSI_plugin_key_integer;
typedef PSI_plugin_key_integer PSI_plugin_key_tinyint;
typedef PSI_plugin_key_integer PSI_plugin_key_smallint;
typedef PSI_plugin_key_integer PSI_plugin_key_mediumint;

/**
  A structure to denote a key of type ulong in an index.
*/
struct PSI_plugin_key_uinteger {
  /** Name of the key column */
  const char *m_name;
  /** Find flags */
  int m_find_flags;
  /** Column is NULL */
  bool m_is_null;
  /** Value of the key column */
  unsigned long m_value;
};
typedef struct PSI_plugin_key_uinteger PSI_plugin_key_uinteger;
typedef PSI_plugin_key_uinteger PSI_plugin_key_utinyint;
typedef PSI_plugin_key_uinteger PSI_plugin_key_usmallint;
typedef PSI_plugin_key_uinteger PSI_plugin_key_umediumint;

/**
  A structure to denote a key of type long long in an index.
*/
struct PSI_plugin_key_bigint {
  /** Name of the key column */
  const char *m_name;
  /** Find flags */
  int m_find_flags;
  /** Column is NULL */
  bool m_is_null;
  /** Value of the key column */
  long long m_value;
};
typedef struct PSI_plugin_key_bigint PSI_plugin_key_bigint;

/**
  A structure to denote a key of type unsigned long long in an index.
*/
struct PSI_plugin_key_ubigint {
  /** Name of the key column */
  const char *m_name;
  /** Find flags */
  int m_find_flags;
  /** Column is NULL */
  bool m_is_null;
  /** Value of the key column */
  unsigned long long m_value;
};
typedef struct PSI_plugin_key_ubigint PSI_plugin_key_ubigint;

/**
  A structure to denote a key of type string in an index.
*/
struct PSI_plugin_key_string {
  /* name of the key column */
  const char *m_name;
  /* find flags */
  int m_find_flags;
  /* is column null */
  bool m_is_null;
  /* buffer to store key column value */
  char *m_value_buffer;
  // FIXME: size_t
  /* length of the key value in buffer */
  unsigned int m_value_buffer_length;
  /* Maximum size of buffer */
  unsigned int m_value_buffer_capacity;
};
typedef struct PSI_plugin_key_string PSI_plugin_key_string;

/**
  Api to read the next record.
  @param handle Table handle.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*rnd_next_t)(PSI_table_handle *handle);

/**
  API to initialize for random scan or read.
  @param handle Table handle.
  @param scan True, if its a random scan.
              False, if its a random read.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*rnd_init_t)(PSI_table_handle *handle, bool scan);

/**
  API to read row from a position which is set in table handle.
  @param handle Table handle.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*rnd_pos_t)(PSI_table_handle *handle);

/**
  API to initialize index(es).
  @param handle Table handle.
  @param idx    Index of the index to be initialized (in case of multiple
                indexes on table)
  @param sorted True if he index is sorted (typically a BTREE), false if not
  sorted (typically a HASH)
  @param index  Initialized index handle.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*index_init_t)(PSI_table_handle *handle, unsigned int idx,
                            bool sorted, PSI_index_handle **index);
/**
  API to read keys in index.
  @param index      Index handle.
  @param reader     Key reader.
  @param idx        Index of the index to be read.
  @param find_flag  find flag.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*index_read_t)(PSI_index_handle *index, PSI_key_reader *reader,
                            unsigned int idx, int find_flag);

/**
  API to read next record with matching index.
  @param handle Table handle.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*index_next_t)(PSI_table_handle *handle);

/**
  API to reset cursor position
  @param handle Table handle.
*/
typedef void (*reset_position_t)(PSI_table_handle *handle);

/**
  API to read a column value from table.
  @param handle Table handle.
  @param field  Field for which value is to be read.
  @param index  Index of field in table column.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*read_column_value_t)(PSI_table_handle *handle, PSI_field *field,
                                   unsigned int index);

/**
  API to write a column value in table.
  @param handle Table handle.
  @param field  Field for which value is to be written.
  @param index  Index of field in table column.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*write_column_value_t)(PSI_table_handle *handle, PSI_field *field,
                                    unsigned int index);
/**
  API to write a record in table.
  @param handle Table handle having new record to be written.
*/
typedef int (*write_row_values_t)(PSI_table_handle *handle);

/**
  API to update a column value in table.
  @param handle Table handle.
  @param field  Field for which value is to be updated.
  @param index  Index of field in table column.

  @return Operation status
  @retval 0    Success
  @retval !=0  Error
*/
typedef int (*update_column_value_t)(PSI_table_handle *handle, PSI_field *field,
                                     unsigned int index);
/**
  API to write a record in table.
  @param handle Table handle having updated record to be updated.
*/
typedef int (*update_row_values_t)(PSI_table_handle *handle);

/**
  API to delete record from table.
  @param handle Table handle having record to be deleted.
*/
typedef int (*delete_row_values_t)(PSI_table_handle *handle);

/**
  API to Open a table handle in plugin/component code and reset position
  pointer when a new table handle in opened in Performance Schema.
  @param pos pos pointer to be updated.

  @return initialized table handle.
*/
typedef PSI_table_handle *(*open_table_t)(PSI_pos **pos);

/**
  API to Close a table handle in plugin/component code and reset position
  pointer when a table handle in closed in Performance Schema.
  @param handle table handle
*/
typedef void (*close_table_t)(PSI_table_handle *handle);

/**
 A structure to keep callback functions to be implemented by
 plugin/component.
*/
struct PFS_engine_table_proxy {
  rnd_next_t rnd_next;
  rnd_init_t rnd_init;
  rnd_pos_t rnd_pos;
  index_init_t index_init;
  index_read_t index_read;
  index_next_t index_next;
  read_column_value_t read_column_value;
  reset_position_t reset_position;
  write_column_value_t write_column_value;
  write_row_values_t write_row_values;
  update_column_value_t update_column_value;
  update_row_values_t update_row_values;
  delete_row_values_t delete_row_values;
  open_table_t open_table;
  close_table_t close_table;
  PFS_engine_table_proxy() = default;
};
typedef struct PFS_engine_table_proxy PFS_engine_table_proxy;

/**
  Types of access allowed to tables.
*/
enum Access_control {
  /* Only Read is allowed */
  READONLY = 0,
  /* Read/Truncate allowed but no Update/Insert/Delete. */
  TRUNCATABLE,
  /* Read/Truncate/Update allowed but no Insert/Delete. */
  UPDATABLE,
  /* Read/Truncate/Insert/UPDATE/Delete allowed. */
  EDITABLE
};

/**
  API to delete/truncate all the rows in a table
*/
typedef int (*delete_all_rows_t)(void);

/**
  API to give number of rows in a table

  @return number of rows.
*/
typedef unsigned long long (*get_row_count_t)(void);

/**
  A share to be initialized by plugin/component code and to be provided
  to add_table() service method of pfs_plugin_table service.
 */
struct PFS_engine_table_share_proxy {
 public:
  /* Callback functions list of APIs */
  PFS_engine_table_proxy m_proxy_engine_table;

  /* Name of the table to be added */
  const char *m_table_name;
  /* Length of the table name */
  unsigned int m_table_name_length;

  /* Table Columns definition */
  const char *m_table_definition;
  unsigned int m_ref_length;

  /* Access allowed on the table */
  enum Access_control m_acl;

  delete_all_rows_t delete_all_rows;
  get_row_count_t get_row_count;
};
typedef struct PFS_engine_table_share_proxy PFS_engine_table_share_proxy;

/**
  Definition of pfs_plugin_table service and its methods.
  @deprecated
  This service is functional but incomplete,
  as many apis are missing.
  Please use pfs_plugin_table_v1
  and pfs_plugin_column_*_v1 instead.
*/
BEGIN_SERVICE_DEFINITION(pfs_plugin_table)
/* Methods to add tables in Performance Schema */
DECLARE_METHOD(int, add_tables,
               (PFS_engine_table_share_proxy * *st_share,
                unsigned int share_count));

/* Methods to delete tables in Performance Schema */
DECLARE_METHOD(int, delete_tables,
               (PFS_engine_table_share_proxy * *st_share,
                unsigned int share_count));

/* TINYINT */
DECLARE_METHOD(void, set_field_tinyint, (PSI_field * f, PSI_tinyint value));
DECLARE_METHOD(void, set_field_utinyint, (PSI_field * f, PSI_utinyint value));
DECLARE_METHOD(void, get_field_tinyint, (PSI_field * f, PSI_tinyint *value));

/* SMALLINT */
DECLARE_METHOD(void, set_field_smallint, (PSI_field * f, PSI_smallint value));
DECLARE_METHOD(void, set_field_usmallint, (PSI_field * f, PSI_usmallint value));
DECLARE_METHOD(void, get_field_smallint, (PSI_field * f, PSI_smallint *value));

/* MEDIUMINT */
DECLARE_METHOD(void, set_field_mediumint, (PSI_field * f, PSI_mediumint value));
DECLARE_METHOD(void, set_field_umediumint,
               (PSI_field * f, PSI_umediumint value));
DECLARE_METHOD(void, get_field_mediumint,
               (PSI_field * f, PSI_mediumint *value));

/* INTEGER(INT) */
DECLARE_METHOD(void, set_field_integer, (PSI_field * f, PSI_int value));
DECLARE_METHOD(void, set_field_uinteger, (PSI_field * f, PSI_uint value));
DECLARE_METHOD(void, get_field_integer, (PSI_field * f, PSI_int *value));
DECLARE_METHOD(void, read_key_integer,
               (PSI_key_reader * reader, PSI_plugin_key_integer *key,
                int find_flag));
DECLARE_METHOD(bool, match_key_integer,
               (bool record_null, long record_value,
                PSI_plugin_key_integer *key));

/* BIGINT */
DECLARE_METHOD(void, set_field_bigint, (PSI_field * f, PSI_bigint value));
DECLARE_METHOD(void, set_field_ubigint, (PSI_field * f, PSI_ubigint value));
DECLARE_METHOD(void, get_field_bigint, (PSI_field * f, PSI_bigint *value));

/* DECIMAL */
DECLARE_METHOD(void, set_field_decimal, (PSI_field * f, PSI_double value));
DECLARE_METHOD(void, get_field_decimal, (PSI_field * f, PSI_double *value));

/* FLOAT */
DECLARE_METHOD(void, set_field_float, (PSI_field * f, PSI_double value));
DECLARE_METHOD(void, get_field_float, (PSI_field * f, PSI_double *value));

/* DOUBLE */
DECLARE_METHOD(void, set_field_double, (PSI_field * f, PSI_double value));
DECLARE_METHOD(void, get_field_double, (PSI_field * f, PSI_double *value));

/* CHAR */
DECLARE_METHOD(void, set_field_char_utf8,
               (PSI_field * f, const char *value, unsigned int length));
DECLARE_METHOD(void, get_field_char_utf8,
               (PSI_field * f, char *str, unsigned int *length));
DECLARE_METHOD(void, read_key_string,
               (PSI_key_reader * reader, PSI_plugin_key_string *key,
                int find_flag));
DECLARE_METHOD(bool, match_key_string,
               (bool record_null, const char *record_string_value,
                unsigned int record_string_length, PSI_plugin_key_string *key));

/* VARCHAR */
DECLARE_METHOD(void, set_field_varchar_utf8, (PSI_field * f, const char *str));
DECLARE_METHOD(void, set_field_varchar_utf8_len,
               (PSI_field * f, const char *str, unsigned int len));
DECLARE_METHOD(void, get_field_varchar_utf8,
               (PSI_field * f, char *str, unsigned int *length));

DECLARE_METHOD(void, set_field_varchar_utf8mb4,
               (PSI_field * f, const char *str));
DECLARE_METHOD(void, set_field_varchar_utf8mb4_len,
               (PSI_field * f, const char *str, unsigned int len));

/* BLOB/TEXT */
DECLARE_METHOD(void, set_field_blob,
               (PSI_field * f, const char *val, unsigned int len));
DECLARE_METHOD(void, get_field_blob,
               (PSI_field * f, char *val, unsigned int *len));

/* ENUM */
DECLARE_METHOD(void, set_field_enum, (PSI_field * f, PSI_enum value));
DECLARE_METHOD(void, get_field_enum, (PSI_field * f, PSI_enum *value));

/* DATE */
DECLARE_METHOD(void, set_field_date,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get_field_date,
               (PSI_field * f, char *val, unsigned int *len));

/* TIME */
DECLARE_METHOD(void, set_field_time,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get_field_time,
               (PSI_field * f, char *val, unsigned int *len));

/* DATETIME */
DECLARE_METHOD(void, set_field_datetime,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get_field_datetime,
               (PSI_field * f, char *val, unsigned int *len));

/* TIMESTAMP */
DECLARE_METHOD(void, set_field_timestamp,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get_field_timestamp,
               (PSI_field * f, char *val, unsigned int *len));

/* YEAR */
DECLARE_METHOD(void, set_field_year, (PSI_field * f, PSI_year value));
DECLARE_METHOD(void, get_field_year, (PSI_field * f, PSI_year *value));

/* NULL */
DECLARE_METHOD(void, set_field_null, (PSI_field * f));

END_SERVICE_DEFINITION(pfs_plugin_table)

/**
  Definition of pfs_plugin_table_v1 service and its methods.
*/
BEGIN_SERVICE_DEFINITION(pfs_plugin_table_v1)
DECLARE_METHOD(int, add_tables,
               (PFS_engine_table_share_proxy * *st_share,
                unsigned int share_count));

DECLARE_METHOD(int, delete_tables,
               (PFS_engine_table_share_proxy * *st_share,
                unsigned int share_count));
DECLARE_METHOD(unsigned int, get_parts_found, (PSI_key_reader * reader));
END_SERVICE_DEFINITION(pfs_plugin_table_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_tiny_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_tinyint value));
DECLARE_METHOD(void, set_unsigned, (PSI_field * f, PSI_utinyint value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_tinyint *value));
DECLARE_METHOD(void, get_unsigned, (PSI_field * f, PSI_utinyint *value));
DECLARE_METHOD(void, read_key,
               (PSI_key_reader * reader, PSI_plugin_key_tinyint *key,
                int find_flag));
DECLARE_METHOD(void, read_key_unsigned,
               (PSI_key_reader * reader, PSI_plugin_key_utinyint *key,
                int find_flag));
DECLARE_METHOD(bool, match_key,
               (bool record_null, long record_value,
                PSI_plugin_key_tinyint *key));
DECLARE_METHOD(bool, match_key_unsigned,
               (bool record_null, unsigned long record_value,
                PSI_plugin_key_utinyint *key));

END_SERVICE_DEFINITION(pfs_plugin_column_tiny_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_small_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_smallint value));
DECLARE_METHOD(void, set_unsigned, (PSI_field * f, PSI_usmallint value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_smallint *value));
DECLARE_METHOD(void, get_unsigned, (PSI_field * f, PSI_usmallint *value));
DECLARE_METHOD(void, read_key,
               (PSI_key_reader * reader, PSI_plugin_key_smallint *key,
                int find_flag));
DECLARE_METHOD(void, read_key_unsigned,
               (PSI_key_reader * reader, PSI_plugin_key_usmallint *key,
                int find_flag));
DECLARE_METHOD(bool, match_key,
               (bool record_null, long record_value,
                PSI_plugin_key_smallint *key));
DECLARE_METHOD(bool, match_key_unsigned,
               (bool record_null, unsigned long record_value,
                PSI_plugin_key_usmallint *key));
END_SERVICE_DEFINITION(pfs_plugin_column_small_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_medium_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_mediumint value));
DECLARE_METHOD(void, set_unsigned, (PSI_field * f, PSI_umediumint value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_mediumint *value));
DECLARE_METHOD(void, get_unsigned, (PSI_field * f, PSI_umediumint *value));
DECLARE_METHOD(void, read_key,
               (PSI_key_reader * reader, PSI_plugin_key_mediumint *key,
                int find_flag));
DECLARE_METHOD(void, read_key_unsigned,
               (PSI_key_reader * reader, PSI_plugin_key_umediumint *key,
                int find_flag));
DECLARE_METHOD(bool, match_key,
               (bool record_null, long record_value,
                PSI_plugin_key_mediumint *key));
DECLARE_METHOD(bool, match_key_unsigned,
               (bool record_null, unsigned long record_value,
                PSI_plugin_key_umediumint *key));
END_SERVICE_DEFINITION(pfs_plugin_column_medium_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_integer_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_int value));
DECLARE_METHOD(void, set_unsigned, (PSI_field * f, PSI_uint value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_int *value));
DECLARE_METHOD(void, get_unsigned, (PSI_field * f, PSI_int *value));
DECLARE_METHOD(void, read_key,
               (PSI_key_reader * reader, PSI_plugin_key_integer *key,
                int find_flag));
DECLARE_METHOD(void, read_key_unsigned,
               (PSI_key_reader * reader, PSI_plugin_key_uinteger *key,
                int find_flag));
DECLARE_METHOD(bool, match_key,
               (bool record_null, long record_value,
                PSI_plugin_key_integer *key));
DECLARE_METHOD(bool, match_key_unsigned,
               (bool record_null, unsigned long record_value,
                PSI_plugin_key_uinteger *key));
END_SERVICE_DEFINITION(pfs_plugin_column_integer_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_bigint_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_bigint value));
DECLARE_METHOD(void, set_unsigned, (PSI_field * f, PSI_ubigint value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_bigint *value));
DECLARE_METHOD(void, get_unsigned, (PSI_field * f, PSI_ubigint *value));
DECLARE_METHOD(void, read_key,
               (PSI_key_reader * reader, PSI_plugin_key_bigint *key,
                int find_flag));
DECLARE_METHOD(void, read_key_unsigned,
               (PSI_key_reader * reader, PSI_plugin_key_ubigint *key,
                int find_flag));
DECLARE_METHOD(bool, match_key,
               (bool record_null, long long record_value,
                PSI_plugin_key_bigint *key));
DECLARE_METHOD(bool, match_key_unsigned,
               (bool record_null, unsigned long long record_value,
                PSI_plugin_key_ubigint *key));
END_SERVICE_DEFINITION(pfs_plugin_column_bigint_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_decimal_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_decimal value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_decimal *value));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_decimal_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_float_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_float value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_float *value));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_float_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_double_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_double value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_double *value));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_double_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_string_v1)
/* CHAR */
DECLARE_METHOD(void, set_char_utf8,
               (PSI_field * f, const char *value, unsigned int length));
DECLARE_METHOD(void, get_char_utf8,
               (PSI_field * f, char *str, unsigned int *length));
DECLARE_METHOD(void, read_key_string,
               (PSI_key_reader * reader, PSI_plugin_key_string *key,
                int find_flag));
DECLARE_METHOD(bool, match_key_string,
               (bool record_null, const char *record_string_value,
                unsigned int record_string_length, PSI_plugin_key_string *key));
/* VARCHAR */
DECLARE_METHOD(void, set_varchar_utf8, (PSI_field * f, const char *str));
DECLARE_METHOD(void, set_varchar_utf8_len,
               (PSI_field * f, const char *str, unsigned int len));
DECLARE_METHOD(void, get_varchar_utf8,
               (PSI_field * f, char *str, unsigned int *length));
DECLARE_METHOD(void, set_varchar_utf8mb4, (PSI_field * f, const char *str));
DECLARE_METHOD(void, set_varchar_utf8mb4_len,
               (PSI_field * f, const char *str, unsigned int len));
END_SERVICE_DEFINITION(pfs_plugin_column_string_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_blob_v1)
DECLARE_METHOD(void, set, (PSI_field * f, const char *val, unsigned int len));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_blob_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_enum_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_enum value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_enum *value));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_enum_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_date_v1)
DECLARE_METHOD(void, set,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_date_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_time_v1)
DECLARE_METHOD(void, set,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_time_v1)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_datetime_v1)
DECLARE_METHOD(void, set,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_datetime_v1)

/*
  SERVICE_DEFINITION(pfs_plugin_column_timestamp_v1)
  Introduced in MySQL 8.0.14
  Deprecated in MySQL 8.0.17
  Status: Deprecated, use version 2 instead.
*/
BEGIN_SERVICE_DEFINITION(pfs_plugin_column_timestamp_v1)
DECLARE_METHOD(void, set,
               (PSI_field * f, const char *str, unsigned int length));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_timestamp_v1)

/*
  SERVICE_DEFINITION(pfs_plugin_column_timestamp_v2)
  Introduced in MySQL 8.0.17
  Status: Active.
*/
BEGIN_SERVICE_DEFINITION(pfs_plugin_column_timestamp_v2)
DECLARE_METHOD(void, set,
               (PSI_field * f, const char *str, unsigned int length));
/* Set time stamp value in microseconds as returned by my_micro_time(). */
DECLARE_METHOD(void, set2, (PSI_field * f, unsigned long long val));
DECLARE_METHOD(void, get, (PSI_field * f, char *val, unsigned int *len));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_timestamp_v2)

BEGIN_SERVICE_DEFINITION(pfs_plugin_column_year_v1)
DECLARE_METHOD(void, set, (PSI_field * f, PSI_year value));
DECLARE_METHOD(void, get, (PSI_field * f, PSI_year *value));
/* No support for indexes. */
END_SERVICE_DEFINITION(pfs_plugin_column_year_v1)

#endif
