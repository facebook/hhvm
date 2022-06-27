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

#include "hphp/compiler/compiler.h"

#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/emitter.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/version.h"

#include "hphp/util/async-func.h"
#include "hphp/util/build-info.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/exception.h"
#include "hphp/util/hdf.h"
#include "hphp/util/logger.h"
#include "hphp/util/process.h"
#include "hphp/util/process-exec.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/text-util.h"
#include "hphp/util/timer.h"
#ifndef _MSC_VER
#include "hphp/util/light-process.h"
#endif

#include "hphp/hhvm/process-init.h"

#include <sys/types.h>
#ifndef _MSC_VER
#include <sys/wait.h>
#include <dlfcn.h>
#endif

#include <boost/algorithm/string/replace.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#include <exception>

#include <folly/portability/SysStat.h>

using namespace boost::program_options;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CompilerOptions {
  std::string format;
  std::string outputDir;
  std::vector<std::string> config;
  std::vector<std::string> confStrings;
  std::vector<std::string> iniStrings;
  std::string inputDir;
  std::vector<std::string> inputs;
  std::string inputList;
  std::vector<std::string> modules;
  std::vector<std::string> excludeDirs;
  std::vector<std::string> excludeFiles;
  std::vector<std::string> excludePatterns;
  std::vector<std::string> excludeStaticDirs;
  std::vector<std::string> excludeStaticFiles;
  std::vector<std::string> excludeStaticPatterns;
  std::vector<std::string> fmodules;
  std::vector<std::string> ffiles;
  std::vector<std::string> cfiles;
  std::vector<std::string> cmodules;
  bool parseOnDemand;
  bool keepTempDir;
  int logLevel;
  std::string filecache;
  bool coredump;
};

///////////////////////////////////////////////////////////////////////////////

struct AsyncFileCacheSaver : AsyncFunc<AsyncFileCacheSaver> {
  AsyncFileCacheSaver(Package *package, const char *name)
      : AsyncFunc<AsyncFileCacheSaver>(this, &AsyncFileCacheSaver::saveCache),
        m_package(package), m_name(name) {
  }

  void saveCache() {
    Timer timer(Timer::WallTime, "saving file cache...");
    m_package->getFileCache()->save(m_name);

    struct stat sb;
    stat(m_name, &sb);
    Logger::Info("%" PRId64" MB %s saved",
                 (int64_t)sb.st_size/(1024*1024), m_name);
  }

private:
  Package *m_package;
  const char *m_name;
};

///////////////////////////////////////////////////////////////////////////////
// forward declarations

int prepareOptions(CompilerOptions &po, int argc, char **argv);
int process(const CompilerOptions &po);
void hhbcTargetInit(const CompilerOptions &po, AnalysisResultPtr ar);
int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr&& ar);
void pcre_init();

///////////////////////////////////////////////////////////////////////////////

