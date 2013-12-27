/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/compiler/package.h"
#include "hphp/compiler/analysis/analysis_result.h"
#include "hphp/compiler/analysis/alias_manager.h"
#include "hphp/compiler/analysis/code_error.h"
#include "hphp/compiler/analysis/emitter.h"
#include "hphp/compiler/analysis/type.h"
#include "hphp/compiler/analysis/symbol_table.h"
#include "hphp/compiler/option.h"
#include "hphp/compiler/parser/parser.h"
#include "hphp/compiler/builtin_symbols.h"
#include "hphp/util/json.h"
#include "hphp/util/logger.h"
#include "hphp/util/db-conn.h"
#include "hphp/util/exception.h"
#include "hphp/util/process.h"
#include "hphp/util/util.h"
#include "hphp/util/timer.h"
#include "hphp/util/hdf.h"
#include "hphp/util/async-func.h"
#include "hphp/util/current-executable.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/externals.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/repo-schema.h"

#include "hphp/hhvm/process-init.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

using namespace boost::program_options;
using std::cout;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct CompilerOptions {
  string target;
  string format;
  string outputDir;
  string syncDir;
  vector<string> config;
  string configDir;
  vector<string> confStrings;
  string inputDir;
  vector<string> inputs;
  string inputList;
  vector<string> includePaths;
  vector<string> modules;
  vector<string> excludeDirs;
  vector<string> excludeFiles;
  vector<string> excludePatterns;
  vector<string> excludeStaticDirs;
  vector<string> excludeStaticFiles;
  vector<string> excludeStaticPatterns;
  vector<string> fmodules;
  vector<string> ffiles;
  vector<string> cfiles;
  vector<string> cmodules;
  bool parseOnDemand;
  string program;
  string programArgs;
  string branch;
  int revision;
  bool genStats;
  bool keepTempDir;
  string dbStats;
  bool noTypeInference;
  int logLevel;
  bool force;
  int optimizeLevel;
  string filecache;
  bool dump;
  string docjson;
  bool coredump;
  bool nofork;
  string optimizations;
};

///////////////////////////////////////////////////////////////////////////////

class AsyncFileCacheSaver : public AsyncFunc<AsyncFileCacheSaver> {
public:
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
int lintTarget(const CompilerOptions &po);
int analyzeTarget(const CompilerOptions &po, AnalysisResultPtr ar);
int phpTarget(const CompilerOptions &po, AnalysisResultPtr ar);
void hhbcTargetInit(const CompilerOptions &po, AnalysisResultPtr ar);
int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr ar,
               AsyncFileCacheSaver &fcThread);
int runTargetCheck(const CompilerOptions &po, AnalysisResultPtr ar,
                   AsyncFileCacheSaver &fcThread);
int runTarget(const CompilerOptions &po);

///////////////////////////////////////////////////////////////////////////////

extern "C" void compiler_hook_initialize();

