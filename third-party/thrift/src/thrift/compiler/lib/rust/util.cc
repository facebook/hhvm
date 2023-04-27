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

#include <thrift/compiler/lib/rust/util.h>

#include <filesystem>
#include <fstream>
#include <vector>

namespace apache {
namespace thrift {
namespace compiler {
namespace rust {

rust_crate_map load_crate_map(const std::string& path) {
  // Each line of the file is:
  // thrift_name crate_name
  //
  // As an example of each value, we might have:
  //   - thrift_name: demo
  //     (this is the name by which the dependency is referred to in thrift)
  //   - crate_name: demo_api
  //     (the Rust code will refer to demo_api::types::WhateverType)

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

  // Map from crate_name to list of thrift_names. Most Thrift crates consist of
  // a single *.thrift file but some may have multiple.
  std::map<std::string, std::vector<std::string>> sources;

  std::string line;
  while (std::getline(in, line)) {
    std::istringstream iss(line);
    std::string thrift_name, crate_name;
    iss >> thrift_name >> crate_name;
    sources[crate_name].push_back(thrift_name);
  }

  for (const auto& source : sources) {
    std::string crate_name;
    auto thrift_names = source.second;
    auto multifile = thrift_names.size() > 1;

    // Look out for our own crate in the cratemap. It will require paths that
    // begin with `crate::module` rather than `::depenency::module`.
    if (source.first == "crate") {
      crate_name = "crate";
      ret.multifile_mode = multifile;
    } else {
      crate_name = "::" + mangle(source.first);
    }

    if (multifile) {
      for (const auto& thrift_name : thrift_names) {
        ret.cratemap[thrift_name] = crate_name + "::" + mangle(thrift_name);
      }
    } else if (crate_name != "crate") {
      ret.cratemap[thrift_names[0]] = crate_name;
    }
  }

  return ret;
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

const std::string typedef_rust_name(const t_typedef* t) {
  if (!t->has_annotation("rust.name")) {
    return mangle_type(t->name());
  }
  return t->get_annotation("rust.name");
}

const std::string struct_rust_name(const t_struct* strct) {
  if (!strct->has_annotation("rust.name")) {
    return mangle_type(strct->get_name());
  }
  return strct->get_annotation("rust.name");
}
} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
