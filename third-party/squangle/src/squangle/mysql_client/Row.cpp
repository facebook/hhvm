/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Row.h"

#include <folly/Conv.h>
#include <re2/re2.h>
#include <chrono>

namespace facebook::common::mysql_client {

std::shared_ptr<RowFields> EphemeralRowFields::makeBufferedFields() const {
  auto num_fields = metadata_->numFields();
  if (num_fields == 0) {
    return nullptr;
  }
  std::vector<std::string> field_names;
  std::vector<std::string> table_names;
  folly::F14NodeMap<std::string, int> field_name_map;
  std::vector<uint64_t> field_flags;
  std::vector<enum_field_types> field_types;

  field_names.reserve(num_fields);
  table_names.reserve(num_fields);
  field_flags.reserve(num_fields);
  field_types.reserve(num_fields);
  for (int i = 0; i < num_fields; ++i) {
    std::string field_name(metadata_->getFieldName(i));
    field_name_map[field_name] = i;
    field_names.push_back(field_name);
    table_names.emplace_back(metadata_->getTableName(i));
    field_flags.push_back(metadata_->getFieldFlags(i));
    field_types.push_back(metadata_->getFieldType(i));
  }
  return std::make_shared<RowFields>(
      std::move(field_name_map),
      std::move(field_names),
      std::move(table_names),
      std::move(field_flags),
      std::move(field_types));
}

InternalRow::Type EphemeralRow::getType(size_t col) const {
  return row_->columnType(col);
}

bool EphemeralRow::getBool(size_t col) const {
  return row_->columnBool(col);
}

int64_t EphemeralRow::getInt64(size_t col) const {
  return row_->columnInt64(col);
}

uint64_t EphemeralRow::getUInt64(size_t col) const {
  return row_->columnUInt64(col);
}

double EphemeralRow::getDouble(size_t col) const {
  return row_->columnDouble(col);
}

folly::StringPiece EphemeralRow::getString(size_t col) const {
  return row_->columnString(col);
}

std::string EphemeralRow::convertToString(size_t col) const {
  switch (getType(col)) {
    case common::mysql_client::InternalRow::Type::Null:
      return "<null>";
    case common::mysql_client::InternalRow::Type::Bool:
      return folly::to<std::string>(getBool(col));
    case common::mysql_client::InternalRow::Type::Int64:
      return folly::to<std::string>(getInt64(col));
    case common::mysql_client::InternalRow::Type::UInt64:
      return folly::to<std::string>(getUInt64(col));
    case common::mysql_client::InternalRow::Type::Double:
      return folly::to<std::string>(getDouble(col));
    case common::mysql_client::InternalRow::Type::String:
      return getString(col).str();
  }
}

bool EphemeralRow::isNull(size_t col) const {
  return getType(col) == InternalRow::Type::Null;
}

int EphemeralRow::numFields() const {
  return row_fields_->numFields();
}

size_t EphemeralRow::calculateRowLength() const {
  size_t rowLength = 0;
  for (int i = 0; i < numFields(); ++i) {
    rowLength += row_->columnLength(i);
  }
  return rowLength;
}

Row::Row(const RowBlock* row_block, size_t row_number)
    : row_block_(row_block), row_number_(row_number) {
  CHECK_LT(row_number, row_block->numRows());
}

size_t Row::size() const {
  return row_block_->numFields();
}

folly::StringPiece Row::operator[](size_t col) const {
  return row_block_->getField<folly::StringPiece>(row_number_, col);
}

folly::StringPiece Row::operator[](folly::StringPiece field) const {
  return row_block_->getField<folly::StringPiece>(row_number_, field);
}

bool Row::isNull(size_t col) const {
  return row_block_->isNull(row_number_, col);
}

bool Row::isNull(folly::StringPiece field) const {
  return row_block_->isNull(row_number_, field);
}

auto Row::begin() const -> Iterator {
  return Iterator(this, 0);
}

auto Row::end() const -> Iterator {
  return Iterator(this, size());
}

folly::dynamic Row::getDynamic(folly::StringPiece l) const {
  return getDynamic(row_block_->fieldIndex(l));
}

folly::dynamic Row::getDynamic(size_t l) const {
  enum_field_types type = row_block_->getFieldType(l);
  try {
    if (row_block_->isNull(row_number_, l)) {
      return folly::dynamic(folly::StringPiece());
    }
    switch (type) {
      // folly::dynamic::Type::DOUBLE
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_FLOAT:
      case MYSQL_TYPE_DOUBLE:
        return folly::dynamic(row_block_->getField<double>(row_number_, l));

      // folly::dynamic::Type::INT64
      case MYSQL_TYPE_BIT:
      case MYSQL_TYPE_TINY:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_LONGLONG:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_YEAR:
        if (row_block_->getFieldFlags(l) & UNSIGNED_FLAG) {
          return folly::dynamic(row_block_->getField<uint64_t>(row_number_, l));
        }
        return folly::dynamic(row_block_->getField<int64_t>(row_number_, l));

      // folly::dynamic::Type::STRING
      default:
        return folly::dynamic(
            row_block_->getField<folly::StringPiece>(row_number_, l));
    }
  } catch (const std::exception&) {
    // If we failed to parse (NULL int, etc), try again as a string
    return folly::dynamic(
        row_block_->getField<folly::StringPiece>(row_number_, l));
  }
}

template <>
bool RowBlock::getField(size_t row, size_t field_num) const {
  CHECK_LT(row, rows_.size());
  CHECK_LT(field_num, row_fields_info_->numFields());
  CHECK(!isNull(row, field_num));

  return rows_[row].as<bool>(field_num, [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, bool>) {
      return arg;
    } else if constexpr (
        (std::is_same_v<T, int64_t>) || (std::is_same_v<T, uint64_t>) ||
        (std::is_same_v<T, double>) ||
        (std::is_same_v<T, folly::StringPiece>)) {
      return folly::to<bool>(arg);
    }

    throw std::runtime_error("Invalid type");
    return false;
  });
}

