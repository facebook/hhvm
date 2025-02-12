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

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_container.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_exception.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_include.h>
#include <thrift/compiler/ast/t_interaction.h>
#include <thrift/compiler/ast/t_interface.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_package.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_service.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/t_union.h>
#include <thrift/compiler/whisker/dsl.h>

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace apache::thrift::compiler {

/**
 * Base class for all template-based code generators using Whisker as the
 * templating engine.
 *
 * This class serves as the basis for both:
 *   - t_mstch_generator, the legacy mstch-based generator
 *   - t_whisker_generator, the pure Whisker-based generator
 */
class t_whisker_base_generator : public t_generator {
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
    std::shared_ptr<whisker::source_resolver> source_resolver;
    whisker::render_options render_options;
  };
  std::optional<cached_render_state> cached_render_state_;
  cached_render_state& render_state();
};

class t_whisker_generator : public t_whisker_base_generator {
 public:
  using t_whisker_base_generator::t_whisker_base_generator;

 protected:
  // Whisker-based code generators are designed to work directly against the
  // Thrift AST node types (t_<node_type> classes). This is different from the
  // mstch-based code generators which have an additional conversion layer from
  // the AST node types → mstch::node.
  //
  // In Whisker, passing around the AST nodes (C++ objects) is achieved using
  // whisker::native_handle. Whisker templates cannot directly interact with
  // native_handle objects — they must delegate to native_functions to extract
  // usable data out of these objects.
  //
  // The make_functions_for_<node_type>() family of functions provide "free
  // functions" that allow extracting usable information out of the
  // aforementioned node type. This leads to a pattern resembling "opaque
  // pointers" commonly used in languages like C:
  //   https://en.wikipedia.org/wiki/Opaque_pointer#C
  //
  // For example:
  //
  //   make_functions_for_named() may expose a native_function, `name_of`,
  //   which corresponds to `const std::string& t_named::name()` member function
  //   in C++.
  //   `name_of` would accept a single argument, native_handle<t_named>, and
  //   return a whisker::string.
  //
  //   In Whisker code, this function can be invoked like so:
  //     {{ (t_named.name_of program_node) }}
  //     {{ (t_named.name_of struct_node) }}
  //     {{ (t_named.name_of <any node deriving from t_named in C++>) }}
  //
  // Note that, for each node type, the result of
  // make_function_for_<node_type>() is injected into the global scope under the
  // corresponding node's class name.
  //
  // By default, this generator will provide the basic functions defined in the
  // AST node classes. Derived generators may override any overload of
  // make_function_for_<node_type>(...) to add or remove new functions to its
  // library.

  virtual whisker::map make_functions_for_const() const;
  virtual whisker::map make_functions_for_container() const;
  virtual whisker::map make_functions_for_enum() const;
  virtual whisker::map make_functions_for_exception() const;
  virtual whisker::map make_functions_for_field() const;
  virtual whisker::map make_functions_for_function() const;
  virtual whisker::map make_functions_for_include() const;
  virtual whisker::map make_functions_for_interaction() const;
  virtual whisker::map make_functions_for_interface() const;
  virtual whisker::map make_functions_for_list() const;
  virtual whisker::map make_functions_for_map() const;
  virtual whisker::map make_functions_for_named() const;
  virtual whisker::map make_functions_for_node() const;
  virtual whisker::map make_functions_for_package() const;
  virtual whisker::map make_functions_for_paramlist() const;
  virtual whisker::map make_functions_for_primitive_type() const;
  virtual whisker::map make_functions_for_program() const;
  virtual whisker::map make_functions_for_service() const;
  virtual whisker::map make_functions_for_set() const;
  virtual whisker::map make_functions_for_sink() const;
  virtual whisker::map make_functions_for_stream() const;
  virtual whisker::map make_functions_for_struct() const;
  virtual whisker::map make_functions_for_structured() const;
  virtual whisker::map make_functions_for_throws() const;
  virtual whisker::map make_functions_for_type() const;
  virtual whisker::map make_functions_for_typedef() const;
  virtual whisker::map make_functions_for_union() const;

  // This list represents the polymorphic class hierarchy of the Thrift's AST.
  // It does not need to be a complete representation but the subset that is
  // specified here must be a correct hierarchy (enforced by the compiler).
  //
  // Each `t_{node_type}` has a corresponding `h_{node}` alias which a
  // polymorphic_native_handle describing its polymorphic hierarchy
  // (subclasses).
  template <typename... Cases>
  using make_handle = whisker::dsl::make_polymorphic_native_handle<Cases...>;

  using h_interaction = make_handle<t_interaction>;
  using h_service = make_handle<t_service>;
  using h_interface = make_handle<t_interface, h_interaction, h_service>;

  using h_function = make_handle<t_function>;
  using h_stream = make_handle<t_stream>;
  using h_sink = make_handle<t_sink>;
  using h_include = make_handle<t_include>;
  using h_package = make_handle<t_package>;

  using h_program = make_handle<t_program>;

  using h_list = make_handle<t_list>;
  using h_set = make_handle<t_set>;
  using h_map = make_handle<t_map>;
  using h_container = make_handle<t_container, h_set, h_list, h_map>;

  using h_const = make_handle<t_const>;
  using h_enum = make_handle<t_enum>;
  using h_enum_value = make_handle<t_enum_value>;
  using h_field = make_handle<t_field>;
  using h_primitive_type = make_handle<t_primitive_type>;

  using h_exception = make_handle<t_exception>;
  using h_union = make_handle<t_union>;
  using h_throws = make_handle<t_throws>;
  using h_paramlist = make_handle<t_paramlist>;
  using h_struct =
      make_handle<t_struct, h_exception, h_union, h_throws, h_paramlist>;
  using h_structured = make_handle<t_structured, h_struct>;
  using h_typedef = make_handle<t_typedef>;
  using h_type = make_handle<
      t_type,
      h_primitive_type,
      h_enum,
      h_container,
      h_structured,
      h_typedef,
      h_interface>;

  using h_named = make_handle<
      t_named,
      h_const,
      h_enum_value,
      h_field,
      h_function,
      h_type,
      h_program>;
  using h_node = make_handle<t_node, h_named>;

 public:
  whisker::map globals() const override;
};

} // namespace apache::thrift::compiler
