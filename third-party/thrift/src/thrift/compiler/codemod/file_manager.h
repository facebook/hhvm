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

#pragma once

#include <stddef.h>
#include <set>
#include <stdexcept>
#include <unordered_set>

#include <folly/FileUtil.h>

#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler::codemod {

struct replacement {
  // TODO(urielrivas): Revisit this whether we want to consider using a
  // source_range directly instead of offset begin_pos, end_pos.
  size_t begin_pos;
  size_t end_pos;
  std::string new_content;
  bool is_include = false;

  constexpr bool operator<(const replacement& replace) const noexcept {
    if (begin_pos != replace.begin_pos) {
      return begin_pos < replace.begin_pos;
    }
    if (end_pos != replace.end_pos) {
      return end_pos < replace.end_pos;
    }
    if (is_include != replace.is_include) {
      // Put includes before other insertions on the first definition (e.g. new
      // structured annotations).
      return is_include;
    }
    return new_content < replace.new_content;
  }
};

/**
 * Class file_manager
 *
 * A manager to control all replacements needed to do for a Thrift file.
 */
class file_manager {
 public:
  file_manager(source_manager& sm, const t_program& program)
      : source_mgr_(sm),
        program_(&program),
        old_content_(sm.get_file(program_->path())->text.data()) {
    for (const auto* include : program_->includes()) {
      includes_.insert(fmt::to_string(include->raw_path()));
    }
  }

  /**
   * Returns the content with all existing replacements applied.
   *
   * This is similar to `apply_replacements()`, but the change does not get
   * written back to file.
   */
  std::string get_new_content() const;

  /**
   * Write all existing replacements back to file.
   *
   * The written content is the same as `get_new_content()`.
   */
  void apply_replacements();

  // Adds a given replacement to the set of replacements.
  void add(replacement replace) { replacements_.insert(std::move(replace)); }

  const std::string_view old_content() const noexcept { return old_content_; }

  // Adds a given include to the set of replacements.
  void add_include(std::string include);

  // Adds a replacement to remove the given element.
  void remove(const t_annotation& annotation);

  // Removes all annotations from a given t_node.
  void remove_all_annotations(const t_node& node);

  /**
   * Converts the source location to offset.
   *
   * Returns the offset of `loc`, i.e. the number of bytes from the beginning of
   * the corresponding source (file) to `loc`.
   */
  size_t to_offset(source_location loc) const {
    source_location start = source_mgr_.get_source_start(loc);
    return source_mgr_.get_text(loc) - source_mgr_.get_text(start);
  }

  /**
   * Returns the range in the source text spanning from the start of the line
   * of `loc` and containing all its leading whitespace (i.e., the
   * "indentation" of that line).
   *
   * Note that the end of the returned range may extend beyond the input `loc`
   * (if the latter is within the leading whitespace), but will never cross
   * lines.
   */
  source_range get_line_leading_whitespace(source_location loc) const;

  void set_namespace(const std::string& language, const std::string& ns);

  size_t get_namespace_offset() const;
  void remove_namespace(std::string language);

 private:
  // Expands backwards begin_offset and forwards end_offset for all whitespaces
  // and trailing commas one_sided = true preserves whitespace after the comma
  // to prevent overlap.
  void expand_over_whitespaces(
      size_t& begin_offset, size_t& end_offset, bool one_sided) const noexcept;

  source_manager source_mgr_;
  const t_program* program_;
  std::string old_content_;
  std::set<replacement> replacements_;
  std::unordered_set<std::string> includes_;
};

} // namespace apache::thrift::compiler::codemod