template <>
int64_t RowBlock::getField(size_t row, size_t field_num) const {
  CHECK_LT(row, rows_.size());
  CHECK_LT(field_num, row_fields_info_->numFields());
  CHECK(!isNull(row, field_num));

  if (isDate(row, field_num)) {
    return getDateField(row, field_num);
  }

  return rows_[row].as<int64_t>(field_num, [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int64_t>) {
      return arg;
    } else if constexpr (
        (std::is_same_v<T, bool>) || (std::is_same_v<T, uint64_t>) ||
        (std::is_same_v<T, double>) ||
        (std::is_same_v<T, folly::StringPiece>)) {
      return folly::to<int64_t>(arg);
    }

    return (int64_t)0;
  });
}

template <>
uint64_t RowBlock::getField(size_t row, size_t field_num) const {
  CHECK_LT(row, rows_.size());
  CHECK_LT(field_num, row_fields_info_->numFields());
  CHECK(!isNull(row, field_num));

  return rows_[row].as<uint64_t>(field_num, [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, uint64_t>) {
      return arg;
    } else if constexpr (
        (std::is_same_v<T, bool>) || (std::is_same_v<T, int64_t>) ||
        (std::is_same_v<T, double>) ||
        (std::is_same_v<T, folly::StringPiece>)) {
      return folly::to<uint64_t>(arg);
    }

    return (uint64_t)0;
  });
}

template <>
double RowBlock::getField(size_t row, size_t field_num) const {
  CHECK_LT(row, rows_.size());
  CHECK_LT(field_num, row_fields_info_->numFields());
  CHECK(!isNull(row, field_num));

  return rows_[row].as<double>(field_num, [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, double>) {
      return arg;
    } else if constexpr (
        (std::is_same_v<T, bool>) || (std::is_same_v<T, int64_t>) ||
        (std::is_same_v<T, uint64_t>) ||
        (std::is_same_v<T, folly::StringPiece>)) {
      return folly::to<double>(arg);
    }

    return 0.0;
  });
}

template <>
folly::StringPiece RowBlock::getField(size_t row, size_t field_num) const {
  CHECK_LT(row, rows_.size());
  CHECK_LT(field_num, row_fields_info_->numFields());

  if (isNull(row, field_num)) {
    return folly::StringPiece();
  }

  return rows_[row].as<folly::StringPiece>(field_num, [&](const auto& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, folly::StringPiece>) {
      return arg;
    } else if constexpr (
        (std::is_same_v<T, bool>) || (std::is_same_v<T, int64_t>) ||
        (std::is_same_v<T, uint64_t>) || (std::is_same_v<T, std::string>)) {
      const auto& data = string_store_.getString(
          RowColumnKey(row, field_num),
          [&]() { return folly::to<std::string>(arg); });
      return folly::StringPiece(data);
    }

    return folly::StringPiece();
  });
}

template <>
folly::fbstring RowBlock::getField(size_t row, size_t field_num) const {
  return folly::fbstring(getField<folly::StringPiece>(row, field_num));
}

template <>
std::string_view RowBlock::getField(size_t row, size_t field_num) const {
  return std::string_view(getField<folly::StringPiece>(row, field_num));
}

