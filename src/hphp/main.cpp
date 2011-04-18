/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <compiler/package.h>
#include <compiler/analysis/analysis_result.h>
#include <compiler/analysis/alias_manager.h>
#include <compiler/analysis/code_error.h>
#include <util/json.h>
#include <util/logger.h>
#include <compiler/analysis/symbol_table.h>
#include <compiler/option.h>
#include <compiler/parser/parser.h>
#include <compiler/builtin_symbols.h>
#include <util/db_conn.h>
#include <util/exception.h>
#include <util/process.h>
#include <util/util.h>
#include <util/timer.h>
#include <util/hdf.h>
#include <util/async_func.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/externals.h>
#include <runtime/base/thread_init_fini.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>

using namespace HPHP;
using namespace std;
using namespace boost::program_options;

///////////////////////////////////////////////////////////////////////////////

struct ProgramOptions {
  string target;
  string format;
  string outputDir;
  string outputFile;
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
  vector<string> parseOnDemandDirs; // parse these directories on-demand
                                    // when parseOnDemand=false
  string program;
  string programArgs;
  string branch;
  int revision;
  bool genStats;
  bool keepTempDir;
  string dbStats;
  bool noTypeInference;
  bool noMinInclude;
  bool noMetaInfo;
  int logLevel;
  bool force;
  int clusterCount;
  int optimizeLevel;
  string filecache;
  string rttiDirectory;
  string javaRoot;
  bool generateFFI;
  bool dump;
  bool coredump;
  bool nofork;
  bool fl_annotate;
  string optimizations;
  string ppp;
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
    Logger::Info("%dMB %s saved", (int64)sb.st_size/(1024*1024), m_name);
  }

private:
  Package *m_package;
  const char *m_name;
};

///////////////////////////////////////////////////////////////////////////////
// forward declarations

int prepareOptions(ProgramOptions &po, int argc, char **argv);
void createOutputDirectory(ProgramOptions &po);
int process(const ProgramOptions &po);
int lintTarget(const ProgramOptions &po);
int analyzeTarget(const ProgramOptions &po, AnalysisResultPtr ar);
int phpTarget(const ProgramOptions &po, AnalysisResultPtr ar);
int cppTarget(const ProgramOptions &po, AnalysisResultPtr ar,
              AsyncFileCacheSaver &fcThread, bool allowSys = true);
int runTargetCheck(const ProgramOptions &po, AnalysisResultPtr ar,
                   AsyncFileCacheSaver &fcThread);
