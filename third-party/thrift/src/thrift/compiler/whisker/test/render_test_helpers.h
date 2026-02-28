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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

  class custom_array : public array {
   public:
    explicit custom_array(array::raw values) : values_(std::move(values)) {}

    static object make(array::raw values) {
      return object(std::make_shared<custom_array>(std::move(values)));
    }

    std::size_t size() const override { return values_.size(); }
    object at(std::size_t index) const override { return values_.at(index); }

   private:
    array::raw values_;
  };

  class custom_map : public map {
   public:
    explicit custom_map(map::raw values) : values_(std::move(values)) {}

    static object make(map::raw values) {
      return object(std::make_shared<custom_map>(std::move(values)));
    }

    std::optional<object> lookup_property(std::string_view id) const override {
      if (auto value = values_.find(id); value != values_.end()) {
        return value->second;
      }
      return std::nullopt;
    }

    std::optional<std::set<std::string>> keys() const override {
      std::set<std::string> keys;
      for (const auto& [key, _] : values_) {
        keys.insert(key);
      }
      return keys;
    }

   private:
    map::raw values_;
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
    resolve_import_result resolve_import(
        std::string_view path,
        source_location,
        diagnostics_engine& diags) override {
      if (auto cached = cached_asts_.find(path); cached != cached_asts_.end()) {
        if (!cached->second.has_value()) {
          return unexpected(parsing_error());
        }
        return &cached->second.value();
      }

      auto source = src_manager_.get_file(path);
      if (!source.has_value()) {
        return nullptr;
      }
      auto ast = whisker::parse(*source, diags);
      auto [result, inserted] =
          cached_asts_.insert({std::string(path), std::move(ast)});
      assert(inserted);
      if (!result->second.has_value()) {
        return unexpected(parsing_error());
      }
      return &result->second.value();
    }

    source_manager& src_manager_;
    std::map<std::string, std::optional<ast::root>, std::less<>> cached_asts_;
  };

  // Render options are "sticky" for each test case, across multiple render(...)
  // calls render within.
  struct render_test_options {
    std::optional<diagnostic_level> strict_boolean_conditional;
    std::optional<diagnostic_level> strict_printable_types;
    std::optional<diagnostic_level> strict_undefined_variables;
    // Backtraces are disabled by default since they add generally add noise.
    bool show_source_backtrace_on_failure = false;
    std::vector<std::function<void(map::raw&)>> libraries_to_load;

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
  void use_library(std::function<void(map::raw&)> library_loader) {
    render_test_options_.libraries_to_load.push_back(std::move(library_loader));
  }

  struct sources_by_path {
    /**
     * Mapping of source (macro or module) paths (delimited by '/') to its
     * source code.
     */
    std::unordered_map<std::string, std::string> value;
  };

  static sources_by_path sources(
      std::initializer_list<std::pair<const std::string, std::string>>
          entries) {
    return {std::unordered_map<std::string, std::string>{entries}};
  }

  struct globals_by_name {
    /**
     * Mapping of name in the global scope to whisker::object.
     */
    map::raw value;
  };

  static globals_by_name globals(
      std::initializer_list<std::pair<const std::string, object>> entries) {
    return {map::raw{entries}};
  }

  std::optional<std::string> render(
      const std::string& source,
      const object& root_context,
      const sources_by_path& sources = {},
      globals_by_name globals = {}) {
    auto& current = last_render_.emplace();

    auto src = current.src_manager.add_virtual_file(path_to_file, source);
    auto ast = parse(src, current.diags);
    if (!ast.has_value()) {
      return std::nullopt;
    }

    render_options options;
    if (!sources.value.empty()) {
      auto source_resolver =
          std::make_unique<in_memory_source_resolver>(current.src_manager);
      for (const auto& [name, content] : sources.value) {
        current.src_manager.add_virtual_file(name, content);
      }
      options.src_resolver = std::move(source_resolver);
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
