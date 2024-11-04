/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Classes representing rows and blocks of rows returned by a MySQL
// query.  Note a query can return multiple blocks.
//
// These classes make heavy use of StringPiece.  This means if you
// wish to store the data in the query, you must copy it out.  Once
// you lose your RowBlock, any Rows or StringPieces referencing it
// will be invalid.

#pragma once

#include <vector>

#include <boost/iterator/iterator_facade.hpp>
#include <glog/logging.h>
#include <chrono>

#include <re2/re2.h>

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/container/F14Map.h>
#include <folly/hash/Hash.h>
#include <folly/json/dynamic.h>

#include "squangle/mysql_client/InternalConnection.h"
#include "squangle/util/StorageRow.h"
#include "squangle/util/StringStore.h"

namespace facebook::common::mysql_client {

class RowBlock;

// A row of returned data.  This makes the columns available either
// positionally or by name, both via operator[].  In addition, the raw
// values are available via iteration.  A Row is only valid for as
// long as the RowBlock it belongs to is valid, so don't save these.
//
// Note that if multiple columns have the same name (as reported by
// the MySQL server when it returns rows), the column name access will
// return only one of them; to get all values, you should use the
// integer indexes in operator[] or iterate over the columns of the
// row directly.
class Row {
 public:
  Row(const RowBlock* row_block, size_t row_number);

  // L should be StringPiece, size_t, or convertible therefrom.  The
  // return value is converted with folly::to<T>.
  template <typename T, typename L>
  T get(const L& l) const;
  // Like above, but a default value is supplied.  If the column is null the
  // default value is returned
  template <typename T, typename L>
  T get(const L& l, T d) const;

  // Similar to above but will return as an optional which will be empty if the
  // column is null
  template <
      typename T,
      template <typename> class Optional = std::optional,
      typename L>
  Optional<T> getOptional(const L& l) const;

  folly::dynamic getDynamic(size_t l) const;
  folly::dynamic getDynamic(folly::StringPiece l) const;

  // Vector-like and map-like access.  Note the above about ambiguity
  // for map access when column names conflict.
  size_t size() const;
  folly::StringPiece operator[](size_t col) const;
  folly::StringPiece operator[](folly::StringPiece field) const;

  // Is the field nullable?
  bool isNull(size_t col) const;
  bool isNull(folly::StringPiece field) const;

  // Our very simple iterator.  Just barely enough to support
  // range-based for loops.
  class Iterator : public boost::iterator_facade<
                       Iterator,
                       const folly::StringPiece,
                       boost::single_pass_traversal_tag,
                       const folly::StringPiece> {
   public:
    Iterator(const Row* row, size_t column_number)
        : row_(row), current_column_number_(column_number) {}

    void increment() {
      ++current_column_number_;
    }
    const folly::StringPiece dereference() const {
      CHECK(current_column_number_ < row_->size());
      return row_->get<folly::StringPiece>(current_column_number_);
    }
    bool equal(const Iterator& other) const {
      return (
          row_ == other.row_ &&
          current_column_number_ == other.current_column_number_);
    }

   private:
    const Row* row_;
    size_t current_column_number_;
  };

  Iterator begin() const;
  Iterator end() const;

 private:
  const RowBlock* row_block_; // unowned
  const size_t row_number_;
};

// RowFields encapsulates the data about the fields (name, flags, types).
class RowFields {
 public:
  RowFields(
      folly::F14NodeMap<std::string, int>&& field_name_map,
      std::vector<std::string>&& field_names,
      std::vector<std::string>&& table_names,
      std::vector<uint64_t>&& mysql_field_flags,
      std::vector<enum_field_types>&& mysql_field_types)
      : num_fields_(field_names.size()),
        field_name_map_(std::move(field_name_map)),
        field_names_(std::move(field_names)),
        table_names_(std::move(table_names)),
        mysql_field_flags_(std::move(mysql_field_flags)),
        mysql_field_types_(std::move(mysql_field_types)) {}
  // Get the MySQL type of the field.
  enum_field_types getFieldType(size_t field_num) const {
    return mysql_field_types_[field_num];
  }

  // Ditto, but by name.
  enum_field_types getFieldType(folly::StringPiece field_name) const {
    return mysql_field_types_[fieldIndex(field_name)];
  }

  // Get the MySQL flags of the field.
  uint64_t getFieldFlags(size_t field_num) const {
    return mysql_field_flags_[field_num];
  }

  // Ditto, but by name.
  uint64_t getFieldFlags(folly::StringPiece field_name) const {
    return mysql_field_flags_[fieldIndex(field_name)];
  }

  // Check if the row contains the field name.
  bool containsFieldName(folly::StringPiece field_name) const {
    return field_name_map_.find(field_name) != field_name_map_.end();
  }

