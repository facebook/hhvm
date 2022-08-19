#ifndef HISTOGRAMS_HISTOGRAM_INCLUDED
#define HISTOGRAMS_HISTOGRAM_INCLUDED

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

/**
  @file sql/histograms/histogram.h
  Histogram base class.

  This file defines the base class for all histogram types. We keep the base
  class itself non-templatized in order to more easily send a histogram as an
  argument, collect multiple histograms in a single collection etc.

  A histogram is stored as a JSON object. This gives the flexibility of storing
  virtually an unlimited number of buckets, data values in its full length and
  easily expanding with new histogram types in the future. They are stored
  persistently in the system table mysql.column_stats.

  We keep all histogram code in the namespace "histograms" in order to avoid
  name conflicts etc.
*/

#include <cstddef>  // size_t
#include <functional>
#include <map>      // std::map
#include <set>      // std::set
#include <string>   // std::string
#include <utility>  // std::pair

#include "lex_string.h"  // LEX_CSTRING
#include "my_base.h"     // ha_rows
#include "sql/histograms/value_map_type.h"
#include "sql/mem_root_allocator.h"   // Mem_root_allocator
#include "sql/stateless_allocator.h"  // Stateless_allocator

class Item;
class Json_dom;
class Json_object;
class THD;
struct TYPELIB;

namespace dd {
class Table;
}  // namespace dd
namespace histograms {
struct Histogram_comparator;
template <class T>
class Value_map;
}  // namespace histograms
struct CHARSET_INFO;
struct MEM_ROOT;
struct TABLE_LIST;

namespace histograms {

/// The default (and invalid) value for "m_null_values_fraction".
static const double INVALID_NULL_VALUES_FRACTION = -1.0;

enum class Message {
  FIELD_NOT_FOUND,
  UNSUPPORTED_DATA_TYPE,
  TEMPORARY_TABLE,
  ENCRYPTED_TABLE,
  VIEW,
  HISTOGRAM_CREATED,
  MULTIPLE_TABLES_SPECIFIED,
  COVERED_BY_SINGLE_PART_UNIQUE_INDEX,
  NO_HISTOGRAM_FOUND,
  HISTOGRAM_DELETED,
  SERVER_READ_ONLY
};

struct Histogram_psi_key_alloc {
  void *operator()(size_t s) const;
};

template <class T>
using Histogram_key_allocator = Stateless_allocator<T, Histogram_psi_key_alloc>;

template <class T>
using value_map_allocator = Mem_root_allocator<std::pair<const T, ha_rows>>;

template <typename T>
using value_map_type =
    std::map<T, ha_rows, Histogram_comparator, value_map_allocator<T>>;

using columns_set = std::set<std::string, std::less<std::string>,
                             Histogram_key_allocator<std::string>>;

using results_map =
    std::map<std::string, Message, std::less<std::string>,
             Histogram_key_allocator<std::pair<const std::string, Message>>>;

/**
  The different operators we can ask histogram statistics for selectivity
  estimations.
*/
enum class enum_operator {
  EQUALS_TO,
  GREATER_THAN,
  LESS_THAN,
  IS_NULL,
  IS_NOT_NULL,
  LESS_THAN_OR_EQUAL,
  GREATER_THAN_OR_EQUAL,
  NOT_EQUALS_TO,
  BETWEEN,
  NOT_BETWEEN,
  IN_LIST,
  NOT_IN_LIST
};

/**
  Histogram base class.
*/
class Histogram {
 public:
  /// All supported histogram types in MySQL.
  enum class enum_histogram_type { EQUI_HEIGHT, SINGLETON };

  /// String representation of the JSON field "histogram-type".
  static constexpr const char *histogram_type_str() { return "histogram-type"; }

  /// String representation of the JSON field "data-type".
  static constexpr const char *data_type_str() { return "data-type"; }

  /// String representation of the JSON field "collation-id".
  static constexpr const char *collation_id_str() { return "collation-id"; }

  /// String representation of the histogram type SINGLETON.
  static constexpr const char *singleton_str() { return "singleton"; }

  /// String representation of the histogram type EQUI-HEIGHT.
  static constexpr const char *equi_height_str() { return "equi-height"; }

 protected:
  double m_sampling_rate;