template <>
std::string RowBlock::getField(size_t row, size_t field_num) const {
  return std::string(getField<folly::StringPiece>(row, field_num));
}

template <>
std::chrono::system_clock::time_point RowBlock::getField(
    size_t row,
    size_t field_num) const {
  auto field_value = getField<folly::StringPiece>(row, field_num);
  return parseDateTime(field_value, getFieldType(field_num));
}

template <>
std::chrono::microseconds RowBlock::getField(size_t row, size_t field_num)
    const {
  auto field_value = getField<folly::StringPiece>(row, field_num);
  return parseTimeOnly(field_value, getFieldType(field_num));
}

time_t RowBlock::getDateField(size_t row, size_t field_num) const {
  auto field_value = getField<folly::StringPiece>(row, field_num);
  auto chrono_time = parseDateTime(field_value, getFieldType(field_num));
  time_t field_timet = std::chrono::system_clock::to_time_t(chrono_time);
  if (field_timet == -1) {
    throw std::range_error("Calendar time cannot be represented as time_t");
  }
  return field_timet;
}

std::chrono::microseconds parseTimeOnly(
    folly::StringPiece mysql_time,
    enum_field_types field_type) {
  static re2::RE2 time_pattern(
      "([-]?\\d{1,3}):(\\d{2}):(\\d{2})(?:\\.(\\d{1,6}))?");
  int hours = 0, minutes = 0, seconds = 0, microseconds = 0;
  std::string microseconds_str;
  if (field_type != MYSQL_TYPE_TIME) {
    throw std::range_error("No conversion available");
  }

  re2::StringPiece re2_mysql_time(mysql_time.data(), mysql_time.size());
  if (!re2::RE2::FullMatch(
          re2_mysql_time.data(),
          time_pattern,
          &hours,
          &minutes,
          &seconds,
          &microseconds_str)) {
    throw std::range_error("Can't parse time");
  }
  if (!microseconds_str.empty()) {
    microseconds_str.resize(6, '0');
    microseconds = folly::to<int>(microseconds_str.c_str());
  }
  auto result = std::chrono::hours(hours) + std::chrono::minutes(minutes) +
      std::chrono::seconds(seconds) + std::chrono::microseconds(microseconds);
  return result;
}

std::chrono::system_clock::time_point parseDateTime(
    folly::StringPiece datetime,
    enum_field_types date_type) {
  const int TM_YEAR_BASE = 1900;

  // Clean struct and set daylight savings to information not available
  struct tm time_tm = {};

  time_tm.tm_isdst = -1;
  std::string microseconds_str;
  int microseconds = 0;

  bool parse_succeeded = false;
  re2::StringPiece re2_datetime(datetime.data(), datetime.size());
  switch (date_type) {
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATETIME:
      static re2::RE2 timestamp_pattern(
          "(\\d{4})-(\\d{2})-(\\d{2}) "
          "(\\d{2}):(\\d{2}):(\\d{2})(?:\\.(\\d{1,6}))?");
      parse_succeeded = re2::RE2::FullMatch(
          re2_datetime,
          timestamp_pattern,
          &time_tm.tm_year,
          &time_tm.tm_mon,
          &time_tm.tm_mday,
          &time_tm.tm_hour,
          &time_tm.tm_min,
          &time_tm.tm_sec,
          &microseconds_str);
      break;
    case MYSQL_TYPE_DATE:
      static re2::RE2 date_pattern("(\\d{4})-(\\d{2})-(\\d{2})");
      parse_succeeded = re2::RE2::FullMatch(
          re2_datetime,
          date_pattern,
          &time_tm.tm_year,
          &time_tm.tm_mon,
          &time_tm.tm_mday);
      break;
    default:
      break;
  }

  if (!parse_succeeded) {
    throw std::range_error("Can't parse date");
  }
  if (!microseconds_str.empty()) {
    microseconds_str.resize(6, '0');
    microseconds = folly::to<int>(microseconds_str.c_str());
  }

  if (time_tm.tm_year) {
    time_tm.tm_year -= TM_YEAR_BASE;
  }

  if (time_tm.tm_mon) {
    time_tm.tm_mon -= 1;
  }

  auto t = mktime(&time_tm);

  if (t == -1) {
    throw std::range_error("Date values are invalid");
  }

  auto chrono_time = std::chrono::system_clock::from_time_t(t);

  chrono_time = chrono_time + std::chrono::microseconds(microseconds);
  return chrono_time;
}

} // namespace facebook::common::mysql_client
