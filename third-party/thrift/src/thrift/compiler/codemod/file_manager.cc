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
#include <glog/logging.h>
#include <folly/FileUtil.h>

#include <thrift/compiler/codemod/file_manager.h>

namespace apache::thrift::compiler::codemod {

namespace {

// Subset of "standard whitespace" characters (as defined in
// https://en.cppreference.com/w/c/string/byte/isspace) that only cause
// horizontal movement (i.e., indentation)
constexpr const char* kHorizontalWhitespaceChars = " \t";

size_t get_first_character_of_this_line(
    const std::string& content, size_t pos) {
  // NOTE: `pos` may be the trailing newline of this line, so need to start
  // search for preceding newline (if any) *before* `pos`.

  // If there are no previous characters, `pos` is the first character of this
  // line (and of the entire content).
  if (pos == 0) {
    return 0;
  }

  // Otherwise, `pos` is not the first character in the content, look for any
  // preceding newline (marking the end of the previous line).
  const std::string::size_type line_start_offset =
      content.find_last_of('\n', pos - 1);

  if (line_start_offset == std::string::npos) {
    // If no preceding newline is found, `pos` is in the first line of
    // `content`, and therefore the line begins at the start of the string.
    return 0;
  } else {
    // Otherwise, the line starts just past the last found newline.
    return line_start_offset + 1;
  }
}

/**
 * Returns true if `pos` falls inside any comment: line comments (`//`, `#`)
 * or block comments. Block comments do not nest in Thrift.
 */
bool is_inside_comment(const std::string& content, size_t pos) {
  // Scans content from the beginning up to `pos`, tracking whether we are
  // inside a block comment while also detecting line comments.
  bool inside_block = false;
  for (size_t i = 0; i < pos && i < content.size(); ++i) {
    char c = content[i];
    // Detect line comments (only outside block comments).
    if (!inside_block &&
        ((c == '/' && i + 1 < content.size() && content[i + 1] == '/') ||
         c == '#')) {
      size_t newline = content.find('\n', i);
      if (newline == std::string::npos || newline >= pos) {
        // `pos` is on this comment line.
        return true;
      }
      i = newline; // loop increment will advance past '\n'
      continue;
    }
    if (!inside_block && c == '/' && i + 1 < content.size() &&
        content[i + 1] == '*') {
      inside_block = true;
      ++i; // skip '*'
    } else if (
        inside_block && c == '*' && i + 1 < content.size() &&
        content[i + 1] == '/') {
      inside_block = false;
      ++i; // skip '/'
    }
  }
  return inside_block;
}

/**
 * Like `std::string::find()`, but skips matches that appear inside comments
 * (line comments starting with `//` or `#`, or block comments).
 */
size_t find_not_in_comment(
    const std::string& content, const std::string& pattern, size_t start_pos) {
  size_t pos = start_pos;
  while (true) {
    pos = content.find(pattern, pos);
    if (pos == std::string::npos) {
      return std::string::npos;
    }
    if (!is_inside_comment(content, pos)) {
      return pos;
    }
    pos += pattern.size();
  }
}

} // namespace

std::string file_manager::get_new_content() const {
  if (replacements_.empty()) {
    return old_content_;
  }

  // Offset since the beginning of the OLD content string, up to which the old
  // content string has been processed, i.e. content and replacements relevant
  // to the substring `old_content_[0:prev_end]` have been completed (note that
  // prev_end is excluded).
  size_t prev_end = 0;

  std::string new_content;

  // Perform the replacements.
  for (const replacement& r : replacements_) {
    // Ensure replacement does not overlap with previous one.
    CHECK(prev_end <= r.begin_pos)
        << "Invalid codemod: replacement overlaps with end of previous replacement ("
        << prev_end << "):  range [" << r.begin_pos << "," << r.end_pos
        << "), new_content=[" << r.new_content << "]";

    new_content.append(old_content_, prev_end, r.begin_pos - prev_end);
    new_content += r.new_content;
    prev_end = r.end_pos;
  }

  // Get the last part of the file.
  new_content += old_content_.substr(prev_end);
  return new_content;
}

void file_manager::apply_replacements() {
  if (replacements_.empty()) {
    fmt::print("No changes produced\n");
    return;
  }

  // No need for catching nor throwing here since if file doesn't exist
  // the constructor itself will throw an exception.
  folly::writeFile(get_new_content(), program_->path().c_str());

  fmt::print("{} replacements\n", replacements_.size());
}

// NOTE: Rely on automated formatting to fix formatting issues.
void file_manager::remove(const t_annotation& annotation) {
  size_t begin_offset = to_offset(annotation.second.src_range.begin);
  size_t end_offset = to_offset(annotation.second.src_range.end);

  expand_over_whitespaces(begin_offset, end_offset, true /* one_sided */);

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
    size_t& begin_offset, size_t& end_offset, bool one_sided) const noexcept {
  while (begin_offset >= 1 && isspace(old_content_[begin_offset - 1])) {
    begin_offset--;
  }

  while (end_offset < old_content_.length() &&
         isspace(old_content_[end_offset])) {
    end_offset++;
  }
  if (end_offset < old_content_.length() && old_content_[end_offset] == ',') {
    end_offset++;
  }
  if (one_sided) {
    return;
  }
  while (
      end_offset < old_content_.length() &&
      (isspace(old_content_[end_offset]) || old_content_[end_offset] == ',')) {
    end_offset++;
  }
}

std::optional<size_t> file_manager::add_include(std::string thrift_include) {
  if (includes_.contains(thrift_include)) {
    return std::nullopt;
  }

  std::string new_include_directive = "include \"" + thrift_include + "\"\n";
  includes_.insert(std::move(thrift_include));

  const size_t new_include_offset = [&]() -> size_t {
    // If there are includes, add new include after the last one.
    if (!program_->includes().empty()) {
      size_t offset = to_offset(program_->includes().back()->src_range().end);
      // Move to the beginning of the next line to handle any trailing comments
      // on the include line (e.g., include "foo.thrift" // comment)
      size_t newline_pos = old_content_.find('\n', offset);
      if (newline_pos != std::string::npos) {
        return newline_pos + 1;
      }
      // If no newline found, place at end of content
      return old_content_.length();
    }

    // Otherwise, add the include before the first of: package, namespace or
    // definition (if any). In any case, add an extra line return to separate.
    new_include_directive += "\n";

    if (program_->package().is_explicit()) {
      return to_offset(program_->package().src_range().begin);
    }

    if (std::optional<size_t> maybe_first_namespace_offset =
            get_first_namespace_offset()) {
      return maybe_first_namespace_offset.value();
    }

    if (!program_->definitions().empty()) {
      // Add the include before the first definition - but check if there is a
      // doc block before it.
      const t_named& first_definition = program_->definitions().front();
      const source_range first_definition_range = first_definition.has_doc()
          ? first_definition.doc_range()
          : first_definition.src_range();
      return to_offset(first_definition_range.begin);
    }

    return 0;
  }();
  replacements_.insert(
      {.begin_pos = new_include_offset,
       .end_pos = new_include_offset,
       .new_content = new_include_directive,
       .is_include = true});
  return new_include_offset;
}

bool file_manager::has_thrift_include(
    const std::string& thrift_include_path) const {
  return includes_.contains(thrift_include_path);
}

void file_manager::remove_all_annotations(const t_node& node) {
  size_t begin_offset = SIZE_MAX;
  size_t end_offset = 0;

  for (const auto& annotation : node.unstructured_annotations()) {
    begin_offset = std::min<size_t>(
        begin_offset, to_offset(annotation.second.src_range.begin));
    end_offset = std::max<size_t>(
        end_offset, to_offset(annotation.second.src_range.end));
  }

  expand_over_whitespaces(begin_offset, end_offset, false /* one_sided */);

  if (begin_offset >= 1 && end_offset < old_content_.length() &&
      old_content_[begin_offset - 1] == '(' &&
      old_content_[end_offset] == ')') {
    --begin_offset;
    ++end_offset;
  }

  replacements_.insert({begin_offset, end_offset, ""});
}

source_range file_manager::get_line_leading_whitespace(
    source_location loc) const {
  const size_t loc_offset = loc.offset();

  const size_t line_start_offset =
      get_first_character_of_this_line(old_content_, loc_offset);

  std::string::size_type whitespace_end_offset = old_content_.find_first_not_of(
      kHorizontalWhitespaceChars, line_start_offset);
  if (whitespace_end_offset == std::string::npos) {
    whitespace_end_offset = old_content_.size();
  }

  const source_location source_start = source_mgr_.get_source_start(loc);
  return source_range{
      .begin = source_start + line_start_offset,
      .end = source_start + whitespace_end_offset,
  };
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
  if (std::optional<size_t> maybe_first_namespace_offset =
          get_first_namespace_offset()) {
    return maybe_first_namespace_offset.value();
  }
  if (!program_->includes().empty()) {
    return to_offset(program_->includes().back()->src_range().end) + 1;
  }
  if (program_->package().is_explicit()) {
    return to_offset(program_->package().src_range().end) + 1;
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
    size_t begin_offset = find_not_in_comment(old_content_, ns_stmt, 0);
    size_t end_offset = old_content_.length();
    if (begin_offset != std::string::npos) {
      end_offset = old_content_.find('\n', begin_offset);
      if (end_offset < old_content_.length()) {
        end_offset++;
      }
    }
    add({begin_offset, end_offset, ""});
  }
}

std::optional<size_t> file_manager::get_first_namespace_offset() const {
  if (program_->namespaces().empty()) {
    return std::nullopt;
  }

  size_t min_offset = old_content_.length();
  // Finds the offset of first namespace statement in the file.
  for (const auto& [lang, _] : program_->namespaces()) {
    const auto ns_stmt = "namespace " + lang;
    const size_t offset = find_not_in_comment(old_content_, ns_stmt, 0);
    if (offset != std::string::npos && min_offset > offset) {
      min_offset = offset;
    }
  }
  if (min_offset != old_content_.length()) {
    return min_offset;
  } else {
    return std::nullopt;
  }
}

} // namespace apache::thrift::compiler::codemod
