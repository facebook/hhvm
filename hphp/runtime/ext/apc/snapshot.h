/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2016 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_SNAPSHOT_H_
#define incl_HPHP_SNAPSHOT_H_

#include <stdio.h>
#include <string>
#include <utility>
#include <vector>

#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/logger.h"

namespace HPHP {

/*
 * Snapshot format
 *
 * The three sections of a serialized snapshot, and their current contents, are:
 *   1. Header - fixed-size metadata
 *   2. Index - all APC prime keys, and those values that always go into RAM
 *   3. Disk - all remaining APC prime values
 * When loading, the Index is fully parsed, but the Disk is just lazily mapped.
 */
struct SnapshotHeader {
  SnapshotHeader() = delete;  // Use factories below.
  static SnapshotHeader makeEmpty() { return SnapshotHeader(0, 0, 0, 0); }
  static SnapshotHeader makeValid(int64_t diskOffsetBytes, int64_t total) {
    return SnapshotHeader(kMagic, kLatestVersion, diskOffsetBytes, total);
  }
  bool isValid() const { return magic == kMagic; }
 private:
  SnapshotHeader(int64_t m, int64_t v, int64_t d, int64_t t)
      : magic(m), version(v), diskOffset(d), totalSize(t) {}
  static constexpr int64_t kMagic = 0xA31415926535C;
 public:
  static constexpr int64_t kLatestVersion = 1;
  const int64_t magic;
  const int64_t version;
  const int64_t diskOffset;  // Number of bytes from file start to Disk section.
  const int64_t totalSize;
};

/*
 * Accepts data to build a snapshot, and then serializes it into a file.
 * Usage: Call the 'add*' methods to build the state, then finally writeToFile.
 * Note: Not thread-safe. No dependencies or side-effects on globals/statics.
 */
struct SnapshotBuilder {
  typedef ConcurrentTableSharedStore::KeyValuePair KeyValuePair;

  enum StringBasedType {
    kSnapString = 0,
    kSnapObject,
    kSnapThrift,
    kSnapOther,
    kNumStringBased
  };
  enum CharBasedType {
    kSnapFalse,
    kSnapTrue,
    kSnapNull
  };

  SnapshotBuilder() = default;

  /*
   * Add APC priming entries from ConcurrentTableSharedStore::constructPrime.
   * Does not use item.value - 'v' contains the value (for object/thrift/other,
   * in their respective serialized format).
   */
  void addInt(int64_t v, KeyValuePair item) {
    m_ints.emplace_back(item.key, v);
  }
  void addString(const String& v, KeyValuePair item) {
    add(kSnapString, v, item);
  }
  void addFalse(KeyValuePair item) { add(kSnapFalse, item); }
  void addTrue(KeyValuePair item) { add(kSnapTrue, item); }
  void addNull(KeyValuePair item) { add(kSnapNull, item); }
  void addObject(const String& v, KeyValuePair item) {
    add(kSnapObject, v, item);
  }
  void addThrift(const String& v, KeyValuePair item) {
    add(kSnapThrift, v, item);
  }
  void addOther(const String& v, KeyValuePair item) {
    add(kSnapOther, v, item);
  }

  void writeToFile(const std::string& filename);

 private:
  void add(StringBasedType type, const String& v, KeyValuePair item) {
    if (item.inMem()) {
      m_stringMem[type].emplace_back(item.key, v.slice().toString());
    } else {
      m_stringDisk.emplace_back(item.key, item);
    }
  }
  void add(CharBasedType type, KeyValuePair item) {
    m_chars.emplace_back(item.key, static_cast<char>(type));
  }

  // Serialization primitives for snapshot format.
  void write(const std::string& s) {
    // Strings are both prefixed by length (to allow internal nulls), and also
    // null-terminated (to be compatible with existing code without copying).
    write32(s.size());
    writeRaw(s.c_str(), s.size() + 1); // \0
  }
  template<class T> void write(const std::pair<std::string, T>& p) {
    write(p.first);
    write(p.second);
  }
  template<class T> void write(const std::vector<T>& v) {
    write32(v.size());
    for (auto& p : v) write(p);
  }
  void write(char ch) { writeRaw(ch); }
  void write(int64_t i) { writeRaw(i); }
  void write32(int32_t i) { writeRaw(i); }
  template<class T> void writeRaw(T t) { writeRaw(&t, sizeof(t)); }
  void writeRaw(const void* p, size_t n) {
    if (::fwrite(p, 1, n, m_file) != n) {
      Logger::Error("Failed to write snapshot");
      exit(1);
    }
  }

  // Index.
  std::vector<std::pair<std::string, int64_t> > m_ints;
  std::vector<std::pair<std::string, char> > m_chars;
  std::vector<std::vector<std::pair<std::string, std::string> > >
    m_stringMem{kNumStringBased};
  // Disk.
  std::vector<std::pair<std::string, KeyValuePair> > m_stringDisk;

  FILE* m_file{nullptr};
};

/*
 * Used to map+load the content of a snapshot during system startup.
 * Usage: Initialize (probing file format), then 'load'.
 */
struct SnapshotLoader {
  // Returns false if unable to read a valid header.
  bool tryInitializeFromFile(const char* filename);
  // Loads the contents from the snapshot (and leaves the Disk mapped).
  void load(ConcurrentTableSharedStore& s);
 private:
  const SnapshotHeader& header() const {
    return *reinterpret_cast<const SnapshotHeader*>(m_begin);
  }

  // Deserialization primitives for snapshot format.
  folly::StringPiece readString() {
    auto size = read32();
    auto s = m_cur;
    m_cur += size + 1; // \0
    return folly::StringPiece(s, size);
  }
  int32_t read32() { return read<int32_t>(); }
  int64_t read64() { return read<int64_t>(); }
  template<class T> const T& read() {
    m_cur += sizeof(T);
    return reinterpret_cast<const T*>(m_cur)[-1];
  }

  const char* m_begin{nullptr};
  const char* m_cur{nullptr};
  int64_t m_size{-1};
  int m_fd{-1};
};

}

#endif // incl_HPHP_SNAPSHOT_H_
