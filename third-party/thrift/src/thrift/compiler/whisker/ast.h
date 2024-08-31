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

#include <thrift/compiler/whisker/source_location.h>

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace whisker::ast {

struct text;
struct newline;
struct comment;
struct section_block;
struct partial_apply;
struct variable;

/**
 * The top-level types of constructs allowed in a Whisker source file.
 */
using body = std::
    variant<text, newline, comment, variable, section_block, partial_apply>;
using bodies = std::vector<body>;

/**
 * The root node of a Whisker AST representing a source file.
 */
struct root {
  source_location loc;
  bodies body_elements;
};

/**
 * Raw text content that should be emitted unchanged in the rendered output.
 * This is guaranteed to be on one line (i.e. not contain a new line).
 */
struct text {
  source_range loc;
  std::string content;
};

/**
 * Raw newline that should be emitted unchanged in the rendered output. One of:
 *   - "\r\n"
 *   - "\n"
 *   - "\r"
 */
struct newline {
  source_range loc;
  std::string text;
};

/**
 * A comment that should be omitted in the rendered output.
 *   https://mustache.github.io/mustache.5.html#Comments
 * Whisker also supports Handlebars' escaped comments:
 *   https://handlebarsjs.com/guide/#template-comments
 */
struct comment {
  source_range loc;
  std::string text;
};

/**
 * A valid Whisker identifier. See whisker::lexer for its definition.
 */
struct identifier {
  source_range loc;
  std::string name;
};

/**
 * A "path" of identifiers that represent a lookup of a variable where each
 * chain component is separated by a dot. This is a subset of Mustache's
 * variables:
 *   https://mustache.github.io/mustache.5.html#Variables
 */
struct variable_lookup {
  source_range loc;
  // this_ref is a special case: {{.}} referring to the current object.
  struct this_ref {};
  std::variant<this_ref, std::vector<identifier>> chain;

  std::string chain_string() const;
};

/**
 * A top-level variable within a template body. It is similar to variable_lookup
 * except its source_range includes the surrounding "{{ }}".
 */
struct variable {
  source_range loc;
  variable_lookup lookup;

  std::string chain_string() const { return lookup.chain_string(); }
};

/**
 * A Whisker construct for conditionals and/or iteration. This matches Mustache:
 *   https://mustache.github.io/mustache.5.html#Sections
 */
struct section_block {
  source_range loc;
  /**
   * {{# ⇒ inverted == true
   * {{^ ⇒ inverted == false
   */
  bool inverted;
  variable_lookup variable;
  bodies body_elements;
};

/*
 * A valid Whisker path component for partial application. See whisker::lexer
 * for its definition.
 */
struct path_component {
  source_range loc;
  std::string value;
};

/**
 * A '/' delimited series of path components representing a POSIX portable file
 * path. This is used for partial applications.
 */
struct partial_lookup {
  source_range loc;
  std::vector<path_component> parts;

  std::string as_string() const;
};

/**
 * A Whisker construct for partially applied templates. This matches Mustache:
 *   https://mustache.github.io/mustache.5.html#Partials
 */
struct partial_apply {
  source_range loc;
  partial_lookup path;
  /**
   * Standalone partial applications exhibit different indentation behavior:
   *   https://github.com/mustache/spec/blob/v1.4.2/specs/partials.yml#L13-L15
   *
   * If this is a standalone partial application, the value is the preceeding
   * whitespace necessary before the partial application interpolation.
   * Otherwise, this is std::nullopt.
   *
   * The contained string is guaranteed to be whitespace only.
   */
  std::optional<std::string> standalone_offset_within_line;

  std::string path_string() const;
};

} // namespace whisker::ast