int buildTarget(const ProgramOptions &po);
int runTarget(const ProgramOptions &po);
int generateSepExtCpp(const ProgramOptions &po, AnalysisResultPtr ar);

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
  try {
    ProgramOptions po;
    void (*compiler_hook_initialize)();
    compiler_hook_initialize =
      (void (*)())dlsym(NULL, "compiler_hook_initialize");
    if (compiler_hook_initialize) compiler_hook_initialize();

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
      if (po.target == "cpp") {
        if (po.format == "exe" || po.format == "lib") {
          ret = buildTarget(po);
        }
      } else if (po.target == "run") {
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
  } catch (std::exception &e) {
    Logger::Error("std::exception: %s\n", e.what());
  } catch (...) {
    Logger::Error("(unknown exception was thrown)\n");
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////

int prepareOptions(ProgramOptions &po, int argc, char **argv) {
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
     "cpp | "
     "sep-ext-cpp | "
     "filecache | "
     "run (default)")
    ("format,f", value<string>(&po.format),
     "lint: (none); \n"
     "analyze: (none); \n"
     "php: trimmed (default) | inlined | pickled | typeinfo |"
     " <any combination of them by any separator>; \n"
     "cpp: cluster (default) | file | sys | exe | lib; \n"
     "run: cluster (default) | file")
    ("cluster-count", value<int>(&po.clusterCount)->default_value(0),
     "Cluster by file sizes and output roughly these many number of files. "
     "Use 0 for no clustering.")
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
    ("output-file", value<string>(&po.outputFile), "output file")
    ("sync-dir", value<string>(&po.syncDir),
     "Files will be created in this directory first, then sync with output "
     "directory without overwriting identical files. Great for incremental "
     "compilation and build.")
    ("optimize-level", value<int>(&po.optimizeLevel)->default_value(1),
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
    ("no-min-include",
     value<bool>(&po.noMinInclude)->default_value(false),
     "turn off minimium include analysis when target is \"analyze\"")
    ("no-meta-info",
     value<bool>(&po.noMetaInfo)->default_value(false),
     "do not generate class map, function jump table and macros "
     "when generating code; good for demo purposes")
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
    ("rtti-directory", value<string>(&po.rttiDirectory)->default_value(""),
     "the directory of rtti profiling data")
    ("java-root",
     value<string>(&po.javaRoot)->default_value("php"),
     "the root package of generated Java FFI classes")
    ("generate-ffi",
     value<bool>(&po.generateFFI)->default_value(false),
     "generate ffi stubs")
    ("dump",
     value<bool>(&po.dump)->default_value(false),
     "dump the program graph")
    ("coredump",
     value<bool>(&po.coredump)->default_value(false),
     "turn on coredump")
    ("nofork",
     value<bool>(&po.nofork)->default_value(false),
     "forking is needed for large compilation to release memory before g++"
     "compilation. turning off forking can help gdb debugging.")
    ("fl-annotate",
     value<bool>(&po.fl_annotate)->default_value(false),
     "Annote emitted source with compiler file-line info")
    ("opts",
     value<string>(&po.optimizations)->default_value("none"),
     "Set optimizations to enable/disable")
    ("ppp",
     value<string>(&po.ppp)->default_value(""),
     "Preprocessed partition configuration. To speed up distcc compilation, "
     "bin/ppp.php can pre-compute better partition between different .cpp "
     "files according to preprocessed file sizes, instead of original file "
     "sizes (default). Run bin/ppp.php to generate an HDF configuration file "
     "to specify here.")
#ifdef COMPILER_ID
    ("compiler-id", "display the git hash for the compiler id")
#endif
    ;

  positional_options_description p;
  p.add("inputs", -1);
  variables_map vm;
  try {
    store(command_line_parser(argc, argv).options(desc).positional(p).run(),
          vm);
    notify(vm);
  } catch (unknown_option e) {
    Logger::Error("Error in command line: %s\n\n", e.what());
    cout << desc << "\n";
    return -1;
  }
  if (argc <= 1 || vm.count("help")) {
    cout << desc << "\n";
    return 1;
  }
  if (vm.count("version")) {
#ifdef HPHP_VERSION
#undef HPHP_VERSION
#endif
#define HPHP_VERSION(v) cout << "HipHop Compiler v" #v << "\n";
#include "../version"

#ifdef COMPILER_ID
cout << "Compiler: " << COMPILER_ID << "\n";
#endif
    return 1;
  }

#ifdef COMPILER_ID
  if (vm.count("compiler-id")) {
    cout << COMPILER_ID << "\n";
    return 1;
  }
#endif

  // log level
  if (po.logLevel != -1) {
    Logger::LogLevel = (Logger::LogLevelType)po.logLevel;
  } else if (po.target == "run") {
    Logger::LogLevel = Logger::LogNone;
  } else {
    Logger::LogLevel = Logger::LogInfo;
  }

  // config and system
  Option::GenerateCPPMain = true;
  if (po.noMetaInfo) {
    Option::GenerateCPPMetaInfo = false;
    Option::GenerateCPPMacros = false;
  }
  Option::FlAnnotate = po.fl_annotate;

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

  if (po.target == "cpp" && po.format == "sys") {
    BuiltinSymbols::NoSuperGlobals = true; // so to generate super globals
    Option::AnalyzePerfectVirtuals = false;
  }
  Option::SystemGen = (po.target == "cpp" && po.format == "sys") ;

  Option::ProgramName = po.program;
  Option::PreprocessedPartitionConfig = po.ppp;

  if (po.target == "cpp") {
    if (po.format.empty()) po.format = "cluster";
  } else if (po.target == "php") {
    if (po.format.empty()) po.format = "trimmed";
  } else if (po.target == "run") {
    if (po.format.empty()) po.format = "cluster";
  }

  if (po.optimizeLevel > 0) {
    Option::PreOptimization = true;
    Option::PostOptimization = true;
  }

  if (po.generateFFI) {
    Option::GenerateFFI = true;
    Option::JavaFFIRootPackage = po.javaRoot;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

int process(const ProgramOptions &po) {
  if (po.coredump) {
#if defined(__APPLE__)
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

  init_thread_locals();

  Timer timer(Timer::WallTime);
  AnalysisResultPtr ar;

  // prepare a package
  Package package(po.inputDir.c_str());
  ar = package.getAnalysisResult();

  std::string errs;
  if (!AliasManager::parseOptimizations(po.optimizations, errs)) {
    Logger::Error("%s\n", errs.c_str());
    return false;
  }

  if (po.target != "php" || po.format != "pickled") {
    if (!BuiltinSymbols::Load(ar, po.target == "cpp" && po.format == "sys")) {
      return false;
    }
    ar->loadBuiltins();
  }

  {
    Timer timer(Timer::WallTime, "parsing inputs");
    if (!po.inputs.empty() && po.target == "php" && po.format == "pickled") {
      for (unsigned int i = 0; i < po.inputs.size(); i++) {
        package.addSourceFile(po.inputs[i].c_str());
      }
      if (!package.parse()) {
        return 1;
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
      {
        if (!package.parse()) {
          return 1;
        }
      }
      ar->analyzeProgram();
    }
  }

  // saving file cache
  AsyncFileCacheSaver fileCacheThread(&package, po.filecache.c_str());
  if (!po.filecache.empty()) {
    fileCacheThread.start();
  }

  if (Option::DumpAst) {
    ar->dump();
  }

  int ret = 0;
  bool fatalErrorOnly = false;
  if (po.target == "analyze") {
    ret = analyzeTarget(po, ar);
  } else if (po.target == "php") {
    ret = phpTarget(po, ar);
  } else if (po.target == "cpp") {
    ret = cppTarget(po, ar, fileCacheThread);
    fatalErrorOnly = true;
  } else if (po.target == "run") {
    ret = runTargetCheck(po, ar, fileCacheThread);
    fatalErrorOnly = true;
  } else if (po.target == "filecache") {
    // do nothing
  } else if (po.target == "sep-ext-cpp") {
    ar->setSepExtension();
    ret = generateSepExtCpp(po, ar);
  } else {
    Logger::Error("Unknown target: %s", po.target.c_str());
    return 1;
  }

  if (Option::DumpAst) {
    ar->dump();
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
      } catch (DatabaseException e) {
        Logger::Error("%s", e.what());
      }
    } else {
      Compiler::SaveErrors((po.outputDir + "/CodeError.js").c_str());
      package.saveStatsToFile((po.outputDir + "/Stats.js").c_str(), seconds);
    }
  } else if (Compiler::HasError()) {
    Logger::Info("saving code errors...");
    Compiler::SaveErrors((po.outputDir + "/CodeError.js").c_str());
  }

  if (!po.filecache.empty()) {
    fileCacheThread.waitForEnd();
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int lintTarget(const ProgramOptions &po) {
  int ret = 0;
  for (unsigned int i = 0; i < po.inputs.size(); i++) {
    string filename = po.inputDir + "/" + po.inputs[i];
    try {
      Scanner scanner(filename.c_str(), Option::ScannerType);
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

int analyzeTarget(const ProgramOptions &po, AnalysisResultPtr ar) {
  int ret = 0;

  if (!po.noTypeInference) {
    Option::GenerateInferredTypes = true;
  }
  if (Option::GenerateInferredTypes) {
    Timer timer(Timer::WallTime, "inferring types");
    ar->inferTypes();
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int phpTarget(const ProgramOptions &po, AnalysisResultPtr ar) {
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

int cppTarget(const ProgramOptions &po, AnalysisResultPtr ar,
              AsyncFileCacheSaver &fcThread, bool allowSys /* = true */) {
  int ret = 0;
  int clusterCount = po.clusterCount;
  // format
  CodeGenerator::Output format = CodeGenerator::InvalidOutput;
  if (po.format == "file") {
    clusterCount = 0;
    format = CodeGenerator::FileCPP;
  } else if (po.format == "cluster") {
    format = CodeGenerator::ClusterCPP;
  } else if (po.format == "sys" && allowSys) {
    clusterCount = 0;
    format = CodeGenerator::SystemCPP;
    ar->setSystem();
  } else if (po.format == "exe" || po.format == "lib") {
    format = CodeGenerator::ClusterCPP;
  }

  if (format == CodeGenerator::InvalidOutput) {
    Logger::Error("Unknown format for CPP target: %s", po.format.c_str());
    return 1;
  }

  if (!po.noTypeInference) {
    Option::GenerateInferredTypes = true;
  }
  if (Option::PreOptimization) {
    Timer timer(Timer::WallTime, "pre-optimizing");
    ar->preOptimize();
  }


  if (!Option::RTTIOutputFile.empty()) {
    if (!po.rttiDirectory.empty()) {
      Option::UseRTTIProfileData = true;
      ar->cloneRTTIFuncs(po.rttiDirectory.c_str());
    } else {
      Option::GenRTTIProfileData = true;
    }
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

  {
    Timer timer(Timer::WallTime, "creating CPP files");
    if (po.syncDir.empty()) {
      ar->setOutputPath(po.outputDir);
      ar->outputAllCPP(format, clusterCount, NULL);
    } else {
      ar->setOutputPath(po.syncDir);
      ar->outputAllCPP(format, clusterCount, &po.outputDir);
      if (!po.filecache.empty()) {
        fcThread.waitForEnd();
      }
      Util::syncdir(po.outputDir, po.syncDir);
      boost::filesystem::remove_all(po.syncDir);
    }
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

int generateSepExtCpp(const ProgramOptions &po, AnalysisResultPtr ar) {
  ar->outputCPPSepExtensionImpl(po.outputFile);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

int buildTarget(const ProgramOptions &po) {
  const char *HPHP_HOME = getenv("HPHP_HOME");
  if (!HPHP_HOME || !*HPHP_HOME) {
    throw Exception("Environment variable HPHP_HOME is not set.");
  }
  string cmd = string(HPHP_HOME) + "/bin/run.sh";
  string flags;
  if (getenv("RELEASE"))      flags += "RELEASE=1 ";
  if (getenv("SHOW_LINK"))    flags += "SHOW_LINK=1 ";
  if (getenv("SHOW_COMPILE")) flags += "SHOW_COMPILE=1 ";
  if (po.format == "lib")     flags += "HPHP_BUILD_LIBRARY=1 ";
  if (Option::GenerateFFI)    flags += "HPHP_BUILD_FFI=1 ";
  const char *argv[] = {"", po.outputDir.c_str(),
                        po.program.c_str(), flags.c_str(), NULL};

  if (getenv("SHOW_COMPILE")) {
    Logger::Info ("Compile command: %s %s %s", po.outputDir.c_str(),
        po.program.c_str(), flags.c_str());
  }
  Timer timer(Timer::WallTime, "compiling and linking CPP files");
  string out, err;
  bool ret = Process::Exec(cmd.c_str(), argv, NULL, out, &err);
  if (getenv("SHOW_COMPILE")) {
    Logger::Info("%s", out.c_str());
  } else {
    Logger::Verbose("%s", out.c_str());
  }
  Logger::Error("%s", err.c_str());
  if (!ret) {
    return 1;
  }

  return 0;
}

int runTargetCheck(const ProgramOptions &po, AnalysisResultPtr ar,
                   AsyncFileCacheSaver &fcThread) {
  // generate code
  if (po.format != "sep" && cppTarget(po, ar, fcThread, false)) {
    return 1;
  }

  // check error
  if (Compiler::HasError() && !po.force) {
    Compiler::DumpErrors();
    return 1;
  }

  return 0;
}

int runTarget(const ProgramOptions &po) {
  int ret = buildTarget(po);
  if (ret) {
    return ret;
  }

  // If there are more than one input files, we need one extra arg to run.
  // If it's missing, we will stop right here, with compiled code.
  if (po.inputs.size() != 1 && po.programArgs.empty() ||
      !po.inputList.empty()) {
    return 0;
  }

  // run the executable
  string cmd = po.outputDir + '/' + po.program + ' ' + "--file " +
    (po.inputs.size() == 1 ? po.inputs[0] : "") + po.programArgs;
  Logger::Info("running executable %s...", cmd.c_str());
  Util::ssystem(cmd.c_str());

  // delete the temporary directory if not needed
  if (!po.keepTempDir) {
    Logger::Info("deleting temporary directory %s...", po.outputDir.c_str());
    boost::filesystem::remove_all(po.outputDir);
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////

void createOutputDirectory(ProgramOptions &po) {
  if (po.outputDir.empty()) {
    string temp = po.outputDir;
    if (temp.empty()) {
      temp = "/tmp";
    }
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
