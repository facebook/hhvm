/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/unit-parser.h"

#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"
#include "hphp/util/logger.h"
#include "hphp/util/sqlite-wrapper.h"

#include <boost/algorithm/string/predicate.hpp>

// Simple replacement for the old Sandbox HHBC repo using the
// UnitEmitter cache hook infrastructure.

namespace HPHP {

namespace {

struct State {
  explicit State(const std::string&);

  static SQLite connect(const std::string&);

  SQLite m_db;
  SQLiteStmt m_find;
  SQLiteStmt m_insert;

  std::mutex m_lock;
};

std::unique_ptr<State> s_state;

///////////////////////////////////////////////////////////////////////////////

State::State(const std::string& path)
  : m_db{connect(path)}
  , m_find{m_db.prepare("SELECT data FROM units WHERE hash = @hash")}
  , m_insert{m_db.prepare("INSERT OR IGNORE INTO units (hash, data) "
                          "VALUES (@hash, @data)")}
{}

SQLite State::connect(const std::string& path) {
  SQLite db{SQLite::connect(path)};

  db.setJournalMode(SQLite::JournalMode::WAL);
  db.setSynchronousLevel(SQLite::SynchronousLevel::NORMAL);
  // No point in waiting long for the database since we can just
  // re-parse it.
  db.setBusyTimeout(500);

  auto txn = db.begin();
  txn.exec(
    "CREATE TABLE IF NOT EXISTS units ("
    " hash TEXT UNIQUE,"
    " data BLOB,"
    " PRIMARY KEY (hash)"
    ")"
  );
  txn.commit();

  return db;
}

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<UnitEmitter> cache_hook(
  const char* filenamePtr,
  const SHA1& sha1,
  folly::StringPiece::size_type codeLen,
  HhvmDeclProvider* provider,
  const std::function<std::unique_ptr<UnitEmitter>(bool)>& compile
) {
  assertx(s_state);

  std::string filename{filenamePtr};
  if (filename.empty() || boost::ends_with(filename, EVAL_FILENAME_SUFFIX)) {
    return compile(true);
  }

  auto const hashStr = sha1.toString();

  auto ue = [&] () -> std::unique_ptr<UnitEmitter> {
    std::lock_guard<std::mutex> _{s_state->m_lock};
    try {
      auto txn = s_state->m_db.begin();
      auto query = txn.query(s_state->m_find);
      query.bindString("@hash", hashStr);
      query.step();
      if (!query.row()) return nullptr;
      const void* blob;
      size_t blobSize;
      query.getBlob(0, blob, blobSize);

      auto const& packageInfo = RepoOptions::forFile(filename).packageInfo();
      auto ue = std::make_unique<UnitEmitter>(sha1, SHA1{}, packageInfo);
      BlobDecoder decoder{blob, blobSize};
      ue->m_filepath = makeStaticString(filename);
      ue->serde(decoder, false);
      decoder.assertDone();
      return ue;
    } catch (const SQLiteExc& exn) {
      if (exn.code() != SQLiteExc::Code::BUSY) {
        Logger::FError("Error reading unit for {} from unit file cache: {}",
                       filename, exn.what());
      }
      return nullptr;
    }
  }();

  if (ue) {
    if (ue->m_ICE) return compile(true);
    return ue;
  }

  ue = compile(true);
  {
    BlobEncoder encoder;
    ue->serde(encoder, false);

    std::lock_guard<std::mutex> _{s_state->m_lock};
    try {
      auto txn = s_state->m_db.begin();
      auto insert = txn.query(s_state->m_insert);
      insert.bindString("@hash", hashStr);
      insert.bindBlob("@data", encoder.data(), encoder.size());
      insert.step();
      txn.commit();
    } catch (const SQLiteExc& exn) {
      if (exn.code() != SQLiteExc::Code::BUSY) {
        Logger::FError("Error committing unit for {} to unit file cache: {}",
                       filename, exn.what());
      }
    }
  }

  ue->finish();
  return ue;
}

///////////////////////////////////////////////////////////////////////////////

static struct UnitFileCacheExtension final : Extension {
  UnitFileCacheExtension() : Extension("unitfilecache", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {
    static std::string filename;

    Config::Bind(filename, ini, config, "UnitFileCache.Path", filename);

    if (!filename.empty()) {
      always_assert(!HPHP::g_unit_emitter_cache_hook);

      replacePlaceholders(filename);

      try {
        s_state = std::make_unique<State>(filename);
        HPHP::g_unit_emitter_cache_hook = &cache_hook;
      } catch (const SQLiteExc& exn) {
        if (exn.code() != SQLiteExc::Code::BUSY) {
          Logger::FError("Unable to open unit file cache at {}: {}",
                         filename,
                         exn.what());
        }
      }
    }
  }

  std::vector<std::string> hackFiles() const override {
    return {};
  }
} s_unitfilecache_extension;

///////////////////////////////////////////////////////////////////////////////

}}
