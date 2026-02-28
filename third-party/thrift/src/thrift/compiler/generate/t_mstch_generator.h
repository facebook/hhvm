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

#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_set>

#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/mstch_compat.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/render.h>

#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_whisker_generator.h>

namespace apache::thrift::compiler {

class t_mstch_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  using compiler_options_map = t_whisker_generator::compiler_options_map;
  void process_options(
      const std::map<std::string, std::string>& options) override {
    t_whisker_generator::process_options(options);
    mstch_context_.options = &compiler_options();
    mstch_context_.prototypes = render_state().prototypes.get();
    mstch_context_.whisker_context = &context();
  }

 protected:
  struct whisker_options {
    /**
     * Whisker, by default, enforces that variables are defined when they are
     * interpolated. mstch on the other hand silently interpolates the empty
     * string. Our templates have come to rely on this behavior.
     *
     * This set specifies names for which Whisker should also silently
     * interpolate empty string. This makes Whisker's rendering backwards
     * compatible with existing Mustache templates.
     */
    std::unordered_set<std::string> allowed_undefined_variables;
  };
  /**
   * Customization point for Whisker's rendering options.
   */
  virtual whisker_options render_options() const { return {}; }

  whisker::source_manager template_source_manager() const final;

  whisker::map::raw globals(prototype_database&) const override;

  using t_whisker_generator::render;
  /**
   * Render the mstch template with name `template_name` in the given context.
   */
  std::string render(
      const std::string& template_name, const mstch::node& context);

  /**
   * Write an output file with the given contents to a path
   * under the output directory.
   */
  void write_output(
      const std::filesystem::path& path, const std::string& data) {
    write_to_file(path, data);
  }

  /**
   * Render the mstch template with name `template_name` in the given context
   * and write to a path under the output directory. Same as calling `render`
   * and `write_output` in succession.
   */
  void render_to_file(
      const mstch::map& context,
      const std::string& template_name,
      const std::filesystem::path& path);

  /**
   * Render the mstch template with name `template_name` with the given context.
   * This writes to a path under the output directory.
   */
  void render_to_file(
      const std::shared_ptr<mstch_base> context,
      const std::string& template_name,
      const std::filesystem::path& path) {
    write_output(path, render(template_name, context));
  }

  // DEPRECATED: use has_compiler_option() instead
  bool has_option(const std::string& option) const;
  // DEPRECATED: use get_compiler_option() instead
  std::optional<std::string> get_option(const std::string& option) const;
  // DEPRECATED: use compiler_options() instead
  const compiler_options_map& options() const { return compiler_options(); }

 private:
  strictness_options strictness() const final;

 protected:
  mstch_context mstch_context_;

  const std::shared_ptr<mstch_base>& cached_program(const t_program* program);
};

} // namespace apache::thrift::compiler
