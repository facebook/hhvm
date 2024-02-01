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
#include "hphp/util/hash-map.h"
#include "hphp/util/mutex.h"
#include "hphp/util/optional.h"

#include "hphp/hhbbc/hhbbc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace hackc {
  struct FileSymbols;
}

struct UnitIndex;
struct VirtualFileSystemWriter;

struct SymbolRefEdge {
  const StringData* sym;
  const StringData* from;
  const StringData* to;
};

struct Package {
  Package(const std::string& root,
          TicketExecutor& executor,
          extern_worker::Client& client,
          bool coredump);

  size_t getTotalFiles() const { return m_total.load(); }

  Optional<std::chrono::microseconds> inputsTime() const {
    return m_inputMicros;
  }
  Optional<std::chrono::microseconds> ondemandTime() const {
    return m_ondemandMicros;
  }

  void addSourceFile(const std::string& fileName);
  void addInputList(const std::string& listFileName);
  void addStaticFile(const std::string& fileName);
  void addDirectory(const std::string& path);
  void addStaticDirectory(const std::string& path);

  void writeVirtualFileSystem(const std::string& path);

  // Configuration for index & parse workers. This should contain any runtime
  // options which can affect HackC (or the interface to it).
  struct Config {
    Config() = default;

    static Config make(bool coredump) {
      Config c;
      #define R(Opt) c.Opt = RO::Opt;
      UNITCACHEFLAGS()
      #undef R
      c.EvalAbortBuildOnCompilerError = RO::EvalAbortBuildOnCompilerError;
      c.EvalAbortBuildOnVerifyError = RO::EvalAbortBuildOnVerifyError;
      c.IncludeRoots = RO::IncludeRoots;
      c.coeffects = CoeffectsConfig::exportForParse();
      c.CoreDump = coredump;
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
        (coeffects)
        (CoreDump)
        ;
    }

    bool CoreDump;

  private:
    #define R(Opt) decltype(RuntimeOption::Opt) Opt;
    UNITCACHEFLAGS()
    #undef R
    bool EvalAbortBuildOnCompilerError;
    bool EvalAbortBuildOnVerifyError;
    decltype(RO::IncludeRoots) IncludeRoots;
    CoeffectsConfig coeffects;
  };

  // Metadata for a parse or index job. Just filenames that we need to
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

  struct DeclNames {
    std::vector<const StringData*> types;
    std::vector<const StringData*> funcs;
    std::vector<const StringData*> constants;
    std::vector<const StringData*> modules;

    bool empty() const {
      return types.empty() && funcs.empty() && constants.empty() &&
        modules.empty();
    }

    template <typename SerDe> void serde(SerDe& sd) {
      sd(types)
        (funcs)
        (constants)
        (modules);
    }
  };

  // Index information collected during parsing, used to construct
  // an autoload index for parse-on-demand.
  struct IndexMeta: DeclNames {
    // If not empty, indexing resulted in an ICE or had parse errors.
    std::string error;

    template<typename SerDe> void serde(SerDe& sd) {
      DeclNames::serde(sd);
      sd(error);
    }
  };

  // Metadata obtained during parsing. Includes SymbolRefs used to
  // drive on-demand parsing as well as toplevel decl names (from UEs)
  // for use later by HHBBC's whole-program Index.
  struct ParseMeta {
    // Unresolved symbols required by hackc before bytecode generation.
    DeclNames m_missing;

    // Unresolved symbols needed by the Unit at runtime but not before
    // bytecode generation. Used to find new files for parse on-demand.
    SymbolRefs m_symbol_refs;

    // If not empty, parsing resulted in an ICE and configuration
    // indicated that this should be fatal.
    std::string m_abort;

    // List of symbol names extracted from UnitEmitters.
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

    // File path copied from UnitEmitter::m_filepath
    const StringData* m_filepath{nullptr};

    // Name of the module that this unit belongs to
    const StringData* m_module_use{nullptr};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_missing)
        (m_symbol_refs)
        (m_abort)
        (m_definitions)
        (m_filepath)
        (m_module_use)
        ;
    }
  };

  // Serialized decls for a single file, along with the IndexMeta
  // enumerating the symbols defined in the file.
  struct UnitDecls {
    DeclNames symbols;
    std::string decls;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(symbols)
        (decls);
    }
  };

  using IndexCallback = std::function<
    void(std::string, IndexMeta, extern_worker::Ref<UnitDecls>)
  >;
  using IndexMetaVec = std::vector<IndexMeta>;

  folly::coro::Task<bool> index(const IndexCallback&);

  using UEVec = std::vector<std::unique_ptr<UnitEmitter>>;
  using FileMetaVec = std::vector<FileMeta>;
  using ParseMetaVec = std::vector<ParseMeta>;
  using FileData = std::tuple<extern_worker::Ref<std::string>,
                              extern_worker::Ref<RepoOptionsFlags>,
                              std::vector<extern_worker::Ref<UnitDecls>>>;

  using ParseCallback = std::function<folly::coro::Task<ParseMetaVec>(
    const extern_worker::Ref<Config>&,
    extern_worker::Ref<FileMetaVec>,
    std::vector<FileData>,
    extern_worker::Client::ExecMetadata
  )>;

  folly::coro::Task<bool> parse(const UnitIndex&, const ParseCallback&);

  // These are meant to be called from extern-worker Jobs to perform
  // the actual parsing.
  static void parseInit(const Config&, FileMetaVec);
  static IndexMetaVec indexFini();
  static ParseMetaVec parseFini();
  static UnitEmitterSerdeWrapper parseRun(const std::string&,
                                          const RepoOptionsFlags&,
                                          const std::vector<UnitDecls>&);

  using LocalCallback = std::function<folly::coro::Task<void>(UEVec)>;
  using ParseMetaItemsToSkipSet = hphp_fast_set<size_t>;
  using EmitCallBackResult = std::pair<ParseMetaVec, ParseMetaItemsToSkipSet>;
  using EmitCallback = std::function<
    folly::coro::Task<EmitCallBackResult>(const std::vector<std::filesystem::path>&)
  >;
  folly::coro::Task<bool> emit(const UnitIndex&, const EmitCallback&,
                               const LocalCallback&, const std::filesystem::path&);

