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

#include <thrift/shared/detail/string.h>
#include <thrift/shared/tree_printer.h>

#include <boost/algorithm/string/replace.hpp>

#include <sstream>

namespace apache::thrift::tree_printer {

namespace {

std::string generate_indentation(
    std::size_t indent,
    const std::vector<std::size_t>& active,
    bool last_child) {
  constexpr std::size_t num_indent_spaces = 3;
  // Start with all spaces
  std::string out(indent * num_indent_spaces, ' ');

  // Fill in "|--"-like structure for the deepest node
  for (std::size_t i : active) {
    auto offset = out.begin() + (i * num_indent_spaces);
    const bool is_final_indent = i == active.back();
    if (is_final_indent) {
      if (last_child) {
        *offset++ = '\\';
      } else {
        *offset++ = '+';
      }
      // Fill in the path to the data of the node with "--"
      static_assert(num_indent_spaces >= 2);
      for (std::size_t j = 0; j < num_indent_spaces - 2; ++j) {
        *offset++ = '-';
      }
    } else {
      *offset = '|';
    }
  }

  // Let's use Unicode box characters for better readability. Since these
  // characters are multi-byte in UTF-8, we perform replacements as a
  // post-processing step. This allows us to calculate the offsets above in a
  // sane manner.
  boost::algorithm::replace_all(out, "+", "├");
  boost::algorithm::replace_all(out, "-", "─");
  boost::algorithm::replace_all(out, "\\", "╰");
  boost::algorithm::replace_all(out, "|", "│");

  return out;
}

} // namespace

void scope::print_recursively(
    std::ostream& out,
    std::size_t indent,
    std::vector<std::size_t>& active,
    bool last_child) const {
  out << generate_indentation(indent, active, last_child);
  out << data_ << '\n';
  if (last_child && !active.empty()) {
    active.pop_back();
  }

  if (children_.empty()) {
    return;
  }
  active.push_back(indent);
  for (const scope& child : children_) {
    child.print_recursively(
        out, indent + 1, active, &child == &children_.back() /* last_child */);
  }
}

std::ostream& operator<<(std::ostream& out, const scope& self) {
  std::vector<std::size_t> active;
  self.print_recursively(out, 0 /* indent */, active, true /* last_child */);
  return out;
}

std::string to_string(const scope& self) {
  std::ostringstream out;
  out << self;
  return std::move(out).str();
}

std::string escape(std::string_view str) {
  return detail::escape(str);
}

} // namespace apache::thrift::tree_printer
