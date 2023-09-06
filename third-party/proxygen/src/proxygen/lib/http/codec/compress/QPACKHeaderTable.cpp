/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/QPACKHeaderTable.h>

#include <glog/logging.h>

namespace {
// For tables 0..384     minFree = 48
// For tables 385..4096  minFree = tableSize/8
// For tables 4096+      minFree = 512

// Determines how much space in the table should be reserved for new inserts.
// Min Free is set to tableSize / kMinFreeSlice
const uint32_t kMinFreeSlice = 8;

const uint32_t kMinMinFree = 48;
const uint32_t kMaxMinFree = 512;

uint32_t getMinFree(uint32_t tableSize) {
  return std::min(std::max(tableSize / kMinFreeSlice, kMinMinFree),
                  kMaxMinFree);
}

} // namespace

namespace proxygen {

QPACKHeaderTable::QPACKHeaderTable(uint32_t capacityVal, bool trackReferences)
    : HeaderTable(capacityVal) {
  if (trackReferences) {
    refCount_ = std::make_unique<std::vector<uint16_t>>(table_.size(), 0);
    minFree_ = getMinFree(capacityVal);
  } else {
    minFree_ = 0;
    disableNamesIndex();
  }
}

bool QPACKHeaderTable::add(HPACKHeader header) {
  if (insertCount_ == std::numeric_limits<uint32_t>::max()) {
    LOG(ERROR) << "Cowardly refusing to add more entries since insertCount_ "
                  " would wrap";
    return false;
  }

  DVLOG(6) << "Adding header=" << header;
  if (!HeaderTable::add(std::move(header))) {
    return false;
  }
  if (refCount_) {
    (*refCount_)[head_] = 0;
  }
  DCHECK_EQ(internalToAbsolute(head_), insertCount_);
  // Increase minUsable_ until the free space + drainedBytes is >= minFree.
  // For HPACK, minFree is 0 and this is a no-op.
  while (capacity_ - bytes_ + drainedBytes_ < minFree_ &&
         minUsable_ <= insertCount_) {
    auto bytes = table_[absoluteToInternal(minUsable_)].bytes();
    VLOG(5) << "Draining absolute index " << minUsable_ << " bytes=" << bytes
            << " drainedBytes_= " << (drainedBytes_ + bytes);
    drainedBytes_ += bytes;
    minUsable_++;
  }
  return true;
}

bool QPACKHeaderTable::setCapacity(uint32_t capacity) {
  if (!HeaderTable::setCapacity(capacity)) {
    return false;
  }
  if (refCount_) {
    minFree_ = getMinFree(capacity);
  } // else minFree is always 0
  return true;
}

uint32_t QPACKHeaderTable::getIndex(const HPACKHeader& header,
                                    bool allowVulnerable) const {
  return getIndexImpl(header.name, header.value, false, allowVulnerable);
}

uint32_t QPACKHeaderTable::getIndex(const HPACKHeaderName& name,
                                    folly::StringPiece value,
                                    bool allowVulnerable) const {
  return getIndexImpl(name, value, false, allowVulnerable);
}

uint32_t QPACKHeaderTable::getIndexImpl(const HPACKHeaderName& headerName,
                                        folly::StringPiece value,
                                        bool nameOnly,
                                        bool allowVulnerable) const {
  auto it = names_.find(headerName);
  if (it == names_.end()) {
    return 0;
  }
  bool encoderHasUnackedEntry = false;
  // Searching backwards gives smallest index, but more likely vulnerable
  // Searching forwards least likely vulnerable but could prevent eviction
  for (auto indexIt = it->second.rbegin(); indexIt != it->second.rend();
       ++indexIt) {
    auto i = *indexIt;
    if (nameOnly || table_[i].value == value) {
      // allow vulnerable or not vulnerable
      if (allowVulnerable || internalToAbsolute(i) <= ackedInsertCount_) {
        // index *may* be draining, caller has to check
        return toExternal(i);
      } else {
        encoderHasUnackedEntry = true;
      }
    }
  }
  if (encoderHasUnackedEntry) {
    return UNACKED;
  }
  return 0;
}

uint32_t QPACKHeaderTable::nameIndex(const HPACKHeaderName& headerName,
                                     bool allowVulnerable) const {
  folly::fbstring value;
  return getIndexImpl(headerName, value, true /* name only */, allowVulnerable);
}

const HPACKHeader& QPACKHeaderTable::getHeader(uint32_t index,
                                               uint32_t base) const {
  CHECK(isValid(index, base));
  return table_[toInternal(index, base)];
}

uint32_t QPACKHeaderTable::removeLast() {
  auto idx = tail();
  if (refCount_) {
    CHECK_EQ((*refCount_)[idx], 0) << "Removed header with nonzero references";
  }
  auto removedBytes = HeaderTable::removeLast();
  // Only non-zero when minUsable_ > insertCount_ - size_.
  if (drainedBytes_ > 0) {
    VLOG(5) << "Removing draining entry=" << idx << " size=" << removedBytes
            << " drainedBytes_=" << drainedBytes_
            << " new drainedBytes_=" << (int32_t(drainedBytes_) - removedBytes);
    CHECK_GE(drainedBytes_, removedBytes);
    drainedBytes_ -= removedBytes;
  } else {
    // Keep minUsable_ as a valid index when evicting an undrained header
    if (size() > 0) {
      minUsable_ = internalToAbsolute(tail());
    } else {
      minUsable_ = insertCount_ + 1;
    }
  }
  return removedBytes;
}

void QPACKHeaderTable::increaseTableLengthTo(uint32_t newLength) {
  HeaderTable::increaseTableLengthTo(newLength);
  if (size_ > 0) {
    DCHECK_EQ(internalToAbsolute(head_), insertCount_);
    DCHECK_EQ(internalToAbsolute(tail()), insertCount_ - size_ + 1);
  }
}

void QPACKHeaderTable::resizeTable(uint32_t newLength) {
  HeaderTable::resizeTable(newLength);
  if (refCount_) {
    refCount_->resize(newLength);
  }
}

void QPACKHeaderTable::updateResizedTable(uint32_t oldTail,
                                          uint32_t oldLength,
                                          uint32_t newLength) {
  HeaderTable::updateResizedTable(oldTail, oldLength, newLength);
  if (refCount_) {
    std::move_backward(refCount_->begin() + oldTail,
                       refCount_->begin() + oldLength,
                       refCount_->begin() + newLength);
  }
}

uint32_t QPACKHeaderTable::evict(uint32_t needed, uint32_t desiredCapacity) {
  if (bytes_ + needed < desiredCapacity ||
      !canEvict(bytes_ + needed - desiredCapacity)) {
    return 0;
  }
  return HeaderTable::evict(needed, desiredCapacity);
}

bool QPACKHeaderTable::canEvict(uint32_t needed) {
  if (size_ == 0 || !refCount_) {
    return needed <= capacity_;
  }
  uint32_t freeable = 0;
  uint32_t i = tail();
  uint32_t nChecked = 0;
  while (nChecked++ < size() && freeable < needed &&
         ((*refCount_)[i] == 0) && // don't evict referenced or unacked headers
         internalToAbsolute(i) <= ackedInsertCount_) {
    freeable += table_[i].bytes();
    i = next(i);
  }
  if (freeable < needed) {
    DVLOG(5) << "header=" << table_[i].name << ":" << table_[i].value
             << " blocked eviction, recount=" << (*refCount_)[i];
    return false;
  }
  return true;
}

bool QPACKHeaderTable::isValid(uint32_t index, uint32_t base) const {
  int64_t testIndex = index;
  if (base > 0) {
    auto baseOffset = ((int64_t)base - (int64_t)insertCount_);
    // recompute relative to current insertCount_.  testIndex may go negative
    // if this is a reference to an entry that hasn't arrived yet
    testIndex -= baseOffset;
  }
  return HeaderTable::isValid(testIndex);
}

// Checks if relativeIndex is draining.  If not, returns the corresponding
// absolute index.  Otherwise, attempt to duplicate.  If duplication is
// successful, and vulnerable references are allowed, return absolute index of
// the duplicate.  If duplication is unsuccessful, or vulnerable references are
// not allowed, return 0.
std::pair<bool, uint32_t> QPACKHeaderTable::maybeDuplicate(
    uint32_t relativeIndex, bool allowVulnerable) {
  if (relativeIndex == UNACKED) {
    return {false, 0};
  }
  DCHECK(isValid(relativeIndex));
  uint32_t absIndex = relativeToAbsolute(relativeIndex);
  DCHECK(!isVulnerable(absIndex) || allowVulnerable);
  if (absIndex < minUsable_) {
    // draining
    const HPACKHeader& header = getHeader(relativeIndex);
    if (canIndex(header.name, header.value)) {
      CHECK(add(header.copy()));
      if (allowVulnerable) {
        return {true, insertCount_};
      } else {
        return {true, 0};
      }
    } else {
      return {false, 0};
    }
  }
  return {false, absIndex};
}

void QPACKHeaderTable::addRef(uint32_t absIndex) {
  // refCount is 16 bits.  It should really never get this big in practice,
  // unless a decoder is not sending HEADER_ACK in a timely way.
  CHECK(refCount_);
  (*refCount_)[absoluteToInternal(absIndex)]++;
}

void QPACKHeaderTable::subRef(uint32_t absIndex) {
  CHECK(refCount_);
  uint32_t index = absoluteToInternal(absIndex);
  CHECK_GT((*refCount_)[index], 0);
  (*refCount_)[index]--;
}

// Converts an array index in [0..table_.size() - 1] to an absolute
// external index
uint32_t QPACKHeaderTable::internalToAbsolute(uint32_t internalIndex) const {
  return relativeToAbsolute(toExternal(internalIndex));
}

// Converts an absolute index to an array index in [0..table_.size() - 1]
uint32_t QPACKHeaderTable::absoluteToInternal(uint32_t absoluteIndex) const {
  return toInternal(absoluteToRelative(absoluteIndex), 0);
}

uint32_t QPACKHeaderTable::toInternal(uint32_t externalIndex,
                                      uint32_t base) const {
  if (base > 0) {
    uint32_t absIndex = base - externalIndex + 1;
    externalIndex = absoluteToRelative(absIndex);
  }
  return HeaderTable::toInternal(externalIndex);
}

} // namespace proxygen
