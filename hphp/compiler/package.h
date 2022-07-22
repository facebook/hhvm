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

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "hphp/util/coro.h"
#include "hphp/util/extern-worker.h"
#include "hphp/util/file-cache.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/mutex.h"
#include "hphp/util/optional.h"

#include "hphp/hhbbc/hhbbc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Package {
  Package(const std::string& root,
          bool parseOnDemand,
          coro::TicketExecutor& executor,
          extern_worker::Client& client);

  using Callback = std::function<void(std::unique_ptr<UnitEmitter>)>;

  bool parse(const Callback&);

  size_t getTotalFiles() const { return m_total.load(); }

  Optional<std::chrono::microseconds> parsingInputsTime() const {
    return m_parsingInputs;
  }
  Optional<std::chrono::microseconds> parsingOndemandTime() const {
    return m_parsingOndemand;
  }

  void addSourceFile(const std::string& fileName);
  void addInputList(const std::string& listFileName);
  void addStaticFile(const std::string& fileName);
  void addDirectory(const std::string& path);
  void addStaticDirectory(const std::string& path);

  std::shared_ptr<FileCache> getFileCache();

  struct Config;

private:

  struct FileAndSize {
    std::filesystem::path m_path;
    size_t size;
  };
  using FileAndSizeVec = std::vector<FileAndSize>;

  struct ParseGroup {
    std::vector<std::filesystem::path> m_files;
    size_t m_size{0};
  };

  using ParseGroups = std::vector<ParseGroup>;

  struct GroupResult {
    ParseGroups m_grouped;
    FileAndSizeVec m_ungrouped;
  };

  void parseAll();

  coro::Task<GroupResult> groupDirectories(std::string);
  void groupFiles(ParseGroups&, FileAndSizeVec);

  coro::Task<FileAndSizeVec> parseGroups(ParseGroups);
  coro::Task<FileAndSizeVec> parseGroup(ParseGroup);

  void resolveOnDemand(FileAndSizeVec&, const SymbolRefs&, bool report = false);

  std::string m_root;

  folly_concurrent_hash_map_simd<std::string, bool> m_parsedFiles;

  std::atomic<bool> m_parseFailed;

  bool m_parseOnDemand;

  std::atomic<size_t> m_total;
  Optional<std::chrono::microseconds> m_parsingInputs;
  Optional<std::chrono::microseconds> m_parsingOndemand;

  const Callback* m_callback;

  folly_concurrent_hash_map_simd<std::string, bool> m_filesToParse;
  std::shared_ptr<FileCache> m_fileCache;
  std::set<std::string> m_directories;
  std::set<std::string> m_staticDirectories;
  hphp_fast_set<std::string> m_extraStaticFiles;
  folly_concurrent_hash_map_simd<
    std::string, std::string
  > m_discoveredStaticFiles;

  coro::TicketExecutor& m_executor;
  extern_worker::Client& m_client;
  coro::AsyncValue<extern_worker::Ref<Config>> m_config;
  extern_worker::RefCache<SHA1, RepoOptionsFlags> m_repoOptions;
};

///////////////////////////////////////////////////////////////////////////////
}
