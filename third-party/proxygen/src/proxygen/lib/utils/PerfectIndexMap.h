/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <glog/logging.h>

#include <folly/FBVector.h>
#include <folly/Optional.h>
#include <proxygen/lib/utils/UtilInl.h>
#include <string>

// TODO: consider changing API methods that take in an reference with versions
// that don't so that callers can give up ownership if they like via std::move
// operator (and subsequently we would use it as well).
// TODO: templatize the use of string as the value type; can instead use
// fbstring

namespace proxygen {

// A type of key-value map implemented by indexing values in a vector.
// Performance is on average that of an unordered_map or better when the
// index is used effectively.  CPU and memory usage are significantly reduced
// due to the use of a perfect hashing function and vectors.

// Key is a one byte (i.e. uint8_t) type.
// OtherKey represents the value of Key for which strings which
//    do not possess unique mappings are mapped to.
// NoneKey represents the value of Key which no strings map to;
//    field is reserved to denote deleted elements within the map.
// PerfectHashStrToKey represents the perfect hash mapping function used
//    internally to map string keys to their Key values
// AllowDuplicates represents a flag that controls whether the map supports
//    duplicate keys.  It is mainly used for optimization purposes as the
//    implementation of default handling adds additional useless work for such
//    cases in which duplicates are not tolerated.
template <typename Key,
          Key OtherKey,
          Key NoneKey,
          Key (*PerfectHashStrToKey)(const std::string &),
          bool AllowDuplicates,
          bool CaseInsensitive>
class PerfectIndexMap {
 public:
  static_assert(sizeof(Key) == 1, "Key must be of size 1 byte.");
  PerfectIndexMap() {
  }
  virtual ~PerfectIndexMap() = default;

  // Getters into the underlying map.

  folly::Optional<std::string> getSingleOrNone(
      const std::string &keyStr) const {
    auto key = PerfectHashStrToKey(keyStr);
    if (key == OtherKey) {
      const std::string *result = getSingleOtherKey(keyStr);
      return (result == nullptr ? folly::none
                                : folly::Optional<std::string>(*result));
    } else {
      return getSingleOrNone(key);
    }
  }

  folly::Optional<std::string> getSingleOtherKeyOrNone(
      const std::string &keyStr) const {
    CHECK(PerfectHashStrToKey(keyStr) == OtherKey);
    const std::string *result = getSingleOtherKey(keyStr);
    return (result == nullptr ? folly::none
                              : folly::Optional<std::string>(*result));
  }

  bool update(Key key,
              const std::string &checkValue,
              const std::string &newValue) {
    CHECK(key != OtherKey && key != NoneKey);
    std::string *getResult = getSingleKeyForUpdate(key);
    if (getResult != nullptr && stringsEqual(*getResult, checkValue)) {
      *getResult = newValue;
      return true;
    }
    return false;
  }

  folly::Optional<std::string> getSingleOrNone(Key key) const {
    CHECK(key != OtherKey && key != NoneKey);
    const std::string *result = getSingleKey(key);
    return (result == nullptr ? folly::none
                              : folly::Optional<std::string>(*result));
  }

  // Adders into the underlying map.

  void add(const std::string &keyStr, const std::string &value) {
    CHECK(AllowDuplicates);
    auto key = PerfectHashStrToKey(keyStr);
    if (key == OtherKey) {
      addOtherKeyToIndex(keyStr, value);
    } else {
      add(key, value);
    }
  }

  void add(Key key, const std::string &value) {
    CHECK(AllowDuplicates && key != OtherKey && key != NoneKey);
    addKeyToIndex(key, value);
  }

  // Setters into the underyling map.
  // Functionally, only one reference to the specified key remains after
  // the operation completes regardless of the state of AllowDuplicates

  void set(const std::string &keyStr, const std::string &value) {
    auto key = PerfectHashStrToKey(keyStr);
    if (key == OtherKey) {
      setOtherKey(keyStr, value);
    } else {
      set(key, value);
    }
  }

  void set(Key key, const std::string &value) {
    CHECK(key != OtherKey && key != NoneKey);
    setKey(key, value);
  }

  // Removers from the underlying map.
  // Functionally, all references to the specified key are removed after the
  // operation completes, regardless of the state of UnqiueEntries

  bool remove(const std::string &keyStr) {
    auto key = PerfectHashStrToKey(keyStr);
    if (key == OtherKey) {
      return removeOtherKey(keyStr);
    } else {
      return remove(key);
    }
  }
  bool remove(Key key) {
    CHECK(key != OtherKey && key != NoneKey);
    return removeKey(key);
  }

  size_t size() {
    return keys_.size() - noneKeyCount_;
  }