  /// The fraction of NULL values in the histogram (between 0.0 and 1.0).
  double m_null_values_fraction;

  /// The character set for the data stored
  const CHARSET_INFO *m_charset;

  /// The number of buckets originally specified
  size_t m_num_buckets_specified;

  /// String representation of the JSON field "buckets".
  static constexpr const char *buckets_str() { return "buckets"; }

  /// String representation of the JSON field "last-updated".
  static constexpr const char *last_updated_str() { return "last-updated"; }

  /// String representation of the JSON field "null-values".
  static constexpr const char *null_values_str() { return "null-values"; }

  static constexpr const char *sampling_rate_str() { return "sampling-rate"; }

  /// String representation of the JSON field "number-of-buckets-specified".
  static constexpr const char *numer_of_buckets_specified_str() {
    return "number-of-buckets-specified";
  }

  /**
    Write the data type of this histogram into a JSON object.

    @param json_object the JSON object where we will write the histogram
                       data type

    @return true on error, false otherwise
  */
  bool histogram_data_type_to_json(Json_object *json_object) const;

  /**
    Return the value that is contained in the JSON DOM object.

    For most types, this function simply returns the contained value. For String
    values, the value is allocated on this histograms MEM_ROOT before it is
    returned. This allows the String value to survive the entire lifetime of the
    histogram object.

    @param json_dom the JSON DOM object to extract the value from
    @param out the value from the JSON DOM object

    @return true on error, false otherwise
  */
  template <class T>
  bool extract_json_dom_value(const Json_dom *json_dom, T *out);

  /**
    Populate the histogram with data from the provided JSON object. The base
    class also provides an implementation that subclasses must call in order
    to populate fields that are shared among all histogram types (character set,
    null values fraction).

    @param json_object the JSON object to read the histogram data from

    @return true on error, false otherwise
  */
  virtual bool json_to_histogram(const Json_object &json_object) = 0;

 private:
  /// The MEM_ROOT where the histogram contents will be allocated.
  MEM_ROOT *m_mem_root;

  /// The type of this histogram.
  const enum_histogram_type m_hist_type;

  /// The type of the data this histogram contains.
  const Value_map_type m_data_type;

  /// Name of the database this histogram represents.
  LEX_CSTRING m_database_name;

  /// Name of the table this histogram represents.
  LEX_CSTRING m_table_name;

  /// Name of the column this histogram represents.
  LEX_CSTRING m_column_name;

  /**
    An internal function for getting the selecitvity estimation.

    This function will read/evaluate the value from the given Item, and pass
    this value on to the correct selectivity estimation function based on the
    data type of the histogram. For instance, if the data type of the histogram
    is INT, we will call "val_int" on the Item to evaulate the value as an
    integer and pass this value on to the next function.

    @param item The Item to read/evaluate the value from.
    @param op The operator we are estimating the selectivity for.
    @param typelib In the case of ENUM or SET data type, this parameter holds
                   the type information. This is needed in order to map a
                   string representation of an ENUM/SET value into its correct
                   integer representation (ENUM/SET values are stored as
                   integer values in the histogram).
    @param[out] selectivity The estimated selectivity, between 0.0 and 1.0
                inclusive.

    @return true on error (i.e the provided item was NULL), false on success.
  */
  bool get_selectivity_dispatcher(Item *item, const enum_operator op,
                                  const TYPELIB *typelib,
                                  double *selectivity) const;

  /**
    An internal function for getting the selecitvity estimation.

    This function will cast the histogram to the correct class (using down_cast)
    and pass the given value on to the correct selectivity estimation function
    for that class.

    @param value The value to estimate the selectivity for.

    @return The estimated selectivity, between 0.0 and 1.0 inclusive.
  */
  template <class T>
  double get_less_than_selectivity_dispatcher(const T &value) const;

  /// @see get_less_than_selectivity_dispatcher
  template <class T>
  double get_greater_than_selectivity_dispatcher(const T &value) const;

  /// @see get_less_than_selectivity_dispatcher
  template <class T>
  double get_equal_to_selectivity_dispatcher(const T &value) const;

