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
#include <thrift/compiler/whisker/object.h>

#include <iosfwd>
#include <memory>

namespace whisker {

/**
 * A resolver of parsed AST by name, primarily used for partial applications:
 *   "{{> path/to/partial }}".
 *
 * This class allows partials to be lazily loaded and parsed only when they are
 * used.
 */
class template_resolver {
 public:
  virtual ~template_resolver() noexcept = default;

  /**
   * Given a partial lookup path (corresponding to ast::partial_lookup), this
   * function tries to resolve it to a parsed AST node.
   *
   * If the path is not known, or if parsing fails, then this function returns
   * std::nullopt. Errors or warnings should be attached to the provided
   * diagnostics_engine.
   */
  virtual std::optional<ast::root> resolve(
      const std::vector<std::string>& partial_path, diagnostics_engine&) = 0;
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
   *   - native_object
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
   * An object that can be used to resolve partial application within
   * Whisker templates: "{{> path/to/partial }}".
   *
   * If this is not set, then all partial applications will fail.
   */
  std::shared_ptr<template_resolver> partial_resolver = nullptr;

  /**
   * A map of identifiers to objects that will be injected as global bindings,
   * before even the root scope. These names will be available in the search
   * path for every lookup.
   */
  map globals;
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
