// Copyright (c) 2021, Facebook, Inc.
// All rights reserved.

#include "hphp/hack/src/hackc/compiler_helpers_ffi.rs"
#include "hphp/hack/src/hackc/ffi_bridge/compiler_ffi.rs"
#include "hphp/hack/src/parser/ffi_bridge/parser_ffi.rs"

#include "hphp/hack/src/hackc/hhbc-ast.h"

#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <folly/String.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <array>
#include <string>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <stdexcept>

namespace po = boost::program_options;
using namespace HPHP::hackc;
using namespace HPHP::hackc::hhbc;

// Proxy for this section of .hhconfig.
char const* auto_namespace_map{"{\"hhvm.aliased_namespaces\":{\"global_value\":{\"Async\": \"HH\\\\Lib\\\\Async\", \"C\": \"FlibSL\\\\C\", \"Dict\": \"FlibSL\\\\Dict\", \"File\": \"HH\\\\Lib\\\\File\", \"IO\": \"HH\\\\Lib\\\\IO\", \"Keyset\": \"FlibSL\\\\Keyset\", \"Locale\": \"FlibSL\\\\Locale\", \"Math\": \"FlibSL\\\\Math\", \"OS\": \"HH\\\\Lib\\\\OS\", \"PHP\": \"FlibSL\\\\PHP\", \"PseudoRandom\": \"FlibSL\\\\PseudoRandom\", \"Regex\": \"FlibSL\\\\Regex\", \"Rx\": \"HH\\\\Rx\", \"SecureRandom\": \"FlibSL\\\\SecureRandom\", \"Str\": \"FlibSL\\\\Str\", \"Vec\": \"FlibSL\\\\Vec\"}}}"};

// NOLINTNEXTLINE
struct options {
  std::string filename;
  bool fallback;
  std::vector<std::string> config_list;
  std::vector<std::string> ini_cli_list;
  bool debug_time;
  boost::optional<std::string> config_file;
  bool dump_symbol_refs;
  bool dump_config;
  bool extract_facts_as_json;
  bool parse;
  bool log_stats;
  bool for_debugger_eval;
  bool disable_toplevel_elaboration;
  bool include_header;
  bool dump_desugared_expression_trees;
  int32_t emit_class_pointers;
  int32_t check_int_overflow;
  bool verify_decls_ffi;
  bool test;
  bool compile_and_print_hhas;
  bool extract_facts;
  bool extract_facts_from_decls;
  bool test_compile_with_decls;
  bool log_decls_requested;
  bool use_serialized_decls;
  bool daemon;

  options() : fallback{false}
            , debug_time{false}
            , dump_symbol_refs{false}
            , dump_config{false}
            , extract_facts_as_json{false}
            , parse{false}
            , log_stats{false}
            , for_debugger_eval{false}
            , disable_toplevel_elaboration{false}
            , include_header{false}
            , dump_desugared_expression_trees{false}
            , emit_class_pointers{0}
            , check_int_overflow{0}
            , verify_decls_ffi{false}
            , test{false}
            , compile_and_print_hhas{false}
            , extract_facts{false}
            , extract_facts_from_decls{false}
            , test_compile_with_decls{false}
            , log_decls_requested{false}
            , use_serialized_decls{false}
            , daemon{false}
  {}
};