  /**
    An internal function for applying the correct function for the given
    operator.

    @param op    The operator to apply
    @param value The value to find the selectivity for.

    @return The estimated selectivity, between 0.0 and 1.0 inclusive.
  */
  template <class T>
  double apply_operator(const enum_operator op, const T &value) const;

 public:
  /**
    Constructor.

    @param mem_root  the mem_root where the histogram contents will be allocated
    @param db_name   name of the database this histogram represents
    @param tbl_name  name of the table this histogram represents
    @param col_name  name of the column this histogram represents
    @param type      the histogram type (equi-height, singleton)
    @param data_type the type of data that this histogram contains
  */
  Histogram(MEM_ROOT *mem_root, const std::string &db_name,
            const std::string &tbl_name, const std::string &col_name,
            enum_histogram_type type, Value_map_type data_type);

  /**
    Copy constructor

    This will make a copy of the provided histogram onto the provided MEM_ROOT.
  */
  Histogram(MEM_ROOT *mem_root, const Histogram &other);

  Histogram(const Histogram &other) = delete;

  /// Destructor.
  virtual ~Histogram() {}

  /// @return the MEM_ROOT that this histogram uses for allocations
  MEM_ROOT *get_mem_root() const { return m_mem_root; }

  /**
    @return name of the database this histogram represents
  */
  const LEX_CSTRING get_database_name() const { return m_database_name; }

  /**
    @return name of the table this histogram represents
  */
  const LEX_CSTRING get_table_name() const { return m_table_name; }

  /**
    @return name of the column this histogram represents
  */
  const LEX_CSTRING get_column_name() const { return m_column_name; }

  /**
    @return type of this histogram
  */
  enum_histogram_type get_histogram_type() const { return m_hist_type; }

  /**
    @return the fraction of NULL values, in the range [0.0, 1.0]
  */
  double get_null_values_fraction() const;

  /// @return the character set for the data this histogram contains
  const CHARSET_INFO *get_character_set() const { return m_charset; }

  /// @return the sampling rate used to generate this histogram
  double get_sampling_rate() const { return m_sampling_rate; }

  /**
    Returns the histogram type as a readable string.

    @return a readable string representation of the histogram type
  */
  virtual std::string histogram_type_to_str() const = 0;

  /**
    @return number of buckets in this histogram
  */
  virtual size_t get_num_buckets() const = 0;

  /**
    Get the estimated number of distinct non-NULL values.
    @return number of distinct non-NULL values
  */
  virtual size_t get_num_distinct_values() const = 0;

  /**
    @return the data type that this histogram contains
  */
  Value_map_type get_data_type() const { return m_data_type; }

  /**
    @return number of buckets originally specified by the user. This may be
            higher than the actual number of buckets in the histogram.
  */
  size_t get_num_buckets_specified() const { return m_num_buckets_specified; }

  /**
    Converts the histogram to a JSON object.

    @param[in,out] json_object output where the histogram is to be stored. The
                   caller is responsible for allocating/deallocating the JSON
                   object

    @return     true on error, false otherwise
  */
  virtual bool histogram_to_json(Json_object *json_object) const = 0;

  /**
    Converts JSON object to a histogram.

    @param  mem_root    MEM_ROOT where the histogram will be allocated
    @param  schema_name the schema name
    @param  table_name  the table name
    @param  column_name the column name
    @param  json_object output where the histogram is stored

    @return nullptr on error. Otherwise a histogram allocated on the provided
            MEM_ROOT.
  */
  static Histogram *json_to_histogram(MEM_ROOT *mem_root,
                                      const std::string &schema_name,
                                      const std::string &table_name,
                                      const std::string &column_name,
                                      const Json_object &json_object);

  /**
    Make a clone of the current histogram

    @param mem_root the MEM_ROOT on which the new histogram will be allocated.

    @return a histogram allocated on the provided MEM_ROOT. Returns nullptr
            on error.
  */
  virtual Histogram *clone(MEM_ROOT *mem_root) const = 0;

  /**
    Store this histogram to persistent storage (data dictionary).

    @param thd Thread handler.

    @return false on success, true on error.
  */
  bool store_histogram(THD *thd) const;