  void setOtherKey(const std::string &keyStr, const std::string &value) {
    bool set = false;
    size_t searchIndex = 0;
    while (searchForOtherKey(keyStr, searchIndex) != -1) {
      if (!set) {
        replaceOtherKeyAtIndex(searchIndex, keyStr, value);
        if (AllowDuplicates) {
          set = true;
        } else {
          return;
        }
      } else {
        removeAtIndex(otherKeyNamesKeysIndex_[searchIndex]);
      }
      ++searchIndex;
    }
    if (!set) {
      addOtherKeyToIndex(keyStr, value);
    }
  }

 private:
  // Utility method for comparing strings private to this class as specified
  // by template parameters.
  bool stringsEqual(const std::string &strA, const std::string &strB) const {
    if (CaseInsensitive) {
      // One might be tempted to merge this statement with that above
      // but that would be wrong.  If CaseInsensitive is true, we do not
      // want any other check evaluating.
      return caseInsensitiveEqual(strA, strB);
    } else {
      return strA == strB;
    }
  }
  // Private implementations of public methods for code reuse.
  // Technically this flow adds more copies (for ex in the case of passing
  // nullptr around as keyStr when not needed) and so slows things down a bit
  // but this way the code can effectively be reused and there is a single
  // implementation source.

  const std::string *getSingleKey(Key key) const {
    const Key *data = keys_.data();
    if (data) {
      auto offset = searchForKey(key, data);
      if (data) {
        if (AllowDuplicates && searchForKey(key, ++data) != -1) {
          return nullptr;
        }
        return &values_[offset];
      }
    }
    return nullptr;
  }
  const std::string *getSingleOtherKey(const std::string &keyStr) const {
    size_t searchIndex = 0;
    auto index = searchForOtherKey(keyStr, searchIndex);
    if (index != -1) {
      if (AllowDuplicates && searchForOtherKey(keyStr, ++searchIndex) != -1) {
        return nullptr;
      }
      return &values_[index];
    }
    return nullptr;
  }

  void setKey(Key key, const std::string &value) {
    const Key *data = keys_.data();
    bool set = false;
    std::ptrdiff_t offset;
    while (data) {
      offset = searchForKey(key, data);
      if (data) {
        if (!set) {
          replaceKeyAtIndex(offset, key, value);
          if (AllowDuplicates) {
            set = true;
          } else {
            return;
          }
        } else {
          removeAtIndex(offset);
        }
        ++data;
      }
    }
    if (!set) {
      addKeyToIndex(key, value);
    }
  }

  bool removeKey(Key key) {
    const Key *data = keys_.data();
    bool anyRemoved = false;
    std::ptrdiff_t offset;
    while (data) {
      offset = searchForKey(key, data);
      if (data) {
        removeAtIndex(offset);
        anyRemoved = true;
        if (!AllowDuplicates) {
          break;
        }
        ++data;
      }
    }
    return anyRemoved;
  }
  bool removeOtherKey(const std::string &keyStr) {
    bool anyRemoved = false;
    size_t searchIndex = 0;
    while (searchForOtherKey(keyStr, searchIndex) != -1) {
      removeAtIndex(otherKeyNamesKeysIndex_[searchIndex]);
      anyRemoved = true;
      if (!AllowDuplicates) {
        break;
      }
      ++searchIndex;
    }
    return anyRemoved;
  }

  // Utility methods for searching our index.  The data pointer / start index
  // are passed by reference so they can be updated by the search itself.  The
  // methods return on the first occurrence found from the specified start
  // searching position.
  std::ptrdiff_t searchForKey(Key key, const Key *&data) const {
    if (data) {
      data = (Key *)memchr(
          (void *)data, key, keys_.size() - (data - keys_.data()));
      if (data) {
        return data - keys_.data();
      }
    }
    return -1;
  }
  std::ptrdiff_t searchForOtherKey(const std::string &keyStr,
                                   size_t &startIndex) const {
    while (startIndex < otherKeyNamesKeysIndex_.size()) {
      // The key can only be OtherKey or NoneKey
      if (keys_[otherKeyNamesKeysIndex_[startIndex]] == OtherKey) {
        if (CaseInsensitive) {
          // One might be tempted to merge this statement with that above
          // but that would be wrong.  If CaseInsensitive is true, we do not
          // want any other check evaluating.
          if (caseInsensitiveEqual(otherKeyNames_[startIndex], keyStr)) {
            return otherKeyNamesKeysIndex_[startIndex];
          }
        } else {
          if (otherKeyNames_[startIndex] == keyStr) {
            return otherKeyNamesKeysIndex_[startIndex];
          }
        }
      }
      ++startIndex;
    }
    return -1;
  }

