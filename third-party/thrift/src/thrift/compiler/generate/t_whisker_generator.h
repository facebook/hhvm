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

#include <thrift/compiler/generate/t_generator.h>
#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/render.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace apache::thrift::compiler {

/**
 * Base class for all template-based code generators using Whisker as the
 * templating engine.
 */
class t_whisker_generator : public t_generator {
 public:
  using t_generator::t_generator;

  /**
   * The subdirectory within templates/ where the Whisker source files reside.
   * This is primarily used to resolve partial application within template
   * files.
   */
  virtual std::string template_prefix() const = 0;

  /**
   * The global context used for whisker rendering. This function can be used,
   * for example, to add globally available helper functions.
   */
  virtual whisker::map globals() const { return {}; }

  // See whisker::render_options
  struct strictness_options {
    bool boolean_conditional = true;
    bool printable_types = true;
    bool undefined_variables = true;
  };
  /**
   * The strictness levels for various levels of diagnostics that Whisker's
   * render provides. The default values are the strictest settings (highly
   * recommended).
   */
  virtual strictness_options strictness() const { return strictness_options(); }

  using templates_map = std::map<std::string, std::string, std::less<>>;
  /**
   * Returns a mapping from Whisker source file paths within templates/ to
   * their text (source) content.
   *
   * This mapping is used to resolve the source code for the root Whisker
   * source file used by this renderer, as well as any partial applications
   * seen during rendering.
   */
  static const templates_map& templates_by_path();

 protected:
  /**
   * Returns the rendered output of a Whisker template source file evaluated
   * with the provided context object.
   *
   * Throws:
   *   - std::runtime_error if rendering fails
   */
  std::string render(
      std::string_view template_file, const whisker::object& context);

  /**
   * Writes some text output at the provided file path, and then records it as
   * a generated output artifact.
   *
   * A newline character is automatically appended to the end of the file if
   * the provided text does not already do so.
   *
   * Throws:
   *   - std::runtime_error if IO fails
   */
  void write_to_file(
      const std::filesystem::path& output_file, std::string_view data);

  /**
   * Writes the rendered output of a Whisker template source file evaluated
   * with the provided context object. The output path is recorded as a
   * generated artifact.
   *
   * Throws:
   *   - std::runtime_error if rendering fails
   */
  void render_to_file(
      const std::filesystem::path& output_file,
      std::string_view template_file,
      const whisker::object& context);

 private:
  struct cached_render_state {
    whisker::diagnostics_engine diagnostic_engine;
    std::shared_ptr<whisker::template_resolver> template_resolver;
    whisker::render_options render_options;
  };
  std::optional<cached_render_state> cached_render_state_;
  cached_render_state& render_state();
};

} // namespace apache::thrift::compiler
