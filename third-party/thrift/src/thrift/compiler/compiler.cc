/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * thrift - a lightweight cross-language rpc/serialization tool
 *
 * This file contains the main compiler engine for Thrift, which invokes the
 * scanner/parser to build the thrift object tree. The interface generation
 * code for each language lives in a file by the language name under the
 * generate/ folder, and all parse structures live in parse/
 *
 */

#include <thrift/compiler/compiler.h>

#ifdef _WIN32
#include <process.h> // @manual
#else
#include <unistd.h>
#endif
#include <ctime>
#include <fstream>
#include <iostream>
#include <set>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>

#include <thrift/compiler/ast/diagnostic_context.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/detail/system.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/generate/t_generator.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/sema/standard_mutator.h>
#include <thrift/compiler/sema/standard_validator.h>
#include <thrift/compiler/validator/validator.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

/**
 * Flags to control code generation
 */
struct gen_params {
  bool gen_recurse = false;
  std::string genfile;
  std::vector<std::string> targets;
  std::string out_path;
  bool add_gen_dir = true;
};

/**
 * Display the usage message.
 */
void usage() {
  fprintf(stderr, "Usage: thrift [options] file\n");
  fprintf(stderr, "Options:\n");
  fprintf(
      stderr,
      "  -o dir      Set the output directory for gen-* packages\n"
      "              (default: current directory)\n");
  fprintf(
      stderr,
      "  -out dir    Set the output location for generated files\n"
      "              (no gen-* folder will be created)\n");
  fprintf(
      stderr,
      "  -I dir      Add a directory to the list of directories\n"
      "              searched for include directives\n");
  fprintf(stderr, "  -nowarn     Suppress all compiler warnings (BAD!)\n");
  fprintf(
      stderr,
      "  -legacy-strict     Strict compiler warnings on (DEPRECATED)\n");
  fprintf(stderr, "  -v[erbose]  Verbose mode\n");
  fprintf(stderr, "  -r[ecurse]  Also generate included files\n");
  fprintf(stderr, "  -debug      Parse debug trace to stdout\n");
  fprintf(
      stderr,
      "  --allow-neg-keys  Allow negative field keys (Used to preserve protocol\n"
      "                    compatibility with older .thrift files)\n");
  fprintf(
      stderr,
      "  --allow-neg-enum-vals Allow negative enum vals (DEPRECATED)\n");
  fprintf(
      stderr,
      "  --allow-64bit-consts  Do not print warnings about using 64-bit constants\n");
  fprintf(
      stderr,
      "  --gen STR   Generate code with a dynamically-registered generator.\n"
      "              STR has the form language[:key1=val1[,key2,[key3=val3]]].\n"
      "              Keys and values are options passed to the generator.\n"
      "              Many options will not require values.\n");
  fprintf(
      stderr,
      "  --record-genfiles FILE\n"
      "              Save the list of generated files to FILE,\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Available generators (and options):\n");

  for (const auto& gen : generator_registry::get_generators()) {
    fmt::print(
        stderr,
        "  {} ({}):\n{}",
        gen.second->name(),
        gen.second->long_name(),
        gen.second->documentation());
  }
}

bool isPathSeparator(const char& c) {
#ifdef _WIN32
  return c == ';';
#else
  return c == ':';
#endif
}