int compiler_main(int argc, char **argv) {
  try {
    CompilerOptions po;
    rds::local::init();
    SCOPE_EXIT { rds::local::fini(); };

    int ret = prepareOptions(po, argc, argv);
    if (ret == 1) return 0; // --help
    if (ret == -1) return -1; // command line error

    Timer totalTimer(Timer::WallTime, "running hphp");
    mkdir(po.outputDir.c_str(), 0777);
    if (ret == 0) ret = process(po);
    if (ret) {
      Logger::Error("hphp failed");
    } else {
      Logger::Info("all files saved in %s ...", po.outputDir.c_str());
    }
    return ret;
  } catch (Exception& e) {
    Logger::Error("Exception: %s", e.getMessage().c_str());
  } catch (std::exception& e) {
    Logger::Error("std::exception: %s", e.what());
  } catch (...) {
    Logger::Error("(non-standard exception \"%s\" was thrown)",
                  current_exception_name().c_str());
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////

namespace {

void applyBuildOverrides(IniSetting::Map& ini, Hdf& config) {
  std::string push_phases = Config::GetString(ini, config, "Build.PushPhases");
  // convert push phases to newline-separated, to make matching them less
  // error-prone.
  replaceAll(push_phases, ",", "\n");
  bool loggedOnce = false;

  for (Hdf hdf = config["Overrides"].firstChild();
       hdf.exists();
       hdf = hdf.next()) {
    if (!loggedOnce) {
      Logger::Info(folly::sformat(
                       "Matching build overrides using: push_phases='{}'",
                       push_phases));
      loggedOnce = true;
    }
    if (Config::matchHdfPattern(push_phases, ini, hdf, "push_phase" , "m")) {
      Logger::Info(folly::sformat("Matched override: {}", hdf.getName()));

      if (hdf.exists("clear")) {
        std::vector<std::string> list;
        hdf["clear"].configGet(list);
        for (auto const& s : list) {
          config.remove(s);
        }
      }
      config.copy(hdf["overwrite"]);
      // no break here, so we can continue to match more overrides
    }
    hdf["overwrite"].setVisited(); // avoid lint complaining
    if (hdf.exists("clear")) {
      // when the tier does not match, "clear" is not accessed
      // mark it visited, so the linter does not complain
      hdf["clear"].setVisited();
    }
  }
}

}

int prepareOptions(CompilerOptions &po, int argc, char **argv) {
  options_description desc("HipHop Compiler for PHP Usage:\n\n"
                           "\thphp <options> <inputs>\n\n"
                           "Options");

  bool dummy;
  bool dummy2;
  std::string dummy3;
  std::string dummy4;

  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("target,t", value<std::string>(&dummy4)->default_value("hhbc"),
     "hhbc") // TODO: T115189426 remove this
    ("format,f", value<std::string>(&po.format),
     "HHBC Output format: binary (default) | hhas | text")
    ("input-dir", value<std::string>(&po.inputDir), "input directory")
    ("inputs,i", value<std::vector<std::string>>(&po.inputs),
     "input file names")
    ("input-list", value<std::string>(&po.inputList),
     "file containing list of file names, one per line")
    ("module", value<std::vector<std::string>>(&po.modules)->composing(),
     "directories containing all input files")
    ("exclude-dir",
     value<std::vector<std::string>>(&po.excludeDirs)->composing(),
     "directories to exclude from the input")
    ("fmodule", value<std::vector<std::string>>(&po.fmodules)->composing(),
     "same with module, except no exclusion checking is performed, so these "
     "modules are forced to be included")
    ("ffile", value<std::vector<std::string>>(&po.ffiles)->composing(),
     "extra PHP files forced to include without exclusion checking")
    ("exclude-file",
     value<std::vector<std::string>>(&po.excludeFiles)->composing(),
     "files to exclude from the input, even if parse-on-demand finds it")
    ("exclude-pattern",
     value<std::vector<std::string>>(&po.excludePatterns)->composing(),
     "regex (in 'find' command's regex command line option format) of files "
     "or directories to exclude from the input, even if parse-on-demand finds "
     "it")
    ("exclude-static-pattern",
     value<std::vector<std::string>>(&po.excludeStaticPatterns)->composing(),
     "regex (in 'find' command's regex command line option format) of files "
     "or directories to exclude from static content cache")
    ("exclude-static-dir",
     value<std::vector<std::string>>(&po.excludeStaticDirs)->composing(),
     "directories to exclude from static content cache")
    ("exclude-static-file",
     value<std::vector<std::string>>(&po.excludeStaticFiles)->composing(),
     "files to exclude from static content cache")
    ("cfile", value<std::vector<std::string>>(&po.cfiles)->composing(),
     "extra static files forced to include without exclusion checking")
    ("cmodule", value<std::vector<std::string>>(&po.cmodules)->composing(),
     "extra directories for static files without exclusion checking")
    ("parse-on-demand", value<bool>(&po.parseOnDemand)->default_value(true),
     "whether to parse files that are not specified from command line")
    ("output-dir,o", value<std::string>(&po.outputDir), "output directory")
    ("sync-dir", value<std::string>(&dummy3), // TODO: T115189426 remove this
     "Files will be created in this directory first, then sync with output "
     "directory without overwriting identical files. Great for incremental "
     "compilation and build.")
    ("gen-stats", value<bool>(&dummy2)->default_value(false), // TODO: T115189426 remove this
     "whether to generate code errors")
    ("keep-tempdir,k", value<bool>(&po.keepTempDir)->default_value(false),
     "whether to keep the temporary directory")
    ("config,c", value<std::vector<std::string>>(&po.config)->composing(),
     "config file name")
    ("config-value,v",
     value<std::vector<std::string>>(&po.confStrings)->composing(),
     "individual configuration string in a format of name=value, where "
     "name can be any valid configuration for a config file")
    ("define,d", value<std::vector<std::string>>(&po.iniStrings)->composing(),
     "define an ini setting in the same format ( foo[=bar] ) as provided in a "
     ".ini file")
    ("log,l",
     value<int>(&po.logLevel)->default_value(-1),
     "-1: (default); 0: no logging; 1: errors only; 2: warnings and errors; "
     "3: informational as well; 4: really verbose.")
    ("force",
     value<bool>(&dummy)->default_value(true), // TODO: T115189426 remove this
     "force to ignore code generation errors and continue compilations")
    ("file-cache",
     value<std::string>(&po.filecache),
     "if specified, generate a static file cache with this file name")
    ("coredump",
     value<bool>(&po.coredump)->default_value(false),
     "turn on coredump")
    ("compiler-id", "display the git hash for the compiler id")
    ("repo-schema", "display the repo schema id used by this app")
    ;

  positional_options_description p;
  p.add("inputs", -1);
  variables_map vm;
  try {
    auto opts = command_line_parser(argc, argv).options(desc)
                                               .positional(p).run();
    try {
      store(opts, vm);
      notify(vm);
#if defined(BOOST_VERSION) && BOOST_VERSION >= 105000 && BOOST_VERSION <= 105400
    } catch (const error_with_option_name &e) {
      std::string wrong_name = e.get_option_name();
      std::string right_name = get_right_option_name(opts, wrong_name);
      std::string message = e.what();
      if (right_name != "") {
        boost::replace_all(message, wrong_name, right_name);
      }
      Logger::Error("Error in command line: %s", message.c_str());
      std::cout << desc << "\n";
      return -1;
#endif
    } catch (const error& e) {
      Logger::Error("Error in command line: %s", e.what());
      std::cout << desc << "\n";
      return -1;
    }
  } catch (const unknown_option& e) {
    Logger::Error("Error in command line: %s", e.what());
    std::cout << desc << "\n";
    return -1;
  } catch (const error& e) {
    Logger::Error("Error in command line: %s", e.what());
    std::cout << desc << "\n";
    return -1;
  } catch (...) {
    Logger::Error("Error in command line parsing.");
    std::cout << desc << "\n";
    return -1;
  }
  if (argc <= 1 || vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  if (vm.count("version")) {
    std::cout << "HipHop Repo Compiler";
    std::cout << " " << HHVM_VERSION;
    std::cout << " (" << (debug ? "dbg" : "rel") << ")\n";
    std::cout << "Compiler: " << compilerId() << "\n";
    std::cout << "Repo schema: " << repoSchemaId() << "\n";
    return 1;
  }

  if (vm.count("compiler-id")) {
    std::cout << compilerId() << "\n";
    return 1;
  }

  if (vm.count("repo-schema")) {
    std::cout << repoSchemaId() << "\n";
    return 1;
  }

  // log level
  if (po.logLevel != -1) {
    Logger::LogLevel = (Logger::LogLevelType)po.logLevel;
  } else {
    Logger::LogLevel = Logger::LogInfo;
  }
  Logger::Escape = false;
  Logger::AlwaysEscapeLog = false;

  tl_heap.getCheck();
  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  for (auto const& file : po.config) {
    Config::ParseConfigFile(file, ini, config);
  }
  for (auto const& iniString : po.iniStrings) {
    Config::ParseIniString(iniString, ini);
  }
  for (auto const& confString : po.confStrings) {
    Config::ParseHdfString(confString, config);
  }
  applyBuildOverrides(ini, config);
  Hdf runtime = config["Runtime"];
  // The configuration command line strings were already processed above
  // Don't process them again.
  //
  // Note that some options depends on RepoAuthoritative, we thus set/unset them
  // here. If we reach this code, we are invoking hhvm --hphp, which is
  // supposed to be in repo mode only. But we are restoring it to false since
  // we need compile_systemlib_string to actually parse the file instead of
  // trying to load it from repo (which is the case when RepoAuthoritative is
  // true).
  RuntimeOption::RepoAuthoritative = true;
  // Set RepoPath to satisfy assertions (we need a path set in
  // RepoAuthoritative). It will never actually be used.
  RuntimeOption::RepoPath = "/tmp/dummy.hhbc";
  // We don't want debug info in repo builds, since we don't support attaching
  // a debugger in repo authoritative mode, but we want the default for debug
  // info to be true so that it's present in sandboxes. Override that default
  // here, since we only get here when building for repo authoritative mode.
  RuntimeOption::RepoDebugInfo = false;
  RuntimeOption::Load(ini, runtime);
  Option::Load(ini, config);
  RuntimeOption::RepoAuthoritative = false;
  RuntimeOption::RepoPath = "";
  RuntimeOption::EvalJit = false;

  std::vector<std::string> badnodes;
  config.lint(badnodes);
  for (auto const& badnode : badnodes) {
    Logger::Error("Possible bad config node: %s", badnode.c_str());
  }

  // we need to initialize pcre cache table very early
  pcre_init();

  if (po.inputDir.empty()) {
    po.inputDir = '.';
  }
  po.inputDir = FileUtil::normalizeDir(po.inputDir);

  if (po.outputDir.empty()) {
    Logger::Error("Error in command line: output-dir must be provided.");
    std::cout << desc << "\n";
    return -1;
  }

  for (auto const& dir : po.excludeDirs) {
    Option::PackageExcludeDirs.insert(FileUtil::normalizeDir(dir));
  }
  for (auto const& file : po.excludeFiles) {
    Option::PackageExcludeFiles.insert(file);
  }
  for (auto const& pattern : po.excludePatterns) {
    Option::PackageExcludePatterns.insert(
      format_pattern(pattern, true /* prefixSlash */));
  }
  for (auto const& dir : po.excludeStaticDirs) {
    Option::PackageExcludeStaticDirs.insert(FileUtil::normalizeDir(dir));
  }
  for (auto const& file : po.excludeStaticFiles) {
    Option::PackageExcludeStaticFiles.insert(file);
  }
  for (auto const& pattern : po.excludeStaticPatterns) {
    Option::PackageExcludeStaticPatterns.insert(
      format_pattern(pattern, true /* prefixSlash */));
  }

  if (po.format.empty()) po.format = "binary";
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

int process(const CompilerOptions &po) {
#ifndef _MSC_VER
  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount,
                           RuntimeOption::EvalRecordSubprocessTimes,
                           {});
#endif

  if (po.coredump) {
#ifdef _MSC_VER
/**
 * Windows actually does core dump size and control at a system, not an app
 * level.  So we do nothing here and are at the mercy of Dr. Watson.
 */
#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct rlimit rl;
    getrlimit(RLIMIT_CORE, &rl);
    rl.rlim_cur = 80000000LL;
    if (rl.rlim_max < rl.rlim_cur) {
      rl.rlim_max = rl.rlim_cur;
    }
    setrlimit(RLIMIT_CORE, &rl);
#else
    struct rlimit64 rl;
    getrlimit64(RLIMIT_CORE, &rl);
    rl.rlim_cur = 8000000000LL;
    if (rl.rlim_max < rl.rlim_cur) {
      rl.rlim_max = rl.rlim_cur;
    }
    setrlimit64(RLIMIT_CORE, &rl);
#endif
  }

  register_process_init();

  Timer timer(Timer::WallTime);
  // prepare a package
  Package package{po.inputDir.c_str(), po.parseOnDemand};
  AnalysisResultPtr ar = package.getAnalysisResult();
  ar->sample().setStr("use_case", Option::ExternWorkerUseCase);
  ar->sample().setInt("use_rich_client", Option::ExternWorkerUseRichClient);
  ar->sample().setInt("use_zippy_rich_client",
      Option::ExternWorkerUseZippyRichClient);
  ar->sample().setInt("use_p2p", Option::ExternWorkerUseP2P);
  ar->sample().setInt("force_subprocess", Option::ExternWorkerForceSubprocess);
  ar->sample().setInt("use_exec_cache", Option::ExternWorkerUseExecCache);
  ar->sample().setInt("parser_group_size", Option::ParserGroupSize);
  ar->sample().setInt("parser_thread_count", Option::ParserThreadCount);

  hhbcTargetInit(po, ar);

  // Track the unit-emitters created for system during
  // hphp_process_init().
  SystemLib::keepRegisteredUnitEmitters(true);
  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };
  SystemLib::keepRegisteredUnitEmitters(false);

  package.createAsyncState();
  SCOPE_EXIT {
    // We need to do this manually, and not rely on ~Package, because
    // it has to be done *before* we call hphp_process_exit().
    if (auto clearer = package.clearAsyncState()) clearer->join();
  };

  // This should be set before parsing anything
  RuntimeOption::EvalLowStaticArrays = false;

  {
    Timer timer2(Timer::WallTime, "parsing");
    if (po.modules.empty() && po.fmodules.empty() &&
        po.ffiles.empty() && po.inputs.empty() && po.inputList.empty()) {
      package.addDirectory("/");
    } else {
      for (auto const& module : po.modules) {
        package.addDirectory(module);
      }
      for (auto const& fmodule : po.fmodules) {
        package.addDirectory(fmodule);
      }
      for (auto const& ffile : po.ffiles) {
        package.addSourceFile(ffile);
      }
      for (auto const& cmodule : po.cmodules) {
        package.addStaticDirectory(cmodule);
      }
      for (auto const& cfile : po.cfiles) {
        package.addStaticFile(cfile);
      }
      for (auto const& input : po.inputs) {
        package.addSourceFile(input);
      }
      if (!po.inputList.empty()) {
        package.addInputList(po.inputList);
      }
    }
    if (!package.parse()) return 1;

    auto const& stats = package.stats();
    Logger::FInfo(
      "{:,} files parsed\n"
      "  Execs: {:,} total, {:,} cache-hits, {:,} optimistically, {:,} fallback\n"
      "  Files: {:,} total, {:,} read, {:,} queried, {:,} uploaded, {:,} fallback\n"
      "  Blobs: {:,} total, {:,} queried, {:,} uploaded, {:,} fallback\n"
      "  {:,} downloads, {:,} throttles",
      package.getTotalFiles(),
      stats.execs.load(),
      stats.execCacheHits.load(),
      stats.optimisticExecs.load(),
      stats.execFallbacks.load(),
      stats.files.load(),
      stats.filesRead.load(),
      stats.filesQueried.load(),
      stats.filesUploaded.load(),
      stats.fileFallbacks.load(),
      stats.blobs.load(),
      stats.blobsQueried.load(),
      stats.blobsUploaded.load(),
      stats.blobFallbacks.load(),
      stats.downloads.load(),
      stats.throttles.load()
    );
    ar->sample().setInt("parsing_micros", timer2.getMicroSeconds());
    ar->sample().setInt("total_parses", package.getTotalFiles());

    ar->sample().setInt("parse_total_execs", stats.execs.load());
    ar->sample().setInt("parse_cache_hits", stats.execCacheHits.load());
    ar->sample().setInt("parse_optimistically", stats.optimisticExecs.load());
    ar->sample().setInt("parse_fallbacks", stats.execFallbacks.load());

    ar->sample().setInt("parse_total_files", stats.files.load());
    ar->sample().setInt("parse_file_reads", stats.filesRead.load());
    ar->sample().setInt("parse_file_queries", stats.filesQueried.load());
    ar->sample().setInt("parse_file_stores", stats.filesUploaded.load());
    ar->sample().setInt("parse_file_fallbacks", stats.fileFallbacks.load());

    ar->sample().setInt("parse_total_blobs", stats.blobs.load());
    ar->sample().setInt("parse_blob_queries", stats.blobsQueried.load());
    ar->sample().setInt("parse_blob_stores", stats.blobsUploaded.load());
    ar->sample().setInt("parse_blob_fallbacks", stats.blobFallbacks.load());

    ar->sample().setInt("parse_total_loads", stats.downloads.load());
    ar->sample().setInt("parse_throttles", stats.throttles.load());
  }

  // Start asynchronously destroying the async state, since it may
  // take a long time. We'll do it in the background while the rest of
  // the compile pipeline runs.
  auto clearer = package.clearAsyncState();
  SCOPE_EXIT { if (clearer) clearer->join(); };

  // saving file cache
  AsyncFileCacheSaver fileCacheThread(&package, po.filecache.c_str());
  if (!po.filecache.empty()) {
    fileCacheThread.start();
  }

  ar->setFinish([&po,&timer,&package](AnalysisResultPtr res) {
      package.resetAr();
    });

  SCOPE_EXIT {
    if (!po.filecache.empty()) {
      fileCacheThread.waitForEnd();
    }
  };

  return hhbcTarget(po, std::move(ar));
}

///////////////////////////////////////////////////////////////////////////////

void hhbcTargetInit(const CompilerOptions &po, AnalysisResultPtr ar) {
  ar->setOutputPath(po.outputDir);
  // Propagate relevant compiler-specific options to the runtime.
  RuntimeOption::RepoPath = ar->getOutputPath() + "/hhvm.hhbc";
  unlink(RuntimeOption::RepoPath.c_str());
}

int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr&& ar) {
  int ret = 0;
  int formatCount = 0;
  const char *type = 0;
  if (po.format.find("text") != std::string::npos) {
    Option::GenerateTextHHBC = true;
    type = "creating text HHBC files";
    formatCount++;
  }
  if (po.format.find("hhas") != std::string::npos) {
    Option::GenerateHhasHHBC = true;
    type = "creating hhas HHBC files";
    formatCount++;
  }
  if (po.format.find("binary") != std::string::npos) {
    Option::GenerateBinaryHHBC = true;
    type = "creating binary HHBC files";
    formatCount++;
  }

  if (formatCount == 0) {
    Logger::Error("Unknown format for HHBC target: %s", po.format.c_str());
    return 1;
  }

  unlink(RuntimeOption::RepoPath.c_str());
  /* without this, emitClass allows classes with interfaces to be
     hoistable */
  SystemLib::s_inited = true;

  // the function is only invoked in hhvm --hphp, which is supposed to be in
  // repo mode only. we are not setting it earlier in `compiler_main` since we
  // want systemlib to be built without repo-auth == true, or otherwise,
  // `compile_systemlib_string` will try to load systemlib from repo, while we
  // are building it.
  RuntimeOption::RepoAuthoritative = true;

  // We don't need these anymore, and since they can consume a lot of
  // memory, free them before doing anything else.
  decltype(Option::AutoloadClassMap){}.swap(Option::AutoloadClassMap);
  decltype(Option::AutoloadFuncMap){}.swap(Option::AutoloadFuncMap);
  decltype(Option::AutoloadConstMap){}.swap(Option::AutoloadConstMap);

  Timer timer(Timer::WallTime, type);
  Compiler::emitAllHHBC(std::move(ar));

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

}