  // What is the name of the i'th column in the result set?
  folly::StringPiece fieldName(size_t i) const {
    return field_names_[i];
  }

  // What is the name of the table (or alias) for the i'th column in the
  // result set?
  folly::StringPiece tableName(size_t i) const {
    return table_names_[i];
  }

  // How many fields and rows do we have?
  size_t numFields() const {
    return num_fields_;
  }

  // Given a field_name, return the numeric column number, or die trying.
  std::optional<size_t> fieldIndexOpt(folly::StringPiece field_name) const {
    auto it = field_name_map_.find(field_name);
    if (it != field_name_map_.end()) {
      return it->second;
    }

    return std::nullopt;
  }

  size_t fieldIndex(folly::StringPiece field_name) const {
    if (auto opt = fieldIndexOpt(field_name); opt) {
      return *opt;
    }

    throw std::out_of_range(fmt::format("Invalid field: {}", field_name));
  }

 private:
  size_t num_fields_;
  const folly::F14NodeMap<std::string, int> field_name_map_;
  const std::vector<std::string> field_names_;
  const std::vector<std::string> table_names_;
  const std::vector<uint64_t> mysql_field_flags_;
  const std::vector<enum_field_types> mysql_field_types_;

  friend class RowBlock;
};

std::chrono::system_clock::time_point parseDateTime(
    folly::StringPiece datetime,
    enum_field_types date_type);

std::chrono::microseconds parseTimeOnly(
    folly::StringPiece mysql_time,
    enum_field_types field_type);

// A RowBlock holds the raw data from part of a MySQL result set.  It
// corresponds roughly to one set of rows (out of potentially many).
// The size of a block can vary based on the whims of the MySQL client
// and server, so don't count on how many you may get or how many rows
// are in each.
//
// Data layout tries to be efficient; values are packed into memory
// tightly and accessed via StringPieces and Rows that point into this
// block.  This prevents frequent allocations.  See data comments for
// details.
//
// Iterator access is provided as well, allowing for use cases like
//
// for (const auto& row : row_block) {
//   ...
// }
class RowBlock {
 public:
  class Iterator;

  explicit RowBlock(std::shared_ptr<RowFields> row_fields)
      : row_fields_info_(row_fields) {}

  ~RowBlock() {}

  // Given a row N and column M, return a T corresponding to the Nth
  // row's Mth column.
  template <typename T>
  T getField(size_t row, size_t field_num) const;

  // Like above, but converting to the specified type T (using
  // folly::to<T>(StringPiece)).
  template <typename T>
  T getField(size_t row, folly::StringPiece field_name) const;

  // Is this field NULL?
  bool isNull(size_t row, size_t field_num) const {
    CHECK_LT(row, rows_.size());
    return rows_[row].isNull(field_num);
  }

  // Ditto, but by name.
  bool isNull(size_t row, folly::StringPiece field_name) const {
    return isNull(row, row_fields_info_->fieldIndex(field_name));
  }

  // Get the MySQL type of the field.
  enum_field_types getFieldType(size_t field_num) const {
    return row_fields_info_->getFieldType(field_num);
  }

  // Ditto, but by name.
  enum_field_types getFieldType(folly::StringPiece field_name) const {
    return row_fields_info_->getFieldType(field_name);
  }

  // Get the MySQL flags of the field.
  uint64_t getFieldFlags(size_t field_num) const {
    return row_fields_info_->getFieldFlags(field_num);
  }

  // Ditto, but by name.
  uint64_t getFieldFlags(folly::StringPiece field_name) const {
    return row_fields_info_->getFieldFlags(field_name);
  }

  // Access the Nth row of this row block as a Row object.
  Row getRow(size_t n) const {
    return Row(this, n);
  }

  RowFields* getRowFields() {
    return row_fields_info_.get();
  }
  // What is the name of the i'th column in the result set?
  folly::StringPiece fieldName(size_t i) const {
    return row_fields_info_->fieldName(i);
  }

  // What is the index of the column labeled n
  std::optional<size_t> fieldIndexOpt(folly::StringPiece n) const {
    return row_fields_info_->fieldIndexOpt(n);
  }
  size_t fieldIndex(folly::StringPiece n) const {
    return row_fields_info_->fieldIndex(n);
  }

  // Is our rowblock empty?
  bool empty() const {
    return rows_.empty();
  }

  // How many fields and rows do we have?
  size_t numFields() const {
    return row_fields_info_->numFields();
  }

  // How many rows are in this RowBlock?
  size_t numRows() const {
    return rows_.size();
  }