// NOLINTNEXTLINE
std::ostream& operator << (std::ostream& os, options const& opts) {
  os << "input-file: " << opts.filename << "\n";
  os << "fallback: " << std::boolalpha << opts.fallback << "\n";
  os << "value:"; for(auto const& cfg : opts.config_list) os << " " << cfg; os << "\n";
  os << "define:"; for(auto const& ini_cli : opts.ini_cli_list) os << " " << ini_cli; os << "\n";
  os << "debug-time: " << std::boolalpha << opts.fallback << "\n";
  os << "config:" << opts.config_file << "\n";
  os << "dump-symbol-refs: " << std::boolalpha << opts.dump_symbol_refs << "\n";
  os << "dump-config: " << std::boolalpha << opts.dump_config << "\n";
  os << "extract-facts-as-json: " << std::boolalpha << opts.extract_facts_as_json << "\n";
  os << "parse: " << std::boolalpha << opts.parse << "\n";
  os << "log-stats: " << std::boolalpha << opts.log_stats << "\n";
  os << "for-debugger-eval: " << std::boolalpha << opts.for_debugger_eval << "\n";
  os << "disable-toplevel-elaboration: " << std::boolalpha << opts.disable_toplevel_elaboration << "\n";
  os << "include-header: " << std::boolalpha << opts.include_header << "\n";
  os << "dump-desugared-expression-trees: " << std::boolalpha << opts.dump_desugared_expression_trees << "\n";
  os << "emit-class-pointers: " << opts.emit_class_pointers << "\n";
  os << "check-int-overflow: " << opts.check_int_overflow << "\n";
  os << "verify-decls-ffi: " << opts.verify_decls_ffi << "\n";
  os << "test: " << opts.test << "\n";
  os << "compile-and-print-hhas: " << opts.compile_and_print_hhas << "\n";
  os << "extract-facts: " << opts.extract_facts << "\n";
  os << "extract-facts-from-decls: " << opts.extract_facts_from_decls << "\n";
  os << "test-compile-with-decls: " << opts.test_compile_with_decls << "\n";
  os << "log-decls-requested: " << opts.log_decls_requested << "\n";
  os << "use-serialized-decls: " << opts.use_serialized_decls << "\n";
  os << "daemon: " << opts.daemon << "\n";

  return os;
}

std::uint8_t flags_from_options(options const& opts) {
  return make_env_flags(
    false,                              // is_systemlib
    false,                              // is_evaled
    opts.for_debugger_eval,             // for_debugger_eval
    opts.dump_symbol_refs,              // dump_symbol_refs
    opts.disable_toplevel_elaboration,  // disable_toplevel_elaboration
    opts.test_compile_with_decls        // enable_decl
  );
}

std::pair<uint32_t, uint32_t> parser_hhbc_flags_from_options(options const& opts) {
  std::string hdf_options = folly::join(" ", opts.config_list);
  std::string ini_options = folly::join(" ", opts.ini_cli_list);
  std::string config_file = opts.config_file ? opts.config_file.get() : "";

  uint32_t parser_flags = hackc_parser_flags_ffi(config_file, hdf_options, ini_options);
  uint32_t hhbc_flags = hackc_hhbc_flags_ffi(config_file, hdf_options, ini_options);
  return {parser_flags, hhbc_flags};
}