int compiler_main(int argc, char **argv) {
  try {
    Hdf empty;
    RuntimeOption::Load(empty);
    initialize_repo();

    // we need to initialize pcre cache table very early
    pcre_init();

    CompilerOptions po;
#ifdef FACEBOOK
    compiler_hook_initialize();
#endif

    int ret = prepareOptions(po, argc, argv);
    if (ret == 1) return 0; // --help
    if (ret == -1) return -1; // command line error

    Timer totalTimer(Timer::WallTime, "running hphp");
    createOutputDirectory(po);
    if (ret == 0) {
      if (!po.nofork && !Process::IsUnderGDB()) {
        int pid = fork();
        if (pid == 0) {
          ret = process(po);
          _exit(ret);
        }
        wait(&ret);
        ret = WIFEXITED(ret) ? WEXITSTATUS(ret) : -1;
      } else {
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
  } catch (Exception &e) {
    Logger::Error("Exception: %s\n", e.getMessage().c_str());
  } catch (const FailedAssertion& fa) {
    fa.print();
    StackTraceNoHeap::AddExtraLogging("Assertion failure", fa.summary);
    abort();
  } catch (std::exception &e) {
    Logger::Error("std::exception: %s\n", e.what());
  } catch (...) {
    Logger::Error("(unknown exception was thrown)\n");
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////

int prepareOptions(CompilerOptions &po, int argc, char **argv) {
  options_description desc("HipHop Compiler for PHP Usage:\n\n"
                           "\thphp <options> <inputs>\n\n"
                           "Options");
  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("target,t", value<string>(&po.target)->default_value("run"),
     "lint | "
     "analyze | "
     "php | "
     "hhbc | "
     "filecache | "
     "run (default)")
    ("format,f", value<string>(&po.format),
     "lint: (none); \n"
     "analyze: (none); \n"
     "php: trimmed (default) | inlined | pickled | typeinfo |"
     " <any combination of them by any separator>; \n"
     "hhbc: binary (default) | text; \n"
     "run: cluster (default) | file")
    ("input-dir", value<string>(&po.inputDir), "input directory")
    ("program", value<string>(&po.program)->default_value("program"),
     "final program name to use")
    ("args", value<string>(&po.programArgs), "program arguments")
    ("inputs,i", value<vector<string> >(&po.inputs), "input file names")
    ("input-list", value<string>(&po.inputList),
     "file containing list of file names, one per line")
    ("include-path",
     value<vector<string> >(&po.includePaths)->composing(),
     "a list of full paths to search for files being included in includes "
     "or requires but cannot be found assuming relative paths")
    ("module", value<vector<string> >(&po.modules)->composing(),
     "directories containing all input files")
    ("exclude-dir", value<vector<string> >(&po.excludeDirs)->composing(),
     "directories to exclude from the input")
    ("fmodule", value<vector<string> >(&po.fmodules)->composing(),
     "same with module, except no exclusion checking is performed, so these "
     "modules are forced to be included")
    ("ffile", value<vector<string> >(&po.ffiles)->composing(),
     "extra PHP files forced to include without exclusion checking")
    ("exclude-file", value<vector<string> >(&po.excludeFiles)->composing(),
     "files to exclude from the input, even if parse-on-demand finds it")
    ("exclude-pattern",
     value<vector<string> >(&po.excludePatterns)->composing(),
     "regex (in 'find' command's regex command line option format) of files "
     "or directories to exclude from the input, even if parse-on-demand finds "
     "it")
    ("exclude-static-pattern",
     value<vector<string> >(&po.excludeStaticPatterns)->composing(),
     "regex (in 'find' command's regex command line option format) of files "
     "or directories to exclude from static content cache")
    ("exclude-static-dir",
     value<vector<string> >(&po.excludeStaticDirs)->composing(),
     "directories to exclude from static content cache")
    ("exclude-static-file",
     value<vector<string> >(&po.excludeStaticFiles)->composing(),
     "files to exclude from static content cache")
    ("cfile", value<vector<string> >(&po.cfiles)->composing(),
     "extra static files forced to include without exclusion checking")
    ("cmodule", value<vector<string> >(&po.cmodules)->composing(),
     "extra directories for static files without exclusion checking")
    ("parse-on-demand", value<bool>(&po.parseOnDemand)->default_value(true),
     "whether to parse files that are not specified from command line")
    ("branch", value<string>(&po.branch), "SVN branch")
    ("revision", value<int>(&po.revision), "SVN revision")
    ("output-dir,o", value<string>(&po.outputDir), "output directory")
    ("sync-dir", value<string>(&po.syncDir),
     "Files will be created in this directory first, then sync with output "
     "directory without overwriting identical files. Great for incremental "
     "compilation and build.")
    ("optimize-level", value<int>(&po.optimizeLevel)->default_value(-1),
     "optimization level")
    ("gen-stats", value<bool>(&po.genStats)->default_value(false),
     "whether to generate code errors")
    ("keep-tempdir,k", value<bool>(&po.keepTempDir)->default_value(false),
     "whether to keep the temporary directory")
    ("db-stats", value<string>(&po.dbStats),
     "database connection string to save code errors: "
     "<username>:<password>@<host>:<port>/<db>")
    ("no-type-inference",
     value<bool>(&po.noTypeInference)->default_value(false),
     "turn off type inference for C++ code generation")
    ("config,c", value<vector<string> >(&po.config)->composing(),
     "config file name")
    ("config-dir", value<string>(&po.configDir),
     "root directory configuration is based on (for example, "
     "excluded directories may be relative path in configuration.")
    ("config-value,v", value<vector<string> >(&po.confStrings)->composing(),
     "individual configuration string in a format of name=value, where "
     "name can be any valid configuration for a config file")
    ("log,l",
     value<int>(&po.logLevel)->default_value(-1),
     "-1: (default); 0: no logging; 1: errors only; 2: warnings and errors; "
     "3: informational as well; 4: really verbose.")
    ("force",
     value<bool>(&po.force)->default_value(true),
     "force to ignore code generation errors and continue compilations")
    ("file-cache",
     value<string>(&po.filecache),
     "if specified, generate a static file cache with this file name")
    ("dump",
     value<bool>(&po.dump)->default_value(false),
     "dump the program graph")
    ("docjson",
     value<string>(&po.docjson)->default_value(""),
     "Filename to generate a JSON file for PHP docs")
    ("coredump",
     value<bool>(&po.coredump)->default_value(false),
     "turn on coredump")
    ("nofork",
     value<bool>(&po.nofork)->default_value(false),
     "forking is needed for large compilation to release memory before g++"
     "compilation. turning off forking can help gdb debugging.")
    ("opts",
     value<string>(&po.optimizations)->default_value(""),
     "Set optimizations to enable/disable")
    ("compiler-id", "display the git hash for the compiler id")
    ("repo-schema", "display the repo schema id used by this app")
    ;

  positional_options_description p;
  p.add("inputs", -1);
  variables_map vm;
  try {
    store(command_line_parser(argc, argv).options(desc).positional(p).run(),
          vm);
    notify(vm);
  } catch (const unknown_option& e) {
    Logger::Error("Error in command line: %s\n\n", e.what());
    cout << desc << "\n";
    return -1;
  }
  if (argc <= 1 || vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }
  if (vm.count("version")) {
#ifdef HHVM_VERSION
#undef HHVM_VERSION
#endif

#ifdef HPHP_COMPILER_STR
#undef HPHP_COMPILER_STR
#endif

#ifdef DEBUG
#define HPHP_COMPILER_STR "HipHop Compiler (Debug Build) v"
#else
#define HPHP_COMPILER_STR "HipHop Compiler v"
#endif

#define HHVM_VERSION(v) cout << HPHP_COMPILER_STR #v << "\n";
#include "../version" // nolint

    cout << "Compiler: " << kCompilerId << "\n";
    cout << "Repo schema: " << kRepoSchemaId << "\n";
    return 1;
  }

  if (vm.count("compiler-id")) {
    cout << kCompilerId << "\n";
    return 1;
  }

  if (vm.count("repo-schema")) {
    cout << kRepoSchemaId << "\n";
    return 1;
  }

  if ((po.target == "hhbc" || po.target == "run") &&
      po.format.find("exe") == string::npos) {
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

  Hdf config;
  for (vector<string>::const_iterator it = po.config.begin();
       it != po.config.end(); ++it) {
    config.append(*it);
  }
  for (unsigned int i = 0; i < po.confStrings.size(); i++) {
    config.fromString(po.confStrings[i].c_str());
  }
  Option::Load(config);
  vector<string> badnodes;
  config.lint(badnodes);
  for (unsigned int i = 0; i < badnodes.size(); i++) {
    Logger::Error("Possible bad config node: %s", badnodes[i].c_str());
  }

  if (po.dump) Option::DumpAst = true;

  if (po.inputDir.empty()) {
    po.inputDir = '.';
  }
  po.inputDir = Util::normalizeDir(po.inputDir);
  if (po.configDir.empty()) {
    po.configDir = po.inputDir;
  }
  po.configDir = Util::normalizeDir(po.configDir);
  Option::RootDirectory = po.configDir;
  Option::IncludeSearchPaths = po.includePaths;

  for (unsigned int i = 0; i < po.excludeDirs.size(); i++) {
    Option::PackageExcludeDirs.insert
      (Util::normalizeDir(po.excludeDirs[i]));
  }
  for (unsigned int i = 0; i < po.excludeFiles.size(); i++) {
    Option::PackageExcludeFiles.insert(po.excludeFiles[i]);
  }
  for (unsigned int i = 0; i < po.excludePatterns.size(); i++) {
    Option::PackageExcludePatterns.insert
      (Util::format_pattern(po.excludePatterns[i], true));
  }
  for (unsigned int i = 0; i < po.excludeStaticDirs.size(); i++) {
    Option::PackageExcludeStaticDirs.insert
      (Util::normalizeDir(po.excludeStaticDirs[i]));
  }
  for (unsigned int i = 0; i < po.excludeStaticFiles.size(); i++) {
    Option::PackageExcludeStaticFiles.insert(po.excludeStaticFiles[i]);
  }
  for (unsigned int i = 0; i < po.excludeStaticPatterns.size(); i++) {
    Option::PackageExcludeStaticPatterns.insert
      (Util::format_pattern(po.excludeStaticPatterns[i], true));
  }

  if (po.target == "hhbc" || po.target == "run") {
    Option::AnalyzePerfectVirtuals = false;
  }

  Option::ProgramName = po.program;

  if (po.format.empty()) {
    if (po.target == "php") {
      po.format = "trimmed";
    } else if (po.target == "run") {
      po.format = "binary";
    } else if (po.target == "hhbc") {
      po.format = "binary";
    }
  }

  if (!po.docjson.empty()) {
    if (po.target != "run" &&
        po.target != "hhbc" &&
        po.target != "analyze") {
      Logger::Error(
        "Cannot generate doc JSON file unless target is "
        "'hhbc', 'run', or 'analyze'");
    } else {
      Option::DocJson = po.docjson;
    }
  }

  if (po.optimizeLevel == -1) {
    po.optimizeLevel = 1;
  }

  // we always do pre/post opt no matter the opt level
  Option::PreOptimization = true;
  Option::PostOptimization = true;
  if (po.optimizeLevel == 0) {
    // --optimize-level=0 is equivalent to --opts=none
    po.optimizations = "none";
    Option::ParseTimeOpts = false;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

int process(const CompilerOptions &po) {
  if (po.coredump) {
#if defined(__APPLE__) || defined(__FreeBSD__)
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

  // lint doesn't need analysis
  if (po.target == "lint") {
    return lintTarget(po);
  }

  register_process_init();
  init_thread_locals();

  Timer timer(Timer::WallTime);
  AnalysisResultPtr ar;

  // prepare a package
  Package package(po.inputDir.c_str());
  ar = package.getAnalysisResult();

  hhbcTargetInit(po, ar);

  std::string errs;
  if (!AliasManager::parseOptimizations(po.optimizations, errs)) {
    Logger::Error("%s\n", errs.c_str());
    return false;
  }

  // one time initialization
  Type::InitTypeHintMap();
  BuiltinSymbols::LoadSuperGlobals();
  ClassInfo::Load();

  bool isPickledPHP = (po.target == "php" && po.format == "pickled");
  if (!isPickledPHP) {
    bool wp = Option::WholeProgram;
    Option::WholeProgram = false;
    BuiltinSymbols::s_systemAr = ar;
    hphp_process_init();
    BuiltinSymbols::s_systemAr.reset();
    Option::WholeProgram = wp;
    if (po.target == "hhbc" && !Option::WholeProgram) {
      // We're trying to produce the same bytecode as runtime parsing.
      // There's nothing to do.
    } else {
      if (!BuiltinSymbols::Load(ar)) {
        return false;
      }
    }
  }

  {
    Timer timer(Timer::WallTime, "parsing inputs");
    if (!po.inputs.empty() && isPickledPHP) {
      for (unsigned int i = 0; i < po.inputs.size(); i++) {
        package.addSourceFile(po.inputs[i].c_str());
      }
    } else {
      ar->setPackage(&package);
      ar->setParseOnDemand(po.parseOnDemand);
      if (!po.parseOnDemand) {
        ar->setParseOnDemandDirs(Option::ParseOnDemandDirs);
      }
      if (po.modules.empty() && po.fmodules.empty() &&
          po.ffiles.empty() && po.inputs.empty() && po.inputList.empty()) {
        package.addAllFiles(false);
      } else {
        for (unsigned int i = 0; i < po.modules.size(); i++) {
          package.addDirectory(po.modules[i], false);
        }
        for (unsigned int i = 0; i < po.fmodules.size(); i++) {
          package.addDirectory(po.fmodules[i], true);
        }
        for (unsigned int i = 0; i < po.ffiles.size(); i++) {
          package.addSourceFile(po.ffiles[i].c_str());
        }
        for (unsigned int i = 0; i < po.cmodules.size(); i++) {
          package.addStaticDirectory(po.cmodules[i].c_str());
        }
        for (unsigned int i = 0; i < po.cfiles.size(); i++) {
          package.addStaticFile(po.cfiles[i].c_str());
        }
        for (unsigned int i = 0; i < po.inputs.size(); i++) {
          package.addSourceFile(po.inputs[i].c_str());
        }
        if (!po.inputList.empty()) {
          package.addInputList(po.inputList.c_str());
        }
      }
    }
    if (po.target != "filecache") {
      if (!package.parse(!po.force)) {
        return 1;
      }
      if (Option::WholeProgram || po.target == "analyze") {
        Timer timer(Timer::WallTime, "analyzeProgram");
        ar->analyzeProgram();
      }
    }
  }

  // saving file cache
  AsyncFileCacheSaver fileCacheThread(&package, po.filecache.c_str());
  if (po.target != "analyze" && !po.filecache.empty()) {
    fileCacheThread.start();
  }

  if (Option::DumpAst) {
    ar->dump();
  }

  int ret = 0;
  if (po.target == "analyze") {
    ret = analyzeTarget(po, ar);
  } else if (po.target == "php") {
    ret = phpTarget(po, ar);
  } else if (po.target == "hhbc") {
    ret = hhbcTarget(po, ar, fileCacheThread);
  } else if (po.target == "run") {
    ret = runTargetCheck(po, ar, fileCacheThread);
  } else if (po.target == "filecache") {
    // do nothing
  } else {
    Logger::Error("Unknown target: %s", po.target.c_str());
    return 1;
  }

  if (Option::DumpAst) {
    ar->dump();
  }

  if (!Option::DocJson.empty()) {
    Timer timer(Timer::WallTime, "Saving doc JSON file");
    ar->docJson(Option::DocJson);
  }

  // saving stats
  if (po.target == "analyze" || po.genStats || !po.dbStats.empty()) {
    int seconds = timer.getMicroSeconds() / 1000000;

    Logger::Info("saving code errors and stats...");
    Timer timer(Timer::WallTime, "saving stats");

    if (!po.dbStats.empty()) {
      try {
        ServerDataPtr server = ServerData::Create(po.dbStats);
        int runId = package.saveStatsToDB(server, seconds, po.branch,
                                          po.revision);
        package.commitStats(server, runId);
      } catch (const DatabaseException& e) {
        Logger::Error("%s", e.what());
      }
    } else {
      Compiler::SaveErrors(ar, (po.outputDir + "/CodeError.js").c_str());
      package.saveStatsToFile((po.outputDir + "/Stats.js").c_str(), seconds);
    }
  } else if (Compiler::HasError()) {
    Logger::Info("saving code errors...");
    Compiler::SaveErrors(ar, (po.outputDir + "/CodeError.js").c_str());
  }

  if (!po.filecache.empty()) {
    fileCacheThread.waitForEnd();
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int lintTarget(const CompilerOptions &po) {
  int ret = 0;
  for (unsigned int i = 0; i < po.inputs.size(); i++) {
    string filename = po.inputDir + "/" + po.inputs[i];
    try {
      Scanner scanner(filename.c_str(), Option::GetScannerType());
      Compiler::Parser parser(scanner, filename.c_str(),
                              AnalysisResultPtr(new AnalysisResult()));
      if (!parser.parse()) {
        Logger::Error("Unable to parse file %s: %s", filename.c_str(),
                      parser.getMessage().c_str());
        ret = 1;
      } else {
        Logger::Info("%s parsed successfully...", filename.c_str());
      }
    } catch (FileOpenException &e) {
      Logger::Error("%s", e.getMessage().c_str());
      ret = 1;
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int analyzeTarget(const CompilerOptions &po, AnalysisResultPtr ar) {
  int ret = 0;

  if (!po.noTypeInference) {
    Option::GenerateInferredTypes = true;
  }
  if (Option::PreOptimization) {
    Timer timer(Timer::WallTime, "pre-optimizing");
    ar->preOptimize();
  }

  if (!Option::AllVolatile) {
    Timer timer(Timer::WallTime, "analyze includes");
    ar->analyzeIncludes();
  }

  if (Option::GenerateInferredTypes) {
    Timer timer(Timer::WallTime, "inferring types");
    ar->inferTypes();
  }
  if (Option::PostOptimization) {
    Timer timer(Timer::WallTime, "post-optimizing");
    ar->postOptimize();
  }
  ar->analyzeProgramFinal();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int phpTarget(const CompilerOptions &po, AnalysisResultPtr ar) {
  int ret = 0;

  // format
  int formatCount = 0;
  if (po.format.find("pickled") != string::npos) {
    Option::GeneratePickledPHP = true;
    formatCount++;
  }
  if (po.format.find("inlined") != string::npos) {
    Option::GenerateInlinedPHP = true;
    formatCount++;
  }
  if (po.format.find("trimmed") != string::npos) {
    Option::GenerateTrimmedPHP = true;
    formatCount++;
  }
  if (po.format.find("typeinfo") != string::npos) {
    Option::GenerateInferredTypes = true;
  }
  if (formatCount == 0) {
    Logger::Error("Unknown format for PHP target: %s", po.format.c_str());
    return 1;
  }

  // analyze
  if (Option::GenerateInferredTypes || Option::ConvertSuperGlobals) {
    Logger::Info("inferring types...");
    ar->inferTypes();
  }

  // generate
  ar->setOutputPath(po.outputDir);
  if (Option::GeneratePickledPHP) {
    Logger::Info("creating pickled PHP files...");
    string outputDir = po.outputDir;
    if (formatCount > 1) outputDir += "/pickled";
    mkdir(outputDir.c_str(), 0777);
    ar->outputAllPHP(CodeGenerator::PickledPHP);
  }
  if (Option::GenerateInlinedPHP) {
    Logger::Info("creating inlined PHP files...");
    string outputDir = po.outputDir;
    if (formatCount > 1) outputDir += "/inlined";
    mkdir(outputDir.c_str(), 0777);
    if (!ar->outputAllPHP(CodeGenerator::InlinedPHP)) {
      ret = -1;
    }
  }
  if (Option::GenerateTrimmedPHP) {
    Logger::Info("creating trimmed PHP files...");
    string outputDir = po.outputDir;
    if (formatCount > 1) outputDir += "/trimmed";
    mkdir(outputDir.c_str(), 0777);
    if (!ar->outputAllPHP(CodeGenerator::TrimmedPHP)) {
      ret = -1;
    }
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
  if (po.format.find("exe") != string::npos) {
    RuntimeOption::RepoCentralPath += ".hhbc";
  }
  unlink(RuntimeOption::RepoCentralPath.c_str());
  RuntimeOption::RepoLocalMode = "--";
  RuntimeOption::RepoDebugInfo = Option::RepoDebugInfo;
  RuntimeOption::RepoJournal = "memory";
  RuntimeOption::EnableHipHopSyntax = Option::EnableHipHopSyntax;
  RuntimeOption::EnableZendCompat = Option::EnableZendCompat;
  RuntimeOption::EvalJitEnableRenameFunction = Option::JitEnableRenameFunction;

  // Turn off commits, because we don't want systemlib to get included
  RuntimeOption::RepoCommit = false;
}

int hhbcTarget(const CompilerOptions &po, AnalysisResultPtr ar,
               AsyncFileCacheSaver &fcThread) {
  int ret = 0;
  int formatCount = 0;
  const char *type = 0;
  if (po.format.find("text") != string::npos) {
    Option::GenerateTextHHBC = true;
    type = "creating text HHBC files";
    formatCount++;
  }
  if (po.format.find("binary") != string::npos) {
    Option::GenerateBinaryHHBC = true;
    type = "creating binary HHBC files";
    formatCount++;
  }
  if (po.format.find("exe") != string::npos) {
    Option::GenerateBinaryHHBC = true;
    type = "creating binary HHBC files";
    formatCount++;
  }

  if (formatCount == 0) {
    Logger::Error("Unknown format for HHBC target: %s", po.format.c_str());
    return 1;
  }

  /* without this, emitClass allows classes with interfaces to be
     hoistable */
  SystemLib::s_inited = true;
  RuntimeOption::RepoCommit = true;
  Option::AutoInline = -1;

  if (po.optimizeLevel > 0) {
    ret = analyzeTarget(po, ar);
  }

  Timer timer(Timer::WallTime, type);
  Compiler::emitAllHHBC(ar);

  if (!po.syncDir.empty()) {
    if (!po.filecache.empty()) {
      fcThread.waitForEnd();
    }
    Util::syncdir(po.outputDir, po.syncDir);
    boost::filesystem::remove_all(po.syncDir);
  }

  if (!ret && po.format.find("exe") != string::npos) {
    /*
     * We need to create an executable with the repo
     * embedded in it.
     * Copy ourself, and embed the repo as a section
     * named "repo".
     */
    string exe = po.outputDir + '/' + po.program;
    string repo = "repo=" + exe + ".hhbc";
    string buf = current_executable_path();
    if (buf.empty()) return -1;

    const char *argv[] = { "objcopy", "--add-section", repo.c_str(),
                           buf.c_str(), exe.c_str(), 0 };
    string out;
    ret = Process::Exec(argv[0], argv, nullptr, out, nullptr) ? 0 : 1;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int runTargetCheck(const CompilerOptions &po, AnalysisResultPtr ar,
                   AsyncFileCacheSaver &fcThread) {
  // generate code
  if (hhbcTarget(po, ar, fcThread)) {
    return 1;
  }

  // check error
  if (Compiler::HasError() && !po.force) {
    Compiler::DumpErrors(ar);
    return 1;
  }

  return 0;
}

int runTarget(const CompilerOptions &po) {
  int ret = 0;

  // If there are more than one input files, we need one extra arg to run.
  // If it's missing, we will stop right here, with compiled code.
  if ((po.inputs.size() != 1 && po.programArgs.empty()) ||
      !po.inputList.empty()) {
    return 0;
  }

  // run the executable
  string cmd;
  if (po.format.find("exe") == string::npos) {
    string buf = current_executable_path();
    if (buf.empty()) return -1;

    cmd += buf;
    cmd += " -vRepo.Authoritative=true";
    if (getenv("HPHP_DUMP_BYTECODE")) cmd += " -vEval.DumpBytecode=1";
    cmd += " -vRepo.Local.Mode=r- -vRepo.Local.Path=";
  }
  cmd += po.outputDir + '/' + po.program;
  cmd += string(" --file ") +
    (po.inputs.size() == 1 ? po.inputs[0] : "") +
    " " + po.programArgs;
  Logger::Info("running executable: %s", cmd.c_str());
  ret = Util::ssystem(cmd.c_str());
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
    string temp = t;
    temp += "/hphp_XXXXXX";
    char path[PATH_MAX + 1];
    strncpy(path, temp.c_str(), PATH_MAX);
    path[PATH_MAX] = '\0';
    po.outputDir = mkdtemp(path);
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
