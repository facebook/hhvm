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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/render.h>

#include <cassert>
#include <functional>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/core.h>
#include <fmt/ranges.h>

namespace whisker {

// Testing harness for rendering and conformance tests
class RenderTest : public testing::Test {
 public:
  static const inline std::string path_to_file = "path/to/test.whisker";

  class empty_native_object : public native_object {};

  class array_like_native_object
      : public native_object,
        public native_object::array_like,
        public std::enable_shared_from_this<array_like_native_object> {
   public:
    explicit array_like_native_object(array values)
        : values_(std::move(values)) {}

    native_object::array_like::ptr as_array_like() const override {
      return shared_from_this();
    }
    std::size_t size() const override { return values_.size(); }
    object::ptr at(std::size_t index) const override {
      return manage_as_static(values_.at(index));
    }

   private:
    array values_;
  };

  class map_like_native_object
      : public native_object,
        public native_object::map_like,
        public std::enable_shared_from_this<map_like_native_object> {
   public:
    explicit map_like_native_object(map values) : values_(std::move(values)) {}

    native_object::map_like::ptr as_map_like() const override {
      return shared_from_this();
    }

    object::ptr lookup_property(std::string_view id) const override {
      if (auto value = values_.find(id); value != values_.end()) {
        return manage_as_static(value->second);
      }
      return nullptr;
    }

    std::optional<std::set<std::string>> keys() const override {
      std::set<std::string> keys;
      for (const auto& [key, _] : values_) {
        keys.insert(key);
      }
      return keys;
    }

   private:
    map values_;
  };

 private:
  struct source_state {
    source_manager src_manager;
    std::vector<diagnostic> diagnostics;
    diagnostics_engine diags;

    source_state()
        : diags(src_manager, [this](diagnostic d) {
            diagnostics.push_back(std::move(d));
          }) {}
    source_state(source_state&&) = delete;
    source_state& operator=(source_state&&) = delete;
  };
  std::optional<source_state> last_render_;

  class in_memory_source_resolver : public source_resolver {
   public:
    explicit in_memory_source_resolver(source_manager& src_manager)
        : src_manager_(src_manager) {}

   private:
    const ast::root* resolve_macro(
        const std::vector<std::string>& macro_path,
        source_location,
        diagnostics_engine& diags) override {
      std::string path = fmt::format("{}", fmt::join(macro_path, "/"));
      if (auto cached = cached_asts_.find(path); cached != cached_asts_.end()) {
        return &cached->second.value();
      }

      auto source = src_manager_.get_file(path);
      if (!source.has_value()) {
        return nullptr;
      }
      auto ast = whisker::parse(*source, diags);
      auto [result, inserted] =
          cached_asts_.insert({std::move(path), std::move(ast)});
      assert(inserted);
      return &result->second.value();
    }

    source_manager& src_manager_;
    std::unordered_map<std::string, std::optional<ast::root>> cached_asts_;
  };

  // Render options are "sticky" for each test case, across multiple render(...)
  // calls render within.
  struct render_test_options {
    std::optional<diagnostic_level> strict_boolean_conditional;
    std::optional<diagnostic_level> strict_printable_types;
    std::optional<diagnostic_level> strict_undefined_variables;
    // Backtraces are disabled by default since they add generally add noise.
    bool show_source_backtrace_on_failure = false;
    std::vector<std::function<void(map&)>> libraries_to_load;

    void apply_to(render_options& options) const {
      if (strict_boolean_conditional.has_value()) {
        options.strict_boolean_conditional = *strict_boolean_conditional;
      }
      if (strict_printable_types.has_value()) {
        options.strict_printable_types = *strict_printable_types;
      }
      if (strict_undefined_variables.has_value()) {
        options.strict_undefined_variables = *strict_undefined_variables;
      }
      options.show_source_backtrace_on_failure =
          show_source_backtrace_on_failure ? diagnostic_level::error
                                           : diagnostic_level::info;
      for (const auto& load : libraries_to_load) {
        load(options.globals);
      }
    }
  };
  render_test_options render_test_options_;

 protected:
  void SetUp() override {
    last_render_ = std::nullopt;
    render_test_options_ = {};
  }

 public:
  void strict_boolean_conditional(diagnostic_level level) {
    render_test_options_.strict_boolean_conditional = level;
  }
  void strict_printable_types(diagnostic_level level) {
    render_test_options_.strict_printable_types = level;
  }
  void strict_undefined_variables(diagnostic_level level) {
    render_test_options_.strict_undefined_variables = level;
  }
  void show_source_backtrace_on_failure(bool enabled) {
    render_test_options_.show_source_backtrace_on_failure = enabled;
  }
  void use_library(std::function<void(map&)> library_loader) {
    render_test_options_.libraries_to_load.push_back(std::move(library_loader));
  }

  struct macros_by_path {
    /**
     * Mapping of macro path (delimited by '/') to the source code.
     */
    std::unordered_map<std::string, std::string> value;
  };

  static macros_by_path macros(
      std::initializer_list<std::pair<const std::string, std::string>>
          entries) {
    return {std::unordered_map<std::string, std::string>{std::move(entries)}};
  }

  struct globals_by_name {
    /**
     * Mapping of name in the global scope to whisker::object.
     */
    map value;
  };

  static globals_by_name globals(
      std::initializer_list<std::pair<const std::string, object>> entries) {
    return {map{std::move(entries)}};
  }

  std::optional<std::string> render(
      const std::string& source,
      const object& root_context,
      const macros_by_path& macros = {},
      globals_by_name globals = {}) {
    auto& current = last_render_.emplace();

    auto src = current.src_manager.add_virtual_file(path_to_file, source);
    auto ast = parse(src, current.diags);
    if (!ast.has_value()) {
      return std::nullopt;
    }

    render_options options;
    if (!macros.value.empty()) {
      auto source_resolver =
          std::make_unique<in_memory_source_resolver>(current.src_manager);
      for (const auto& [name, content] : macros.value) {
        current.src_manager.add_virtual_file(name, content);
      }
      options.source_resolver = std::move(source_resolver);
    }
    options.globals = std::move(globals.value);
    render_test_options_.apply_to(options);

    std::ostringstream out;
    if (whisker::render(
            out, *ast, root_context, current.diags, std::move(options))) {
      return out.str();
    }
    return std::nullopt;
  }

  const std::vector<diagnostic>& diagnostics() const {
    if (last_render_.has_value()) {
      return last_render_.value().diagnostics;
    }
    throw std::logic_error("Unreachable");
  }

  diagnostic error_backtrace(std::string message) {
    return diagnostic(
        diagnostic_level::error,
        fmt::format("The source backtrace is:\n{}", std::move(message)),
        "" /* file */);
  }
};

} // namespace whisker
