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

#include "hphp/runtime/base/coeffects-config.h"
#include "hphp/runtime/base/unit-cache.h"

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

  // Configuration for parse workers. This should contain any runtime
  // options which can affect HackC (or the interface to it).
  struct Config {
    Config() = default;

    static Config make() {
      Config c;
      #define R(Opt) c.Opt = RO::Opt;
      UNITCACHEFLAGS()
        #undef R
        c.EvalAbortBuildOnCompilerError = RO::EvalAbortBuildOnCompilerError;
      c.EvalAbortBuildOnVerifyError = RO::EvalAbortBuildOnVerifyError;
      c.IncludeRoots = RO::IncludeRoots;
      c.coeffects = CoeffectsConfig::exportForParse();
      return c;
    }

    void apply() const {
      #define R(Opt) RO::Opt = Opt;
      UNITCACHEFLAGS()
        #undef R
        RO::EvalAbortBuildOnCompilerError = EvalAbortBuildOnCompilerError;
      RO::EvalAbortBuildOnVerifyError = EvalAbortBuildOnVerifyError;
      RO::IncludeRoots = IncludeRoots;
      CoeffectsConfig::importForParse(coeffects);
    }

    template <typename SerDe> void serde(SerDe& sd) {
      #define R(Opt) sd(Opt);
      UNITCACHEFLAGS()
        #undef R
        sd(EvalAbortBuildOnCompilerError)
        (EvalAbortBuildOnVerifyError)
        (IncludeRoots)
        (coeffects);
    }

  private:
    #define R(Opt) decltype(RuntimeOption::Opt) Opt;
    UNITCACHEFLAGS()
    #undef R
    bool EvalAbortBuildOnCompilerError;
    bool EvalAbortBuildOnVerifyError;
    decltype(RO::IncludeRoots) IncludeRoots;
    CoeffectsConfig coeffects;
  };

  // Metadata for a parse job. Just filename things that we need to
  // resolve when we have the whole source tree available.
  struct FileMeta {
    FileMeta() = default;
    FileMeta(std::string f, Optional<std::string> o)
      : m_filename{std::move(f)}, m_targetPath{std::move(o)} {}

    // The (relative) filename of the file
    std::string m_filename;
    // If the file is a symlink, what its target is
    Optional<std::string> m_targetPath;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_filename)
        (m_targetPath);
    }
  };

  // Metadata obtained during parsing. Mainly symbol refs used to
  // drive on-demand parsing.
  struct ParseMeta {
    // Symbols present in the unit. This will be used to find new files
    // for parse on-demand.
    SymbolRefs m_symbol_refs;
    // If not empty, parsing resulted in an ICE and configuration
    // indicated that this should be fatal.
    std::string m_abort;

    struct Definitions {
      std::vector<const StringData*> m_classes;
      std::vector<const StringData*> m_enums;
      std::vector<const StringData*> m_funcs;
      std::vector<const StringData*> m_methCallers;
      std::vector<const StringData*> m_typeAliases;
      std::vector<const StringData*> m_constants;
      std::vector<const StringData*> m_modules;

      template <typename SerDe> void serde(SerDe& sd) {
        sd(m_classes)
          (m_enums)
          (m_funcs)
          (m_methCallers)
          (m_typeAliases)
          (m_constants)
          (m_modules);
      }
    };
    Definitions m_definitions;
    const StringData* m_filepath{nullptr};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_symbol_refs)
        (m_abort)
        (m_definitions)
        (m_filepath);
    }
  };

  using UEVec = std::vector<std::unique_ptr<UnitEmitter>>;
  using FileMetaVec = std::vector<FileMeta>;
  using ParseMetaVec = std::vector<ParseMeta>;
  using FileData = std::tuple<extern_worker::Ref<std::string>,
                              extern_worker::Ref<RepoOptionsFlags>>;

  using LocalCallback = std::function<coro::Task<void>(UEVec)>;
  using Callback = std::function<coro::Task<ParseMetaVec>(
    const extern_worker::Ref<Config>&,
    extern_worker::Ref<FileMetaVec>,
    std::vector<FileData>,
    bool
  )>;

  coro::Task<bool> parse(const Callback&, const LocalCallback&);

  // These are meant to be called from extern-worker Jobs to perform
  // the actual parsing.
  static void parseInit(const Config&, FileMetaVec);
  static ParseMetaVec parseFini();
  static UnitEmitterSerdeWrapper parseRun(const std::string&,
                                          const RepoOptionsFlags&);
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

  coro::Task<void> parseAll();

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
