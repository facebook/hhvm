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

#include <string>
#include <fmt/core.h>
#include <folly/FileUtil.h>

#include <thrift/compiler/codemod/file_manager.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace codemod {

void file_manager::apply_replacements() {
  size_t prev_end = 0;
  std::string new_content;

  if (replacements_.empty()) {
    fmt::print("No changes produced\n");
    return;
  }

  // Perform the replacements.
  for (const auto& r : replacements_) {
    // Only apply replacements that are not overlapped with previous one.
    if (prev_end <= r.begin_pos) {
      new_content.append(old_content_, prev_end, r.begin_pos - prev_end);
      new_content += r.new_content;
      prev_end = r.end_pos;
    }
  }

  // Get the last part of the file.
  new_content += old_content_.substr(prev_end);

  // No need for catching nor throwing here since if file doesn't exist
  // the constructor itself will throw an exception.
  folly::writeFile(new_content, program_->path().c_str());

  fmt::print("{} replacements\n", replacements_.size());
}

// NOTE: Rely on automated formatting to fix formatting issues.
void file_manager::remove(const t_annotation& annotation) {
  size_t begin_offset = to_offset(annotation.second.src_range.begin);
  size_t end_offset = to_offset(annotation.second.src_range.end);

  expand_over_whitespaces(begin_offset, end_offset);

  if (end_offset < old_content_.length() && old_content_[end_offset] == ',') {
    end_offset++;
  } else if (
      begin_offset >= 1 && end_offset < old_content_.length() &&
      old_content_[begin_offset - 1] == '(' &&
      old_content_[end_offset] == ')') {
    --begin_offset;
    ++end_offset;
  }

  add({begin_offset, end_offset, ""});
}

// Also expands over trailing commas.
void file_manager::expand_over_whitespaces(
    size_t& begin_offset, size_t& end_offset) const noexcept {
  while (begin_offset >= 1 && isspace(old_content_[begin_offset - 1])) {
    begin_offset--;
  }

  while (
      end_offset < old_content_.length() &&
      (isspace(old_content_[end_offset]) || old_content_[end_offset] == ',')) {
    end_offset++;
  }
}

void file_manager::add_include(std::string include) {
  if (includes_.find(include) == includes_.end()) {
    std::string curr_include = "include \"" + include + "\"\n";
    includes_.insert(std::move(include));
    size_t offset;
    if (!program_->includes().empty()) {
      offset = to_offset(program_->includes().back()->src_range().end) + 1;
    } else {
      offset = program_->definitions().empty()
          ? 0
          : to_offset(program_->definitions().front().src_range().begin);
      curr_include += "\n";
    }
    replacements_.insert({offset, offset, curr_include, true});
  }
}

void file_manager::remove_all_annotations(const t_node& node) {
  size_t begin_offset = SIZE_MAX;
  size_t end_offset = 0;

  for (const auto& annotation : node.annotations()) {
    begin_offset = std::min<size_t>(
        begin_offset, to_offset(annotation.second.src_range.begin));
    end_offset = std::max<size_t>(
        end_offset, to_offset(annotation.second.src_range.end));
  }

  expand_over_whitespaces(begin_offset, end_offset);

  if (begin_offset >= 1 && end_offset < old_content_.length() &&
      old_content_[begin_offset - 1] == '(' &&
      old_content_[end_offset] == ')') {
    --begin_offset;
    ++end_offset;
  }

  replacements_.insert({begin_offset, end_offset, ""});
}

void file_manager::set_namespace(
    const std::string& language, const std::string& ns) {
  size_t offset = get_namespace_offset();
  replacements_.insert(
      {offset, offset, fmt::format("namespace {} \"{}\"\n", language, ns)});
}

/*
 * If namespaces are provided, then the new namespace
 * should be placed alongwith other namespaces.
 * Otherwise the new content should be placed after includes and before
 * definitions.
 */
size_t file_manager::get_namespace_offset() const {
  if (!program_->namespaces().empty()) {
    size_t min_offset = old_content_.length();
    // Finds the offset of first namespace statement in the file.
    for (const auto& [lang, _] : program_->namespaces()) {
      auto ns_stmt = "namespace " + lang;
      auto offset = old_content_.find(ns_stmt, 0);
      if (offset != std::string::npos && min_offset > offset) {
        min_offset = offset;
      }
    }
    if (min_offset != old_content_.length()) {
      return min_offset;
    }
  }
  if (!program_->includes().empty()) {
    return to_offset(program_->includes().back()->src_range().end) + 1;
  }
  if (!program_->definitions().empty()) {
    return to_offset(program_->definitions().front().src_range().begin);
  }
  return 0;
}

void file_manager::remove_namespace(std::string language) {
  if (!program_->namespaces().empty() &&
      program_->namespaces().find(language) != program_->namespaces().end()) {
    // get offsets for the namespace statement
    auto ns_stmt = fmt::format("namespace {} ", language);
    auto begin_offset = old_content_.find(ns_stmt, 0);
    size_t end_offset = old_content_.length();
    if (begin_offset != std::string::npos) {
      end_offset = old_content_.find("\n", begin_offset);
      if (end_offset < old_content_.length()) {
        end_offset++;
      }
    }
    add({begin_offset, end_offset, ""});
  }
}
} // namespace codemod
} // namespace compiler
} // namespace thrift
} // namespace apache
