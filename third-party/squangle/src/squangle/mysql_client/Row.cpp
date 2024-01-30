/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Row.h"

#include <re2/re2.h>
#include <chrono>
#include <numeric>

namespace facebook {
namespace common {
namespace mysql_client {

std::shared_ptr<RowFields> EphemeralRowFields::makeBufferedFields() const {
  if (num_fields_ == 0) {
    return nullptr;
  }
  std::vector<std::string> field_names;
  std::vector<std::string> table_names;
  folly::F14NodeMap<std::string, int> field_name_map;
  std::vector<uint64_t> mysql_field_flags;
  std::vector<enum_field_types> mysql_field_types;

  field_names.reserve(num_fields_);
  table_names.reserve(num_fields_);
  mysql_field_flags.reserve(num_fields_);
  mysql_field_types.reserve(num_fields_);
  for (int i = 0; i < num_fields_; ++i) {
    MYSQL_FIELD* mysql_field = &fields_[i];
    field_names.emplace_back(mysql_field->name, mysql_field->name_length);
    table_names.emplace_back(mysql_field->table, mysql_field->table_length);
    mysql_field_flags.push_back(mysql_field->flags);
    mysql_field_types.push_back(mysql_field->type);
    field_name_map[mysql_field->name] = i;
  }
  return std::make_shared<RowFields>(
      std::move(field_name_map),
      std::move(field_names),
      std::move(table_names),
      std::move(mysql_field_flags),
      std::move(mysql_field_types));
}

folly::StringPiece EphemeralRow::operator[](size_t col) const {
  DCHECK_LT(col, row_fields_->numFields());
  auto length = field_lengths_[col];
  return folly::StringPiece(mysql_row_[col], mysql_row_[col] + length);
}

bool EphemeralRow::isNull(size_t col) const {
  DCHECK_LT(col, row_fields_->numFields());
  return (mysql_row_[col] == nullptr);
}

int EphemeralRow::numFields() const {
  return row_fields_->numFields();
}

uint64_t EphemeralRow::calculateRowLength() const {
  return std::reduce(field_lengths_, field_lengths_ + numFields());
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
          return folly::dynamic(
              row_block_->getField<unsigned long>(row_number_, l));
        }
        return folly::dynamic(row_block_->getField<long>(row_number_, l));

      // folly::dynamic::Type::STRING
      default:
        return folly::dynamic(
            row_block_->getField<std::string>(row_number_, l));
    }
  } catch (const std::exception&) {
    // If we failed to parse (NULL int, etc), try again as a string
    return folly::dynamic(row_block_->getField<std::string>(row_number_, l));
  }
}

template <>
folly::StringPiece RowBlock::getField(size_t row, size_t field_num) const {
  size_t entry = row * row_fields_info_->numFields() + field_num;
  if (null_values_[entry]) {
    return folly::StringPiece(nullptr, nullptr);
  }

  size_t field_size;

  if (entry == field_offsets_.size() - 1) {
    field_size = buffer_.size() - field_offsets_[entry];
  } else {
    field_size = field_offsets_[entry + 1] - field_offsets_[entry];
  }

  return field_size != 0
      ? folly::StringPiece(&buffer_[field_offsets_[entry]], field_size)
      : folly::StringPiece();
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

template <>
time_t RowBlock::getField(size_t row, size_t field_num) const {
  if (isDate(row, field_num)) {
    return getDateField(row, field_num);
  }
  return folly::to<time_t>(getField<folly::StringPiece>(row, field_num));
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
  struct tm time_tm = {0};

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
} // namespace mysql_client
} // namespace common
} // namespace facebook