options get_opts(int argc, char const* argv[]) {
  options opts;

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "Display this list of options")
    ("input-file", po::value(&opts.filename), "input file")
    ("config,c", po::value(&opts.config_file), "Config file in JSON format")
    ("value,v", po::value(&opts.config_list), "Configuration: Server.Port=<value>\n    Allows overriding config options passed on a a file")
    ("define,d", po::value(&opts.ini_cli_list), "INI format configuration options: hhvm.php7.all=<value>\n Allows overridinng config options passed on a file")
    ("fallback", po::bool_switch(&opts.fallback), "Enables fallback compilation")
    ("extract-facts-as-json", po::bool_switch(&opts.extract_facts_as_json), "Extract facts from the source code in JSON format")
    ("parse", po::bool_switch(&opts.parse), "Render the source text parse tree")
    ("debug-time", po::bool_switch(&opts.debug_time), "Enables debug logging for elapsed time")
    ("dump-symbol-refs", po::bool_switch(&opts.dump_symbol_refs), "Dump symbol ref sections of HHAS")
    ("dump-config", po::bool_switch(&opts.dump_config), "Dump configuration settings")
    ("enable-logging-stats", po::bool_switch(&opts.log_stats), "Starts logging stats")
    ("for-debugger-eval", po::bool_switch(&opts.for_debugger_eval), "Mutate the program as if we're in the debugger repl")
    ("disable-toplevel-elaboration", po::bool_switch(&opts.disable_toplevel_elaboration), "Disable toplevel definition elaboration")
    ("include-header", po::bool_switch(&opts.include_header), "Include JSON header")
    ("dump-desugared-expression-trees", po::bool_switch(&opts.dump_desugared_expression_trees), "Print the source code with expression tree literals desugared. Best effort debugging tool.")
    ("emit-class-pointers", po::value(&opts.emit_class_pointers), "Emit class pointers")
    ("check-int-overflow", po::value(&opts.check_int_overflow), "Check integer overflow if arg > 0")
    ("verify-decls-ffi", po::bool_switch(&opts.verify_decls_ffi), "Verify decls ffi")
    ("test", po::bool_switch(&opts.test), "Test FFIs")
    ("compile-and-print-hhas", po::bool_switch(&opts.compile_and_print_hhas), "Compile source text to HhasProgram")
    ("extract-facts", po::bool_switch(&opts.extract_facts), "Extract facts from source text and print the json")
    ("extract-facts-from-decls", po::bool_switch(&opts.extract_facts_from_decls), "Parse decls from source text, transform them into facts, and print the facts in JSON format")
    ("test-compile-with-decls", po::bool_switch(&opts.test_compile_with_decls), "Compile file with decls from the same file available during compilation")
    ("log-decls-requested", po::bool_switch(&opts.log_decls_requested), "Instead of printing the hhas, print a list of the decls requested during compilation")
    ("use-serialized-decls", po::bool_switch(&opts.use_serialized_decls), "Use serialized decl instead of decl pointer as the decl provider API")
    ("daemon", po::bool_switch(&opts.daemon), "Runs in daemon mode for testing purposes. Do not rely on for production")
    ;
  po::positional_options_description p;
  p.add("input-file", -1)
    ;
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if(vm.count("help")) {
    std::cout << "usage: " << argv[0] << " [options] file\n";
    std::cout << desc << std::endl;

    std::exit(1);
  }

  if(!vm.count("input-file") && vm["test"].defaulted() && vm["daemon"].defaulted()) {
    std::cout << "usage: " << argv[0] << " [options] file\n";

    std::exit(1);
  }

  return opts;
}

namespace {
  // Used for `test`. Increments the failed test count and the failed FFIs after
  // running each `test_func` on a specific FFI, e.g. `test_parse`, `test_compile_ffi`
  std::pair<std::set<int>, std::set<std::string>> update_test_results (
    options const& opts,
    std::vector<std::pair<std::string, bool>> const& tests,
    int test_idx,
    std::pair<std::set<int>, std::set<std::string>>& total_failed,
    std::function< bool(options const&, std::string const&, bool) > test_func,
    char const* ffi
  ) {
    std::string const& test = tests[test_idx].first;
    bool should_fatal = tests[test_idx].second;

    std::cout << "* Testing " << ffi << "...\n";

    std::set<int> failed_tests = total_failed.first;
    std::set<std::string> failed_ffis = total_failed.second;

    bool failed = !test_func(opts, test, should_fatal);
    if (failed) {
      std::cout << "Failed!\n" << std::endl;
      failed_ffis.insert(ffi);
      failed_tests.insert(test_idx);
    } else {
      std::cout << "Passed!\n" << std::endl;
    }

    return { failed_tests, failed_ffis };
  }

  inline HhasProgram const* hhasProgramRaw(::rust::Box<HhasProgramWrapper> const& program) {
    return (HhasProgram const*)(&(*program));
  }

  // Used for `test_compile_hhas_ffi` to check basic information about a compiled hhas program
  void report(::rust::Box<HhasProgramWrapper> const& prog_wrapped) {
    auto check_member = [] (auto const& slice_member, char const* member_name) {
      int num_elems = slice_member.len;
      void const* elem_data = slice_member.data;

      std::cout << "Found " << num_elems << " " << member_name << "\n";

      if (num_elems != 0 && elem_data == nullptr) {
        std::cerr << member_name << " data invalid\n";
      }
    };

    HhasProgram const* prog = hhasProgramRaw(prog_wrapped);

    check_member(prog->functions, "functions");
    check_member(prog->classes, "classes");
    check_member(prog->typedefs, "typedefs");
    check_member(prog->constants, "constants");
    check_member(prog->symbol_refs.includes, "includes");
    check_member(prog->symbol_refs.constants, "constant symbols");
    check_member(prog->symbol_refs.functions, "function symbols");
    check_member(prog->symbol_refs.classes, "class symbols");
  }

