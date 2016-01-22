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
#include "hphp/runtime/ext/apc/snapshot.h"

#include <sys/mman.h> // mmap
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <folly/String.h>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/concurrent-shared-store.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/fb/ext_fb.h" // fb_unserialize
#include "hphp/util/logger.h"

namespace HPHP {

void SnapshotBuilder::writeToFile(const std::string& filename) {
  m_file = fopen(filename.c_str(), "w+");
  if (!m_file) {
    Logger::Error("Failed to open snapshot file %s", filename.c_str());
    exit(1);
  }
  // Invalid header for now, to avoid truncated output due to errors looking
  // valid (and to allow computing diskOffset while serializing).
  writeRaw(SnapshotHeader::makeEmpty());
  // Index
  write(m_ints);
  write(m_chars);
  write(m_stringMem);
  write32(m_stringDisk.size());
  for (const auto& p : m_stringDisk) {
    write(p.first);
    // KeyValuePair uses sign of sSize for type info; preserve it here...
    write32(p.second.sSize);
  }
  // Disk
  auto diskOffset = ftello(m_file);
  for (const auto& p : m_stringDisk) {
    // ...but remember to ignore it when we need the actual size.
    writeRaw(p.second.sAddr, abs(p.second.sSize) + 1); // \0
  }
  // Write real header, as we are confident the output is complete.
  auto totalSize = ftello(m_file);
  rewind(m_file);
  writeRaw(SnapshotHeader::makeValid(diskOffset, totalSize));
  fclose(m_file);
  m_file = nullptr;
}

bool SnapshotLoader::tryInitializeFromFile(const char* filename) {
  m_fd = open(filename, O_RDONLY);
  if (m_fd == -1) {
    Logger::Error("Failed to open snapshot file %s", filename);
    return false;
  }
  SnapshotHeader header = SnapshotHeader::makeEmpty();
  auto bytesRead = ::read(m_fd, &header, sizeof(header));
  if (bytesRead != sizeof(header) || !header.isValid()) {
    // Info rather than error, since probing the format is OK.
    Logger::Info("No valid snapshot header in %s", filename);
    close(m_fd);
    return false;
  }
  if (header.version > SnapshotHeader::kLatestVersion) {
    Logger::Error("Unsupported snapshot version %" PRId64 " in %s",
                  header.version, filename);
    close(m_fd);
    return false;
  }
  m_size = lseek(m_fd, 0, SEEK_END);
  if (m_size != header.totalSize) {
    Logger::Error("Incorrect file size in %s", filename);
    close(m_fd);
    return false;
  }
  lseek(m_fd, 0, SEEK_SET);
  m_begin = (char*)mmap(nullptr, m_size, PROT_READ, MAP_SHARED, m_fd, 0);
  if (m_begin == (char*)-1) {
    Logger::Error("Failed to mmap %s", filename);
    close(m_fd);
    return false;
  }
  m_cur = m_begin + sizeof(SnapshotHeader);
  return true;
}

typedef ConcurrentTableSharedStore::KeyValuePair KeyValuePair;

void SnapshotLoader::load(ConcurrentTableSharedStore& s) {
  // This could share code with apc_load_impl_compressed, but that function
  // should go away together with the shared object format.
  {
    std::vector<KeyValuePair> ints(read32());
    for (auto& item : ints) {
      item.key = readString().begin();
      s.constructPrime(read64(), item);
    }
    s.prime(ints);
  }
  {
    std::vector<KeyValuePair> chars(read32());
    for (auto& item : chars) {
      item.key = readString().begin();
      switch (static_cast<SnapshotBuilder::CharBasedType>(read<char>())) {
        case SnapshotBuilder::kSnapFalse:
          s.constructPrime(false, item);
          break;
        case SnapshotBuilder::kSnapTrue:
          s.constructPrime(true, item);
          break;
        case SnapshotBuilder::kSnapNull:
          s.constructPrime(uninit_null(), item);
          break;
        default:
          assert(false);
          break;
      }
    }
    s.prime(chars);
  }
  auto numStringBased = read32();
  CHECK(numStringBased == SnapshotBuilder::kNumStringBased);
  for (int i = 0; i < numStringBased; ++i) {
    auto type = static_cast<SnapshotBuilder::StringBasedType>(i);
    std::vector<KeyValuePair> items(read32());
    for (auto& item : items) {
      item.key = readString().begin();
      auto data = readString();
      String value(data.begin(), data.size(), CopyString);
      switch (type) {
        case SnapshotBuilder::kSnapString:
          s.constructPrime(value, item, false);
          break;
        case SnapshotBuilder::kSnapObject:
          s.constructPrime(value, item, true);
          break;
        case SnapshotBuilder::kSnapThrift: {
          Variant success;
          Variant v = HHVM_FN(fb_unserialize)(value, ref(success));
          if (same(success, false)) {
            throw Exception("bad apc archive, fb_unserialize failed");
          }
          s.constructPrime(v, item);
          break;
        }
        case SnapshotBuilder::kSnapOther: {
          Variant v = unserialize_from_string(value);
          if (same(v, false)) {
            throw Exception("bad apc archive, unserialize_from_string failed");
          }
          s.constructPrime(v, item);
          break;
        }
        default:
          assert(false);
          break;
      }
    }
    s.prime(items);
  }
  {
    const char* disk = m_begin + header().diskOffset;
    std::vector<KeyValuePair> items(read32());
    for (auto& item : items) {
      item.key = readString().begin();
      item.sSize = read32();
      item.sAddr = const_cast<char*>(disk);
      disk += abs(item.sSize) + 1;  // \0
    }
    assert(disk == m_begin + m_size);
    s.prime(items);
  }
  // TODO(9755912): Use 'madvise' to indicate we are done with Index now,
  // or better+harder: avoid copying the keys and guard against freeing them.
  assert(m_cur == m_begin + header().diskOffset);
}

}