  // Iterator support.  Allows iteration over the rows in this block.
  // Like Row::Iterator, this is mainly for simple range-based for
  // iteration.
  class Iterator : public boost::iterator_facade<
                       Iterator,
                       const Row,
                       boost::single_pass_traversal_tag,
                       const Row> {
   public:
    Iterator(const RowBlock* row_block, size_t row_number)
        : row_block_(row_block), current_row_number_(row_number) {}

    explicit Iterator() = default;

    void increment() {
      ++current_row_number_;
    }
    const Row dereference() const {
      return row_block_->getRow(current_row_number_);
    }
    bool equal(const Iterator& other) const {
      return (
          row_block_ == other.row_block_ &&
          current_row_number_ == other.current_row_number_);
    }

   private:
    const RowBlock* row_block_;
    size_t current_row_number_;
  };

  Iterator begin() const {
    return Iterator(this, 0);
  }

  Iterator end() const {
    return Iterator(this, numRows());
  }

  // Functions called when building a RowBlock.  Not for general use.
  void startRow() {
    rows_.emplace_back(row_fields_info_->numFields());
  }
  void finishRow() {
    CHECK_EQ(rows_.back().count(), row_fields_info_->numFields());
  }
  template <typename T>
  void appendValue(T value) {
    auto& row = rows_.back();
    CHECK_LT(row.count(), row_fields_info_->numFields());
    row.appendValue(std::forward<T>(value));
  }
  // Special override for folly::StringPiece to match existing code
  template <>
  void appendValue(folly::StringPiece value) {
    auto& row = rows_.back();
    CHECK_LT(row.count(), row_fields_info_->numFields());
    row.appendValue(value);
  }
  void appendNull() {
    auto& row = rows_.back();
    CHECK_LT(row.count(), row_fields_info_->numFields());
    row.appendNull();
  }

  // Let the compiler make our move operations.  We disallow copies below.
  RowBlock(RowBlock&&) = default;
  RowBlock& operator=(RowBlock&&) = default;

 private:
  time_t getDateField(size_t row, size_t field_num) const;

  bool isDate(size_t /*row*/, size_t field_num) const {
    switch (getFieldType(field_num)) {
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_DATE:
        return true;
      default:
        return false;
    }
  }

  std::vector<StorageRow> rows_;

  // Storage for strings when we convert a column to std::string_view or
  // folly::StringPiece.  Must be `mutable` because this can occur on a
  // `getField()` call which is a `const` method.
  using RowColumnKey = std::pair<size_t, size_t>;
  mutable StringStore<RowColumnKey> string_store_;

  // field_name_map_ and field_names_ are owned by the RowFields shared between
  // RowBlocks of same query
  std::shared_ptr<RowFields> row_fields_info_;

  RowBlock(const RowBlock&) = delete;
  RowBlock& operator=(const RowBlock&) = delete;
};

class EphemeralRowFields {
 public:
  explicit EphemeralRowFields(std::unique_ptr<InternalRowMetadata> metadata)
      : metadata_(std::move(metadata)) {}

  size_t numFields() const {
    return metadata_->numFields();
  }

  std::optional<size_t> fieldIndexOpt(const char* field_name) const {
    return fieldIndexOpt(std::string_view(field_name));
  }

  std::optional<size_t> fieldIndexOpt(folly::StringPiece field_name) const {
    return fieldIndexOpt(
        std::string_view(field_name.data(), field_name.size()));
  }

  std::optional<size_t> fieldIndexOpt(std::string_view field_name) const {
    for (size_t ii = 0; ii < numFields(); ii++) {
      if (field_name == metadata_->getFieldName(ii)) {
        return ii;
      }
    }

    return std::nullopt;
  }

  size_t fieldIndex(const char* field_name) const {
    return fieldIndex(std::string_view(field_name));
  }

  size_t fieldIndex(folly::StringPiece field_name) const {
    return fieldIndex(std::string_view(field_name.data(), field_name.size()));
  }

  size_t fieldIndex(std::string_view field_name) const {
    if (auto opt = fieldIndexOpt(field_name); opt) {
      return *opt;
    }

    throw std::out_of_range(fmt::format("Invalid field: {}", field_name));
  }

  enum_field_types fieldType(size_t index) const {
    CHECK_LT(index, metadata_->numFields());
    return metadata_->getFieldType(index);
  }

  template <typename StringLike = folly::StringPiece>
  StringLike fieldName(size_t index) const {
    auto res = metadata_->getFieldName(index);
    return StringLike(res.data(), res.size());
  }

  std::shared_ptr<RowFields> makeBufferedFields() const;

  EphemeralRowFields(EphemeralRowFields const&) = delete;
  EphemeralRowFields& operator=(EphemeralRowFields const&) = delete;

  EphemeralRowFields(EphemeralRowFields&&) = default;
  EphemeralRowFields& operator=(EphemeralRowFields&&) = default;

