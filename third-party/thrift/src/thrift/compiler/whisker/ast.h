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
  bodies bodies;
};

/**
 * Raw text content that should be emitted unchanged in the rendered output.
 * This is guaranteed to be on one line (i.e. not contain a new line).
 */
struct text {
  source_range loc;
  std::string text;
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
 * A "path" of identifiers that represent a lookup of a variable or partial.
 * Unlike variable_lookup, the self-referencing dot is not a valid lookup_path.
 */
struct lookup_path {
  source_range loc;
  std::vector<identifier> parts;

  std::string as_string(char separator) const;
};

/**
 * A "path" of identifiers that represent a lookup of a variable where each
 * path component is separated by a dot. This is a subset of Mustache's
 * variables:
 *   https://mustache.github.io/mustache.5.html#Variables
 */
struct variable_lookup {
  source_range loc;
  // this_ref is a special case: {{.}} referring to the current object.
  struct this_ref {};
  std::variant<this_ref, lookup_path> path;

  std::string path_string() const;
};

/**
 * A top-level variable within a template body. It is similar to variable_lookup
 * except its source_range includes the surrounding "{{ }}".
 */
struct variable {
  source_range loc;
  variable_lookup lookup;

  std::string path_string() const { return lookup.path_string(); }
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
  bodies bodies;
};

/**
 * A Whisker construct for partially applied templates. This matches Mustache:
 *   https://mustache.github.io/mustache.5.html#Partials
 */
struct partial_apply {
  source_range loc;
  lookup_path path;
};

} // namespace whisker::ast
