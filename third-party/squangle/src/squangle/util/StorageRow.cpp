/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/util/StorageRow.h"

namespace facebook::common::mysql_client {

StorageRow::StorageRow(size_t expectedSize) {
  control_.reserve(expectedSize);
}

// A string can be stored two different ways.
// 1) If the string is 4096 bytes or less we store it directly in `data_`
// 2) If the string is larger than 4096 bytes we store the string data in a
// separate vector (long_strings_).  This means that our `control_` vector can
// be a vector of `uint32_t` instead of `size_t`. By making the maximum for any
// one column 4K, we could theoretically handle 1 million 4K columns before
// hitting an overflow situation.
void StorageRow::appendValue(folly::StringPiece data) {
  if (data.size() <= kSmallStringLimit) {
    writeControl(dtShortString);
    writeBytes(data);
  } else {
    writeControl(dtLongString);
    if (!long_strings_) {
      long_strings_ = std::make_unique<LongStringsVector>();
    }
    writeVarUInt(long_strings_->size());
    long_strings_->push_back(std::string(data));
  }
}

void StorageRow::appendValue(bool data) {
  writeControl(data ? dtBoolTrue : dtBoolFalse);
}

void StorageRow::appendValue(uint64_t data) {
  writeControl(dtUnsignedInteger);
  if (data <= std::numeric_limits<uint8_t>::max()) {
    // We can store the value in a single byte without data loss
    writeByte((std::byte)(uint8_t)data);
  } else if (data <= std::numeric_limits<uint16_t>::max()) {
    // We can store the value in two bytes without data loss
    uint16_t val16 = data;
    writeBytes(&val16, sizeof(val16));
  } else if (data <= std::numeric_limits<uint32_t>::max()) {
    // We can store the value in four bytes without data loss
    uint32_t val32 = data;
    writeBytes(&val32, sizeof(val32));
  } else {
    // We need the full eight bytes to store the data
    writeBytes(&data, sizeof(data));
  }
}

void StorageRow::appendValue(int64_t data) {
  writeControl(dtSignedInteger);
  if (data >= std::numeric_limits<int8_t>::min() &&
      data <= std::numeric_limits<int8_t>::max()) {
    // We can store the value in a single byte without data loss
    writeByte((std::byte)(uint8_t)data);
  } else if (
      data >= std::numeric_limits<int16_t>::min() &&
      data <= std::numeric_limits<int16_t>::max()) {
    // We can store the value in two bytes without data loss
    int16_t val16 = data;
    writeBytes(&val16, sizeof(val16));
  } else if (
      data >= std::numeric_limits<int32_t>::min() &&
      data <= std::numeric_limits<int32_t>::max()) {
    int32_t val32 = data;
    // We can store the value in four bytes without data loss
    writeBytes(&val32, sizeof(val32));
  } else {
    // We need the full eight bytes to store the data
    writeBytes(&data, sizeof(data));
  }
}

void StorageRow::appendValue(double data) {
  writeControl(dtDouble);
  writeDouble(data);
}

void StorageRow::appendNull() {
  writeControl(dtNull);
}

bool StorageRow::isNull(size_t column) const {
  DCHECK_LT(column, control_.size());
  return readControl(column, nullptr) == dtNull;
}

void StorageRow::writeControl(DataType data_type) {
  // The control byte is a combination of the data type (3 high bits) and the
  // offset in `data_` where any data is stored (29 bits)
  DCHECK_LT((uint8_t)data_type, 1U << kTypeBits);
  uint32_t offset = (uint32_t)data_.size();
  DCHECK_LE(offset, kOffsetMask);
  control_.push_back(((uint32_t)data_type << kOffsetBits) | offset);
}

void StorageRow::writeByte(std::byte byte) {
  data_.push_back(byte);
}

void StorageRow::writeDouble(double value) {
  writeBytes(&value, sizeof(value));
}

void StorageRow::writeBytes(const void* data, size_t size) {
  if (size > 0) {
    auto orig_size = data_.size();
    data_.resize(orig_size + size);
    checked_memcpy(&data_[orig_size], data_.size() - orig_size, data, size);
  }
}

void StorageRow::writeBytes(folly::StringPiece str) {
  writeBytes(str.data(), str.size());
}

// This is a way of encoding an integer into the smallest number of bytes
// needed to hold it.  Each byte of the encoded integer contains up to 7 bits
// of the original data.  The high bit is used to know if there is more data
// after this - i.e. it is set in all bytes in the encoded data _except_ the
// last byte.
void StorageRow::writeVarUInt(uint64_t var) {
  do {
    auto byte = (std::byte)var & kVarUIntMask;
    var >>= kVarUIntShift;
    if (var) {
      // Set the high bit to indicate that integer is not done
      byte |= kVarUIntContinuation;
    }
    writeByte(byte);
  } while (var);
}

StorageRow::DataType StorageRow::readControl(size_t column, Offsets* offsets)
    const {
  DCHECK_LT(column, control_.size());

  uint32_t control = control_[column];
  if (offsets) {
    offsets->curr = control & kOffsetMask;
    offsets->end = (column + 1 == control_.size())
        ? data_.size()
        : (control_[column + 1] & kOffsetMask);
    DCHECK_LE(offsets->curr, offsets->end);
  }
  return DataType(control >> kOffsetBits);
}

std::byte StorageRow::readByte(Offsets& offsets) const {
  DCHECK_LT(offsets.curr, offsets.end);
  return data_[offsets.curr++];
}

uint64_t StorageRow::readUnsignedInteger(Offsets& offsets) const {
  DCHECK_GT(offsets.end, offsets.curr);
  switch (offsets.end - offsets.curr) {
    case 1:
      return (uint64_t)readByte(offsets);

    case 2: {
      return readBytes<uint16_t>(offsets);
    }

    case 4: {
      return readBytes<uint32_t>(offsets);
    }

    case 8: {
      return readBytes<uint64_t>(offsets);
    }

    default:
      DCHECK(false) << "Space for an integer (" << (offsets.end - offsets.curr)
                    << ") is not 1, 2, 4, or 8";
  }

  folly::assume_unreachable();
}

int64_t StorageRow::readSignedInteger(Offsets& offsets) const {
  DCHECK_GT(offsets.end, offsets.curr);
  switch (offsets.end - offsets.curr) {
    case 1:
      return (int8_t)readByte(offsets);

    case 2: {
      return readBytes<int16_t>(offsets);
    }

    case 4: {
      return readBytes<int32_t>(offsets);
    }

    case 8: {
      return readBytes<int64_t>(offsets);
    }

    default:
      DCHECK(false) << "Space for an integer (" << (offsets.end - offsets.curr)
                    << ") is not 1, 2, 4, or 8";
  }

  folly::assume_unreachable();
}

uint64_t StorageRow::readVarUInt(Offsets& offsets) const {
  uint64_t res = 0;
  for (;;) {
    auto byte = readByte(offsets);
    res |= res << kVarUIntShift | (uint64_t)(byte & kVarUIntMask);
    if (!(uint8_t)(byte & kVarUIntContinuation)) {
      break;
    }
  }

  return res;
}

double StorageRow::readDouble(Offsets& offsets) const {
  return readBytes<double>(offsets);
}

folly::StringPiece StorageRow::readLongString(Offsets& offsets) const {
  auto long_string_offset = readVarUInt(offsets);
  DCHECK(long_strings_);
  DCHECK_LT(long_string_offset, long_strings_->size());
  return (*long_strings_)[long_string_offset];
}

folly::StringPiece StorageRow::readShortString(Offsets& offsets) const {
  if (offsets.curr == offsets.end) {
    return folly::StringPiece();
  }

  DCHECK_LT(offsets.curr, offsets.end);
  DCHECK_LE(offsets.end, data_.size());
  auto res = folly::StringPiece(
      (const char*)&data_[offsets.curr], offsets.end - offsets.curr);
  offsets.curr = offsets.end;
  return res;
}

} // namespace facebook::common::mysql_client
