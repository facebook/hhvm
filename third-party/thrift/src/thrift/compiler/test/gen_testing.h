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

#include <memory>
#include <string>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_const_value.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_struct.h>

namespace apache::thrift::compiler::gen {

// Adds a new structured annotation type to the given program, then provides
// access to that type and program.
//
// It is safe to destroy this class at any time.
struct base_annotation_builder {
  t_program& program;
  t_struct& type;

  static std::unique_ptr<t_const_value> make_string(const char* value) {
    return std::make_unique<t_const_value>(value);
  }

  static std::unique_ptr<t_const_value> make_integer(int64_t value) {
    return std::make_unique<t_const_value>(value);
  }

  const std::string& uri() { return type.uri(); }

 protected:
  // Abstract base class
  ~base_annotation_builder() = default;
  base_annotation_builder(t_program& p, std::string name, std::string_view uri)
      : program(p),
        // TODO(afuller): Consider upgrading to *find or* add.
        type(p.add_def(std::make_unique<t_struct>(&p, std::move(name)), uri)) {}

  std::unique_ptr<t_const> make_inst(
      std::unique_ptr<t_const_value> value, std::string name = {}) {
    value->set_ttype(type);
    return std::make_unique<t_const>(
        &program, &type, std::move(name), std::move(value));
  }
};

struct base_thrift_annotation_builder : base_annotation_builder {
 protected:
  // Abstract base class
  ~base_thrift_annotation_builder() = default;
  base_thrift_annotation_builder(t_program& p, const std::string& name)
      : base_annotation_builder(
            p, name, "facebook.com/thrift/annotation/" + name) {}

  base_thrift_annotation_builder(
      t_program& p, const std::string& lang, const std::string& name)
      : base_annotation_builder(
            p, name, "facebook.com/thrift/annotation/" + lang + "/" + name) {}
};

struct adapter_builder : base_thrift_annotation_builder {
  explicit adapter_builder(t_program& p, const std::string& lang)
      : base_thrift_annotation_builder(p, lang, "Adapter") {}

  std::unique_ptr<t_const> make(const char* name) {
    auto map = t_const_value::make_map();
    map->add_map(make_string("name"), make_string(name));
    return make_inst(std::move(map));
  }
};

struct inject_metadata_fields_builder : base_thrift_annotation_builder {
  explicit inject_metadata_fields_builder(t_program& p)
      : base_thrift_annotation_builder(p, "InjectMetadataFields") {}

  std::unique_ptr<t_const> make(const char* type_name) {
    auto map = t_const_value::make_map();
    map->add_map(make_string("type"), make_string(type_name));
    return make_inst(std::move(map));
  }
};

// A builder for default configured thrift/annotation annotations.
struct thrift_annotation_builder : base_thrift_annotation_builder {
  static thrift_annotation_builder box(t_program& p) { return {p, "Box"}; }
  static thrift_annotation_builder terse(t_program& p) {
    return {p, "TerseWrite"};
  }
  static thrift_annotation_builder transitive(t_program& p) {
    return {p, "Transitive"};
  }
  static thrift_annotation_builder dummy(t_program& p) { return {p, "Dummy"}; }

  std::unique_ptr<t_const> make() {
    return make_inst(std::make_unique<t_const_value>());
  }

 protected:
  using base_thrift_annotation_builder::base_thrift_annotation_builder;
};

struct cpp_annotation_builder : base_thrift_annotation_builder {
  std::unique_ptr<t_const> make() {
    return make_inst(std::make_unique<t_const_value>());
  }

 protected:
  cpp_annotation_builder(t_program& p, const std::string& name)
      : base_thrift_annotation_builder(p, "cpp", name) {}
};

} // namespace apache::thrift::compiler::gen
