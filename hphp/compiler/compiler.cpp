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

#include "hphp/compiler/option.h"
#include "hphp/compiler/package.h"

#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/options.h"

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/version.h"

#include "hphp/runtime/vm/builtin-symbol-map.h"
#include "hphp/runtime/vm/disas.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/async-func.h"
#include "hphp/util/build-info.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/exception.h"
#include "hphp/util/hdf.h"
#include "hphp/util/job-queue.h"
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

#include <cerrno>
#include <exception>
#include <filesystem>
#include <fstream>

#include <folly/portability/SysStat.h>

using namespace boost::program_options;

namespace coro = folly::coro;

namespace HPHP {

using namespace extern_worker;

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

struct CompilerOptions {
  std::string outputDir;
  std::vector<std::string> config;
  std::vector<std::string> confStrings;
  std::vector<std::string> iniStrings;
  std::string repoOptionsDir;
  std::string inputDir;
  std::vector<std::string> inputs;
  std::string inputList;
  std::vector<std::string> dirs;
  std::vector<std::string> excludeDirs;
  std::vector<std::string> excludeFiles;
  std::vector<std::string> excludePatterns;
  std::vector<std::string> excludeStaticDirs;
  std::vector<std::string> excludeStaticFiles;
  std::vector<std::string> excludeStaticPatterns;
  std::vector<std::string> cfiles;
  std::vector<std::string> cdirs;
  std::string push_phases;
  std::string matched_overrides;
  int logLevel;
  std::string filecache;
  bool coredump;
  std::string ondemandEdgesPath;
};

///////////////////////////////////////////////////////////////////////////////

void applyBuildOverrides(IniSetting::Map& ini,
                         Hdf& config,
                         CompilerOptions& po) {
  std::string push_phases = Config::GetString(ini, config, "Build.PushPhases");
  po.push_phases = push_phases;
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
                       po.push_phases));
      loggedOnce = true;
    }
    if (Config::matchHdfPattern(push_phases, ini, hdf, "push_phase" , "m")) {
      Logger::Info(folly::sformat("Matched override: {}", hdf.getName()));
      folly::format(
        &po.matched_overrides,
        "{}{}",
        po.matched_overrides.empty() ? "" : ",",
        hdf.getName()
      );

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

// Parse queryStr as a JSON-encoded watchman query expression, adding the the
// directories specified in the query to package. Only supports 'expression'
// queries and the 'dirname' term.
bool addAutoloadQueryToPackage(Package& package, const std::string& queryStr) {
  try {
    auto query = folly::parseJson(queryStr);
    if (!query.isObject()) {
      Logger::FError("Autoload.Query is not a JSON Object");
      return false;
    }
    auto expr = query["expression"];
    for (auto& term : expr) {
      if (term.isArray() && term[0] == "dirname") {
        Logger::FInfo("adding autoload dir {}", term[1].asString());
        package.addDirectory(term[1].asString());
      }
    }
    return true;
  } catch (const folly::json::parse_error& e) {
    Logger::FError("Error JSON-parsing Autoload.Query = \"{}\": {}",
                   queryStr, e.what());
    return false;
  }
}

void addListToPackage(Package& package, const std::vector<std::string>& dirs,
                     const CompilerOptions& po) {
  namespace fs = std::filesystem;
  std::string prefix{""};
  if (po.repoOptionsDir != po.inputDir) {
    auto const input = fs::path(po.inputDir);
    auto const rdr = fs::path(po.repoOptionsDir);
    prefix = fs::relative(po.repoOptionsDir, po.inputDir).native();
    if (!prefix.empty() && prefix.back() != '/') prefix += '/';
  }
  for (auto const& dir : dirs) {
    Logger::FInfo("adding autoload dir {}", dir);
    package.addDirectory(prefix + dir);
  }
}

void addInputsToPackage(Package& package, const CompilerOptions& po) {
  if (po.dirs.empty() && po.inputs.empty() && po.inputList.empty()) {
    package.addDirectory("/");
  } else {
    for (auto const& dir : po.dirs) {
      package.addDirectory(dir);
    }
    for (auto const& cdir : po.cdirs) {
      package.addStaticDirectory(cdir);
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
}

void genText(const UnitEmitter& ue, const std::string& outputPath) {
  assertx(Option::GenerateTextHHBC || Option::GenerateHhasHHBC);

  auto const unit = ue.create();

  auto const basePath = [&] {
    auto fullPath = outputPath;
    if (!fullPath.empty() &&
        !FileUtil::isDirSeparator(fullPath[fullPath.size() - 1])) {
      fullPath += FileUtil::getDirSeparator();
    }

    auto const fileName = "php/" + unit->filepath()->toCppString();
    if (fileName.size() > 4 &&
        fileName.substr(fileName.length() - 4) == ".php") {
      fullPath += fileName.substr(0, fileName.length() - 4);
    } else {
      fullPath += fileName;
    }

    for (auto pos = outputPath.size(); pos < fullPath.size(); pos++) {
      if (FileUtil::isDirSeparator(fullPath[pos])) {
        mkdir(fullPath.substr(0, pos).c_str(), 0777);
      }
    }
    return fullPath;
  }();

  if (Option::GenerateTextHHBC) {
    auto const fullPath = basePath + ".hhbc.txt";
    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      f << "Hash: " << ue.sha1().toString() << std::endl;
      f << unit->toString();
      f.close();
    }
  }

  if (Option::GenerateHhasHHBC) {
    auto const fullPath = basePath + ".hhas";
    std::ofstream f(fullPath.c_str());
    if (!f) {
      Logger::Error("Unable to open %s for write", fullPath.c_str());
    } else {
      f << disassemble(unit.get());
      f.close();
    }
  }
}

/*
 * It's an invariant that symbols in the repo must be Unique and
 * Persistent. Verify all relevant symbols are unique and set the
 * appropriate Attrs.
 */
struct SymbolSets {
  SymbolSets() {
    // These aren't stored in the repo, but we still need to check for
    // collisions against them, so put them in the maps.
    for (auto const& kv : Native::getConstants()) {
      assertx(kv.second.m_type != KindOfUninit);
      add(constants, kv.first, nullptr, "constant");
    }
  }

  // For local parses, where we have an UnitEmitter
  void add(UnitEmitter& ue) {
    // Verify uniqueness of symbols and set Attrs appropriately.
    auto const path = ue.m_filepath;

    add(units, path, path, "unit");

    for (auto const pce : ue.preclasses()) {
      pce->setAttrs(pce->attrs() | AttrPersistent);
      if (pce->attrs() & AttrEnum) add(enums, pce->name(), path, "enum");
      add(classes, pce->name(), path, "class", typeAliases);
    }
    for (auto& fe : ue.fevec()) {
      if (fe->attrs & AttrIsMethCaller) {
        if (addNoFail(funcs, fe->name, path, "function")) {
          fe->attrs |= AttrPersistent;
        }
      } else {
        fe->attrs |= AttrPersistent;
        add(funcs, fe->name, path, "function");
      }
    }
    for (auto& te : ue.typeAliases()) {
      te->setAttrs(te->attrs() | AttrPersistent);
      add(typeAliases, te->name(), path, "type alias", classes);
    }
    for (auto& c : ue.constants()) {
      c.attrs |= AttrPersistent;
      add(constants, c.name, path, "constant");
    }
    for (auto& m : ue.modules()) {
      m.attrs |= AttrPersistent;
      add(modules, m.name, path, "module");
    }
  }

  // For remote parses, where we don't have an UnitEmitter
  void add(const Package::ParseMeta::Definitions& d, const StringData* path) {
    add(units, path, path, "unit");

    for (auto const& c : d.m_classes) {
      add(classes, c, path, "class", typeAliases);
    }
    for (auto const& e : d.m_enums) {
      add(enums, e, path, "enum");
      add(classes, e, path, "class", typeAliases);
    }
    for (auto const& f : d.m_funcs) {
      add(funcs, f, path, "function");
    }
    for (auto const& m : d.m_methCallers) {
      addNoFail(funcs, m, path, "function");
    }
    for (auto const& a : d.m_typeAliases) {
      add(typeAliases, a, path, "type alias", classes);
    }
    for (auto const& c : d.m_constants) {
      add(constants, c, path, "constant");
    }
    for (auto const& m : d.m_modules) {
      add(modules, m, path, "module");
    }
  }

  struct NonUnique : std::runtime_error {
    using std::runtime_error::runtime_error;
  };

private:
  template <typename T>
  void add(T& map,
           const StringData* name,
           const StringData* unit,
           const char* type) {
    assertx(name->isStatic());
    assertx(!unit || unit->isStatic());
    auto const ret = map.emplace(name, unit);
    if (!ret.second) return fail(name, unit, ret.first->second, type);
  }

  template <typename T>
  bool addNoFail(T& map,
                 const StringData* name,
                 const StringData* unit,
                 const char* type) {
    assertx(name->isStatic());
    assertx(!unit || unit->isStatic());
    return map.emplace(name, unit).second;
  }

  template <typename T, typename E>
  void add(T& map,
           const StringData* name,
           const StringData* unit,
           const char* type,
           const E& other) {
    assertx(name->isStatic());
    assertx(!unit || unit->isStatic());
    auto const it = other.find(name);
    if (it != other.end()) return fail(name, unit, it->second, "symbol");
    add(map, name, unit, type);
  }

  [[noreturn]]
  void fail(const StringData* name,
            const StringData* unit1,
            const StringData* unit2,
            const char* type) {
    auto const filename = [] (const StringData* u) {
      if (!u) return "BUILTIN";
      return u->data();
    };

    throw NonUnique{
      folly::sformat(
        "More than one {} with the name {}. In {} and {}",
        type,
        name,
        filename(unit1),
        filename(unit2)
      )
    };
  }

  using IMap = folly_concurrent_hash_map_simd<
    const StringData*,
    const StringData*,
    string_data_hash,
    string_data_isame
  >;
  using Map = folly_concurrent_hash_map_simd<
    const StringData*,
    const StringData*,
    string_data_hash,
    string_data_same
  >;

  IMap enums;
  IMap classes;
  IMap funcs;
  IMap typeAliases;
  Map constants;
  Map modules;
  Map units;
};

RepoGlobalData getGlobalData() {
  auto const now = std::chrono::high_resolution_clock::now();
  auto const nanos =
    std::chrono::duration_cast<std::chrono::nanoseconds>(
      now.time_since_epoch()
    );

  auto gd                        = RepoGlobalData{};
  gd.Signature                   = nanos.count();
  gd.CheckPropTypeHints          = RuntimeOption::EvalCheckPropTypeHints;
  gd.PHP7_NoHexNumerics          = RuntimeOption::PHP7_NoHexNumerics;
  gd.PHP7_Substr                 = RuntimeOption::PHP7_Substr;
  gd.PHP7_Builtins               = RuntimeOption::PHP7_Builtins;
  gd.HardGenericsUB              = RuntimeOption::EvalEnforceGenericsUB >= 2;
  gd.EnableIntrinsicsExtension   = RuntimeOption::EnableIntrinsicsExtension;
  gd.ForbidDynamicCallsToFunc    = RuntimeOption::EvalForbidDynamicCallsToFunc;
  gd.ForbidDynamicCallsWithAttr  =
    RuntimeOption::EvalForbidDynamicCallsWithAttr;
  gd.ForbidDynamicCallsToClsMeth =
    RuntimeOption::EvalForbidDynamicCallsToClsMeth;
  gd.ForbidDynamicCallsToInstMeth =
    RuntimeOption::EvalForbidDynamicCallsToInstMeth;
  gd.ForbidDynamicConstructs     = RuntimeOption::EvalForbidDynamicConstructs;
  gd.LogKnownMethodsAsDynamicCalls =
    RuntimeOption::EvalLogKnownMethodsAsDynamicCalls;
  gd.EnableArgsInBacktraces      = RuntimeOption::EnableArgsInBacktraces;
  gd.NoticeOnBuiltinDynamicCalls =
    RuntimeOption::EvalNoticeOnBuiltinDynamicCalls;
  gd.InitialTypeTableSize =
    RuntimeOption::EvalInitialTypeTableSize;
  gd.InitialFuncTableSize =
    RuntimeOption::EvalInitialFuncTableSize;
  gd.InitialStaticStringTableSize =
    RuntimeOption::EvalInitialStaticStringTableSize;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.AbortBuildOnVerifyError = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.RaiseClassConversionNoticeSampleRate =
    RuntimeOption::EvalRaiseClassConversionNoticeSampleRate;
  gd.ClassPassesClassname = RuntimeOption::EvalClassPassesClassname;
  gd.ClassnameNoticesSampleRate = RuntimeOption::EvalClassnameNoticesSampleRate;
  gd.StringPassesClass = RuntimeOption::EvalStringPassesClass;
  gd.ClassNoticesSampleRate = RuntimeOption::EvalClassNoticesSampleRate;
  gd.ClassStringHintNoticesSampleRate = RO::EvalClassStringHintNoticesSampleRate;
  gd.ClassIsStringNotices = RuntimeOption::EvalClassIsStringNotices;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;
  gd.TraitConstantInterfaceBehavior =
    RuntimeOption::EvalTraitConstantInterfaceBehavior;
  gd.BuildMayNoticeOnMethCallerHelperIsObject =
    RO::EvalBuildMayNoticeOnMethCallerHelperIsObject;
  gd.DiamondTraitMethods = RuntimeOption::EvalDiamondTraitMethods;
  gd.EvalCoeffectEnforcementLevels = RO::EvalCoeffectEnforcementLevels;
  gd.EmitBespokeTypeStructures = RO::EvalEmitBespokeTypeStructures;
  gd.ActiveDeployment = RO::EvalActiveDeployment;
  gd.ModuleLevelTraits = RO::EvalModuleLevelTraits;
  gd.TreatCaseTypesAsMixed = RO::EvalTreatCaseTypesAsMixed;
  gd.JitEnableRenameFunction = RO::EvalJitEnableRenameFunction;
  gd.RenamableFunctions = RO::RenamableFunctions;
  gd.NonInterceptableFunctions = RO::NonInterceptableFunctions;

  if (Option::ConstFoldFileBC) {
    gd.SourceRootForFileBC.emplace(RO::SourceRoot);
  }

  for (auto const& elm : RuntimeOption::ConstantFunctions) {
    auto const s = internal_serialize(tvAsCVarRef(elm.second));
    gd.ConstantFunctions.emplace_back(elm.first, s.toCppString());
  }
  std::sort(gd.ConstantFunctions.begin(), gd.ConstantFunctions.end());

  return gd;
}

void setCoredumps(CompilerOptions& po) {
#ifdef _MSC_VER
/**
 * Windows actually does core dump size and control at a system, not an app
 * level. So we do nothing here and are at the mercy of Dr. Watson.
 */
#elif defined(__APPLE__) || defined(__FreeBSD__)
  struct rlimit rl;
  getrlimit(RLIMIT_CORE, &rl);
  if (!po.coredump) {
    po.coredump = rl.rlim_cur > 0;
    return;
  }
  rl.rlim_cur = 80000000LL;
  if (rl.rlim_max < rl.rlim_cur) {
    rl.rlim_max = rl.rlim_cur;
  }
  setrlimit(RLIMIT_CORE, &rl);
#else
  struct rlimit64 rl;
  getrlimit64(RLIMIT_CORE, &rl);
  if (!po.coredump) {
    po.coredump = rl.rlim_cur > 0;
    return;
  }
  rl.rlim_cur = 8000000000LL;
  if (rl.rlim_max < rl.rlim_cur) {
    rl.rlim_max = rl.rlim_cur;
  }
  setrlimit64(RLIMIT_CORE, &rl);
#endif
}

int prepareOptions(CompilerOptions &po, int argc, char **argv) {
  options_description desc("HipHop Compiler for PHP Usage:\n\n"
                           "\thphp <options> <inputs>\n\n"
                           "Options");

  std::vector<std::string> formats;

  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("format,f", value<std::vector<std::string>>(&formats)->composing(),
     "HHBC Output format: binary (default) | hhas | text | none")
    ("repo-options-dir", value<std::string>(&po.repoOptionsDir),
     "repo options directory")
    ("input-dir", value<std::string>(&po.inputDir), "input directory")
    ("inputs,i", value<std::vector<std::string>>(&po.inputs)->composing(),
     "input file names")
    ("input-list", value<std::string>(&po.inputList),
     "file containing list of file names, one per line")
    ("dir", value<std::vector<std::string>>(&po.dirs)->composing(),
     "directories containing all input files")
    ("exclude-dir",
     value<std::vector<std::string>>(&po.excludeDirs)->composing(),
     "directories to exclude from the input")
    ("exclude-file",
     value<std::vector<std::string>>(&po.excludeFiles)->composing(),
     "files to exclude from the input, even if referenced by included files")
    ("exclude-pattern",
     value<std::vector<std::string>>(&po.excludePatterns)->composing(),
     "regex (in 'find' command's regex command line option format) of files "
     "or directories to exclude from the input, even if referenced by "
     "included files")
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
    ("cdir", value<std::vector<std::string>>(&po.cdirs)->composing(),
     "extra directories for static files without exclusion checking")
    ("output-dir,o", value<std::string>(&po.outputDir), "output directory")
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
    ("file-cache",
     value<std::string>(&po.filecache),
     "if specified, generate a static file cache with this file name")
    ("coredump",
     value<bool>(&po.coredump)->default_value(false),
     "turn on coredump")
    ("compiler-id", "display the git hash for the compiler id")
    ("repo-schema", "display the repo schema id used by this app")
    ("report-ondemand-edges",
     value<std::string>(&po.ondemandEdgesPath),
     "Write parse-on-demand dependency edges to the specified file")
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

  if (po.outputDir.empty()) {
    Logger::Error("Error in command line: output-dir must be provided.");
    std::cout << desc << "\n";
    return -1;
  }

  // log level
  if (po.logLevel != -1) {
    Logger::LogLevel = (Logger::LogLevelType)po.logLevel;
  } else {
    Logger::LogLevel = Logger::LogInfo;
  }
  Logger::Escape = false;
  Logger::AlwaysEscapeLog = false;

  if (!formats.empty()) {
    for (auto const& format : formats) {
      if (format == "text") {
        Option::GenerateTextHHBC = true;
      } else if (format == "hhas") {
        Option::GenerateHhasHHBC = true;
      } else if (format == "binary") {
        Option::GenerateBinaryHHBC = true;
      } else if (format == "none") {
        if (formats.size() > 1) {
          Logger::Error("Cannot specify 'none' with other formats");
          return -1;
        }
        Option::NoOutputHHBC = true;
      } else {
        Logger::Error("Unknown format for HHBC target: %s", format.c_str());
        std::cout << desc << "\n";
        return -1;
      }
    }
  } else {
    Option::GenerateBinaryHHBC = true;
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
  applyBuildOverrides(ini, config, po);
  Hdf runtime = config["Runtime"];
  // The configuration command line strings were already processed above
  // Don't process them again.
  //
  // Note that some options depends on RepoAuthoritative, we thus
  // set/unset them here. We restore it to false since we need
  // compile_systemlib_string to actually parse the file instead of
  // trying to load it from repo (which is the case when
  // RepoAuthoritative is true).
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
  RuntimeOption::EvalLowStaticArrays = false;

  std::vector<std::string> badnodes;
  config.lint(badnodes);
  for (auto const& badnode : badnodes) {
    Logger::Error("Possible bad config node: %s", badnode.c_str());
  }

  // we need to initialize pcre cache table very early
  pcre_init();

  if (po.inputDir.empty()) po.inputDir = '.';
  po.inputDir = FileUtil::normalizeDir(po.inputDir);

  if (po.repoOptionsDir.empty()) {
    po.repoOptionsDir = po.inputDir;
  } else {
    po.repoOptionsDir = FileUtil::normalizeDir(po.repoOptionsDir);
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

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

Options makeExternWorkerOptions(const CompilerOptions& po) {
  Options options;
  options
    .setUseCase(Option::ExternWorkerUseCase)
    .setFeaturesFile(Option::ExternWorkerFeaturesFile)
    .setUseSubprocess(Option::ExternWorkerForceSubprocess
                      ? Options::UseSubprocess::Always
                      : Option::ExternWorkerAllowFallback
                        ? Options::UseSubprocess::Fallback
                        : Options::UseSubprocess::Never)
    .setCacheExecs(Option::ExternWorkerUseExecCache)
    .setCleanup(Option::ExternWorkerCleanup)
    .setUseEdenFS(RO::EvalUseEdenFS)
    .setUseRichClient(Option::ExternWorkerUseRichClient)
    .setUseZippyRichClient(Option::ExternWorkerUseZippyRichClient)
    .setUseP2P(Option::ExternWorkerUseP2P)
    .setCasConnectionCount(Option::ExternWorkerCasConnectionCount)
    .setEngineConnectionCount(Option::ExternWorkerEngineConnectionCount)
    .setAcConnectionCount(Option::ExternWorkerAcConnectionCount)
    .setVerboseLogging(Option::ExternWorkerVerboseLogging);
  if (Option::ExternWorkerTimeoutSecs > 0) {
    options.setTimeout(std::chrono::seconds{Option::ExternWorkerTimeoutSecs});
  }
  if (!Option::ExternWorkerWorkingDir.empty()) {
    options.setWorkingDir(Option::ExternWorkerWorkingDir);
  } else {
    options.setWorkingDir(po.outputDir);
  }
  if (Option::ExternWorkerThrottleRetries >= 0) {
    options.setThrottleRetries(Option::ExternWorkerThrottleRetries);
  }
  if (Option::ExternWorkerThrottleBaseWaitMSecs >= 0) {
    options.setThrottleBaseWait(
      std::chrono::milliseconds{Option::ExternWorkerThrottleBaseWaitMSecs}
    );
  }
  return options;
}

void logPhaseStats(const std::string& phase, const Package& package,
    extern_worker::Client& client, StructuredLogEntry& sample, int64_t micros)
{
  auto const& stats = client.getStats();
  Logger::FInfo(
    "{}",
    stats.toString(
      phase,
      folly::sformat("total package files {:,}", package.getTotalFiles())
    )
  );

  sample.setInt(phase + "_total_files", package.getTotalFiles());

  sample.setInt(phase + "_micros", micros);
  if (auto const t = package.inputsTime()) {
    sample.setInt(
      phase + "_input_micros",
      std::chrono::duration_cast<std::chrono::microseconds>(*t).count()
    );
  }
  if (auto const t = package.ondemandTime()) {
    sample.setInt(
      phase + "_ondemand_micros",
      std::chrono::duration_cast<std::chrono::microseconds>(*t).count()
    );
  }

  stats.logSample(phase, sample);
  sample.setStr(phase + "_fellback", client.fellback() ? "true" : "false");
}

namespace {
  // Upload all builtin decls, and pass their IndexMeta summary and
  // Ref<UnitDecls> to callback() to include in the overall UnitIndex. This
  // makes systemlib decls visible to files being compiled as part of the
  // full repo build, but does not make repo decls available to systemlib.
  coro::Task<bool> indexBuiltinSymbolDecls(
    const Package::IndexCallback& callback,
    TicketExecutor& executor,
    extern_worker::Client& client
  ) {
    std::vector<coro::TaskWithExecutor<void>> tasks;
    auto const declCallback = [&](auto const* d) -> coro::Task<void> {
      auto const symbols = hackc::decls_to_symbols(*d->decls);
      auto summary = summary_of_symbols(symbols);
      callback(
        "",
        summary,
        co_await client.store(Package::UnitDecls{
          summary,
          std::string{d->serialized.begin(), d->serialized.end()}
        })
      );
      co_return;
    };
    for (auto const& d: Native::getAllBuiltinDecls()) {
      tasks.emplace_back(declCallback(d).scheduleOn(executor.sticky()));
    }
    co_await coro::collectAllRange(std::move(tasks));
    co_return true;
  }
}

// Compute a UnitIndex by parsing decls for all autoload-eligible files.
// If no Autoload.Query is specified by RepoOptions, this just indexes
// the input files.
std::unique_ptr<UnitIndex> computeIndex(
    const CompilerOptions& po,
    StructuredLogEntry& sample,
    TicketExecutor& executor,
    extern_worker::Client& client
) {
  auto index = std::make_unique<UnitIndex>();
  auto const indexUnit = [&] (
      std::string&& rpath,
      Package::IndexMeta&& meta,
      Ref<Package::UnitDecls>&& declsRef
  ) {
    auto locations = std::make_shared<UnitIndex::Locations>(
        std::move(rpath), std::move(declsRef)
    );
    auto insert = [&](auto const& names, auto& map, const char* kind) {
      for (auto name : names) {
        auto const ret = map.emplace(name, locations);
        if (!ret.second) {
          Logger::FWarning("Duplicate {} {} in {} and {}",
              kind, name, ret.first->first, locations->rpath
          );
        }
      }
    };
    insert(meta.types, index->types, "type");
    insert(meta.funcs, index->funcs, "function");
    insert(meta.constants, index->constants, "constant");
    insert(meta.modules, index->modules, "module");
  };

  Package indexPackage{po.inputDir, executor, client, po.coredump};
  Timer indexTimer(Timer::WallTime, "indexing");

  auto const& repoFlags = RepoOptions::forFile(po.repoOptionsDir).flags();
  auto const& dirs = repoFlags.autoloadRepoBuildSearchDirs();
  auto const queryStr = repoFlags.autoloadQuery();
  if (!dirs.empty()) {
    addListToPackage(indexPackage, dirs, po);
  } else if (!queryStr.empty()) {
    // Index the files specified by Autoload.Query
    if (!addAutoloadQueryToPackage(indexPackage, queryStr)) return nullptr;
  } else {
    // index just the input files
    addInputsToPackage(indexPackage, po);
  }
  // Here, we are doing the following in parallel:
  // * Indexing the build package
  // * Indexing builtin decls to be used by decl driven bytecode compilation
  // If DDB is not enabled, we will return early from the second task.
  auto const [indexingRepoOK, indexingSystemlibDeclsOK] =
    coro::blockingWait(coro::collectAll(
      indexPackage.index(indexUnit),
      coro::co_invoke([&]() -> coro::Task<bool> {
        if (RO::EvalEnableDecl) {
          co_return co_await
            indexBuiltinSymbolDecls(indexUnit, executor, client);
        }
        co_return true;
      })
    ));

  if (!indexingRepoOK || !indexingSystemlibDeclsOK) return nullptr;

  logPhaseStats("index", indexPackage, client, sample,
                indexTimer.getMicroSeconds());
  Logger::FInfo("index size: types={:,} funcs={:,} constants={:,} modules={:,}",
      index->types.size(),
      index->funcs.size(),
      index->constants.size(),
      index->modules.size()
  );
  client.resetStats();

  return index;
}

///////////////////////////////////////////////////////////////////////////////

// Parses a file and produces an UnitEmitter. Used when we're not
// going to run HHBBC.
struct ParseJob {
  static std::string name() { return "hphpc-parse"; }

  static void init(const Package::Config& config,
                   Package::FileMetaVec meta) {
    Package::parseInit(config, std::move(meta));
  }
  static Package::ParseMetaVec fini() {
    return Package::parseFini();
  }

  static UnitEmitterSerdeWrapper run(const std::string& contents,
                                     const RepoOptionsFlags& flags,
                                     Variadic<Package::UnitDecls> decls) {
    return Package::parseRun(contents, flags, std::move(decls.vals));
  }
};

using WPI = HHBBC::WholeProgramInput;

// Parses a file (as ParseJob does), but then hands the UnitEmitter
// off to HHBBC to produce a WholeProgramInput key and value. This is
// for when we are going to run HHBBC.
struct ParseForHHBBCJob {
  static std::string name() { return "hphpc-parse-for-hhbbc"; }

  static void init(const Package::Config& config,
                   const HHBBC::Config& hhbbcConfig,
                   Package::FileMetaVec meta) {
    Package::parseInit(config, std::move(meta));
    HHBBC::options = hhbbcConfig.o;
    hhbbcConfig.gd.load(true);
  }
  static std::tuple<Package::ParseMetaVec, std::vector<WPI::Key>> fini() {
    return std::make_tuple(Package::parseFini(), std::move(s_inputKeys));
  }

  static Variadic<WPI::Value> run(const std::string& contents,
                                  const RepoOptionsFlags& flags,
                                  Variadic<Package::UnitDecls> decls) {
    auto wrapper = Package::parseRun(contents, flags, std::move(decls.vals));
    if (!wrapper.m_ue) return {};

    std::vector<WPI::Value> values;
    for (auto& [key, value] : WPI::make(std::move(wrapper.m_ue))) {
      s_inputKeys.emplace_back(std::move(key));
      values.emplace_back(std::move(value));
    }
    return Variadic<WPI::Value>{std::move(values)};
  }

  static std::vector<WPI::Key> s_inputKeys;
};

std::vector<WPI::Key> ParseForHHBBCJob::s_inputKeys;

Job<ParseJob> s_parseJob;
Job<ParseForHHBBCJob> s_parseForHHBBCJob;

// A ParsedFile owns all the HHBC state associated with a parsed source
// file before we have decided to add it to the Program.
struct ParsedFile {
  explicit ParsedFile(Package::ParseMeta m)
    : parseMeta(std::move(m))
  {}
  ParsedFile(Package::ParseMeta m, Ref<UnitEmitterSerdeWrapper> w)
    : parseMeta(std::move(m)), ueRef(std::move(w))
  {}

  Package::ParseMeta parseMeta;
  Optional<Ref<UnitEmitterSerdeWrapper>> ueRef;
  std::vector<std::pair<WPI::Key, Ref<WPI::Value>>> hhbbcInputs;
};

using ParsedFiles = folly_concurrent_hash_map_simd<
  std::string,
  std::unique_ptr<ParsedFile>
>;

///////////////////////////////////////////////////////////////////////////////

bool process(CompilerOptions &po) {
#ifndef _MSC_VER
  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount,
                           RuntimeOption::EvalRecordSubprocessTimes,
                           {});
#endif

  setCoredumps(po);

  register_process_init();

  StructuredLogEntry sample;
  sample.setStr("debug", debug ? "true" : "false");
  sample.setStr("use_case", Option::ExternWorkerUseCase);
  sample.setStr("features_file", Option::ExternWorkerFeaturesFile);
  sample.setInt("use_rich_client", Option::ExternWorkerUseRichClient);
  sample.setInt("use_zippy_rich_client",
      Option::ExternWorkerUseZippyRichClient);
  sample.setInt("use_p2p", Option::ExternWorkerUseP2P);
  sample.setInt("cas_connection_count", Option::ExternWorkerCasConnectionCount);
  sample.setInt("engine_connection_count", Option::ExternWorkerEngineConnectionCount);
  sample.setInt("ac_connection_count", Option::ExternWorkerAcConnectionCount);
  sample.setInt("force_subprocess", Option::ExternWorkerForceSubprocess);
  sample.setInt("use_exec_cache", Option::ExternWorkerUseExecCache);
  sample.setInt("timeout_secs", Option::ExternWorkerTimeoutSecs);
  sample.setInt("cleanup", Option::ExternWorkerCleanup);
  sample.setInt("throttle_retries", Option::ExternWorkerThrottleRetries);
  sample.setInt("throttle_base_wait_ms",
                Option::ExternWorkerThrottleBaseWaitMSecs);
  sample.setStr("working_dir", Option::ExternWorkerWorkingDir);
  sample.setInt("parser_group_size", Option::ParserGroupSize);
  sample.setInt("parser_dir_group_size_limit", Option::ParserDirGroupSizeLimit);
  sample.setInt("parser_thread_count", Option::ParserThreadCount);
  sample.setInt("parser_optimistic_store", Option::ParserOptimisticStore);
  sample.setInt("parser_async_cleanup", Option::ParserAsyncCleanup);
  sample.setStr("push_phases", po.push_phases);
  sample.setStr("matched_overrides", po.matched_overrides);
  sample.setStr("use_hphpc", "true");
  sample.setStr("use_hhbbc", RO::EvalUseHHBBC ? "true" : "false");

  // Track the unit-emitters created for system during
  // hphp_process_init().
  SystemLib::keepRegisteredUnitEmitters(true);
  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };
  SystemLib::keepRegisteredUnitEmitters(false);

  auto const outputFile = po.outputDir + "/hhvm.hhbc";
  unlink(outputFile.c_str());

  auto executor = std::make_unique<TicketExecutor>(
    "HPHPcWorker",
    0,
    size_t(Option::ParserThreadCount <= 0 ? 1 : Option::ParserThreadCount),
    [] {
      hphp_thread_init();
      hphp_session_init(Treadmill::SessionKind::CompilerEmit);
    },
    [] {
      hphp_context_exit();
      hphp_session_exit();
      hphp_thread_exit();
    },
    std::chrono::minutes{15}
  );
  auto client =
    std::make_unique<Client>(executor->sticky(), makeExternWorkerOptions(po));

  sample.setStr("extern_worker_impl", client->implName());
  sample.setStr("extern_worker_session", client->session());

  auto index = computeIndex(po, sample, *executor, *client);
  if (!index) return false;

  // Always used, but we can clear it early to save memory.
  Optional<SymbolSets> unique;
  unique.emplace();

  // HHBBC specific state (if we're going to run it).
  Optional<WPI> hhbbcInputs;
  Optional<CoroAsyncValue<Ref<HHBBC::Config>>> hhbbcConfig;
  if (RO::EvalUseHHBBC) {
    hhbbcInputs.emplace();
    // We want to do this as early as possible
    hhbbcConfig.emplace(
      [&client] () {
        return client->store(HHBBC::Config::get(getGlobalData()));
      },
      executor->sticky()
    );
  }

  hphp_fast_set<const StringData*> moduleInDeployment;
  if (!RO::EvalActiveDeployment.empty()) {
    // Many files will be in the same module, so it is better to precompute
    // a mapping of whether a given module is in the current deployment
    auto const& packageInfo =
      RepoOptions::forFile(po.repoOptionsDir).packageInfo();
    auto const it = packageInfo.deployments().find(RO::EvalActiveDeployment);
    if (it == end(packageInfo.deployments())) {
      Logger::FError("The active deployment is set to {}; "
                     "however, it is not defined in the {}/{} file",
                     RO::EvalActiveDeployment,
                     po.repoOptionsDir,
                     kPackagesToml);
      return false;
    }

    moduleInDeployment.reserve(index->modules.size());
    for (auto const& [module, _] : index->modules) {
      assertx(!moduleInDeployment.contains(module));
      if (packageInfo.moduleInDeployment(module,
                                         it->second,
                                         DeployKind::HardOrSoft)) {
        moduleInDeployment.insert(module);
      }
    }
    // Check for the default module separately since there is no module
    // declaration for the default module.
    static auto const defaultModule = makeStaticString(Module::DEFAULT);
    if (packageInfo.moduleInDeployment(defaultModule,
                                       it->second,
                                       DeployKind::HardOrSoft)) {
      moduleInDeployment.insert(defaultModule);
    }
  }

  Optional<RepoAutoloadMapBuilder> autoload;
  Optional<RepoFileBuilder> repo;
  std::atomic<uint32_t> nextSn{0};
  std::atomic<size_t> numUnits{0};
  std::mutex repoLock;

  // Emit a fully processed unit (either processed by HHBBC or not).
  auto const emitUnit = [&] (std::unique_ptr<UnitEmitter> ue) {
    assertx(ue);
    if (Option::NoOutputHHBC) return;

    assertx(Option::GenerateBinaryHHBC ||
            Option::GenerateTextHHBC ||
            Option::GenerateHhasHHBC);

    if (Option::GenerateTextHHBC || Option::GenerateHhasHHBC) {
      auto old_repo_auth = RuntimeOption::RepoAuthoritative;
      RuntimeOption::RepoAuthoritative = RuntimeOption::EvalUseHHBBC;
      SCOPE_EXIT { RuntimeOption::RepoAuthoritative = old_repo_auth; };
      genText(*ue, po.outputDir);
    }

    if (!Option::GenerateBinaryHHBC) return;

    ++numUnits;

    if (!RO::EvalUseHHBBC) {
      // HHBBC assigns m_sn and the SHA1, but we have to do it ourself
      // if we're not running it.
      auto const sn = nextSn++;
      ue->m_symbol_refs.clear();
      ue->m_sn = sn;
      ue->setSha1(SHA1 { sn });
      unique->add(*ue);
    }

    autoload->addUnit(*ue);
    RepoFileBuilder::EncodedUE encoded{*ue};
    std::scoped_lock<std::mutex> _{repoLock};
    repo->add(encoded);
  };

  // This will contain all files eligible to be in the program: input files
  // and all ondemand-eligible files, except files excluded by CLI options.
  auto parsedFiles = std::make_unique<ParsedFiles>();

  // Process unit-emitters produced locally (usually systemlib stuff).
  auto const emitLocalUnit = [&] (Package::UEVec ues) -> coro::Task<void> {
    if (RO::EvalUseHHBBC) {
      // If we're using HHBBC, turn them into WholeProgramInput
      // key/values (after checking uniqueness), upload the values,
      // and store them in the WholeProgramInput.
      std::vector<WPI::Key> keys;
      std::vector<WPI::Value> values;

      for (auto& ue : ues) {
        unique->add(*ue);
        for (auto& [key, value] : WPI::make(std::move(ue))) {
          keys.emplace_back(std::move(key));
          values.emplace_back(std::move(value));
        }
      }

      if (keys.empty()) co_return;
      auto valueRefs = co_await client->storeMulti(std::move(values));

      auto const numKeys = keys.size();
      assertx(valueRefs.size() == numKeys);

      for (size_t i = 0; i < numKeys; ++i) {
        hhbbcInputs->add(std::move(keys[i]), std::move(valueRefs[i]));
      }
      co_return;
    }

    // Otherwise just emit it
    for (auto& ue : ues) emitUnit(std::move(ue));
    co_return;
  };

  // Parse a group of files remotely
  auto const parseRemoteUnit = [&] (const Ref<Package::Config>& config,
                                    Ref<Package::FileMetaVec> fileMetas,
                                    std::vector<Package::FileData> files,
                                    Client::ExecMetadata metadata)
    -> coro::Task<Package::ParseMetaVec> {
    if (RO::EvalUseHHBBC) {
      // Run the HHBBC parse job, which produces WholeProgramInput
      // key/values.
      auto hhbbcConfigRef = co_await hhbbcConfig->getCopy();
      auto [inputValueRefs, metaRefs] =
        co_await client->exec(
          s_parseForHHBBCJob,
          std::make_tuple(
            config,
            std::move(hhbbcConfigRef),
            std::move(fileMetas)
          ),
          std::move(files),
          std::move(metadata)
        );

      // The parse metadata and the keys are loaded, but the values
      // are kept as Refs.
      auto [parseMetas, inputKeys] = co_await client->load(std::move(metaRefs));

      // Stop now if the index contains any missing decls.
      // parseRun() will retry this job with additional inputs.
      if (index->containsAnyMissing(parseMetas)) {
        co_return parseMetas;
      }

      always_assert(parseMetas.size() == inputValueRefs.size());
      auto const numKeys = inputKeys.size();
      size_t keyIdx = 0;
      for (size_t i = 0, n = parseMetas.size(); i < n; i++) {
        auto& p = parseMetas[i];
        p.m_missing = Package::DeclNames{}; // done with this list now.
        if (!p.m_filepath) continue;
        auto& valueRefs = inputValueRefs[i];
        auto filename = p.m_filepath->toCppString();
        auto pf = std::make_unique<ParsedFile>(std::move(p));
        pf->hhbbcInputs.reserve(valueRefs.size());
        for (auto& r : valueRefs) {
          always_assert(keyIdx < numKeys);
          pf->hhbbcInputs.emplace_back(
            std::move(inputKeys[keyIdx]), std::move(r)
          );
          ++keyIdx;
        }
        parsedFiles->emplace(filename, std::move(pf));
      }

      // Indicate we're done by returning an empty vec.
      co_return Package::ParseMetaVec{};
    }

    // Otherwise, do a "normal" (non-HHBBC parse job), load the
    // unit-emitters and parse metadata, and emit the unit-emitters.
    auto [ueRefs, metaRefs] =
      co_await client->exec(
        s_parseJob,
        std::make_tuple(config, std::move(fileMetas)),
        std::move(files),
        std::move(metadata)
      );

    auto parseMetas = co_await client->load(std::move(metaRefs));

    // Stop now if the index contains any missing decls.
    // parseRun() will retry this job with additional inputs.
    if (index->containsAnyMissing(parseMetas)) {
      co_return parseMetas;
    }

    always_assert(parseMetas.size() == ueRefs.size());
    for (size_t i = 0, n = parseMetas.size(); i < n; i++) {
      auto& p = parseMetas[i];
      p.m_missing = Package::DeclNames{}; // done with this list now.
      if (!p.m_filepath) continue;
      auto filename = p.m_filepath->toCppString();
      auto pf = std::make_unique<ParsedFile>(
        std::move(p), std::move(ueRefs[i])
      );
      parsedFiles->emplace(filename, std::move(pf));
    }

    // Indicate we're done by returning an empty vec.
    co_return Package::ParseMetaVec{};
  };

  // Emit a group of files that were parsed remotely
  auto const emitRemoteUnit = [&] (
      const std::vector<std::filesystem::path>& rpaths
  ) -> coro::Task<Package::EmitCallBackResult> {
    Package::ParseMetaVec parseMetas;
    Package::ParseMetaItemsToSkipSet itemsToSkip;

    auto const shouldIncludeInBuild = [&] (const Package::ParseMeta& p) {
      if (RO::EvalActiveDeployment.empty()) return true;
      // If the unit defines any modules, then it is always included
      if (!p.m_definitions.m_modules.empty()) return true;
      return p.m_module_use && moduleInDeployment.contains(p.m_module_use);
    };

    if (RO::EvalUseHHBBC) {
      // Retrieve HHBBC WPI (Key, Ref<Value>) pairs that were already parsed.
      // No Async I/O is necessary in this case.
      for (size_t i = 0, n = rpaths.size(); i < n; ++i) {
        auto& rpath = rpaths[i];
        auto it = parsedFiles->find(rpath.native());
        if (it == parsedFiles->end()) {
          // If you see this error in a test case, add a line to to test.php.hphp_opts:
          // --inputs=hphp/path/to/file.inc
          Package::ParseMeta bad;
          bad.m_abort = folly::sformat("Unknown include file: {}\n", rpath.native());
          parseMetas.emplace_back(std::move(bad));
          continue;
        }
        auto& pf = it->second;
        parseMetas.emplace_back(std::move(pf->parseMeta));
        auto& p = parseMetas.back();
        if (!p.m_filepath) continue;
        if (!shouldIncludeInBuild(p)) {
          Logger::FVerbose("Dropping {} from the repo build because module {} is "
                           "not part of {} deployment",
                           p.m_filepath,
                           p.m_module_use ? p.m_module_use->data() : "top-level",
                           RO::EvalActiveDeployment);
          itemsToSkip.insert(i);
          continue;
        }
        // We don't have unit-emitters to do uniqueness checking, but
        // the parse metadata has the definitions we can use instead.
        unique->add(p.m_definitions, p.m_filepath);
        auto inputs = std::move(pf->hhbbcInputs);
        for (auto& e : inputs) {
          hhbbcInputs->add(std::move(e.first), std::move(e.second));
        }
      }
      co_return std::make_pair(std::move(parseMetas),
                               std::move(itemsToSkip));
    }

    // Otherwise, retrieve ParseMeta and load unit-emitters from a normal
    // ParseJob, then emit the unit-emitters.
    std::vector<Ref<UnitEmitterSerdeWrapper>> ueRefs;
    ueRefs.reserve(rpaths.size());
    for (size_t i = 0, n = rpaths.size(); i < n; ++i) {
      auto& rpath = rpaths[i];
      auto it = parsedFiles->find(rpath);
      if (it == parsedFiles->end()) {
        // If you see this error in a test case, add a line to to test.php.hphp_opts:
        // --inputs=hphp/path/to/file.inc
        Package::ParseMeta bad;
        bad.m_abort = folly::sformat("Unknown include file: {}", rpath.native());
        parseMetas.emplace_back(std::move(bad));
        continue;
      }
      auto& pf = it->second;
      auto& p = pf->parseMeta;
      if (!shouldIncludeInBuild(p)) {
        Logger::FVerbose("Dropping {} from the repo build because module {} is "
                         "not part of {} deployment",
                         p.m_filepath,
                         p.m_module_use ? p.m_module_use->data() : "top-level",
                         RO::EvalActiveDeployment);
        itemsToSkip.insert(i);
        continue;
      }
      parseMetas.emplace_back(std::move(pf->parseMeta));
      ueRefs.emplace_back(std::move(*pf->ueRef));
    }

    always_assert(parseMetas.size() == ueRefs.size());
    auto ueWrappers = co_await client->load(std::move(ueRefs));

    for (auto& wrapper : ueWrappers) {
      if (!wrapper.m_ue) continue;
      emitUnit(std::move(wrapper.m_ue));
    }
    co_return std::make_pair(std::move(parseMetas),
                             std::move(itemsToSkip));
  };

  {
    // Parsing phase: compile all input files and autoload files to bytecode.
    // Deferring emit reduces wall time by parsing all files in parallel in
    // one pass, then computing the full transitive closure of ondemand files
    // in one go while emitting. Unreferenced ondemand files are discarded.
    auto parsePackage = std::make_unique<Package>(
      po.inputDir,
      *executor,
      *client,
      po.coredump
    );
    Timer parseTimer(Timer::WallTime, "parsing");

    // Parse the input files specified on the command line
    addInputsToPackage(*parsePackage, po);
    auto const& repoFlags = RepoOptions::forFile(po.repoOptionsDir).flags();
    auto const& dirs = repoFlags.autoloadRepoBuildSearchDirs();
    auto const queryStr = repoFlags.autoloadQuery();
    if (!dirs.empty()) {
      addListToPackage(*parsePackage, dirs, po);
    } else if (!queryStr.empty()) {
      // Parse all the files specified by Autoload.Query
      if (!addAutoloadQueryToPackage(*parsePackage, queryStr)) return false;
    }

    if (!coro::blockingWait(parsePackage->parse(*index,
                                                parseRemoteUnit))) {
      return false;
    }

    logPhaseStats("parse", *parsePackage, *client, sample,
                  parseTimer.getMicroSeconds());
    client->resetStats();
  }

  auto package = std::make_unique<Package>(
    po.inputDir,
    *executor,
    *client,
    po.coredump
  );

  {
    // Emit phase: emit systemlib units, all input files, and the transitive
    // closure of files referenced by symbolRefs.
    Timer emitTimer(Timer::WallTime, "emit");
    addInputsToPackage(*package, po);

    if (!RO::EvalUseHHBBC && Option::GenerateBinaryHHBC) {
      // Initialize autoload and repo for emitUnit() to populate
      autoload.emplace();
      repo.emplace(outputFile);
    }

    if (!coro::blockingWait(package->emit(*index, emitRemoteUnit, emitLocalUnit,
                                          po.ondemandEdgesPath))) {
      return false;
    }

    // We didn't run any extern worker jobs, and in HHBBC mode we
    // also didn't load anything. Most of these stats are zero but a
    // few are still interesting.
    logPhaseStats("emit", *package, *client, sample,
                  emitTimer.getMicroSeconds());
  }

  std::thread fileCache{
    [&, package = std::move(package), parsedFiles = std::move(parsedFiles),
      index = std::move(index)] () mutable {
      {
        Timer t{Timer::WallTime, "dropping unused files"};
        parsedFiles.reset();
      }
      {
        Timer t{Timer::WallTime, "dropping index"};
        index.reset();
      }
      SCOPE_EXIT { package.reset(); };
      if (po.filecache.empty()) return;
      Timer _{Timer::WallTime, "saving file cache..."};
      HphpSessionAndThread session{Treadmill::SessionKind::CompilerEmit};
      package->writeVirtualFileSystem(po.filecache.c_str());
      struct stat sb;
      stat(po.filecache.c_str(), &sb);
      Logger::Info("%" PRId64" MB %s saved",
                   (int64_t)sb.st_size/(1024*1024), po.filecache.c_str());
    }
  };
  SCOPE_EXIT { fileCache.join(); };

  std::thread asyncDispose;
  SCOPE_EXIT { if (asyncDispose.joinable()) asyncDispose.join(); };
  auto const dispose = [&] (std::unique_ptr<TicketExecutor> e,
                            std::unique_ptr<Client> c) {
    if (!Option::ParserAsyncCleanup) {
      // If we don't want to cleanup asynchronously, do so now.
      c.reset();
      e.reset();
      return;
    }
    // All the thread does is reset the unique_ptr to run the dtor.
    asyncDispose = std::thread{
      [e = std::move(e), c = std::move(c)] () mutable {
        c.reset();
        e.reset();
      }
    };
  };

  auto const logSample = [&] {
    // Only log big builds.
    if (numUnits >= RO::EvalHHBBCMinUnitsToLog) {
      sample.force_init = true;
      StructuredLog::log("hhvm_whole_program", sample);
    }
    return true;
  };

  auto const finish = [&] {
    if (!Option::GenerateBinaryHHBC) return true;
    Timer _{Timer::WallTime, "finalizing repo"};
    auto const& packageInfo =
      RepoOptions::forFile(po.repoOptionsDir).packageInfo();
    repo->finish(getGlobalData(), *autoload, packageInfo);
    return true;
  };
  if (!RO::EvalUseHHBBC) {
    logSample();
    dispose(std::move(executor), std::move(client));
    return finish();
  }

  // We don't need these anymore, and since they can consume a lot of
  // memory, free them before doing anything else.
  unique.reset();
  hhbbcConfig.reset();

  assertx(!autoload.has_value());
  assertx(!repo.has_value());
  if (Option::GenerateBinaryHHBC) {
    autoload.emplace();
    repo.emplace(outputFile);
  }

  if (Option::ConstFoldFileBC) {
    HHBBC::options.SourceRootForFileBC = RO::SourceRoot;
  }
  HHBBC::options.CoreDump = po.coredump;

  Timer timer{Timer::WallTime, "running HHBBC"};
  HphpSession session{Treadmill::SessionKind::HHBBC};

  client->resetStats();
  HHBBC::trace_time::register_client_stats(client->getStatsPtr());

  HHBBC::whole_program(
    std::move(*hhbbcInputs),
    HHBBC::Config::get(getGlobalData()),
    std::move(executor),
    std::move(client),
    emitUnit,
    dispose,
    &sample,
    Option::ParserThreadCount > 0 ? Option::ParserThreadCount : 0
  );

  finish();
  sample.setInt("hhbbc_micros", timer.getMicroSeconds());
  logSample();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

int compiler_main(int argc, char **argv) {
  try {
    rds::local::init();
    SCOPE_EXIT { rds::local::fini(); };

    CompilerOptions po;
    auto const ret = prepareOptions(po, argc, argv);
    if (ret == 1) return 0; // --help
    if (ret != 0) return ret; // command line error

    Timer totalTimer(Timer::WallTime, "running hphp");
    always_assert_flog(
      mkdir(po.outputDir.c_str(), 0777) == 0 || errno == EEXIST,
      "Unable to mkdir({}): {}",
      po.outputDir.c_str(),
      folly::errnoStr(errno)
    );
    if (!process(po)) {
      Logger::Error("hphp failed");
      return -1;
    } else {
      Logger::Info("all files saved in %s ...", po.outputDir.c_str());
      return 0;
    }
  } catch (const Exception& e) {
    Logger::Error("Exception: %s", e.getMessage().c_str());
  } catch (const std::exception& e) {
    Logger::Error("std::exception: %s", e.what());
  } catch (...) {
    Logger::Error("(non-standard exception \"%s\" was thrown)",
                  current_exception_name().c_str());
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////////////

}
