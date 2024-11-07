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

#include <thrift/compiler/generate/rust/uri.h>
#include <thrift/compiler/generate/rust/util.h>

#ifdef _WIN32
#include <filesystem>
#endif
#include <fstream>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace apache::thrift::compiler::rust {

namespace {
struct CrateInfo {
  std::vector<std::string> thrift_names;
  std::string label;
};

void split(
    std::vector<std::string>& out,
    std::string_view input,
    std::string_view delimiter) {
  while (true) {
    auto i = input.find(delimiter);
    if (i == std::string_view::npos) {
      out.emplace_back(input);
      return;
    }
    out.emplace_back(input.substr(0, i));
    input = input.substr(i + delimiter.length());
  }
}
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
  const auto& crate_map_path = path;
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

    if (multifile || dependency_path != "crate") {
      for (const auto& thrift_name : thrift_names) {
        auto& cratemap_entry = ret.cratemap[thrift_name];
        if (dependency_path != "crate") {
          split(cratemap_entry.dependency_path, dependency_path, "->");
        }
        cratemap_entry.multifile = multifile;
        cratemap_entry.label = label;
      }
    }
  }

  return ret;
}

rust_crate_index::rust_crate_index(
    const t_program* current_program,
    std::map<std::string, rust_crate> cratemap)
    : cratemap(std::move(cratemap)) {
  // Traverse the entire include tree in depth-first order to resolve relative
  // import paths into absolute paths. Depth-first traversal mimicks the
  // semantics of C++ '#include' with include guards.
  compute_absolute_paths_of_includes(current_program, current_program->path());
}

void rust_crate_index::compute_absolute_paths_of_includes(
    const t_program* program, const std::string& absolute_path) {
  thrift_file_absolute_paths[program] = absolute_path;

  for (auto include : program->includes()) {
    auto dependency = include->get_program();

    if (thrift_file_absolute_paths.find(dependency) !=
        thrift_file_absolute_paths.end()) {
      // Already visited.
      continue;
    }

    auto raw_path = fmt::to_string(include->raw_path());
    if (cratemap.find(raw_path) != cratemap.end()) {
      // Include's path is already an absolute path.
      compute_absolute_paths_of_includes(dependency, raw_path);
      continue;
    }

    std::string::size_type slash = absolute_path.find_last_of("/\\");
    if (slash != std::string::npos) {
      std::string concatenated_path =
          fmt::format("{}/{}", absolute_path.substr(0, slash), raw_path);
      if (cratemap.find(concatenated_path) != cratemap.end()) {
        // Include's path is relative to the Thrift file containing the include.
        compute_absolute_paths_of_includes(dependency, concatenated_path);
        continue;
      }
    }

    // Otherwise not found, but this isn't an error unless something must refer
    // to this program later.
  }
}

const rust_crate* rust_crate_index::find(const t_program* program) const {
  auto absolute_paths_entry = thrift_file_absolute_paths.find(program);
  const std::string& absolute_path =
      absolute_paths_entry == thrift_file_absolute_paths.end()
      ? program->path()
      : absolute_paths_entry->second;

  auto crate = cratemap.find(absolute_path);
  if (crate == cratemap.end()) {
    return nullptr;
  } else {
    return &crate->second;
  }
}

std::vector<const rust_crate*> rust_crate_index::direct_dependencies() const {
  std::vector<const rust_crate*> direct_dependencies;
  std::unordered_set<std::string_view> distinct_names;
  for (const auto& entry : cratemap) {
    const rust_crate& crate = entry.second;
    if (crate.dependency_path.size() != 1) {
      // Not a direct dependency.
      continue;
    }
    std::string_view crate_name = crate.dependency_path[0];
    if (distinct_names.insert(crate_name).second) {
      direct_dependencies.push_back(&crate);
    }
  }
  return direct_dependencies;
}

static bool is_legal_crate_name(const std::string& name) {
  return name == mangle(name) && name != "core" && name != "std";
}

std::string rust_crate::import_name(const t_program* program) const {
  std::string path;

  if (dependency_path.empty()) {
    path = "crate";
  } else {
    for (const auto& dep : dependency_path) {
      path += path.empty() ? "::" : "::__dependencies::";
      path += mangle_crate_name(dep);
    }
  }

  if (multifile) {
    path += "::" + multifile_module_name(program);
  }

  return path;
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

std::string mangle_crate_name(const std::string& name) {
  if (is_legal_crate_name(name)) {
    return name;
  } else {
    return name + "_";
  }
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

} // namespace apache::thrift::compiler::rust