// Returns the input file name if successful, otherwise returns an empty
// string.
std::string parse_args(
    const std::vector<std::string>& arguments,
    parsing_params& pparams,
    gen_params& gparams,
    diagnostic_params& dparams) {
  // Check for necessary arguments, you gotta have at least a filename and
  // an output language flag.
  if (arguments.size() < 3 && gparams.targets.empty()) {
    usage();
    return {};
  }

  // A helper that grabs the next argument, if possible.
  // Outputs an error and returns nullptr if not.
  size_t arg_i = 1; // Skip the binary name.
  auto consume_arg = [&](const char* arg_name) -> const std::string* {
    // Note: The input filename must be the last argument.
    if (arg_i + 2 >= arguments.size()) {
      fprintf(
          stderr,
          "!!! Missing %s between %s and '%s'\n",
          arg_name,
          arguments[arg_i].c_str(),
          arguments[arg_i + 1].c_str());
      usage();
      return nullptr;
    }
    return &arguments[++arg_i];
  };

  // Hacky parameter handling... I didn't feel like using a library sorry!
  bool nowarn =
      false; // Guard so --nowarn and --legacy-strict are order agnostic.
  for (; arg_i < arguments.size() - 1;
       ++arg_i) { // Last argument is the src file.
    // Parse flag.
    std::string flag;
    if (arguments[arg_i].size() < 2 || arguments[arg_i][0] != '-') {
      fprintf(stderr, "!!! Expected flag, got: %s\n", arguments[arg_i].c_str());
      usage();
      return {};
    } else if (arguments[arg_i][1] == '-') {
      flag = arguments[arg_i].substr(2);
    } else {
      flag = arguments[arg_i].substr(1);
    }

    // Interpret flag.
    if (flag == "debug") {
      dparams.debug = true;
    } else if (flag == "nowarn") {
      dparams.warn_level = 0;
      nowarn = true;
    } else if (flag == "legacy-strict") {
      pparams.strict = 255;
      if (!nowarn) { // Don't override nowarn.
        dparams.warn_level = 2;
      }
    } else if (flag == "v" || flag == "verbose") {
      dparams.info = true;
    } else if (flag == "r" || flag == "recurse") {
      gparams.gen_recurse = true;
    } else if (flag == "allow-neg-keys") {
      pparams.allow_neg_field_keys = true;
    } else if (flag == "allow-neg-enum-vals") {
      // no-op
    } else if (flag == "allow-64bit-consts") {
      pparams.allow_64bit_consts = true;
    } else if (flag == "record-genfiles") {
      auto* arg = consume_arg("genfile file specification");
      if (arg == nullptr) {
        return {};
      }
      gparams.genfile = *arg;
    } else if (flag == "gen") {
      auto* arg = consume_arg("generator specification");
      if (arg == nullptr) {
        return {};
      }
      gparams.targets.push_back(*arg);
    } else if (flag == "I") {
      auto* arg = consume_arg("include directory");
      if (arg == nullptr) {
        return {};
      }
      // An argument of "-I\ asdf" is invalid and has unknown results
      pparams.incl_searchpath.push_back(*arg);
    } else if (flag == "o" || flag == "out") {
      auto* arg = consume_arg("output directory");
      if (arg == nullptr) {
        return {};
      }
      std::string out_path = *arg;
      bool add_gen_dir = (flag == "o");

      // Strip out trailing \ on a Windows path
      if (detail::platform_is_windows()) {
        int last = out_path.length() - 1;
        if (out_path[last] == '\\') {
          out_path.erase(last);
        }
      }

      if (!add_gen_dir) {
        // Invoker specified `-out blah`. We are supposed to output directly
        // into blah, e.g. `blah/Foo.java`. Make the directory if necessary,
        // just like how for `-o blah` we make `o/gen-java`
        boost::system::error_code errc;
        boost::filesystem::create_directory(out_path, errc);
        if (errc) {
          fprintf(
              stderr,
              "Output path %s is unusable or not a directory\n",
              out_path.c_str());
          return {};
        }
      }
      if (!boost::filesystem::is_directory(out_path)) {
        fprintf(
            stderr,
            "Output path %s is unusable or not a directory\n",
            out_path.c_str());
        return {};
      }
      gparams.out_path = std::move(out_path);
      gparams.add_gen_dir = add_gen_dir;
    } else {
      fprintf(
          stderr, "!!! Unrecognized option: %s\n", arguments[arg_i].c_str());
      usage();
      return {};
    }
  }

  if (const char* env_p = std::getenv("THRIFT_INCLUDE_PATH")) {
    std::vector<std::string> components;
    boost::algorithm::split(components, env_p, isPathSeparator);
    pparams.incl_searchpath.insert(
        pparams.incl_searchpath.end(), components.begin(), components.end());
  }

  // You gotta generate something!
  if (gparams.targets.empty()) {
    fprintf(stderr, "!!! No output language(s) specified\n\n");
    usage();
    return {};
  }

  // Return the input file name.
  assert(arg_i == arguments.size() - 1);
  return arguments[arg_i];
}

enum class parse_control : bool { stop, more };