private:

  struct FileAndSize {
    std::filesystem::path m_path;
    size_t size;
  };
  using FileAndSizeVec = std::vector<FileAndSize>;

  struct Group {
    std::vector<std::filesystem::path> m_files;
    size_t m_size{0};
  };

  using Groups = std::vector<Group>;

  struct GroupResult {
    Groups m_grouped;
    FileAndSizeVec m_ungrouped;
  };

  struct OndemandInfo {
    FileAndSizeVec m_files;
    std::vector<SymbolRefEdge> m_edges;
  };

  // Partition all files specified for this package into groups.
  // If filterFiles/Dirs==true, ignore excluded files and/or directories
  // according to options.
  folly::coro::Task<Groups> groupAll(bool filterFiles, bool filterDirs);
  folly::coro::Task<GroupResult>
  groupDirectories(std::string, bool filterFiles, bool filterDirs);
  void groupFiles(Groups&, FileAndSizeVec);

  folly::coro::Task<void> prepareInputs(Group,
    std::vector<std::filesystem::path>& paths,
    std::vector<FileMeta>& metas,
    std::vector<folly::coro::Task<extern_worker::Ref<RepoOptionsFlags>>>& options
  );

  folly::coro::Task<void> indexAll(const IndexCallback&);
  folly::coro::Task<void> indexGroups(const IndexCallback&, Groups);
  folly::coro::Task<void> indexGroup(const IndexCallback&, Group);

  folly::coro::Task<void> parseAll(const ParseCallback&, const UnitIndex&);
  folly::coro::Task<void> parseGroups(Groups, const ParseCallback&, const UnitIndex&);
  folly::coro::Task<void> parseGroup(Group, const ParseCallback&, const UnitIndex&);

  void resolveDecls(const UnitIndex&, const FileMetaVec&,
      const std::vector<ParseMeta>&, std::vector<FileData>&, size_t attempts);

  folly::coro::Task<void> emitAll(const EmitCallback&, const UnitIndex&,
                                  const std::filesystem::path&);
  folly::coro::Task<OndemandInfo>
  emitGroups(Groups, const EmitCallback&, const UnitIndex&);
  folly::coro::Task<OndemandInfo>
  emitGroup(Group, const EmitCallback&, const UnitIndex&);

  void resolveOnDemand(OndemandInfo&, const StringData* fromFile,
      const SymbolRefs&, const UnitIndex&, bool report = false);

  std::string m_root;

  folly_concurrent_hash_map_simd<std::string, bool> m_seenFiles;

  std::atomic<bool> m_failed;

  std::atomic<size_t> m_total;
  Optional<std::chrono::microseconds> m_inputMicros;
  Optional<std::chrono::microseconds> m_ondemandMicros;

  folly_concurrent_hash_map_simd<std::string, bool> m_filesToParse;
  std::set<std::string> m_directories;
  std::set<std::string> m_staticDirectories;
  hphp_fast_set<std::string> m_extraStaticFiles;
  folly_concurrent_hash_map_simd<
    std::string, std::string
  > m_discoveredStaticFiles;

  TicketExecutor& m_executor;
  extern_worker::Client& m_client;
  CoroAsyncValue<extern_worker::Ref<Config>> m_config;

  // Content-store for options: Map<hash(options), options>
  extern_worker::RefCache<SHA1, RepoOptionsFlags> m_repoOptions;
};

struct UnitIndex final {
  struct Locations {
    Locations(std::string rpath,
              extern_worker::Ref<Package::UnitDecls> ref)
      : rpath(std::move(rpath)), declsRef(std::move(ref))
    {}

    // Relative path to source file in local filesystem
    std::string rpath;

    // handle to serialized decls in extern blob store
    extern_worker::Ref<Package::UnitDecls> declsRef;
  };

  using TMap = folly_concurrent_hash_map_simd<
    const StringData*, std::shared_ptr<Locations>,
    string_data_hash,
    string_data_tsame
  >;
  using FMap = folly_concurrent_hash_map_simd<
    const StringData*, std::shared_ptr<Locations>,
    string_data_hash,
    string_data_fsame
  >;
  using Map = folly_concurrent_hash_map_simd<
    const StringData*, std::shared_ptr<Locations>
  >;

  // Returns true if any ParseMeta in parseMetas references a missing
  // symbol that is present in this index.
  bool containsAnyMissing(const Package::ParseMetaVec& parseMetas) const;

  TMap types;
  FMap funcs;
  Map constants;
  Map modules;
};

// Given the result of running `hackc::decls_to_symbols()`, create a
// `Package::IndexMeta` containing the toplevel symbol names.
Package::IndexMeta summary_of_symbols(const hackc::FileSymbols&);

///////////////////////////////////////////////////////////////////////////////
}
