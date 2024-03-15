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

#include <memory>
#include <optional>
#include <stdexcept>

#include <boost/filesystem.hpp>
#include <thrift/compiler/detail/mustache/mstch.h>

#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_generator.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_mstch_generator : public t_generator {
 public:
  using t_generator::t_generator;

  void process_options(
      const std::map<std::string, std::string>& options) override {
    options_ = options;
    mstch_context_.options = options;
    gen_template_map(template_prefix());
  }

  virtual std::string template_prefix() const = 0;

  // Return true to use <% and %> as delimiters instead of {{ and }}.
  virtual bool convert_delimiter() const { return false; }

 protected:
  /**
   * If true, typedefs will be automatically resolved to their underlying type.
   */
  virtual bool should_resolve_typedefs() const { return false; }

  /**
   * Fetches a particular template from the template map, throwing an error
   * if the template doesn't exist
   */
  const std::string& get_template(const std::string& template_name);

  /**
   * Returns the map of (file_name, template_contents) for each template
   * file for this generator
   */
  const std::map<std::string, std::string>& get_template_map() {
    return template_map_;
  }

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
      const boost::filesystem::path& path, const std::string& data);

  /**
   * Render the mstch template with name `template_name` in the given context
   * and write to a path under the output directory. Same as calling `render`
   * and `write_output` in succession.
   */
  void render_to_file(
      const mstch::map& context,
      const std::string& template_name,
      const boost::filesystem::path& path);

  /**
   * Render the mstch template with name `template_name` in the context of the
   * given Thrift AST and write to a path under the output directory. Same as
   * calling `dump` and `render` and `write_output` in succession.
   */
  template <typename T>
  void render_to_file(
      const T& obj,
      const std::string& template_name,
      const boost::filesystem::path& path) {
    render_to_file(dump(obj), template_name, path);
  }

  template <typename T>
  void render_to_file(
      const T& obj,
      const mstch::map& extra_context,
      const std::string& template_name,
      const boost::filesystem::path& path) {
    mstch::map result = this->dump(obj);
    result.insert(extra_context.begin(), extra_context.end());
    this->render_to_file(result, template_name, path);
  }

  /**
   * Render the mstch template with name `template_name` with the given context.
   * This writes to a path under the output directory.
   */
  void render_to_file(
      const std::shared_ptr<mstch_base> context,
      const std::string& template_name,
      const boost::filesystem::path& path) {
    write_output(path, render(template_name, context));
  }

  /**
   * Dump map of command line flags passed to generator
   */
  mstch::map dump_options();

  /**
   * Subclasses should call the dump functions to convert elements
   * of the Thrift AST into maps that can be passed into mstch.
   */
  mstch::map dump(const t_program&);
  mstch::map dump(const t_structured&, bool shallow = false);
  mstch::map dump(const t_field&, int32_t index = 0);
  mstch::map dump(const t_type&);
  mstch::map dump(const t_enum&);
  mstch::map dump(const t_enum_value&);
  mstch::map dump(const t_service&);
  mstch::map dump(const t_function&);
  mstch::map dump(const t_typedef&);
  mstch::map dump(const t_const&);
  mstch::map dump(const t_const_value&);
  mstch::map dump(const std::map<t_const_value*, t_const_value*>::value_type&);

  using annotation = std::pair<std::string, deprecated_annotation_value>;
  mstch::map dump(const annotation&);
  mstch::map dump(const std::string&);

  /**
   * Subclasses should override these functions to extend the behavior of
   * the dump functions. These will be passed the map after the default
   * dump has run, and can modify the maps in whichever ways necessary.
   */
  virtual mstch::map extend_program(const t_program&);
  virtual mstch::map extend_struct(const t_structured&);
  virtual mstch::map extend_field(const t_field&);
  virtual mstch::map extend_type(const t_type&);
  virtual mstch::map extend_enum(const t_enum&);
  virtual mstch::map extend_enum_value(const t_enum_value&);
  virtual mstch::map extend_service(const t_service&);
  virtual mstch::map extend_function(const t_function&);
  virtual mstch::map extend_typedef(const t_typedef&);
  virtual mstch::map extend_const(const t_const&);
  virtual mstch::map extend_const_value(const t_const_value&);
  virtual mstch::map extend_const_value_map_elem(
      const std::map<t_const_value*, t_const_value*>::value_type&);
  virtual mstch::map extend_annotation(const annotation&);

  template <typename element>
  mstch::map dump_elem(const element& elem, int32_t /*index*/) {
    return dump(elem);
  }

  mstch::map dump_elem(const t_field& elem, int32_t index) {
    return dump(elem, index);
  }

  template <typename container>
  mstch::array dump_elems(const container& elems) {
    using T = typename container::value_type;
    mstch::array result;
    int32_t index = 0;
    for (auto itr = elems.begin(); itr != elems.end(); ++itr) {
      auto map =
          dump_elem(*as_const_pointer<std::remove_pointer_t<T>>(*itr), index);
      result.push_back(map);
      ++index;
    }
    add_first_last(result);
    return result;
  }

  void add_first_last(mstch::array& elems) {
    for (auto itr = elems.begin(); itr != elems.end(); ++itr) {
      std::get<mstch::map>(*itr).emplace("first?", itr == elems.begin());
      std::get<mstch::map>(*itr).emplace(
          "last?", std::next(itr) == elems.end());
    }
  }

  bool has_option(const std::string& option) const;
  std::optional<std::string> get_option(const std::string& option) const;
  const std::map<std::string, std::string>& options() const { return options_; }

 private:
  /**
   * Option pairs specified on command line for influencing generation behavior.
   */
  std::map<std::string, std::string> options_;

  std::map<std::string, std::string> template_map_;

  void gen_template_map(const boost::filesystem::path& root);

  /**
   * For every key in the map, prepends a prefix to that key for mstch.
   */
  static mstch::map prepend_prefix(const std::string& prefix, mstch::map map);

  template <typename T>
  static const T* as_const_pointer(const T* x) {
    return x;
  }

  template <typename T>
  static const T* as_const_pointer(const T& x) {
    return std::addressof(x);
  }

 protected:
  mstch_context mstch_context_;

  const std::shared_ptr<mstch_base>& cached_program(const t_program* program);
};

} // namespace compiler
} // namespace thrift
} // namespace apache
