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
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/array-provenance.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/extern-compiler.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/version.h"
#include "hphp/system/systemlib.h"

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
using std::cout;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CompilerOptions {
  std::string target;
  std::string format;
  std::string outputDir;
  std::string syncDir;
  std::vector<std::string> config;
  std::string configDir;
  std::vector<std::string> confStrings;
  std::vector<std::string> iniStrings;
  std::string inputDir;
  std::vector<std::string> inputs;
  std::string inputList;
  std::vector<std::string> includePaths;
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
  std::string program;
  std::string programArgs;
  std::string branch;
  int revision;
  bool genStats;
  bool keepTempDir;
  int logLevel;
  bool force;
  std::string filecache;
  bool coredump;
  bool nofork;
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
void createOutputDirectory(CompilerOptions &po);
int process(const CompilerOptions &po);
int phpTarget(const CompilerOptions &po, AnalysisResultPtr ar);
void hhbcTargetInit(const CompilerOptions &po, AnalysisResultPtr ar);
int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr&& ar,
               AsyncFileCacheSaver &fcThread);
int runTarget(const CompilerOptions &po);
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
    createOutputDirectory(po);
    if (ret == 0) {
#ifndef _MSC_VER
      if (!po.nofork && !Process::IsUnderGDB()) {
        int pid = fork();
        if (pid == 0) {
          ret = process(po);
          _exit(ret);
        }
        wait(&ret);
        ret = WIFEXITED(ret) ? WEXITSTATUS(ret) : -1;
      } else
#endif
      {
        ret = process(po);
      }
    }
    if (ret == 0) {
      if (po.target == "run") {
        ret = runTarget(po);
      }
    }
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
    Logger::Error("(unknown exception was thrown)");
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
  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("target,t", value<std::string>(&po.target)->default_value("run"),
     "hhbc | "
     "cache | "
     "filecache | "
     "run (default)")
    ("format,f", value<std::string>(&po.format),
     "hhbc: binary (default) | hhas | text | exe; \n"
     "run: binary (default) | hhas | text | exe")
    ("input-dir", value<std::string>(&po.inputDir), "input directory")
    ("program", value<std::string>(&po.program)->default_value("program"),
     "final program name to use")
    ("args", value<std::string>(&po.programArgs), "program arguments")
    ("inputs,i", value<std::vector<std::string>>(&po.inputs),
     "input file names")
    ("input-list", value<std::string>(&po.inputList),
     "file containing list of file names, one per line")
    ("include-path",
     value<std::vector<std::string>>(&po.includePaths)->composing(),
     "a list of full paths to search for files being included in includes "
     "or requires but cannot be found assuming relative paths")
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
    ("branch", value<std::string>(&po.branch), "SVN branch")
    ("revision", value<int>(&po.revision), "SVN revision")
    ("output-dir,o", value<std::string>(&po.outputDir), "output directory")
    ("sync-dir", value<std::string>(&po.syncDir),
     "Files will be created in this directory first, then sync with output "
     "directory without overwriting identical files. Great for incremental "
     "compilation and build.")
    ("gen-stats", value<bool>(&po.genStats)->default_value(false),
     "whether to generate code errors")
    ("keep-tempdir,k", value<bool>(&po.keepTempDir)->default_value(false),
     "whether to keep the temporary directory")
    ("config,c", value<std::vector<std::string>>(&po.config)->composing(),
     "config file name")
    ("config-dir", value<std::string>(&po.configDir),
     "root directory configuration is based on (for example, "
     "excluded directories may be relative path in configuration.")
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
     value<bool>(&po.force)->default_value(true),
     "force to ignore code generation errors and continue compilations")
    ("file-cache",
     value<std::string>(&po.filecache),
     "if specified, generate a static file cache with this file name")
    ("coredump",
     value<bool>(&po.coredump)->default_value(false),
     "turn on coredump")
    ("nofork",
     value<bool>(&po.nofork)->default_value(false),
     "forking is needed for large compilation to release memory before g++"
     "compilation. turning off forking can help gdb debugging.")
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
      cout << desc << "\n";
      return -1;
