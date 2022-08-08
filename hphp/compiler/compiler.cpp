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

#include "hphp/hhbbc/hhbbc.h"
#include "hphp/hhbbc/options.h"

#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/version.h"

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

#include <exception>
#include <filesystem>
#include <fstream>

#include <folly/portability/SysStat.h>

using namespace boost::program_options;

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
  std::string push_phases;
  std::string matched_overrides;
  bool parseOnDemand;
  bool keepTempDir;
  int logLevel;
  std::string filecache;
  bool coredump;
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

void addInputsToPackage(Package& package, const CompilerOptions& po) {
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
      assertx(kv.second.m_type != KindOfUninit ||
              kv.second.dynamic());
      add(constants, kv.first, nullptr, "constant");
    }
  }

  // For local parses, where we have an UnitEmitter
  void add(UnitEmitter& ue) {
    // Verify uniqueness of symbols and set Attrs appropriately.
    auto const path = ue.m_filepath;

    add(units, path, path, "unit");

    for (size_t n = 0; n < ue.numPreClasses(); ++n) {
      auto pce = ue.pce(n);
      pce->setAttrs(pce->attrs() | AttrUnique | AttrPersistent);
      if (pce->attrs() & AttrEnum) add(enums, pce->name(), path, "enum");
      add(classes, pce->name(), path, "class", typeAliases);
    }
    for (auto& fe : ue.fevec()) {
      if (fe->attrs & AttrIsMethCaller) {
        if (addNoFail(funcs, fe->name, path, "function")) {
          fe->attrs |= AttrUnique | AttrPersistent;
        }
      } else {
        fe->attrs |= AttrUnique | AttrPersistent;
        add(funcs, fe->name, path, "function");
      }
    }
    for (auto& te : ue.typeAliases()) {
      te->setAttrs(te->attrs() | AttrUnique | AttrPersistent);
      add(typeAliases, te->name(), path, "type alias", classes);
    }
    for (auto& c : ue.constants()) {
      c.attrs |= AttrUnique | AttrPersistent;
      add(constants, c.name, path, "constant");
    }
    for (auto& m : ue.modules()) {
      m.attrs |= AttrUnique | AttrPersistent;
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
  gd.InitialNamedEntityTableSize =
    RuntimeOption::EvalInitialNamedEntityTableSize;
  gd.InitialStaticStringTableSize =
    RuntimeOption::EvalInitialStaticStringTableSize;
  gd.HackArrCompatSerializeNotices =
    RuntimeOption::EvalHackArrCompatSerializeNotices;
  gd.AbortBuildOnVerifyError = RuntimeOption::EvalAbortBuildOnVerifyError;
  gd.EmitClassPointers = RuntimeOption::EvalEmitClassPointers;
  gd.EmitClsMethPointers = RuntimeOption::EvalEmitClsMethPointers;
  gd.IsVecNotices = RuntimeOption::EvalIsVecNotices;
  gd.RaiseClassConversionWarning =
    RuntimeOption::EvalRaiseClassConversionWarning;
  gd.ClassPassesClassname = RuntimeOption::EvalClassPassesClassname;
  gd.ClassnameNotices = RuntimeOption::EvalClassnameNotices;
  gd.ClassIsStringNotices = RuntimeOption::EvalClassIsStringNotices;
  gd.StrictArrayFillKeys = RuntimeOption::StrictArrayFillKeys;
  gd.TraitConstantInterfaceBehavior =
    RuntimeOption::EvalTraitConstantInterfaceBehavior;
  gd.BuildMayNoticeOnMethCallerHelperIsObject =
    RO::EvalBuildMayNoticeOnMethCallerHelperIsObject;
  gd.DiamondTraitMethods = RuntimeOption::EvalDiamondTraitMethods;
  gd.EvalCoeffectEnforcementLevels = RO::EvalCoeffectEnforcementLevels;
  gd.EmitBespokeTypeStructures = RO::EvalEmitBespokeTypeStructures;

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

void setCoredumps(const CompilerOptions& po) {
  if (!po.coredump) return;
#ifdef _MSC_VER
/**
 * Windows actually does core dump size and control at a system, not an app
 * level. So we do nothing here and are at the mercy of Dr. Watson.
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

int prepareOptions(CompilerOptions &po, int argc, char **argv) {
  options_description desc("HipHop Compiler for PHP Usage:\n\n"
                           "\thphp <options> <inputs>\n\n"
                           "Options");

  bool dummy;
  bool dummy2;
  std::string dummy3;
  std::string dummy4;

  std::vector<std::string> formats;

  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("target,t", value<std::string>(&dummy4)->default_value("hhbc"),
     "hhbc") // TODO: T115189426 remove this
    ("format,f", value<std::vector<std::string>>(&formats)->composing(),
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
     "modules are forced to be included") // TODO: T115189426 remove this
    ("ffile", value<std::vector<std::string>>(&po.ffiles)->composing(),
     "extra PHP files forced to include without exclusion checking") // TODO: T115189426 remove this
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

Options makeExternWorkerOptions() {
  Options options;
  options
    .setUseCase(Option::ExternWorkerUseCase)
    .setUseSubprocess(Option::ExternWorkerForceSubprocess
                      ? Options::UseSubprocess::Always
                      : Options::UseSubprocess::Fallback)
    .setCacheExecs(Option::ExternWorkerUseExecCache)
    .setCleanup(Option::ExternWorkerCleanup)
    .setUseEdenFS(RO::EvalUseEdenFS)
    .setUseRichClient(Option::ExternWorkerUseRichClient)
    .setUseZippyRichClient(Option::ExternWorkerUseZippyRichClient)
    .setUseP2P(Option::ExternWorkerUseP2P)
    .setVerboseLogging(Option::ExternWorkerVerboseLogging);
  if (Option::ExternWorkerTimeoutSecs > 0) {
    options.setTimeout(std::chrono::seconds{Option::ExternWorkerTimeoutSecs});
  }
  if (!Option::ExternWorkerWorkingDir.empty()) {
    options.setWorkingDir(Option::ExternWorkerWorkingDir);
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
    "  {}: total package files {:,}\n"
    "  Execs: {:,} total, {:,} cache-hits, {:,} optimistically, {:,} fallback\n"
    "  Files: {:,} total, {:,} read, {:,} queried, {:,} uploaded ({:,} bytes), {:,} fallback\n"
    "  Blobs: {:,} total, {:,} queried, {:,} uploaded ({:,} bytes), {:,} fallback\n"
    "  Cpu: {:,} usec usage, {:,} allocated cores\n"
    "  Mem: {:,} max used, {:,} reserved\n"
    "  {:,} downloads ({:,} bytes), {:,} throttles",
    phase,
    package.getTotalFiles(),
    stats.execs.load(),
    stats.execCacheHits.load(),
    stats.optimisticExecs.load(),
    stats.execFallbacks.load(),
    stats.files.load(),
    stats.filesRead.load(),
    stats.filesQueried.load(),
    stats.filesUploaded.load(),
    stats.fileBytesUploaded.load(),
    stats.fileFallbacks.load(),
    stats.blobs.load(),
    stats.blobsQueried.load(),
    stats.blobsUploaded.load(),
    stats.blobBytesUploaded.load(),
    stats.blobFallbacks.load(),
    stats.execCpuUsec.load(),
    stats.execAllocatedCores.load(),
    stats.execMaxUsedMem.load(),
    stats.execReservedMem.load(),
    stats.downloads.load(),
    stats.bytesDownloaded.load(),
    stats.throttles.load()
  );
  sample.setInt(phase + "_total_files", package.getTotalFiles());

  sample.setInt(phase + "_micros", micros);
  if (auto const t = package.parsingInputsTime()) {
    sample.setInt(
      phase + "_input_micros",
      std::chrono::duration_cast<std::chrono::microseconds>(*t).count()
    );
  }
  if (auto const t = package.parsingOndemandTime()) {
    sample.setInt(
      phase + "_ondemand_micros",
      std::chrono::duration_cast<std::chrono::microseconds>(*t).count()
    );
  }

  sample.setInt(phase + "_total_execs", stats.execs.load());
  sample.setInt(phase + "_cache_hits", stats.execCacheHits.load());
  sample.setInt(phase + "_optimistically", stats.optimisticExecs.load());
  sample.setInt(phase + "_fallbacks", stats.execFallbacks.load());

  sample.setInt(phase + "_total_files", stats.files.load());
  sample.setInt(phase + "_file_reads", stats.filesRead.load());
  sample.setInt(phase + "_file_queries", stats.filesQueried.load());
  sample.setInt(phase + "_file_stores", stats.filesUploaded.load());
  sample.setInt(phase + "_file_stores_bytes", stats.fileBytesUploaded.load());
  sample.setInt(phase + "_file_fallbacks", stats.fileFallbacks.load());

  sample.setInt(phase + "_total_blobs", stats.blobs.load());
  sample.setInt(phase + "_blob_queries", stats.blobsQueried.load());
  sample.setInt(phase + "_blob_stores", stats.blobsUploaded.load());
  sample.setInt(phase + "_blob_stores_bytes", stats.blobBytesUploaded.load());
  sample.setInt(phase + "_blob_fallbacks", stats.blobFallbacks.load());

  sample.setInt(phase + "_total_loads", stats.downloads.load());
  sample.setInt(phase + "_total_loads_bytes", stats.bytesDownloaded.load());
  sample.setInt(phase + "_throttles", stats.throttles.load());

  sample.setInt(phase + "_exec_cpu_usec", stats.execCpuUsec.load());
  sample.setInt(phase + "_exec_allocated_cores", stats.execAllocatedCores.load());
  sample.setInt(phase + "_exec_max_used_mem", stats.execMaxUsedMem.load());
  sample.setInt(phase + "_exec_reserved_mem", stats.execReservedMem.load());

  sample.setStr(phase + "_fellback",
    client.fellback() ? "true" : "false"
  );
}

// Compute a UnitIndex by parsing decls for all autoload-eligible files.
// If no Autoload.Query is specified by RepoOptions, this just indexes
// the input files.
bool compute_index(
    const CompilerOptions& po,
    StructuredLogEntry& sample,
    coro::TicketExecutor& executor,
    extern_worker::Client& client,
    UnitIndex& index
) {
  auto const onIndex = [&] (std::string&& rpath, Package::IndexMeta&& meta) {
    auto const interned_rpath = makeStaticString(rpath);
    auto insert = [&](auto const& names, auto& map, const char* kind) {
      for (auto name : names) {
        auto const ret = map.emplace(name, interned_rpath);
        if (!ret.second) {
          Logger::FWarning("Duplicate {} {} in {} and {}",
              kind, name, ret.first->first, interned_rpath
          );
        }
      }
    };
    insert(meta.types, index.types, "type");
    insert(meta.funcs, index.funcs, "function");
    insert(meta.constants, index.constants, "constant");
  };

  Package index_package{po.inputDir, false, executor, client};
  Timer indexTimer(Timer::WallTime, "indexing");

  auto const& repoFlags = RepoOptions::forFile(po.inputDir.c_str()).flags();
  auto const queryStr = repoFlags.autoloadQuery();
  if (!queryStr.empty() && po.parseOnDemand) {
    // Index the files specified by Autoload.Query
    if (!addAutoloadQueryToPackage(index_package, queryStr)) return false;
  } else {
    // index just the input files
    addInputsToPackage(index_package, po);
  }

  if (!coro::wait(index_package.index(onIndex))) return false;

  logPhaseStats("index", index_package, client, sample,
                indexTimer.getMicroSeconds());
  Logger::FInfo("index size: types={:,} funcs={:,} constants={:,}",
      index.types.size(),
      index.funcs.size(),
      index.constants.size()
  );
  client.resetStats();

  return true;
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
                                     const RepoOptionsFlags& flags) {
    return Package::parseRun(contents, flags);
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
                                  const RepoOptionsFlags& flags) {
    auto wrapper = Package::parseRun(contents, flags);
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

///////////////////////////////////////////////////////////////////////////////

bool process(const CompilerOptions &po) {
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
  sample.setInt("use_rich_client", Option::ExternWorkerUseRichClient);
  sample.setInt("use_zippy_rich_client",
      Option::ExternWorkerUseZippyRichClient);
  sample.setInt("use_p2p", Option::ExternWorkerUseP2P);
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
  sample.setStr("use_hhbbc", RO::EvalUseHHBBC ? "true" : "false");

  // Track the unit-emitters created for system during
  // hphp_process_init().
  SystemLib::keepRegisteredUnitEmitters(true);
  hphp_process_init();
  SCOPE_EXIT { hphp_process_exit(); };
  SystemLib::keepRegisteredUnitEmitters(false);

  auto const outputFile = po.outputDir + "/hhvm.hhbc";
  unlink(outputFile.c_str());

  auto executor = std::make_unique<coro::TicketExecutor>(
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
    std::make_unique<Client>(executor->sticky(), makeExternWorkerOptions());

  sample.setStr("extern_worker_impl", client->implName());

  UnitIndex index;
  if (!compute_index(po, sample, *executor, *client, index)) return false;

  auto package = std::make_unique<Package>(
    po.inputDir,
    po.parseOnDemand,
    *executor,
    *client
  );

  // Always used, but we can clear it early to save memory.
  Optional<SymbolSets> unique;
  unique.emplace();

  Optional<RepoAutoloadMapBuilder> autoload;
  Optional<RepoFileBuilder> repo;
  std::atomic<uint32_t> nextSn{0};
  std::mutex repoLock;

  // HHBBC specific state (if we're going to run it).
  Optional<WPI> hhbbcInputs;
  Optional<coro::AsyncValue<Ref<HHBBC::Config>>> hhbbcConfig;
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

  // Emit a fully processed unit (either processed by HHBBC or not).
  auto const emit = [&] (std::unique_ptr<UnitEmitter> ue) {
    assertx(ue);
    assertx(Option::GenerateBinaryHHBC ||
            Option::GenerateTextHHBC ||
            Option::GenerateHhasHHBC);

    if (Option::GenerateTextHHBC || Option::GenerateHhasHHBC) {
      genText(*ue, po.outputDir);
    }

    if (!Option::GenerateBinaryHHBC) return;

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

  // Process unit-emitters produced locally (usually systemlib stuff).
  auto const local = [&] (Package::UEVec ues) -> coro::Task<void> {
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

      if (keys.empty()) HPHP_CORO_RETURN_VOID;
      auto valueRefs = HPHP_CORO_AWAIT(client->storeMulti(std::move(values)));

      auto const numKeys = keys.size();
      assertx(valueRefs.size() == numKeys);

      for (size_t i = 0; i < numKeys; ++i) {
        hhbbcInputs->add(std::move(keys[i]), std::move(valueRefs[i]));
      }
      HPHP_CORO_RETURN_VOID;
    }

    // Otherwise just emit it
    for (auto& ue : ues) emit(std::move(ue));
    HPHP_CORO_RETURN_VOID;
  };

  // Parse a group of files remotely
  auto const remote = [&] (const Ref<Package::Config>& config,
                           Ref<Package::FileMetaVec> fileMetas,
                           std::vector<Package::FileData> files,
                           bool optimistic)
    -> coro::Task<Package::ParseMetaVec> {
    if (RO::EvalUseHHBBC) {
      // Run the HHBBC parse job, which produces WholeProgramInput
      // key/values.
      auto hhbbcConfigRef = HPHP_CORO_AWAIT(hhbbcConfig->getCopy());
      auto [inputValueRefs, metaRefs] = HPHP_CORO_AWAIT(
        client->exec(
          s_parseForHHBBCJob,
          std::make_tuple(
            config,
            std::move(hhbbcConfigRef),
            std::move(fileMetas)
          ),
          std::move(files),
          optimistic
        )
      );

      // The parse metadata and the keys are loaded, but the values
      // are kept as Refs.
      auto [parseMetas, inputKeys] =
        HPHP_CORO_AWAIT(client->load(std::move(metaRefs)));

      // We don't have unit-emitters to do uniqueness checking, but
      // the parse metadata has the definitions we can use instead.
      for (auto const& p : parseMetas) {
        if (!p.m_filepath) continue;
        unique->add(p.m_definitions, p.m_filepath);
      }

      auto const numKeys = inputKeys.size();
      size_t keyIdx = 0;
      for (auto& v : inputValueRefs) {
        for (auto& r : v) {
          always_assert(keyIdx < numKeys);
          hhbbcInputs->add(std::move(inputKeys[keyIdx]), std::move(r));
          ++keyIdx;
        }
      }
      always_assert(keyIdx == numKeys);

      HPHP_CORO_MOVE_RETURN(parseMetas);
    }

    // Otherwise, do a "normal" (non-HHBBC parse job), load the
    // unit-emitters and parse metadata, and emit the unit-emitters.
    auto [wrapperRefs, metaRefs] = HPHP_CORO_AWAIT(
      client->exec(
        s_parseJob,
        std::make_tuple(
          config,
          std::move(fileMetas)
        ),
        std::move(files),
        optimistic
      )
    );

    auto [wrappers, parseMetas] = HPHP_CORO_AWAIT(coro::collect(
      client->load(std::move(wrapperRefs)),
      client->load(std::move(metaRefs))
    ));

    for (auto& wrapper : wrappers) {
      if (!wrapper.m_ue) continue;
      emit(std::move(wrapper.m_ue));
    }
    HPHP_CORO_MOVE_RETURN(parseMetas);
  };

  {
    Timer parseTimer(Timer::WallTime, "parsing");
    addInputsToPackage(*package, po);

    if (!RO::EvalUseHHBBC && Option::GenerateBinaryHHBC) {
      autoload.emplace();
      repo.emplace(outputFile);
    }
    if (!coro::wait(package->parse(index, remote, local))) return false;

    logPhaseStats("parse", *package, *client, sample,
                  parseTimer.getMicroSeconds());
  }

  // We don't need the ondemand/autoload index after this point.
  index.clear();

  std::thread fileCache{
    [&, package = std::move(package)] () mutable {
      SCOPE_EXIT { package.reset(); };
      if (po.filecache.empty()) return;
      Timer _{Timer::WallTime, "saving file cache..."};
      HphpSessionAndThread session{Treadmill::SessionKind::CompilerEmit};
      package->getFileCache()->save(po.filecache.c_str());
      struct stat sb;
      stat(po.filecache.c_str(), &sb);
      Logger::Info("%" PRId64" MB %s saved",
                   (int64_t)sb.st_size/(1024*1024), po.filecache.c_str());
    }
  };
  SCOPE_EXIT { fileCache.join(); };

  std::thread asyncDispose;
  SCOPE_EXIT { if (asyncDispose.joinable()) asyncDispose.join(); };
  auto const dispose = [&] (std::unique_ptr<coro::TicketExecutor> e,
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

  auto const finish = [&] {
    if (!Option::GenerateBinaryHHBC) return true;
    Timer _{Timer::WallTime, "finalizing repo"};
    repo->finish(getGlobalData(), *autoload);
    return true;
  };
  if (!RO::EvalUseHHBBC) {
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

  Timer timer{Timer::WallTime, "running HHBBC"};
  HphpSession session{Treadmill::SessionKind::HHBBC};
  HHBBC::whole_program(
    std::move(*hhbbcInputs),
    std::move(executor),
    std::move(client),
    emit,
    dispose,
    sample,
    Option::ParserThreadCount > 0 ? Option::ParserThreadCount : 0
  );
  return finish();
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
    mkdir(po.outputDir.c_str(), 0777);
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