  // Utility print for daemon mode compatibility
  // Prints the number of characters of the compiled result to stdout along with \n and flushes
  // Then prints the compiled result, \n, and flushes
  // Do not rely on daemon mode for production use cases
  void daemon_compatible_print(options const& opts, std::string& output) {
    // Need to account for utf-8 encoding and text streams with the python test runner
    // A whole mess: https://stackoverflow.com/questions/3586923/counting-unicode-characters-in-c
    int len = 0;
    for (int i = 0; i < output.length(); i++) {
      if ((output[i] & 0xc0) != 0x80) {
        len++;
      }
    }

    if (opts.daemon) {
      // Add one for the newline character
      std::cout << (len + 1) << std::endl;
    }
    std::cout << output << std::endl;
  }
}

// Parser options
constexpr bool codegen = true;
constexpr bool hhvm_compat_mode = true;
constexpr bool php5_compat_mode = true;
constexpr bool allow_new_attribute_syntax = true;
constexpr bool enable_xhp_class_modifier = true;
constexpr bool disable_xhp_element_mangling = false;
constexpr bool disable_xhp_children_declarations = false;
constexpr bool disallow_fun_and_cls_meth_pseudo_funcs = true;
constexpr bool interpret_soft_types_as_like_types = true;

rust::String parse_(const std::string& source_text) {
  ParserEnv env = {
    codegen
  , hhvm_compat_mode
  , php5_compat_mode
  , allow_new_attribute_syntax
  , enable_xhp_class_modifier
  , disable_xhp_element_mangling
  , disable_xhp_children_declarations
  , disallow_fun_and_cls_meth_pseudo_funcs
  , interpret_soft_types_as_like_types
  };
  return hackc_parse_positioned_full_trivia_cpp_ffi(source_text, env);
}

int32_t getFactsOpts() {
  int32_t flags =
    (int)php5_compat_mode << 0 |
    (int)hhvm_compat_mode << 1 |
    (int)allow_new_attribute_syntax << 2 |
    (int)enable_xhp_class_modifier << 3 |
    (int)disable_xhp_element_mangling << 4;

  return flags;
}

int32_t getDeclOpts() {
  int32_t flags =
    (int)disable_xhp_element_mangling << 0 |
    (int)interpret_soft_types_as_like_types << 1 |
    (int)allow_new_attribute_syntax << 2 |
    (int)enable_xhp_class_modifier << 3 |
    (int)php5_compat_mode << 4 |
    (int)hhvm_compat_mode << 5;

  return flags;
}

