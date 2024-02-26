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

#include "hphp/compiler/compiler-systemlib.h"

#include "hphp/hack/src/hackc/ffi_bridge/decl_provider.h"
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs.h"

#include "hphp/hhvm/process-init.h"

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/preg.h"

#include "hphp/runtime/ext/extension-registry.h"

#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/runtime/version.h"

#include "hphp/util/build-info.h"
#include "hphp/util/logger.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/timer.h"

#include <boost/filesystem.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>


#include <filesystem>
#include <iostream>

using namespace boost::program_options;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

struct CompilerOptions {
  std::string outputDir;
  std::string inputDir;
  std::vector<std::string> inputs;
};

///////////////////////////////////////////////////////////////////////////////

int prepareOptions(CompilerOptions &po, int argc, char **argv) {
  options_description desc("HipHop Systemlib Compiler for PHP Usage:\n\n"
                           "\thhvm --compile-systemlib <options> <inputs>\n\n"
                           "Options");

  std::vector<std::string> formats;

  desc.add_options()
    ("help", "display this message")
    ("version", "display version number")
    ("output-dir", value<std::string>(&po.outputDir), "output directory")
    ("input-dir", value<std::string>(&po.inputDir), "input directory")
    ("inputs,i", value<std::vector<std::string>>(&po.inputs)->composing(),
     "input file names")
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

  if (po.outputDir.empty()) {
    Logger::Error("Error in command line: output-dir must be provided.");
    std::cout << desc << "\n";
    return -1;
  }

  if (po.inputDir.empty()) po.inputDir = '.';
  po.inputDir = FileUtil::normalizeDir(po.inputDir);

  IniSetting::Map ini = IniSetting::Map::object;
  Hdf config;
  Hdf runtime = config["Runtime"];
  RuntimeOption::Load(ini, runtime);
  // Option::Load(ini, config);

  pcre_init();

  return 0;
}

bool compile_systemlib(const std::filesystem::path& path, std::string output_dir, const Extension* extension) {
  std::string content;
  boost::filesystem::load_string_file(path.string(), content);

  // Create Unit Emitter
  std::string fname = "/:"+path.filename().string();
  auto ue = compile_systemlib_string_to_ue(content.c_str(), content.length(), fname.c_str(), extension);
  assertx(ue);

  if (!ue->check(getenv("HHVM_VERIFY_VERBOSE_SYSTEM"))) {
    Logger::Error("Verification failed for unit %s. Re-run with "
                  "HHVM_VERIFY_VERBOSE_SYSTEM=1 to see more details.",
                  fname.c_str());
    return false;
  }

  if (ue->m_fatalUnit) {
    Logger::Error("Something went wrong when compiling %s because a fatal unit was created",
                  fname.c_str());
    return false;
  }

  UnitEmitterSerdeWrapper uew = std::move(ue);

  BlobEncoder uew_encoder;
  uew.serde(uew_encoder);

  boost::filesystem::save_string_file(output_dir + "/" + path.filename().string() + ".ue", std::string(static_cast<const char*>(uew_encoder.data()), uew_encoder.size()));

  // Create Decls
  auto const& defaults = RepoOptions::defaultsForSystemlib();

  hackc::DeclParserConfig options;
  defaults.flags().initDeclConfig(options);

  auto decls = hackc::direct_decl_parse_and_serialize(
    options,
    fname,
    {(const uint8_t*)content.data(), content.size()}
  );

  if (decls.has_errors) {
    Logger::Error("Something went wrong when getting decls for %s because it has errors",
                  fname.c_str());
    return false;
  }

  auto serialized = hackc::decls_holder_to_binary(*decls.decls);
  boost::filesystem::save_string_file(output_dir + "/" + path.filename().string() + ".decls", std::string(serialized.begin(), serialized.end()));

  return true;
}

bool process(CompilerOptions &po) {
  register_process_init();

  hphp_process_init(true);
  ExtensionRegistry::moduleRegisterNative();
  SCOPE_EXIT { hphp_process_exit(); };

  auto files = std::map<std::string, std::filesystem::path>();

  for (const auto& input : po.inputs) {
    auto input_path = std::filesystem::path(po.inputDir + input);
    if (std::filesystem::is_directory(input_path)) {
      for (const auto& entry : std::filesystem::directory_iterator(input_path)) {
        auto path = entry.path();
        files[path.filename().string()] = path;
      }
    } else {
      files[input_path.filename().string()] = input_path;
    }
  }

  for (auto extension : ExtensionRegistry::getExtensions()) {
    for (auto file : extension->hackFiles()) {
      auto path = files.at("ext_" + file);
      if (!compile_systemlib(path.string(), po.outputDir, extension)) {
        return false;
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

int compiler_systemlib_main(int argc, char **argv) {
  try {
    rds::local::init();
    SCOPE_EXIT { rds::local::fini(); };

    CompilerOptions po;
    auto const ret = prepareOptions(po, argc, argv);
    if (ret == 1) return 0; // --help
    if (ret != 0) return ret; // command line error

    Timer totalTimer(Timer::WallTime, "running compile systemlib");
    always_assert_flog(
      mkdir(po.outputDir.c_str(), 0777) == 0 || errno == EEXIST,
      "Unable to mkdir({}): {}",
      po.outputDir.c_str(),
      folly::errnoStr(errno)
    );

    if (!process(po)) {
      Logger::Error("compile systemlib failed");
      return -1;
    } else {
      Logger::Info("all files saved in %s ...", po.outputDir.c_str());
      return 0;
    }
    return 0;
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
