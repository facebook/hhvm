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

#include <thrift/compiler/lib/rust/uri.h>
#include <thrift/compiler/lib/rust/util.h>

#ifdef _WIN32
#include <filesystem>
#endif
#include <fstream>
#include <vector>

namespace apache {
namespace thrift {
namespace compiler {
namespace rust {

namespace {
struct CrateInfo {
  std::vector<std::string> thrift_names;
  std::string label;
};
} // namespace

rust_crate_map load_crate_map(const std::string& path) {
  // Each line of the file is:
  // path/to/thrift_name.thrift crate_name //path/to:target-rust
  //
  // The first column is the string by which this Thrift file would be
  // identified in `include` statements within other Thrift sources. The
  // substring between the last '/' and the last '.' is Thrift package name.
  // For example a struct `Struct` within a Thrift file `thrift_name.thrift`
  // would be named as `thrift_name.Struct` in downstream Thrift files.
  //
  // The second column is the Rust crate name. Rust code in downstream targets
  // will refer to `crate_name::Struct`. By default, the Rust crate name is
  // derived from the Thrift package name (sanitized to avoid Rust keywords) but
  // may also be set to something different by a `namespace rust` statement.
  //
  // The third column is a build-system-specific label that identifies how one
  // might build the Rust generated code for this Thrift file. In Buck, this
  // would be the target label of a `thrift_library` target. This column is
  // optional. If present, it is included in various deprecation messages or
  // comments.

  rust_crate_map ret;
#ifdef _WIN32
  // Relative path appended to current working directory can easily exceed
  // MAX_PATH which is 260 chars. Computing absolute path allows to shorten it.
  auto crate_map_path =
      std::filesystem::absolute(std::filesystem::u8path(path));
#else
  auto crate_map_path = path;
#endif
  auto in = std::ifstream(crate_map_path);
  if (!in.is_open()) {
    std::ostringstream error_message;
    error_message << "Can't open crate map: " << path;
    throw std::runtime_error(error_message.str());
  }

  // Map from Rust crate name to list of Thrift sources. Most Thrift libraries
  // consist of a single *.thrift file but some may have multiple.
  std::map<std::string, CrateInfo> sources;

  std::string line;
  while (std::getline(in, line)) {
    std::istringstream iss(line);
    std::string thrift_name, dependency_path, label;
    iss >> thrift_name >> dependency_path >> label;
    sources[dependency_path].label = label;
    sources[dependency_path].thrift_names.push_back(thrift_name);
  }

  for (const auto& source : sources) {
    auto dependency_path = source.first;
    auto label = source.second.label;
    auto thrift_names = source.second.thrift_names;
    auto multifile = thrift_names.size() > 1;

    // Look out for our own crate in the cratemap. It will require paths that
    // begin with `crate::module` rather than `::depenency::module`.
    if (dependency_path == "crate") {
      ret.multifile_mode = multifile;
      ret.label = label;
    }

    if (dependency_path.find("->") != std::string::npos) {
      // TODO(dtolnay) Process transitive dependencies.
      continue;
    }

    if (multifile || dependency_path != "crate") {
      for (const auto& thrift_name : thrift_names) {
        ret.cratemap[thrift_name].name = dependency_path;
        ret.cratemap[thrift_name].multifile = multifile;
        ret.cratemap[thrift_name].label = label;
      }
    }
  }

  return ret;
}

rust_crate_index::rust_crate_index(
    const t_program* current_program,
    std::map<std::string, rust_crate> cratemap)
    : cratemap(std::move(cratemap)) {
  auto current_program_path = current_program->path();
  std::string::size_type slash = current_program_path.find_last_of("/\\");
  if (slash != std::string::npos) {
    directory_of_current_program = current_program_path.substr(0, slash);
  }
}

const rust_crate* rust_crate_index::find(const t_program* program) const {
  auto crate = cratemap.find(program->path());
  if (crate == cratemap.end()) {
    crate = cratemap.find(
        fmt::format("{}/{}", directory_of_current_program, program->path()));
  }
  if (crate == cratemap.end()) {
    return nullptr;
  } else {
    return &crate->second;
  }
}

static bool is_legal_crate_name(const std::string& name) {
  return name == mangle(name) && name != "core" && name != "std";
}

std::string rust_crate::import_name(const t_program* program) const {
  std::string absolute_crate_name;

  if (name == "crate") {
    absolute_crate_name = "crate";
  } else if (is_legal_crate_name(name)) {
    absolute_crate_name = "::" + name;
  } else {
    absolute_crate_name = "::" + name + "_";
  }

  if (multifile) {
    return absolute_crate_name + "::" + multifile_module_name(program);
  } else {
    return absolute_crate_name;
  }
}

std::string mangle(const std::string& name) {
  static const char* raw_identifiable_keywords[] = {
      "abstract", "alignof", "as",      "async",    "await",    "become",
      "box",      "break",   "const",   "continue", "do",       "else",
      "enum",     "extern",  "false",   "final",    "fn",       "for",
      "if",       "impl",    "in",      "let",      "loop",     "macro",
      "match",    "mod",     "move",    "mut",      "offsetof", "override",
      "priv",     "proc",    "pub",     "pure",     "ref",      "return",
      "sizeof",   "static",  "struct",  "trait",    "true",     "type",
      "typeof",   "unsafe",  "unsized", "use",      "virtual",  "where",
      "while",    "yield",
  };

  static const char* keywords_that_participate_in_name_resolution[] = {
      "crate",
      "super",
      "self",
      "Self",
  };

  constexpr const char* keyword_error_message = R"ERROR(
    Found a rust keyword that participates in name resolution.
    Please use the `rust.name` annotation to create an alias for)ERROR";