  // Utility methods for adding / modifying to our index.
  void addKeyToIndex(Key key, const std::string &value) {
    keys_.push_back(key);
    values_.emplace_back(value);
  }
  void addOtherKeyToIndex(const std::string &keyStr, const std::string &value) {
    keys_.push_back(OtherKey);
    otherKeyNames_.emplace_back(keyStr);
    otherKeyNamesKeysIndex_.push_back(keys_.size() - 1);
    values_.emplace_back(value);
    ++otherKeyCount_;
  }
  void replaceKeyAtIndex(std::ptrdiff_t index,
                         Key key,
                         const std::string &value) {
    keys_[index] = key;
    values_[index] = value;
  }
  void replaceOtherKeyAtIndex(size_t namesIndex,
                              const std::string &keyStr,
                              const std::string &value) {
    otherKeyNames_[namesIndex] = keyStr;
    values_[otherKeyNamesKeysIndex_[namesIndex]] = value;
  }

  // Utility methods for removing from our index.
  void removeAtIndex(std::ptrdiff_t index) {
    CHECK(keys_[index] != NoneKey);
    if (keys_[index] == OtherKey) {
      --otherKeyCount_;
    }
    keys_[index] = NoneKey;
    ++noneKeyCount_;
    // We purposefully omit actually clearing some other data structures
    // as it is often useful for debugging purposes to leave this data around.
  }
  std::string *getSingleKeyForUpdate(Key key) {
    const Key *data = keys_.data();
    if (data) {
      auto offset = searchForKey(key, data);
      if (data) {
        if (AllowDuplicates && searchForKey(key, ++data) != -1) {
          return nullptr;
        }
        return &values_[offset];
      }
    }
    return nullptr;
  }

  /*
   * Illustrating the below data structures, for a map where:
   *     otherKeyCount_ == 2
   *     noneKeyCount_ == 1
   *     keys_.size() == 5
   *     using OK = OtherKey
   *     using NK = NoneKey
   *     using K = some key which is not OK or NK
   *     using --- = out of bounds
   * The above config implies the size of our map is 4, with 2 OKs and 2 Ks.
   * Also take note from a debugging standpoint, the fact that no entry in
   * otherKeyNames_ maps back to index 3 (via otherKeyNamesKeysIndex_) suggests
   * the removed key (NK) was regular K, not an OK, before being removed.
   *
   * ------|-------|-------------------------|----------------|-----------------|
   * Index | keys_ | otherKeyNamesKeysIndex_ | otherKeyNames_ |     values_ |
   * ______|_______|_________________________|________________|_________________|
   *   0   |  OK1  |           0             |   <OK1_name>   |    <OK1_value> |
   *   1   |  K1   |           4             |   <OK2_name>   |     <K1_value> |
   *   2   |  K2   |          ---            |     ---        |     <K2_value> |
   *   3   |  NK   |          ---            |     ---        |     <NK_value> |
   *   4   |  OK2  |          ---            |     ---        |    <OK2_value> |
   *   5   |  ---  |          ---            |     ---        |       --- |
   *  ...  |  ...  |          ...            |     ...        |       ... |
   * ----------------------------------------------------------------------------
   */

  size_t otherKeyCount_{0};
  size_t noneKeyCount_{0};
  // And using the above two fields we can obviously infer what the
  // notOtherKeyCount is.

  // 1-byte Key vector used as a primary index into values_.
  // Thus keys_.size() == values_.size().
  // The total size of the map is thus always <= keys_.size().
  // The reason it is not always == is because the map supports removals and on
  // such operations, we do not resize this vector.
  folly::fbvector<Key> keys_;

  // A vector used to reverse lookup a particular OtherKey's entry
  // (in otherKeyNames_) position within keys_.
  // Strictly speaking: otherKeyNamesKeysIndex_.size() == otherKeyNames_.size().
  // Stated differently, the N-th OtherKey's name is in otherKeyNames_[N] and
  // its corresponding value is in values_(namesKeyIndex[N]) because
  // namesKeyIndex[N] refers to the entry's position in keys_.
  folly::fbvector<size_t> otherKeyNamesKeysIndex_;

  // Storage for names whose keys are mapped to OtherKey.
  // This vector can never have more elements than the total numer of OtherKey
  // entries in the map as it should be equal to this count.
  // Thus strictly speaking: otherKeyNames_.size() <= keys_.size() where the
  // only time they could ever be equal is if EVERY entry EVER inserted into
  // the map was an OtherKey.
  folly::fbvector<std::string> otherKeyNames_;

  // Storage for all values, OtherKey or other.
  // Thus values_.size() == keys_.size().
  folly::fbvector<std::string> values_;
};

} // namespace proxygen
