/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Conv.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <glog/logging.h>

#include "secure_lib/secure_string.h"

namespace facebook::common::mysql_client {

/**
The StorageRow class is an attempt to store MySQL row data in a space efficient
manner that works for both the MySQL protocol (data always returned as strings)
and the Thrift protocol (data returned in native format).

Each entry has at minimum an entry in `offset_`.  The high bit of the offset is
set if the column is null.  Otherwise the other 31 bits are the offset into
`data_` where the data for the column starts.

Assuming the data is not null, information about the column's contents is
then written into `data_`.  The first byte is a combination of 2 bits for the
basic type (bool, integer, double, string) and 6 bits for flags.  For bool data,
the value is stored as a flag in the control flags and no other data is
necessary.

For integers we store whether the data was written as signed or unsigned as a
flag in the control flags.  Two more bits are used to store whether the data is
written as a single byte, two bytes, four bytes or eight bytes.  We could
support more versions (3 bytes, 5 bytes, etc.) but the code to handle these is
more complicated so I stuck to the standard sizes.

Doubles are stored natively.  There isn't any compression we can do that isn't
lossy.

Strings are stored one of two ways with a flag in the control flags used to
differentiate the two.  Strings of 4096 bytes or less are stored straight in
`data_`, byte for byte (without the null terminator).  Strings longer than that
have an index written into `data_` (using the variable integer format) and then
are put into `long_strings_` (at the specified index) individually. The benefit
of doing this is that we can make the type of `control_` a vector of `uint32_t`
instead of `size_t` saving 4 bytes per column.
**/

class StorageRow {
 private:
  static constexpr auto kTypeBits = 3;
  static constexpr auto kOffsetBits = 32 - kTypeBits;
  static constexpr auto kOffsetMask = (1U << kOffsetBits) - 1;
  static constexpr auto kSmallStringLimit = 4096;

  // Used for variable sized integers (used for 'long' strings)
  static constexpr auto kVarUIntContinuation = (std::byte)0x80;
  static constexpr auto kVarUIntMask = (std::byte)0x7F;
  static constexpr auto kVarUIntShift = 7;

 public:
  explicit StorageRow(size_t expectedSize);

  void appendValue(folly::StringPiece data);
  void appendValue(const char* str) {
    appendValue(folly::StringPiece(str));
  }
  void appendValue(bool data);
  void appendValue(uint64_t data);
  void appendValue(int64_t data);
  void appendValue(double data);

  void appendNull();

  size_t count() const {
    return control_.size();
  }

  bool isNull(size_t column) const;

  template <typename T, typename Func>
  T as(size_t column, Func&& func) const {
    DCHECK_LT(column, control_.size());
    DCHECK(!isNull(column));

    Offsets offsets;
    auto data_type = readControl(column, &offsets);
    // Double-check that all the bytes were consumed when we are done.  If not
    // that means we have a bug in the encoding protocol.
    auto guard =
        folly::makeGuard([&] { DCHECK_EQ(offsets.curr, offsets.end); });

    switch (data_type) {
      case dtNull:
        DCHECK(false);
        throw std::runtime_error(
            "data type is null - this should have been checked earlier");

      case dtBoolTrue:
        return func(true);

      case dtBoolFalse:
        return func(false);

      case dtUnsignedInteger:
        return func(readUnsignedInteger(offsets));

      case dtSignedInteger:
        return func(readSignedInteger(offsets));

      case dtDouble:
        return func(readDouble(offsets));

      case dtShortString:
        return func(readShortString(offsets));

      case dtLongString:
        return func(readLongString(offsets));

      default:
        DCHECK(false);
        throw std::runtime_error("Invalid data type");
    }

    folly::assume_unreachable();
  }

 private:
  // With only 8 values this enum can fit in 3 bits.  The `control_` member is a
  // vector of `uint32_t` where the high 3 bits represent values in this enum
  // and the low 29 bits represent the offset into `data_` for the column.
  enum DataType {
    dtNull,
    dtBoolTrue,
    dtBoolFalse,
    dtSignedInteger,
    dtUnsignedInteger,
    dtDouble,
    dtShortString,
    dtLongString,
  };

  struct Offsets {
    uint32_t curr;
    uint32_t end;
  };

  void writeControl(DataType data_type);

  void writeByte(std::byte byte);
  void writeDouble(double value);
  void writeBytes(const void* data, size_t size);
  void writeBytes(folly::StringPiece str);
  void writeVarUInt(uint64_t var);

  DataType readControl(size_t column, Offsets* offsets) const;
  std::byte readByte(Offsets& offsets) const;
  uint64_t readUnsignedInteger(Offsets& offsets) const;
  int64_t readSignedInteger(Offsets& offsets) const;
  uint64_t readVarUInt(Offsets& offsets) const;

  template <typename T>
  T readBytes(Offsets& offsets) const {
    DCHECK_LE(offsets.curr + sizeof(T), offsets.end);
    T res;
    checked_memcpy(&res, sizeof(res), &data_[offsets.curr], sizeof(res));
    offsets.curr += sizeof(res);
    return res;
  }

  double readDouble(Offsets& offsets) const;
  folly::StringPiece readShortString(Offsets& offsets) const;
  folly::StringPiece readLongString(Offsets& offsets) const;

  // This holds the offsets into `data_` for each column in this row.  Every
  // column has to be represented (even if null).  We can use a uint32_t here
  // because we pull large strings into a separate location.  31 bits (1 bit is
  // used for null values) gives us 2Gb of space per row.  Strings longer than
  // 4Kb stored elsewhere, so in the worst case we would store 4K (column_size)
  // * 4K (maximum_cols_per_table) for about 16Mb in data_.
  std::vector<uint32_t> control_;
  // This holds all the basic data, not including long strings) for the row. The
  // data for any particular column is accessed via `control_[column]` and the
  // number of bytes for that column can be derived by subtracting the offset
  // from the next column's offset.
  std::vector<std::byte> data_;

  // This holds any long (>4096 bytes) strings.  It is more efficient to hold
  // them outside of `data_` as this allows us to use 32 bits for offsets per
  // column instead of 64. Use a unique pointer for this vector so that when we
  // don't need any long strings we only take up 8 bytes for the unique_ptr vs.
  // 24 for a vector<std::string>.
  using LongStringsVector = std::vector<std::string>;
  std::unique_ptr<LongStringsVector> long_strings_;
};

} // namespace facebook::common::mysql_client