 private:
  std::unique_ptr<InternalRowMetadata> metadata_;
};

class EphemeralRow {
 public:
  EphemeralRow(
      std::unique_ptr<InternalRow> row,
      std::shared_ptr<EphemeralRowFields> row_fields)
      : row_(std::move(row)), row_fields_(std::move(row_fields)) {}

  // Beginning simple, just give the basic indexing.
  InternalRow::Type getType(size_t col) const;

  bool getBool(size_t col) const;

  int64_t getInt64(size_t col) const;

  uint64_t getUInt64(size_t col) const;

  double getDouble(size_t col) const;

  folly::StringPiece getString(size_t col) const;

  // Helper function to convert the data to string format - note this can be
  // expensive as it always generates a new string.  This is useful for logging
  // and other non-performance critical code.
  std::string convertToString(size_t col) const;

  bool isNull(size_t col) const;

  int numFields() const;

  // Calculates the number of bytes in the row data
  uint64_t calculateRowLength() const;

  const EphemeralRowFields& getRowFields() const {
    return *row_fields_;
  }

  EphemeralRow(EphemeralRow const&) = delete;
  EphemeralRow& operator=(EphemeralRow const&) = delete;

  EphemeralRow(EphemeralRow&&) = default;
  EphemeralRow& operator=(EphemeralRow&&) = default;

  EphemeralRow() = default;

  // Attempt to convert the column specified by colName to the type specified in
  // the template.  Note: calling this on a null column will throw an exception
  template <typename DataType>
  DataType convertTo(folly::StringPiece colName) const {
    return convertTo<DataType>(getRowFields().fieldIndex(colName));
  }

  // Attempt to convert the column specified by colIndex to the type specified
  // in the template.  Note: calling this on a null column will throw an
  // exception, so check for null first
  template <typename DataType>
  DataType convertTo(size_t colIndex) const {
    switch (getType(colIndex)) {
      case common::mysql_client::InternalRow::Type::Null:
        throw std::runtime_error(fmt::format(
            "Column {} ({}) has a null value",
            colIndex,
            getRowFields().fieldName(colIndex)));
      case common::mysql_client::InternalRow::Type::Bool:
        return folly::to<DataType>(getBool(colIndex));
      case common::mysql_client::InternalRow::Type::Int64:
        return folly::to<DataType>(getInt64(colIndex));
      case common::mysql_client::InternalRow::Type::UInt64:
        return folly::to<DataType>(getUInt64(colIndex));
      case common::mysql_client::InternalRow::Type::Double:
        return folly::to<DataType>(getDouble(colIndex));
      case common::mysql_client::InternalRow::Type::String:
        return folly::to<DataType>(getString(colIndex));
    }
  }

 private:
  std::unique_ptr<InternalRow> row_;
  std::shared_ptr<EphemeralRowFields> row_fields_;
};

// Declarations of specializations and trivial implementations.
template <>
bool RowBlock::getField(size_t row, size_t field_num) const;

template <>
int64_t RowBlock::getField(size_t row, size_t field_num) const;

template <>
uint64_t RowBlock::getField(size_t row, size_t field_num) const;

template <>
double RowBlock::getField(size_t row, size_t field_num) const;

template <>
std::string RowBlock::getField(size_t row, size_t field_num) const;

template <>
std::string_view RowBlock::getField(size_t row, size_t field_num) const;

template <>
folly::fbstring RowBlock::getField(size_t row, size_t field_num) const;

template <>
folly::StringPiece RowBlock::getField(size_t row, size_t field_num) const;

template <>
time_t RowBlock::getField(size_t row, size_t field_num) const;

template <>
std::chrono::system_clock::time_point RowBlock::getField(
    size_t row,
    size_t field_num) const;

template <>
std::chrono::microseconds RowBlock::getField(size_t row, size_t field_num)
    const;

template <typename T>
T RowBlock::getField(size_t row, size_t field_num) const {
  return folly::to<T>(getField<folly::StringPiece>(row, field_num));
}

template <typename T>
T RowBlock::getField(size_t row, folly::StringPiece field_name) const {
  return getField<T>(row, row_fields_info_->fieldIndex(field_name));
}

template <typename T, typename L>
T Row::get(const L& l) const {
  return row_block_->getField<T>(row_number_, l);
}

template <typename T, typename L>
T Row::get(const L& l, T d) const {
  if (isNull(l)) {
    return d;
  }
  return get<T>(l);
}

// Similar to above but will return as an optional which will be empty if the
// column is null
template <typename T, template <typename> class Optional, typename L>
Optional<T> Row::getOptional(const L& l) const {
  if (isNull(l)) {
    return {};
  }
  return Optional(get<T>(l));
}

} // namespace facebook::common::mysql_client
