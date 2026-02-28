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

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/expected.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/source_location.h>

#include <iosfwd>

namespace whisker {

/**
 * A resolver of parsed AST by name. This is used to resolve import statements:
 *   {{#import "path/to/file" as foo}}
 *
 * Macros are also resolved using this class:
 *   {{> path/to/macro }}.
 */
class source_resolver {
 public:
  virtual ~source_resolver() noexcept = default;

  struct parsing_error {};
  using resolve_import_result =
      whisker::expected<const ast::root*, parsing_error>;
  /**
   * Resolves a Whisker source that is the target of an import statement:
   *   {{#import "path/to/file" as foo}}
   *
   * The returned object must be kept alive by the implementation for the
   * duration of rendering.
   *
   * For multiple calls to the same path, or if two paths refer to the same
   * underlying file, the same object (pointer) should be returned. This is used
   * by the renderer for caching.
   *
   * If the path is not known then this function returns nullptr.
   *
   * If parsing fails, then parsing_error is returned. Any error or warning
   * messages should be attached to the provided diagnostics_engine.
   */
  virtual resolve_import_result resolve_import(
      std::string_view path,
      source_location include_from,
      diagnostics_engine&) = 0;

  using resolve_macro_result = resolve_import_result;
  /**
   * Given a lookup path (corresponding to ast::macro_lookup), this function
   * tries to resolve it to a parsed AST node.
   *
   * The returned object must satisfy the same requirements as
   * resolve_import(...).
   *
   * The default implementation delegates to resolve_import(...) with the
   * provided path joined using "/" as a delimiter.
   */
  virtual resolve_macro_result resolve_macro(
      const std::vector<std::string>& path,
      source_location include_from,
      diagnostics_engine&);
};

struct render_options {
  /**
   * If set to diagnostic_level::error, then rendering will fail if a
   * conditionally rendered block's (e.g. section blocks) condition is not
   * *exactly* a whisker::boolean. This behavior is the recommended and the
   * default but differs from Mustache and Handlebars.
   *
   * If set to diagnostic_level::warning or below, then rendering will
   * continue in "non-strict" mode and a diagnostic will be reported.
   *
   * In non-strict mode, all value types are allowed and will be coerced to a
   * boolean by the following rules:
   *   - The following values are considered "falsy":
   *     - false (duh!)
   *     - null
   *     - 0 (i64)
   *     - ±0.0, NaN (f64)
   *     - empty string
   *     - empty array
   *   - All other values are considered "truthy".
   *
   * This *mostly* matches "falsy" values in JavaScript for the subset of its
   * types that have an analog in Whisker:
   *   https://developer.mozilla.org/en-US/docs/Glossary/Falsy
   *
   * Empty arrays are not "falsy" in JavaScript, but they are in Whisker. This
   * is for conformance with the Mustache spec:
   *   https://github.com/mustache/spec/blob/v1.4.2/specs/inverted.yml#L67-L71
   *
   * Note that arrays and maps are also supported values in section blocks but
   * they perform iteration or unpacking respectively. Therefore, they are not
   * subject to this option. Inverted section-blocks, however, are considered
   * conditionals.
   */
  diagnostic_level strict_boolean_conditional = diagnostic_level::error;

  /**
   * If set to diagnostic_level::error, then rendering will fail if a
   * {{variable}} is used while rendering that is not a strictly "printable"
   * type. This behavior is the recommended and the default but differs from
   * Mustache and Handlebars.
   *
   * The following types are considered printable:
   *   - i64
   *   - string
   *
   * If set to diagnostic_level::warning or below, then rendering will
   * continue in "non-strict" mode and a diagnostic will be reported.
   *
   * In non-strict mode, in addition to the printable types above, the
   * following types are also allowed:
   *   - f64 (fmt::format("{}", ...))
   *   - bool (as "true" or "false")
   *   - null (as <empty-string>)
   *
   * The following types are never printable:
   *   - array
   *   - map
   *   - native_function
   *   - native_handle
   */
  diagnostic_level strict_printable_types = diagnostic_level::error;

  /**
   * According to Mustache:
   *     By default a variable "miss" returns an empty string. This can usually
   *     be configured in your Mustache library. The Ruby version of Mustache
   *     supports raising an exception in this situation, for instance.
   *
   * If set to diagnostic_level::error, then rendering will fail if a
   * {{variable}} is used but its value is not found in the current scope.
   *
   * If set to diagnostic_level::warning or below, then rendering will continue
   * in "non-strict" mode and a diagnostic will be reported.
   *
   * In non-strict mode, the value of a missing variable resolves to
   * whisker::null. This value, in practice, acts similar to the empty string:
   *   - whisker::null renders as an empty string (see strict_printable_types).
   *   - whisker::null is falsy (see strict_boolean_conditional).
   */
  diagnostic_level strict_undefined_variables = diagnostic_level::error;

  /**
   * The diagnostic level at which the trace of source locations of macro
   * applications will be printed in case there is an error during rendering.
   *
   * This is useful for debugging and is set to the highest level by default.
   */
  diagnostic_level show_source_backtrace_on_failure = diagnostic_level::error;

  /**
   * An object that can be used to resolve macros within Whisker templates, like
   * "{{> path/to/macro }}".
   *
   * If this is not set, then all macro applications will fail.
   */
  std::shared_ptr<source_resolver> src_resolver = nullptr;

  /**
   * A map of identifiers to objects that will be injected as global bindings,
   * before even the root scope. These names will be available in the search
   * path for every lookup.
   */
  map::raw globals;
};

/**
 * Renders the given AST node with the provided context. The output string is
 * written to the provided output stream.
 *
 * Returns true if the render was successful, false otherwise. Errors are
 * attached to the diagnostics_engine provided.
 *
 * In case of failure, the output stream may contain partial output. This
 * output is indeterminate and trying to use it is undefined behavior.
 */
bool render(
    std::ostream& out,
    const ast::root&,
    const object& root_context,
    diagnostics_engine&,
    render_options);

} // namespace whisker
