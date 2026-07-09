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

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include <fmt/core.h>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_program_bundle.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/codemod/codemod.h>
#include <thrift/compiler/codemod/file_manager.h>

using apache::thrift::compiler::kHackConstantsClassUri;
using apache::thrift::compiler::kHackNamePrefixUri;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::t_package;
using apache::thrift::compiler::t_program;
using apache::thrift::compiler::t_program_bundle;
using apache::thrift::compiler::codemod::file_manager;

namespace {

constexpr std::string_view kHackAnnotationInclude =
    "thrift/annotation/hack.thrift";
constexpr std::string_view kHackAnnotationIncludeFbcode =
    "fbcode/thrift/annotation/hack.thrift";

std::string get_php_namespace_prefix(const t_program& program) {
  std::string php_ns = program.get_namespace("php");
  std::replace(php_ns.begin(), php_ns.end(), '.', '_');
  php_ns.push_back('_');
  return php_ns;
}

class MigratePhpNamespaceToHackNamePrefix final {
 public:
  MigratePhpNamespaceToHackNamePrefix(
      source_manager& sm,
      t_program& program,
      bool mangled_services,
      bool remove_only)
      : fm_(sm, program),
        program_(program),
        mangled_services_(mangled_services),
        remove_only_(remove_only) {}

  void run() {
    if (!should_migrate()) {
      return;
    }

    // Avoid inserting new content into the file in remove-only mode
    if (!remove_only_) {
      const std::optional<size_t> maybe_new_include_offset =
          maybe_add_include();
      add_program_annotations(maybe_new_include_offset);
      add_empty_hack_namespace_for_named_package();
    }

    // Regardless, always remove the PHP namespace from the file
    remove_php_namespace();
    fm_.apply_replacements();
  }

 private:
  file_manager fm_;
  const t_program& program_;
  bool mangled_services_;
  bool remove_only_;

  bool should_migrate() const {
    const auto& namespaces = program_.namespaces();
    auto php_it = namespaces.find("php");
    if (php_it == namespaces.end() || php_it->second->ns().empty()) {
      return false;
    }
    if (remove_only_) {
      // In remove-only mode we only care that there is a non-empty
      // `namespace php` to strip. Hack-side state (existing `namespace hack`
      // or `@hack.NamePrefix`) is irrelevant since we never write any.
      return true;
    }
    if (namespaces.count("hack") != 0) {
      return false;
    }
    return !program_.find_structured_annotation_or_null(kHackNamePrefixUri);
  }

  std::optional<size_t> maybe_add_include() {
    if (fm_.has_thrift_include(std::string(kHackAnnotationInclude)) ||
        fm_.has_thrift_include(std::string(kHackAnnotationIncludeFbcode))) {
      return std::nullopt;
    }
    return fm_.add_include(std::string(kHackAnnotationInclude));
  }

  void remove_php_namespace() {
    const auto php_it = program_.namespaces().find("php");
    if (php_it == program_.namespaces().end()) {
      return;
    }

    size_t begin_offset = fm_.to_offset(php_it->second->src_range().begin);
    size_t end_offset = fm_.old_content().find(
        '\n', fm_.to_offset(php_it->second->src_range().end));
    if (end_offset == std::string::npos) {
      end_offset = fm_.old_content().size();
    } else {
      ++end_offset;
    }

    if (begin_offset == 0 && end_offset < fm_.old_content().size() &&
        fm_.old_content()[end_offset] == '\n') {
      ++end_offset;
    }
    fm_.add({begin_offset, end_offset, ""});
  }

  void add_program_annotations(std::optional<size_t> maybe_new_include_offset) {
    const std::string php_prefix = get_php_namespace_prefix(program_);
    std::vector<std::string> annotations;

    if (program_.services().empty() || !mangled_services_) {
      annotations.emplace_back(
          fmt::format("@hack.NamePrefix{{prefix = \"{}\"}}", php_prefix));

      if (!program_.services().empty()) {
        annotations.emplace_back(
            "@hack.LegacyAlwaysIncludeNamePrefixInProcessor");
      }
    } else {
      annotations.emplace_back(
          fmt::format(
              "@hack.NamePrefix{{prefix = \"{}\", apply_to_services = true}}",
              php_prefix));
    }

    if (!program_.structured_definitions().empty()) {
      annotations.emplace_back("@hack.LegacyOmitPrefixInNameString");
    }

    if (!program_.consts().empty() &&
        !program_.find_structured_annotation_or_null(kHackConstantsClassUri)) {
      annotations.emplace_back(
          fmt::format(
              "@hack.ConstantsClass{{name = \"{}CONSTANTS\"}}", php_prefix));
    }

    std::string content;

    for (const auto& annotation : annotations) {
      content = fmt::format("{}{}\n", content, annotation);
    }

    const t_package& package = program_.package();
    if (package.is_explicit()) {
      const size_t offset = package.src_range().begin.offset();
      fm_.add({offset, offset, content});
      return;
    }

    size_t offset = 0;
    if (maybe_new_include_offset.has_value()) {
      offset = *maybe_new_include_offset;
    } else if (!program_.includes().empty()) {
      offset = fm_.to_offset(program_.includes().back()->src_range().end) + 1;
    } else {
      offset = fm_.get_namespace_offset();
    }

    content = content + "package;\n\n";
    if (!maybe_new_include_offset.has_value() && !program_.includes().empty()) {
      content = "\n" + content;
    }
    fm_.add({offset, offset, content});
  }

  void add_empty_hack_namespace_for_named_package() {
    const t_package& package = program_.package();
    if (!package.is_explicit() || package.empty()) {
      return;
    }

    size_t offset = fm_.to_offset(package.src_range().end);
    offset = fm_.old_content().find('\n', offset);
    if (offset == std::string::npos) {
      offset = fm_.old_content().size();
    } else {
      ++offset;
    }
    fm_.add({offset, offset, "\nnamespace hack \"\"\n"});
  }
};

} // namespace

int main(int argc, char** argv) {
  bool mangled_services = false;
  bool remove_only = false;
  std::vector<char*> filtered_argv;
  filtered_argv.reserve(argc);
  filtered_argv.push_back(argv[0]);

  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg == "--mangledsvcs") {
      mangled_services = true;
      continue;
    }
    if (arg == "--remove-only") {
      remove_only = true;
      continue;
    }
    filtered_argv.push_back(argv[i]);
  }

  if (filtered_argv.size() <= 1) {
    fmt::print(
        stderr,
        "Usage: {} [--mangledsvcs] [--remove-only] <thrift-file>\n",
        filtered_argv.front());
    return 1;
  }

  if (remove_only && mangled_services) {
    fmt::print(
        stderr,
        "--remove-only and --mangledsvcs are mutually exclusive: "
        "--remove-only does not emit @hack.NamePrefix, so service "
        "naming is not preserved by this tool in that mode.\n");
    return 1;
  }

  return apache::thrift::compiler::run_codemod(
      static_cast<int>(filtered_argv.size()),
      filtered_argv.data(),
      [mangled_services, remove_only](
          source_manager& sm, t_program_bundle& pb) {
        MigratePhpNamespaceToHackNamePrefix(
            sm, *pb.root_program(), mangled_services, remove_only)
            .run();
      });
}