// `callback(key, value)` will be called for each key=value generator's option.
// If there is no value, `value` will be empty string.
void parse_generator_options(
    const std::string& option_string,
    std::function<parse_control(std::string, std::string)> callback) {
  std::vector<std::string> parts;
  bool inside_braces = false;
  boost::algorithm::split(parts, option_string, [&inside_braces](char c) {
    if (c == '{' || c == '}') {
      inside_braces = (c == '{');
    }
    return c == ',' && !inside_braces;
  });
  for (const auto& part : parts) {
    auto key = part.substr(0, part.find('='));
    auto value = part.substr(std::min(key.size() + 1, part.size()));
    if (callback(std::move(key), std::move(value)) == parse_control::stop) {
      break;
    }
  }
}

// Generate code.
bool generate(
    const gen_params& params,
    t_program& program,
    std::set<std::string>& already_generated,
    t_program_bundle& program_bundle,
    diagnostics_engine& diags) {
  // Oooohh, recursive code generation, hot!!
  if (params.gen_recurse) {
    // Add the path we are about to generate.
    already_generated.emplace(program.path());
    for (const auto& include : program.get_included_programs()) {
      if (!already_generated.count(include->path()) &&
          !generate(
              params, *include, already_generated, program_bundle, diags)) {
        return false;
      }
    }
  }

  // Generate code!
  try {
    bool success = true;
    std::ofstream genfile;
    if (!params.genfile.empty()) {
      genfile.open(params.genfile);
    }
    for (auto target : params.targets) {
      auto colon_pos = target.find(':');
      auto generator_name = target.substr(0, colon_pos);
      auto options = std::map<std::string, std::string>();
      parse_generator_options(
          target.substr(colon_pos + 1), [&](std::string k, std::string v) {
            options[std::move(k)] = std::move(v);
            return parse_control::more;
          });
      auto generator = generator_registry::make_generator(
          generator_name, program, diags.source_mgr(), program_bundle);
      if (!generator) {
        fmt::print(
            stderr, "Error: Invalid generator name: {}\n", generator_name);
        usage();
        continue;
      }
      generator->process_options(options, params.out_path, params.add_gen_dir);

      auto validator_diags = diagnostics_engine(
          diags.source_mgr(),
          [&](diagnostic d) {
            if (d.level() == diagnostic_level::error) {
              success = false;
            }
            diags.report(std::move(d));
          },
          diags.params());
      validator_list validators(validator_diags);
      generator->fill_validator_list(validators);
      validators.traverse(&program);
      if (!success) {
        continue;
      }

      diags.report(
          source_location(),
          diagnostic_level::info,
          "generating \"{}\"",
          target);

      generator->generate_program();
      if (genfile.is_open()) {
        for (const std::string& s : generator->get_genfiles()) {
          genfile << s << "\n";
        }
      }
    }
    return success;
  } catch (const std::string& s) {
    printf("Error: %s\n", s.c_str());
    return false;
  } catch (const char* exc) {
    printf("Error: %s\n", exc);
    return false;
  }
}

bool generate(
    const gen_params& params,
    t_program_bundle& program_bundle,
    diagnostics_engine& diags) {
  std::set<std::string> already_generated;
  return generate(
      params,
      *program_bundle.get_root_program(),
      already_generated,
      program_bundle,
      diags);
}

std::string get_include_path(
    const std::vector<std::string>& generator_targets,
    const std::string& input_filename) {
  std::string include_prefix;
  for (const auto& target : generator_targets) {
    const auto colon_pos = target.find(':');
    if (colon_pos == std::string::npos) {
      continue;
    }
    const auto lang_name = target.substr(0, colon_pos);
    if (lang_name != "cpp2" && lang_name != "mstch_cpp2") {
      continue;
    }

    const auto lang_args = target.substr(colon_pos + 1);
    parse_generator_options(lang_args, [&](std::string k, std::string v) {
      if (k.find("include_prefix") != std::string::npos) {
        include_prefix = std::move(v);
        return parse_control::stop;
      }
      return parse_control::more;
    });
  }

  // infer cpp include prefix from the filename passed in if none specified.
  if (include_prefix == "") {
    if (input_filename.rfind('/') != std::string::npos) {
      include_prefix = input_filename.substr(0, input_filename.rfind('/'));
    }
  }

  return include_prefix;
}

} // namespace

