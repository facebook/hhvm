/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/vm/source-location.h"

#include "hphp/util/compact-tagged-ptrs.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ArrayData;
struct RepoAutoloadMapBuilder;
struct RepoGlobalData;
struct SHA1;
struct StringData;
struct UnitEmitter;

namespace Native {
struct FuncTable;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * RepoFileBuilder is used to create a RepoFile (you can either use a
 * RepoFile, or create one, there's no simultaneous creation/use).
 *
 * Usage is simple. Instantiate one, call add() with as many
 * UnitEmitters as needed, then call finish().
 *
 * System errors (for example, while writing to the file) result in
 * always_assert. This avoids having to write exception handling code,
 * and such errors are necessarily fatal anyways in the context where
 * we're using this.
 *
 * Unless stated otherwise, no member functions on RepoFileBuilder are
 * meant to be called concurrently and are meant to be used in a
 * single-threaded manner.
 */

struct RepoFileBuilder {
  // Create a new repo file at the given path. The file will actually
  // be created with a temporary name, then renamed to the given name
  // when finish() is called.
  explicit RepoFileBuilder(const std::string& path);
  ~RepoFileBuilder();

  // Encoding an UnitEmitter is expensive, so one can do it ahead of
  // time (in different threads), then add it directly.
  struct EncodedUE {
    explicit EncodedUE(const UnitEmitter& ue);
   private:
    friend struct RepoFileBuilder;
    const StringData* path;
    int64_t sn;
    std::vector<char> blob;
  };

  // Add the given UnitEmitter to the repo file. This will encode it
  // and then write it to disk. This can only be called before
  // finish() is called.
  void add(const EncodedUE& ue);
  void add(const UnitEmitter& ue) { add(EncodedUE{ue}); }

  // "Finish" the repo file by writing RepoGlobalData, the
  // RepoAutoloadMapBuilder, the literal string table, and indexing
  // information into the file. Once successful, the temporary file is
  // renamed to its final name. Once finish() is called, nothing can
  // be done with the RepoFileBuilder except destroy it.
  void finish(const RepoGlobalData&, const RepoAutoloadMapBuilder&);

  RepoFileBuilder(const RepoFileBuilder&) = delete;
  RepoFileBuilder(RepoFileBuilder&&) = delete;
  RepoFileBuilder& operator=(const RepoFileBuilder&) = delete;
  RepoFileBuilder& operator=(RepoFileBuilder&&) = delete;
private:
  struct Data;
  std::unique_ptr<Data> m_data;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Reader equivalent of RepoFileBuilder. Reads an existing repo file
 * and makes it available for querying.
 *
 * RepoFile is a strong singleton. It is not actually possible to
 * instantiate it, and all interaction is done with static class
 * methods.
 *
 * Like RepoFileBuilder, system errors result in always_assert as we
 * don't have a good way to recover from such errors.
 */
struct RepoFile {
  // To support lazy-loading, RepoFile needs to know where to load
  // certain pieces of data. It is the responsibility of the caller to
  // provide such offsets. The offset is abstracted away as a "token"
  // which the callers can store.
  using Token = uint64_t;

  // Initialize the repo file at the given path. This opens the file
  // and loads some indexing information. No other functions on
  // RepoFile can be called until the repo is initialized. This
  // function cannot be called concurrently.
  static void init(const std::string& path);
  // "Destroys" the repo file. This releases any open file descriptors
  // on the file and destroys any indexing information. If the repo
  // file hasn't been initialized, this does nothing. This function
  // cannot be called concurrently.
  static void destroy();

  // Called after a fork. Tears down RepoFile state and re-initializes
  // it.
  static void postfork();

  // Retrieve the RepoGlobalData which was stored in this repo
  // file. The data is actually loaded during init(), so this does no
  // file I/O. Unlike other querying functions, this does not require
  // loadGlobalTables() be called.
  static const RepoGlobalData& globalData();

  /*
   * Load the various global tables stored in the repo file. This
   * includes the literal string table, the global array type table,
   * the repo autoload map, and various unit emitter indexing
   * information.
   *
   * This must be called after init(), but before any of the below
   * querying functions are called. It can only be called once, and
   * cannot be called concurrently.
   *
   * If "lazyLitstr" is true, the literal strings are not actually
   * loaded, and will be loaded on demand.
   */
  static void loadGlobalTables(bool lazyLiterals);

  /*
   * Query functions:
   *
   * Unlike otherwise stated, these all can be called concurrently,
   * and must be called only after loadGlobalTables() is called.
   */

  // Load a literal string on-demand. The given index is the literal's
  // position in the literal string table.
  static StringData* loadLitstr(size_t);

  // Load a literal array on-demand. The given index is the literal's
  // position in the literal array table.
  static ArrayData* loadLitarray(size_t);

  // Return the paths corresponding to all UnitEmitters in the repo
  // file.
  static std::vector<const StringData*> enumerateUnits();

  // Load the UnitEmitter with the given path `searchPath`, returning
  // nullptr if none exists. The UnitEmitter will be assigned the path
  // `path' (it might be different from `searchPath` due to SourceRoot
  // issues). If `lazy` is true, the bytecode and line tables are not
  // loaded with the UnitEmitter and will instead be loaded on demand.
  static std::unique_ptr<UnitEmitter>
  loadUnitEmitter(const StringData* searchPath,
                  const StringData* path,
                  const Native::FuncTable& nativeFuncs,
                  bool lazy);

  // Load the bytecode for the UnitEmitter given by the SN. The token
  // gives the offset of the bytecode. The bytecode will be read into
  // the given buffer (the given buffer length must match the size of
  // the bytecode). This is used for lazy bytecode loading (if lazy
  // loading was specified when the UnitEmitter was loaded).
  static void loadBytecode(int64_t unitSn,
                           Token token,
                           unsigned char* bc,
                           size_t bclen);

  // Load the line table for the UnitEmitter given by the SN. The
  // token gives the offset of the line table. This is used for lazy
  // loading line tables (if lazy loading was specified when the
  // UnitEmitter was loaded).
  static LineTable loadLineTable(int64_t unitSn,
                                 Token token);

  // Map an UnitEmitter's path to its associated SN (returning -1 if
  // no such UnitEmitter exists).
  static int64_t findUnitSN(const StringData* path);
  // Map an UnitEmitter's SN to its associated path (returning nullptr
  // if no such UnitEmitter exists).
  static const StringData* findUnitPath(int64_t unitSn);
  // Map an UnitEmitter's SHA1 to its associated path (returning
  // nullptr if no such UnitEmitter exists). NB: SHA1 isn't really "a
  // thing" for RepoFile. We assume the SHA1 matches the SN. This is
  // only provided for compatibility for tc-print.
  static const StringData* findUnitPath(const SHA1&);

  RepoFile() = delete;
  RepoFile(const RepoFile&) = delete;
  RepoFile(RepoFile&&) = delete;
  RepoFile& operator=(const RepoFile&) = delete;
  RepoFile& operator=(RepoFile&&) = delete;
  ~RepoFile() = delete;
};

///////////////////////////////////////////////////////////////////////////////

// Represents either a pointer to T, or a RepoFile::Token. Does not do
// any resource management of its own (it won't clone or delete the
// memory pointed to by the T*). This always keeps the lower 2-bits
// free so it can be used in a LockFreePtrWrapper.
template <typename T>
struct TokenOrPtr {
  TokenOrPtr() : m_compact{toOpaquePtr(nullptr)} {}

  using Token = RepoFile::Token;

  bool isPtr() const { return getCompact().tag() == 0; }
  bool isToken() const { return getCompact().tag() == 1; }

  T* ptr() const {
    assertx(isPtr());
    return getCompact().ptr();
  }
  Token token() const {
    assertx(isToken());
    return (Token)(uintptr_t)getCompact().ptr();
  }

  static TokenOrPtr FromPtr(T* ptr) {
    return TokenOrPtr{toOpaquePtr(ptr)};
  }
  static TokenOrPtr FromToken(Token token) {
    return TokenOrPtr{toOpaqueToken(token)};
  }
private:
  using Compact = CompactTaggedPtr<T, uint8_t>;
  typename Compact::Opaque m_compact;

  explicit TokenOrPtr(typename Compact::Opaque o) : m_compact{o} {}

  Compact getCompact() const { return Compact{m_compact >> 2}; }

  static typename Compact::Opaque toOpaquePtr(T* p) {
    auto const c = Compact{0, p}.getOpaque();
    assertx(((c << 2) >> 2) == c);
    return c << 2;
  }
  static typename Compact::Opaque toOpaqueToken(Token token) {
    auto const c = Compact{1, (T*)(uintptr_t)token}.getOpaque();
    assertx(((c << 2) >> 2) == c);
    return c << 2;
  }
};

///////////////////////////////////////////////////////////////////////////////

}
