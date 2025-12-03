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
#include <optional>
#include <set>
#include <unordered_set>
#include <fmt/core.h>

#include <folly/FileUtil.h>

#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler::codemod {

/**
 * Captures the intent to replace a given range of "old" Thrift (IDL) source
 * with "new" content.
 *
 * More specifically, the range `[begin_pos, end_pos)` of the old content should
 * be replaced with `new_content`.
 *
 * Instances of this type are orderable. The `is_include` flag is used to ensure
 * that, when comparing replacements with identical ranges, includes are
 * considered "less-than" non-includes.
 */
struct replacement {
  /**
   * Start offset, in bytes since the beginning of the "old" content, of the
   * range affected by this replacement.
   *
   * Assumption(s):
   *   begin_pos >= 0
   *   begin_pos <= end_pos
   *
   * Bytes in old_content whose offset are less than begin_pos are not affected
   * by this replacement.
   *
   * If `begin_pos == end_pos`, then `new_content` is simply inserted at offset
   * `begin_pos`. Otherwise, the `old_content` bytes with offset
   * [`begin_pos`, `end_pos`) are replaced with `new_content`.
   */
  size_t begin_pos;

  /**
   * End offset, in bytes since the beginning of the "old" content, of the
   * range affected by this replacement.
   *
   * Assumption(s):
   *   end_pos >= begin_pos
   *   end_pos <= old_content.size()
   *
   * See: `begin_pos`
   *
   */
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
        old_source_view_(sm.get_file(program_->path()).value()),
        old_content_(old_source_view_.text.data()) {
    for (const t_include* include : program_->includes()) {
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

  /**
   * Adds a given replacement to the set of replacements.
   *
   * The range of the given replacement SHOULD NOT overlap with any existing
   * replacement.
   */
  void add(replacement replace) { replacements_.insert(std::move(replace)); }

  const std::string_view old_content() const noexcept { return old_content_; }

  /**
   * Adds a given include to the set of replacements.
   *
   * If an include with the given path already exists, this is a no-op.
   * Otherwise, a new replacement is added, which whill insert a new `include`
   * directive at the following location:
   * - if the original program contains includes: immediately after the last
   *   include, or
   * - if the original program contains any definition: before the first
   *   definition, or
   * - at the beginning of the file.
   *
   * @param thrift_include Path of the (.thrift) include to add, if not already
   * present.
   */
  void add_include(std::string thrift_path);

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

  /**
   * Returns an offset (relative to old_content) suitable for the insertion of a
   * new `namespace` directive.
   *
   * The returned offset corresponds to:
   * 1. if program has any `namespace`: the first character of the first one, or
   * 2. if program has includes: the character immediately following the last
   *    include, or
   * 3. if program has a `package` directive: the character imemdiately
   *    following it, or
   * 4. if program has any definition: the first character of the first
   *    definition.
   *
   * The order above should provide the following Thrift IDL structure:
   * <includes...>
   * package
   * <namespaces...>
   * <definitions...>
   */
  size_t get_namespace_offset() const;

  void remove_namespace(std::string language);

 private:
  // Expands backwards begin_offset and forwards end_offset for all whitespaces
  // and trailing commas one_sided = true preserves whitespace after the comma
  // to prevent overlap.
  void expand_over_whitespaces(
      size_t& begin_offset, size_t& end_offset, bool one_sided) const noexcept;

  /**
   * Returns the offset (in old_content_) of the first character of the first
   * `namespace` directive - or nullopt if no such directive is found.
   */
  std::optional<size_t> get_first_namespace_offset() const;

  source_manager& source_mgr_;
  const t_program* program_;
  source_view old_source_view_;
  std::string old_content_;
  std::set<replacement> replacements_;
  std::unordered_set<std::string> includes_;
};

} // namespace apache::thrift::compiler::codemod
