/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HeaderTable.h>

#include <glog/logging.h>

namespace proxygen {

uint32_t HeaderTable::initialTableLength(uint32_t capacity) {
  auto maxTableLength = getMaxTableLength(capacity);
  return (maxTableLength == 1) ? 1 : (maxTableLength / 2);
}

void HeaderTable::init(uint32_t capacityVal) {
  bytes_ = 0;
  size_ = 0;
  head_ = 0;
  capacity_ = capacityVal;
  uint32_t initLength = initialTableLength(capacity_);
  table_.reserve(initLength);
  for (uint32_t i = 0; i < initLength; i++) {
    table_.emplace_back();
  }
  names_.clear();
}

bool HeaderTable::add(HPACKHeader header) {
  if (header.bytes() > capacity_) {
    // Per the RFC spec https://tools.ietf.org/html/rfc7541#page-11, we must
    // flush the underlying table if a request is made for a header that is
    // larger than the current table capacity
    reset();
    return false;
  }

  // Make the necessary room in the table if appropriate per RFC spec
  if ((bytes_ + header.bytes()) > capacity_) {
    if (evict(header.bytes(), capacity_) == 0) {
      return false;
    }
  }

  if (size_ == length()) {
    increaseTableLengthTo(
        std::min((uint32_t)ceil(size_ * 1.5), getMaxTableLength(capacity_)));
  }
  head_ = next(head_);
  // index name
  if (indexNames_) {
    names_[header.name].push_back(head_);
  }
  bytes_ += header.bytes();
  table_[head_] = std::move(header);

  ++size_;
  ++insertCount_;
  return true;
}

std::pair<uint32_t, uint32_t> HeaderTable::getIndex(
    const HPACKHeader& header) const {
  return getIndexImpl(header.name, header.value, false);
}

std::pair<uint32_t, uint32_t> HeaderTable::getIndex(
    const HPACKHeaderName& name, folly::StringPiece value) const {
  return getIndexImpl(name, value, false);
}

std::pair<uint32_t, uint32_t> HeaderTable::getIndexImpl(
    const HPACKHeaderName& headerName,
    folly::StringPiece value,
    bool nameOnly) const {
  CHECK(indexNames_);
  auto it = names_.find(headerName);
  if (it == names_.end()) {
    return {0, 0};
  }
  for (auto indexIt = it->second.rbegin(); indexIt != it->second.rend();
       ++indexIt) {
    auto i = *indexIt;
    if (nameOnly || table_[i].value == value) {
      return {toExternal(i), 0};
    }
  }
  return {0, toExternal(*it->second.rbegin())};
}

bool HeaderTable::hasName(const HPACKHeaderName& headerName) {
  CHECK(indexNames_);
  return names_.find(headerName) != names_.end();
}

uint32_t HeaderTable::nameIndex(const HPACKHeaderName& headerName) const {
  folly::StringPiece value;
  return getIndexImpl(headerName, value, true /* name only */).first;
}

const HPACKHeader& HeaderTable::getHeader(uint32_t index) const {
  CHECK(isValid(index));
  return table_[toInternal(index)];
}

uint32_t HeaderTable::getMaxTableLength(uint32_t capacityVal) const {
  // At a minimum an entry will take 32 bytes
  // No need to add an extra slot; i.e. a capacity of 32 to 63 bytes can hold
  // at most one entry.
  return (capacityVal >> 5);
}

uint32_t HeaderTable::removeLast() {
  auto t = tail();
  // remove the first element from the names index
  if (indexNames_) {
    auto names_it = names_.find(table_[t].name);
    DCHECK(names_it != names_.end());
    auto& ilist = names_it->second;
    DCHECK_EQ(ilist.front(), t);
    ilist.pop_front();

    // remove the name if there are no indices associated with it
    if (ilist.empty()) {
      names_.erase(names_it);
    }
  }
  const auto& header = table_[t];
  uint32_t headerBytes = header.bytes();
  bytes_ -= headerBytes;
  DVLOG(10) << "Removing local idx=" << t << " name=" << header.name
            << " value=" << header.value;
  --size_;
  return headerBytes;
}

void HeaderTable::reset() {
  names_.clear();

  bytes_ = 0;
  size_ = 0;

  // Capacity remains unchanged and for now we leave head_ index the same
}

bool HeaderTable::setCapacity(uint32_t newCapacity) {
  if (newCapacity == capacity_) {
    return true;
  } else if (newCapacity < capacity_) {
    // NOTE: currently no actual resizing is performed...
    evict(0, newCapacity);
    if (bytes_ > newCapacity) {
      // eviction failed!
      return false;
    }
  } else {
    // NOTE: due to the above lack of resizing, we must determine whether a
    // resize is actually appropriate (to handle cases where the underlying
    // vector is still >= to the size related to the new capacity requested)
    uint32_t newLength = initialTableLength(newCapacity);
    if (newLength > length()) {
      increaseTableLengthTo(newLength);
    }
  }
  capacity_ = newCapacity;
  return true;
}

void HeaderTable::increaseTableLengthTo(uint32_t newLength) {
  DCHECK_GE(newLength, length());
  uint32_t oldTail = (size_ > 0) ? tail() : 0;
  auto oldLength = length();
  resizeTable(newLength);

  // TODO: referenence to head here is incompatible with baseIndex
  if (size_ > 0 && oldTail > head_) {
    // the list wrapped around, need to move oldTail..oldLength to the end
    // of the now-larger table_
    updateResizedTable(oldTail, oldLength, newLength);
    // Update the names indecies that pointed to the old range
    if (indexNames_) {
      for (auto& names_it : names_) {
        for (auto& idx : names_it.second) {
          if (idx >= oldTail) {
            DCHECK_LT(idx + (length() - oldLength), length());
            idx += (length() - oldLength);
          } else {
            // remaining indecies in the list were smaller than oldTail, so
            // should be indexed from 0
            break;
          }
        }
      }
    }
  }
}

void HeaderTable::resizeTable(uint32_t newLength) {
  table_.resize(newLength);
}

void HeaderTable::updateResizedTable(uint32_t oldTail,
                                     uint32_t oldLength,
                                     uint32_t newLength) {
  std::move_backward(table_.begin() + oldTail,
                     table_.begin() + oldLength,
                     table_.begin() + newLength);
}

uint32_t HeaderTable::evict(uint32_t needed, uint32_t desiredCapacity) {
  uint32_t previousSize = size_;
  while (size_ > 0 && (bytes_ + needed > desiredCapacity)) {
    removeLast();
  }
  return previousSize - size_;
}

bool HeaderTable::isValid(uint32_t index) const {
  bool result = false;
  result = 0 < index && index <= size_;
  if (!result) {
    LOG(ERROR) << "Invalid index=" << index << " size_=" << size_;
  }
  return result;
}

uint32_t HeaderTable::next(uint32_t i) const {
  return (i + 1) % length();
}

uint32_t HeaderTable::tail() const {
  // tail is private, and only called in the encoder, where head_ is always
  // valid
  DCHECK_GT(size_, 0) << "tail() undefined";
  return (head_ + length() - size_ + 1) % length();
}

uint32_t HeaderTable::toExternal(uint32_t internalIndex) const {
  return toExternal(head_, length(), internalIndex);
}

uint32_t HeaderTable::toExternal(uint32_t head,
                                 uint32_t length,
                                 uint32_t internalIndex) {
  return ((head + length - internalIndex) % length) + 1;
}

uint32_t HeaderTable::toInternal(uint32_t externalIndex) const {
  return toInternal(head_, length(), externalIndex);
}

uint32_t HeaderTable::toInternal(uint32_t head,
                                 uint32_t length,
                                 uint32_t externalIndex) {
  // remove the offset
  --externalIndex;
  return (head + length - externalIndex) % length;
}

bool HeaderTable::operator==(const HeaderTable& other) const {
  if (size() != other.size()) {
    return false;
  }
  if (bytes() != other.bytes()) {
    return false;
  }
  return true;
}

std::ostream& operator<<(std::ostream& os, const HeaderTable& table) {
  os << std::endl;
  for (size_t i = 1; i <= table.size(); i++) {
    const HPACKHeader& h = table.getHeader(i);
    os << '[' << i << "] (s=" << h.bytes() << ") " << h.name << ": " << h.value
       << std::endl;
  }
  os << "total size: " << table.bytes() << std::endl;
  return os;
}

} // namespace proxygen