  for (auto& s : keywords_that_participate_in_name_resolution) {
    if (name == s) {
      std::ostringstream error_message;
      error_message << keyword_error_message << " " << name;
      throw std::runtime_error(error_message.str());
    }
  }

  for (auto& s : raw_identifiable_keywords) {
    if (name == s) {
      return "r#" + name;
    }
  }

  return name;
}

std::string mangle_type(const std::string& name) {
  static const char* primitives[] = {
      "i8",
      "u8",
      "i16",
      "u16",
      "i32",
      "u32",
      "i64",
      "u64",
      "i128",
      "u128",
      "f32",
      "f64",
      "isize",
      "usize",
      "str",
      "bool",
  };

  for (auto s : primitives) {
    if (name == s) {
      return name + '_';
    }
  }

  return mangle(name);
}

// Convert CamelCase to snake_case.
std::string snakecase(const std::string& name) {
  std::ostringstream snake;

  char last = '_';
  for (auto ch : name) {
    if (isupper(ch)) {
      if (last != '_') {
        // Don't insert '_' after an existing one, such as in `Sample_CalcRs`.
        // Also don't put a '_' right at the front.
        snake << '_';
      }
      last = (char)tolower(ch);
    } else {
      last = ch;
    }
    snake << last;
  }

  return snake.str();
}

// Convert snake_case to UpperCamelCase.
std::string camelcase(const std::string& name) {
  std::ostringstream camel;

  size_t i = 0;
  for (; i < name.size() && name[i] == '_'; i++) {
    // Copy same number of leading underscores.
    camel << '_';
  }

  auto underscore = true;
  for (; i < name.size(); i++) {
    if (name[i] == '_') {
      underscore = true;
    } else if (underscore) {
      camel << (char)toupper(name[i]);
      underscore = false;
    } else {
      camel << name[i];
    }
  }

  return camel.str();
}

std::string quote(const std::string& data, bool do_backslash) {
  std::ostringstream quoted;
  quoted << '"';

  for (auto ch : data) {
    if (ch == '\\') {
      quoted << "\\\\";
    } else if (ch == '\t') {
      quoted << '\\' << 't';
    } else if (ch == '\r') {
      quoted << '\\' << 'r';
    } else if (ch == '\n') {
      quoted << '\\' << 'n';
    } else if ((do_backslash && ch == '\\') || ch == '"') {
      quoted << '\\' << ch;
    } else if (ch < '\x7f') {
      quoted << ch;
    } else {
      throw std::runtime_error("Non-ASCII string literal not implemented");
    }
  }

  quoted << '"';
  return quoted.str();
}

std::string get_annotation_property_string(
    const t_const* annotation, const std::string& key) {
  if (annotation) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == key) {
        return item.second->get_string();
      }
    }
  }
  return "";
}

bool get_annotation_property_bool(
    const t_const* annotation, const std::string& key) {
  if (annotation) {
    for (const auto& item : annotation->value()->get_map()) {
      if (item.first->get_string() == key) {
        return item.second->get_bool();
      }
    }
  }
  return false;
}

namespace {

std::string unmangled_rust_name(const t_named* node) {
  if (const t_const* annot =
          node->find_structured_annotation_or_null(kRustNameUri)) {
    return get_annotation_property_string(annot, "name");
  }
  return node->get_annotation("rust.name");
}

} // namespace

std::string type_rust_name(const t_type* t) {
  if (!t->has_annotation("rust.name") &&
      !t->find_structured_annotation_or_null(kRustNameUri)) {
    return mangle_type(t->name());
  }
  return unmangled_rust_name(t);
}

std::string named_rust_name(const t_named* node) {
  if (!node->has_annotation("rust.name") &&
      !node->find_structured_annotation_or_null(kRustNameUri)) {
    return mangle(node->name());
  }
  return unmangled_rust_name(node);
}

std::string multifile_module_name(const t_program* program) {
  const std::string& namespace_rust = program->get_namespace("rust");

  // If source file has `namespace rust cratename.modulename` then modulename.
  auto separator = namespace_rust.find('.');
  if (separator != std::string::npos) {
    return namespace_rust.substr(separator + 1);
  }

  // Otherwise, the module is named after the source file, modulename.thrift.
  return mangle(program->name());
}

} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