void parse(options const& opts) {
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(opts.filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  ::rust::String parse_tree{parse_(source_text)};
  std::string output = std::string(parse_tree);
  daemon_compatible_print(opts, output);
}

bool test_parse(
  options const& /* opts */,
  std::string const& source_text,
  bool /* should_fatal */
) {
  ::rust::String parse_tree{parse_(source_text)};
  if (parse_tree.empty()) {
    return false;
  }
  return true;
}

std::string extract_facts_as_json_(options const& opts, std::string const& source_text) {
  int32_t flags = getFactsOpts();

  ::rust::String facts_json{hackc_extract_facts_as_json_cpp_ffi(flags, opts.filename, source_text)};

  return std::string(facts_json);
}

void extract_facts_as_json(options const& opts) {
  std::string filename{opts.filename.c_str()};
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  std::string facts = extract_facts_as_json_(opts, source_text);
  daemon_compatible_print(opts, facts);
}

bool test_extract_facts_as_json(options const& opts, std::string const& source_text, bool should_fatal) {
  std::string facts = extract_facts_as_json_(opts, source_text.c_str());
  if (facts == "") {
    if (!should_fatal) {
      return false;
    }
  }
  return true;
}

FactsResult extract_facts_(options const& opts, std::string const& source_text) {
  int32_t flags = getFactsOpts();

  return hackc_extract_facts_cpp_ffi(flags, opts.filename, source_text);
}

void extract_facts(options const& opts) {
  std::string filename{opts.filename.c_str()};
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  FactsResult facts{extract_facts_(opts, source_text)};
  ::rust::String facts_as_json{hackc_facts_to_json_cpp_ffi(facts, source_text)};
  std::string output = std::string(facts_as_json);
  daemon_compatible_print(opts, output);
}

bool test_extract_facts(options const& opts, std::string const& source_text, bool should_fatal) {
  FactsResult facts{extract_facts_(opts, source_text.c_str())};
  ::rust::String facts_as_json{hackc_facts_to_json_cpp_ffi(facts, source_text)};
  if (std::string(facts_as_json) == "") {
    if (!should_fatal) {
      return false;
    }
  }
  return true;
}

FactsResult extract_facts_from_decls_(options const& opts, std::string const& source_text) {
  char const* filename{opts.filename.c_str()};

  int32_t flags = getDeclOpts();

  ::rust::Box<DeclParserOptions>  decl_opts =
    hackc_create_direct_decl_parse_options(flags, auto_namespace_map);
  DeclResult result = hackc_direct_decl_parse(*decl_opts, filename, source_text);

  return hackc_decls_to_facts_cpp_ffi(flags, result, source_text);
}

void extract_facts_from_decls(options const& opts) {
  char const* filename{opts.filename.c_str()};
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  FactsResult facts{extract_facts_from_decls_(opts, source_text)};
  ::rust::String facts_as_json{hackc_facts_to_json_cpp_ffi(facts, source_text)};
  std::string output = std::string(facts_as_json);
  daemon_compatible_print(opts, output);
}

bool test_extract_facts_from_decls(options const& opts, std::string const& source_text, bool should_fatal) {
  FactsResult facts{extract_facts_from_decls_(opts, source_text.c_str())};
  ::rust::String facts_as_json{hackc_facts_to_json_cpp_ffi(facts, source_text)};
  if (std::string(facts_as_json) == "") {
    if (!should_fatal) {
      return false;
    }
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////////////
// Mock DeclProvider

// c.f. `enum ExternalDeclProviderResult` in 'hhbc/external_decl_provider/lib.rs'.
struct DeclProviderResult {
    enum class Tag {
      Missing,
      Decls,
      Bytes,
    };
    struct DeclProviderDecls_Body {
      Decls const* _0;
    };
    struct DeclProviderBytes_Body {
      Bytes const* _0;
    };
    Tag tag;
    union {
      DeclProviderDecls_Body decl_provider_decls_result;
      DeclProviderBytes_Body decl_provider_bytes_result;
    };
};

struct DeclProvider {
  virtual DeclProviderResult getDecl(int, char const*) const {
    return DeclProviderResult{DeclProviderResult::Tag::Missing, {}};
  }
};

namespace {
// A slightly more useful mock decl provider. Rather than never providing any decls
// this method provides decls that exist in the current file that is being compiled
struct SingleFileMockDeclProvider : DeclProvider {
  struct access {
    int kind;
    std::string symbol;
    bool found;
  };
  std::vector<access> requests;

  DeclResult* single_file_decl;
  bool use_serialized_decls;

  explicit SingleFileMockDeclProvider(DeclResult* decls, bool use_serialized_decls): single_file_decl{decls}, use_serialized_decls{use_serialized_decls} {}

  DeclProviderResult getDecl(int kind, char const* symbol) const override {
    auto& self = const_cast<SingleFileMockDeclProvider&>(*this);
    if (hackc_decl_exists(*single_file_decl->decls, kind, symbol)) {
      self.requests.push_back({kind, symbol, true});
      if (self.use_serialized_decls) {
        DeclProviderResult r;
        r.tag = DeclProviderResult::Tag::Bytes;
        r.decl_provider_bytes_result = DeclProviderResult::DeclProviderBytes_Body{&(*(single_file_decl->serialized))};
        return r;
      } else {
        DeclProviderResult r;
        r.tag = DeclProviderResult::Tag::Decls;
        r.decl_provider_decls_result = DeclProviderResult::DeclProviderDecls_Body{&(*(single_file_decl->decls))};
        return r;
      }
    } else {
      self.requests.push_back({kind, symbol, false});
      return DeclProviderResult{DeclProviderResult::Tag::Missing, {}};
    }
  }
};

std::ostream& operator<<(std::ostream& os, SingleFileMockDeclProvider const& p) {
  for(auto const& access: p.requests) {
    os << access.symbol << ", " << access.kind << ", " << std::boolalpha << access.found << std::endl;
  }
  return os;
}

extern "C" {
  DeclProviderResult decl_provider_get_decl(void* provider, int kind, char const* symbol) {
    return ((DeclProvider*)provider)->getDecl(kind, symbol);
  }
}//extern "C"

}//namespace<anonymous>

///////////////////////////////////////////////////////////////////////////////////

std::string compile_from_text_(
  options const& opts,
  DeclProvider const* decl_provider,
  std::string const& source_text
) {
  auto [parser_flags, hhbc_flags] = parser_hhbc_flags_from_options(opts);

  NativeEnv env{
    reinterpret_cast<std::uintptr_t>(decl_provider)
    , reinterpret_cast<std::uintptr_t>(&decl_provider_get_decl)
    , opts.filename.c_str()
    , auto_namespace_map
    , "" // include roots
    , opts.emit_class_pointers // emit_class_pointers
    , opts.check_int_overflow // check_int_overflow
    , hhbc_flags
    , parser_flags
    , flags_from_options(opts)
  };

  ::rust::Vec<std::uint8_t> hhas{hackc_compile_from_text_cpp_ffi(env, source_text)};

  return std::string(hhas.begin(), hhas.end());
}

void compile_from_text(options const& opts) {
  char const* filename{opts.filename.c_str()};
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  DeclProvider decl_provider;
  std::string codes = compile_from_text_(opts, &decl_provider, source_text);
  daemon_compatible_print(opts, codes);
}

bool test_compile_ffi(
  options const& opts,
  std::string const& source_text,
  bool /* should_fatal */
) {
  DeclProvider decl_provider;
  std::string codes = compile_from_text_(opts, &decl_provider, source_text);
  return !codes.empty();
}

void compile_from_text_with_same_file_decl(options const& opts) {
  char const* filename{opts.filename.c_str()};

  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  int32_t flags = getDeclOpts();

  ::rust::Box<DeclParserOptions> decl_opts = hackc_create_direct_decl_parse_options(flags, auto_namespace_map);
  DeclResult result = hackc_direct_decl_parse(*decl_opts, filename, source_text);

  SingleFileMockDeclProvider decl_provider{ &result, opts.use_serialized_decls };

  std::string codes = compile_from_text_(opts, &decl_provider, source_text);

  if (opts.log_decls_requested) {
    // TODO: Make this compatible with daemon mode
    std::cout << decl_provider << std::endl;
  } else {
    daemon_compatible_print(opts, codes);
  }
}

::rust::Box<HhasProgramWrapper> compile_hhas_from_text_(
  options const& opts,
  std::string const& source_text
) {
  DeclProvider decl_provider;

  auto [parser_flags, hhbc_flags] = parser_hhbc_flags_from_options(opts);

  NativeEnv env{
    reinterpret_cast<std::uintptr_t>(&decl_provider)
    , reinterpret_cast<std::uintptr_t>(&decl_provider_get_decl)
    , opts.filename
    , auto_namespace_map
    , "" // include roots
    , opts.emit_class_pointers // emit_class_pointers
    , opts.check_int_overflow // check_int_overflow
    , hhbc_flags // hhbc_flags
    , parser_flags // parser_flags
    , flags_from_options(opts)
  };

  return hackc_compile_hhas_from_text_cpp_ffi(env, source_text);
}

void print_hhas_to_string(
  options const& opts,
  HhasProgramWrapper const& prog
) {
  DeclProvider decl_provider;

  auto [parser_flags, hhbc_flags] = parser_hhbc_flags_from_options(opts);

  NativeEnv env{
    reinterpret_cast<std::uintptr_t>(&decl_provider)
    , reinterpret_cast<std::uintptr_t>(&decl_provider_get_decl)
    , opts.filename
    , auto_namespace_map
    , "" // include roots
    , opts.emit_class_pointers // emit_class_pointers
    , opts.check_int_overflow // check_int_overflow
    , hhbc_flags // hhbc_flags
    , parser_flags // parser_flags
    , flags_from_options(opts)
  };

  ::rust::Vec<std::uint8_t> hhbc{hackc_hhas_to_string_cpp_ffi(env, prog)};
  std::cout << std::string(hhbc.begin(), hhbc.end()) << std::endl;
}

void compile_hhas_from_text(options const& opts) {
  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(opts.filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string source_text{os.str()};

  ::rust::Box<HhasProgramWrapper> codes = compile_hhas_from_text_(opts, source_text);

  print_hhas_to_string(opts, *codes);
  report(codes);

  return;
}

bool test_compile_hhas_ffi(
  options const& opts,
  std::string const& source_text,
  bool should_fatal
) {
  ::rust::Box<HhasProgramWrapper> prog_wrapped = compile_hhas_from_text_(opts, source_text);
  HhasProgram const* prog = hhasProgramRaw(prog_wrapped);

  bool fatal = (prog->fatal.tag == Maybe<Triple<FatalOp, HhasPos, Str>>::Tag::Just);
  if (fatal && !should_fatal) {
    std::cerr << "Program fataled even though it should not\n";
    return false;
  } else if (!fatal && should_fatal) {
    std::cerr << "Program should fatal but didn't\n";
    return false;
  } else {
    print_hhas_to_string(opts, *prog_wrapped);
    report(prog_wrapped);
  }

  return true;
}

void verify_decls_ffi(options const& opts) {
  char const* filename{opts.filename.c_str()};

  std::ifstream ifs;
  ifs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  ifs.open(filename);
  std::ostringstream os;
  os << ifs.rdbuf();
  std::string text{os.str()};

  int32_t flags = getDeclOpts();

  ::rust::Box<DeclParserOptions> decl_opts = hackc_create_direct_decl_parse_options(flags, auto_namespace_map);
  DeclResult result = hackc_direct_decl_parse(*decl_opts, filename, text);

  hackc_print_decls(*result.decls);
  std::cout << "Decl-hash: " << result.hash << "\n";
  hackc_print_serialized_size(*result.serialized);

  bool check_deserialization = hackc_verify_deserialization(
    *result.serialized,
    *result.decls
  );

  std::cout << "Decl-deserialization test: " << (check_deserialization ? "passed" : "failed") << "\n";
}

bool test_decls_ffi(
  options const& opts,
  std::string const& source_text,
  bool /* should_fatal */
) {
  int32_t flags = getDeclOpts();

  ::rust::Box<DeclParserOptions> decl_opts = hackc_create_direct_decl_parse_options(flags, auto_namespace_map);
  DeclResult result = hackc_direct_decl_parse(*decl_opts, opts.filename, source_text);

  bool check_deserialization = hackc_verify_deserialization(
    *result.serialized,
    *result.decls
  );

  if (!check_deserialization) {
    return false;
  }

  return true;
}

void test(options const& opts) {
  std::vector<std::pair<std::string, bool>> tests = {
    {"", false},
    {R"(
      <<__EntryPoint>>
      function main(): void {
        echo "Hello World!\n";
      }
    )", false},
    {R"(
      function test(): void {
        $y = 4;
        $_ = (int $x = $y): void ==> {};
      }
    )", false},
    {R"(
      namespace consts {
        const x = "x";
      }
      use const consts\x as A;
    )", false},
    {R"(
      function f() {
        g($a);
        h() -> d;
        f() + 1;
      }
    )", false},
    {R"(
      function f(
        shape('x' => ?int) $s,
      ): int {
        return Shapes::idx($s, 'x', 12);
      }
    )", false},
    {R"(
      <<__EntryPoint>>
      function fatal_prog() {
        var($x);
      }
    )", true},
  };

  // [failed_tests, failed_ffis]
  std::pair<std::set<int>, std::set<std::string>> failed = {
    std::set<int>(), std::set<std::string>()
  };

  std::cout << "Start testing FFIs..." << std::endl;
  for (int i = 0; i < tests.size(); i++) {
    std::cout << "\n--------------------------------------------\nRunning test " << i << "\n--------------------------------------------\n";
    std::cout << "Source text:" << tests[i].first << "\n";
    failed = update_test_results(
      opts, tests, i, failed, test_parse, "parse");
    failed = update_test_results(
      opts, tests, i, failed, test_extract_facts_as_json, "extract_facts_as_json");
    failed = update_test_results(
      opts, tests, i, failed, test_extract_facts, "extract_facts");
    failed = update_test_results(
      opts, tests, i, failed, test_extract_facts_from_decls, "test_extract_facts_from_decls");
    failed = update_test_results(
      opts, tests, i, failed, test_compile_ffi, "compile_ffi");
    failed = update_test_results(
      opts, tests, i, failed, test_compile_hhas_ffi, "compile_hhas_ffi");
    failed = update_test_results(
      opts, tests, i, failed, test_decls_ffi, "decls_ffi");
  }

  std::set<int> failed_tests = failed.first;
  std::set<std::string> failed_ffis = failed.second;

  std::cout << "Finished all tests" << "\n--------------------------------------------\n";
  if (failed_tests.size() == 0) {
    std::cout << "Passed all tests!" << std::endl;
  } else {
    std::cout << "The following FFI(s) failed:\n";
    char const* sep = "";
    for (std::string const& failed_ffi: failed_ffis) {
      std::cout << sep << failed_ffi;
      sep = ", ";
    }
    std::cout << "\n" << std::endl;

    std::cout << "The following test(s) failed:\n";
    sep = "";
    for (int const& failed_test: failed_tests) {
      std::cout << sep << failed_test;
      sep = ", ";
    }
    std::cout << "\n" << std::endl;

    std::exit(failed_tests.size());
  }
}

// These modes of compilation can also be run in daemon mode.
// Incompatible modes are dispatched to in main.
int dispatch(options const& opts) {
  try {
    if (opts.parse) {
      parse(opts);
    }
    else if (opts.extract_facts_as_json) {
      extract_facts_as_json(opts);
    }
    else if (opts.extract_facts) {
      extract_facts(opts);
    }
    else if (opts.extract_facts_from_decls) {
      extract_facts_from_decls(opts);
    }
    else if (opts.test_compile_with_decls) {
      compile_from_text_with_same_file_decl(opts);
    }
    else {
      compile_from_text(opts);
    }
  }
  catch(std::ifstream::failure&) {
    std::cerr << "Unable to open \"" << opts.filename << "\" for reading" << std::endl;
    return 1;
  }
  catch(std::exception const& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch(...) {
    std::cerr << "Unhandled exception" << std::endl;
    return 1;
  }
  return 0;
}

// In daemon mode, hh_single_compile_cpp blocks waiting for a filename on stdin
// Then, using the originally invoked options, dispatches that file to be compiled
void daemon_mode(options& opts) {
  std::string file;
  while (true) {
    std::getline(std::cin, file);
    opts.filename = file;
    if (dispatch(opts)) {
      return;
    }
  }
}

int main(int argc,  char const* argv[]) {
  options opts = get_opts(argc, argv);

  try {
    if (opts.test) {
      test(opts);
    }
    else if (opts.verify_decls_ffi) {
      verify_decls_ffi(opts);
    }
    else if (opts.compile_and_print_hhas) {
      compile_hhas_from_text(opts);
    }
    else if (opts.daemon) {
      daemon_mode(opts);
    }
    else {
      return dispatch(opts);
    }
  }
  catch(std::ifstream::failure&) {
    std::cerr << "Unable to open \"" << opts.filename << "\" for reading" << std::endl;
    return 1;
  }
  catch(std::exception const& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  catch(...) {
    std::cerr << "Unhandled exception" << std::endl;
    return 1;
  }

  return 0;
}