std::unique_ptr<t_program_bundle> parse_and_mutate_program(
    source_manager& sm,
    diagnostic_context& ctx,
    const std::string& filename,
    parsing_params params,
    bool return_nullptr_on_failure,
    t_program_bundle* already_parsed) {
  auto programs =
      parse_ast(sm, ctx, filename, std::move(params), already_parsed);
  if (!programs || ctx.has_errors()) {
    // Mutations should be only performed on a valid AST.
    return !return_nullptr_on_failure ? std::move(programs) : nullptr;
  }
  auto result = standard_mutators()(ctx, *programs);
  if (result.unresolvable_typeref && return_nullptr_on_failure) {
    // Stop processing if there is unresolvable typeref.
    programs = nullptr;
  }
  return programs;
}

std::pair<std::unique_ptr<t_program_bundle>, diagnostic_results>
parse_and_mutate_program(
    source_manager& sm,
    const std::string& filename,
    parsing_params params,
    diagnostic_params dparams) {
  diagnostic_results results;
  diagnostic_context ctx(sm, results, std::move(dparams));
  return {
      parse_and_mutate_program(sm, ctx, filename, std::move(params)), results};
}

std::unique_ptr<t_program_bundle> parse_and_dump_diagnostics(
    const std::string& filename,
    source_manager& sm,
    parsing_params pparams,
    diagnostic_params dparams) {
  diagnostic_results results;
  diagnostic_context ctx(sm, results, std::move(dparams));
  auto program =
      parse_and_mutate_program(sm, ctx, filename, std::move(pparams));
  for (const auto& diag : results.diagnostics()) {
    std::cerr << diag << "\n";
  }
  return program;
}

std::unique_ptr<t_program_bundle> parse_and_get_program(
    source_manager& sm, const std::vector<std::string>& arguments) {
  // Parse arguments.
  parsing_params pparams;
  pparams.allow_missing_includes = true;
  gen_params gparams;
  diagnostic_params dparams;
  gparams.targets.push_back(""); // Avoid needing to pass --gen
  std::string filename = parse_args(arguments, pparams, gparams, dparams);
  if (filename.empty()) {
    return {};
  }

  diagnostic_context ctx(
      sm,
      [](const diagnostic& d) { fmt::print(stderr, "{}\n", d.str()); },
      diagnostic_params::only_errors());
  return parse_ast(sm, ctx, filename, std::move(pparams));
}

compile_result compile(
    const std::vector<std::string>& arguments, source_manager& source_mgr) {
  compile_result result;

  // Parse arguments.
  parsing_params pparams;
  gen_params gparams;
  diagnostic_params dparams;
  std::string input_filename = parse_args(arguments, pparams, gparams, dparams);
  if (input_filename.empty()) {
    return result;
  }
  diagnostic_context ctx(source_mgr, result.detail, std::move(dparams));

  // Parse it!
  auto programs = parse_and_mutate_program(
      source_mgr,
      ctx,
      input_filename,
      pparams,
      true /* return_nullptr_on_failure */);
  if (!programs) {
    return result;
  }

  // Load standard library if available.
  try {
    auto path = source_manager::find_include_file(
        "thrift/lib/thrift/schema.thrift", "", pparams.incl_searchpath);
    if (!programs->find_program(path)) {
      auto inc = parse_and_mutate_program(
          source_mgr,
          ctx,
          path,
          pparams,
          true /* return_nullptr_on_failure */,
          programs.get());
      if (inc) {
        programs->add_implicit_includes(std::move(inc));
      }
    }
  } catch (const std::runtime_error& ex) {
    ctx.warning(
        source_location{},
        "Could not load Thrift standard libraries: {}",
        ex.what());
  }

  programs->root_program()->set_include_prefix(
      get_include_path(gparams.targets, input_filename));

  // Validate it!
  validator::validate(programs->root_program(), ctx);
  standard_validator()(ctx, *programs->root_program());
  if (result.detail.has_error()) {
    return result;
  }

  // Generate it!
  ctx.begin_visit(*programs->root_program());
  try {
    if (generate(gparams, *programs, ctx)) {
      result.retcode = compile_retcode::success;
    }
  } catch (const std::exception& e) {
    ctx.error(*programs->root_program(), "{}", e.what());
  }
  ctx.end_visit(*programs->root_program());
  return result;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