  /**
    Get selectivity estimation.

    This function will try and get the selectivity estimation for a predicate
    on the form "COLUMN OPERATOR CONSTANT", for instance "SELECT * FROM t1
    WHERE col1 > 23;".

    This function will take care of several of things, for instance checking
    that the value we are estimating the selectivity for is a constant value.

    The order of the Items provided does not matter. For instance, of the
    operator argument given is "EQUALS_TO", it does not matter if the constant
    value is provided as the first or the second argument; this function will
    take care of this.

    @param items            an array of items that contains both the field we
                            are estimating the selectivity for, as well as the
                            user-provided constant values.
    @param item_count       the number of Items in the Item array.
    @param op               the predicate operator
    @param[out] selectivity the calculated selectivity if a usable histogram was
                            found

    @retval true if an error occurred (the Item provided was not a constant
    value or similar).
    @return false if success
  */
  bool get_selectivity(Item **items, size_t item_count, enum_operator op,
                       double *selectivity) const;

  /**
    @return the fraction of non-null values in the histogram.
  */
  double get_non_null_values_frequency() const {
    return 1.0 - get_null_values_fraction();
  }
};

/**
  Create a histogram from a value map.

  This function will build a histogram from a value map. The histogram type
  depends on both the size of the input data, as well as the number of buckets
  specified. If the number of distinct values is less than or equal to the
  number of buckets, a Singleton histogram will be created. Otherwise, an
  equi-height histogram will be created.

  The histogram will be allocated on the supplied mem_root, and it is the
  callers responsibility to properly clean up when the histogram isn't needed
  anymore.

  @param   mem_root        the MEM_ROOT where the histogram contents will be
                           allocated
  @param   value_map       a value map containing [value, frequency]
  @param   num_buckets     the maximum number of buckets to create
  @param   db_name         name of the database this histogram represents
  @param   tbl_name        name of the table this histogram represents
  @param   col_name        name of the column this histogram represents

  @return  a histogram, using at most "num_buckets" buckets. The histogram
           type depends on the size of the input data, and the number of
           buckets
*/
template <class T>
Histogram *build_histogram(MEM_ROOT *mem_root, const Value_map<T> &value_map,
                           size_t num_buckets, const std::string &db_name,
                           const std::string &tbl_name,
                           const std::string &col_name);

/**
  Create or update histograms for a set of columns of a given table.

  This function will try to create histogram statistics for all the columns
  specified. If one of the columns fail, it will continue to the next one and
  try.

  @param thd Thread handler.
  @param table The table where we should look for the columns/data.
  @param columns Columns specified by the user.
  @param num_buckets The maximum number of buckets to create in each
         histogram.
  @param results A map where the result of each operation is stored.

  @return false on success, true on error.
*/
bool update_histogram(THD *thd, TABLE_LIST *table, const columns_set &columns,
                      int num_buckets, results_map &results);

/**
  Drop histograms for all columns in a given table.

  @param thd Thread handler.
  @param table The table where we should look for the columns.
  @param original_table_def Original table definition.
  @param results A map where the result of each operation is stored.

  @return false on success, true on error.
*/
bool drop_all_histograms(THD *thd, const TABLE_LIST &table,
                         const dd::Table &original_table_def,
                         results_map &results);

/**
  Drop histograms for a set of columns in a given table.

  This function will try to drop the histogram statistics for all specified
  columns. If one of the columns fail, it will continue to the next one and try.

  @param thd Thread handler.
  @param table The table where we should look for the columns.
  @param columns Columns specified by the user.
  @param results A map where the result of each operation is stored.

  @return false on success, true on error.
*/
bool drop_histograms(THD *thd, const TABLE_LIST &table,
                     const columns_set &columns, results_map &results);

/**
  Rename histograms for all columns in a given table.

  @param thd             Thread handler.
  @param old_schema_name The old schema name
  @param old_table_name  The old table name
  @param new_schema_name The new schema name
  @param new_table_name  The new table name
  @param results         A map where the result of each operation is stored.

  @return false on success, true on error.
*/
bool rename_histograms(THD *thd, const char *old_schema_name,
                       const char *old_table_name, const char *new_schema_name,
                       const char *new_table_name, results_map &results);

bool find_histogram(THD *thd, const std::string &schema_name,
                    const std::string &table_name,
                    const std::string &column_name,
                    const Histogram **histogram);
}  // namespace histograms

#endif