#endif
    } catch (const error& e) {
      Logger::Error("Error in command line: %s", e.what());
      cout << desc << "\n";
      return -1;
    }
  } catch (const unknown_option& e) {
    Logger::Error("Error in command line: %s", e.what());
    cout << desc << "\n";
    return -1;
  } catch (const error& e) {
    Logger::Error("Error in command line: %s", e.what());
    cout << desc << "\n";
    return -1;
  } catch (...) {
    Logger::Error("Error in command line parsing.");
    cout << desc << "\n";
    return -1;
  }
  if (argc <= 1 || vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }
  if (vm.count("version")) {
    cout << "HipHop Repo Compiler";
    cout << " " << HHVM_VERSION;
    cout << " (" << (debug ? "dbg" : "rel") << ")\n";
    cout << "Compiler: " << compilerId() << "\n";
    cout << "Repo schema: " << repoSchemaId() << "\n";
    return 1;
  }

  if (vm.count("compiler-id")) {
    cout << compilerId() << "\n";
    return 1;
  }

  if (vm.count("repo-schema")) {
    cout << repoSchemaId() << "\n";
    return 1;
  }

  if (po.target != "run" &&
      po.target != "hhbc" &&
      po.target != "cache" &&
      po.target != "filecache") {
    Logger::Error("Error in command line: target '%s' is not supported.",
                  po.target.c_str());
    // desc[ription] is the --help output
    cout << desc << "\n";
    return -1;
  }

  if ((po.target == "hhbc" || po.target == "run") &&
      po.format.find("exe") == std::string::npos) {
    if (po.program == "program") {
      po.program = "hhvm.hhbc";
    }
  }

  // log level
  if (po.logLevel != -1) {
    Logger::LogLevel = (Logger::LogLevelType)po.logLevel;
  } else if (po.target == "run") {
    Logger::LogLevel = Logger::LogNone;
  } else {
    Logger::LogLevel = Logger::LogInfo;
  }

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
  // We don't want debug info in repo builds, since we don't support attaching
  // a debugger in repo authoritative mode, but we want the default for debug
  // info to be true so that it's present in sandboxes. Override that default
  // here, since we only get here when building for repo authoritative mode.
  RuntimeOption::RepoDebugInfo = false;
  // Default RepoLocalMode to off so we build systemlib from source.
  // This can be overridden when running lots of repo builds (eg
  // test/run) for better performance.
  RuntimeOption::RepoLocalMode = RepoMode::Closed;
  RuntimeOption::RepoJournal = "memory";
  RuntimeOption::Load(ini, runtime);
  Option::Load(ini, config);
  RuntimeOption::RepoAuthoritative = false;
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
  if (po.configDir.empty()) {
    po.configDir = po.inputDir;
  }
  po.configDir = FileUtil::normalizeDir(po.configDir);
  Option::RootDirectory = po.configDir;
  Option::IncludeSearchPaths = po.includePaths;

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

  Option::ProgramName = po.program;

  if (po.format.empty() && (po.target == "run" || po.target == "hhbc")) {
    po.format = "binary";
  }

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
  Package package(po.inputDir.c_str());
  AnalysisResultPtr ar = package.getAnalysisResult();

  hhbcTargetInit(po, ar);

  bool processInitRan = false;
  SCOPE_EXIT {
    if (processInitRan) {
      hphp_process_exit();
    }
  };

  bool wp = Option::WholeProgram;
  Option::WholeProgram = false;
  BuiltinSymbols::s_systemAr = ar;
  hphp_process_init();
  processInitRan = true;
  BuiltinSymbols::s_systemAr.reset();
  Option::WholeProgram = wp;

  // This should be set before parsing anything
  RuntimeOption::EvalLowStaticArrays = false;

  LitstrTable::init();
  LitstrTable::get().setWriting();
  LitarrayTable::init();
  LitarrayTable::get().setWriting();

  {
    Timer timer2(Timer::WallTime, "parsing inputs");
    ar->setPackage(&package);
    ar->setParseOnDemand(po.parseOnDemand);
    if (!po.parseOnDemand) {
      ar->setParseOnDemandDirs(Option::ParseOnDemandDirs);
    }
    if (po.modules.empty() && po.fmodules.empty() &&
        po.ffiles.empty() && po.inputs.empty() && po.inputList.empty()) {
      package.addAllFiles(false);
    } else {
      for (auto const& module : po.modules) {
        package.addDirectory(module, false /*force*/);
      }
      for (auto const& fmodule : po.fmodules) {
        package.addDirectory(fmodule, true /*force*/);
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
    if (po.target != "filecache") {
      if (!package.parse(!po.force)) return 1;

      Logger::Info(
        "%ld total parses, %ld cache hits",
        package.getTotalParses(),
        package.getParseCacheHits()
      );

      // nuke the compiler pool so we don't waste memory on ten gajillion
      // instances of hackc
      if (auto const new_workers = RO::EvalHackCompilerSecondaryWorkers) {
        Logger::Info(
          "shutting down hackc worker pool and resizing to %ld workers",
          new_workers
        );
        RO::EvalHackCompilerWorkers = new_workers;
        compilers_shutdown();
      }
    }
  }

  // saving file cache
  AsyncFileCacheSaver fileCacheThread(&package, po.filecache.c_str());
  if (!po.filecache.empty()) {
    fileCacheThread.start();
  }

  ar->setFinish([&po,&timer,&package](AnalysisResultPtr res) {
      // saving stats
      if (po.genStats) {
        int seconds = timer.getMicroSeconds() / 1000000;

        Logger::Info("saving code errors and stats...");
        Timer timer(Timer::WallTime, "saving stats");
        package.saveStatsToFile((po.outputDir + "/Stats.js").c_str(), seconds);
      }
      package.resetAr();
    });

  SCOPE_EXIT {
    if (!po.filecache.empty()) {
      fileCacheThread.waitForEnd();
    }
  };

  int ret = 0;
  if (po.target == "hhbc" || po.target == "run") {
    ret = hhbcTarget(po, std::move(ar), fileCacheThread);
  } else if (po.target == "filecache" || po.target == "cache") {
    ar->finish();
  } else {
    Logger::Error("Unknown target: %s", po.target.c_str());
    return 1;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void hhbcTargetInit(const CompilerOptions &po, AnalysisResultPtr ar) {
  if (po.syncDir.empty()) {
    ar->setOutputPath(po.outputDir);
  } else {
    ar->setOutputPath(po.syncDir);
  }
  // Propagate relevant compiler-specific options to the runtime.
  RuntimeOption::RepoCentralPath = ar->getOutputPath() + '/' + po.program;
  if (po.format.find("exe") != std::string::npos) {
    RuntimeOption::RepoCentralPath += ".hhbc";
  }
  unlink(RuntimeOption::RepoCentralPath.c_str());

  if (RuntimeOption::RepoLocalMode != RepoMode::ReadWrite) {
    // No point writing to the central repo, because we're going to
    // remove it before writing the final repo.
    RuntimeOption::RepoCommit = false;
  }
}

int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr&& ar,
               AsyncFileCacheSaver &fcThread) {
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
  if (po.format.find("exe") != std::string::npos) {
    Option::GenerateBinaryHHBC = true;
    type = "creating binary HHBC files";
    formatCount++;
  }

  if (formatCount == 0) {
    Logger::Error("Unknown format for HHBC target: %s", po.format.c_str());
    return 1;
  }

  RuntimeOption::RepoLocalMode = RepoMode::Closed;
  unlink(RuntimeOption::RepoCentralPath.c_str());
  /* without this, emitClass allows classes with interfaces to be
     hoistable */
  SystemLib::s_inited = true;
  RuntimeOption::RepoCommit = true;

  // the function is only invoked in hhvm --hphp, which is supposed to be in
  // repo mode only. we are not setting it earlier in `compiler_main` since we
  // want systemlib to be built without repo-auth == true, or otherwise,
  // `compile_systemlib_string` will try to load systemlib from repo, while we
  // are building it.
  RuntimeOption::RepoAuthoritative = true;

  Timer timer(Timer::WallTime, type);
  Compiler::emitAllHHBC(std::move(ar));

  if (!po.syncDir.empty()) {
    if (!po.filecache.empty()) {
      fcThread.waitForEnd();
    }
    FileUtil::syncdir(po.outputDir, po.syncDir);
    boost::filesystem::remove_all(po.syncDir);
  }

  if (!ret && po.format.find("exe") != std::string::npos) {
    /*
     * We need to create an executable with the repo
     * embedded in it.
     * Copy ourself, and embed the repo as a section
     * named "repo".
     */
    std::string exe = po.outputDir + '/' + po.program;
    std::string repo = "repo=" + exe + ".hhbc";
    std::string buf = current_executable_path();
    if (buf.empty()) return -1;

    const char *argv[] = { "objcopy", "--add-section", repo.c_str(),
                           buf.c_str(), exe.c_str(), 0 };
    std::string out;
    ret = proc::exec(argv[0], argv, nullptr, out, nullptr) ? 0 : 1;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int runTarget(const CompilerOptions &po) {
  int ret = 0;

  // If there are more than one input files, we need one extra arg to run.
  // If it's missing, we will stop right here, with compiled code.
  if ((po.inputs.size() != 1 && po.programArgs.empty()) ||
      !po.inputList.empty()) {
    return 0;
  }

  // run the executable
  std::string cmd;
  if (po.format.find("exe") == std::string::npos) {
    std::string buf = current_executable_path();
    if (buf.empty()) return -1;

    cmd += buf;
    cmd += " -vRepo.Authoritative=true";
    if (getenv("HPHP_DUMP_BYTECODE")) cmd += " -vEval.DumpBytecode=1";
    if (getenv("HPHP_INTERP"))        cmd += " -vEval.Jit=0";
    cmd += " -vRepo.Local.Mode=r- -vRepo.Local.Path=";
  }
  cmd += po.outputDir + '/' + po.program;
  cmd += std::string(" --file ") +
    (po.inputs.size() == 1 ? po.inputs[0] : "") +
    " " + po.programArgs;
  Logger::Info("running executable: %s", cmd.c_str());
  ret = FileUtil::ssystem(cmd.c_str());
  if (ret && ret != -1) ret = 1;

  // delete the temporary directory if not needed
  if (!po.keepTempDir) {
    Logger::Info("deleting temporary directory %s...", po.outputDir.c_str());
    boost::filesystem::remove_all(po.outputDir);
  }
  return ret;
}

void createOutputDirectory(CompilerOptions &po) {
  if (po.outputDir.empty()) {
    const char *t = getenv("TEMP");
    if (!t) {
      t = "/tmp";
    }
    std::string temp = t;
    temp += "/hphp_XXXXXX";
    std::vector<char> path(begin(temp), end(temp));
    path.push_back('\0');
    po.outputDir = mkdtemp(&path[0]);
    Logger::Info("creating temporary directory %s ...", po.outputDir.c_str());
  }
  mkdir(po.outputDir.c_str(), 0777);

  if (!po.syncDir.empty()) {
    Logger::Info("re-creating sync directory %s ...", po.syncDir.c_str());
    boost::filesystem::remove_all(po.syncDir);
    mkdir(po.syncDir.c_str(), 0777);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
