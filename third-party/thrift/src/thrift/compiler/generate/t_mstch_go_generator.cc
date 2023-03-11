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
#include <set>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fmt/format.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/lib/go/util.h>

namespace apache {
namespace thrift {
namespace compiler {

namespace {

struct go_codegen_data {
  // the import path for the supporting library
  std::string thrift_lib_import =
      "github.com/facebook/fbthrift/thrift/lib/go/thrift";
  // package name override (otherwise inferred from thrift file by default)
  std::string package_override;

  // whether to use ctx.Context in processor code
  bool use_context = false;
  // whether to generate code compatible with the old Go generator
  // (to make the migration easier)
  bool compat = true;

  // Key: package name according to Thrift.
  // Value: package name to use in generated code.
  std::map<std::string, std::string> go_package_map;
  std::map<std::string, int32_t> go_package_name_collisions = {
      {"thrift", 0},
      {"context", 0},
      {"fmt", 0},
  };
  // Req/Resp structs are internal and must be unexported (i.e. lowercase)
  // This set will help us track these srtucts by name.
  std::set<std::string> req_resp_struct_names;
  // The current program being generated.
  const t_program* current_program;
};

// To avoid conflict with methods (e.g. Error(), String())
static const std::set<std::string> reserved_field_names = {
    "Error",
    "String",
};

std::string doc_comment(const t_node* node) {
  auto in = std::istringstream(node->get_doc());

  std::string line;
  std::ostringstream out;
  while (std::getline(in, line)) {
    out << "// " << line << std::endl;
  }
  return out.str();
}

std::string get_go_package_alias(
    const t_program* program, const go_codegen_data& options) {
  if (program == options.current_program) {
    return "";
  }

  auto package = go::get_go_package_name(program, options.package_override);
  auto iter = options.go_package_map.find(package);
  if (iter != options.go_package_map.end()) {
    return iter->second;
  }
  throw std::runtime_error("unable to determine Go package alias");
}

std::string go_package_alias_prefix(
    const t_program* program, const go_codegen_data& options) {
  auto alias = get_go_package_alias(program, options);
  if (alias == "") {
    return "";
  } else {
    return alias + ".";
  }
}

class t_mstch_go_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "go"; }

  void generate_program() override;

 private:
  void set_mstch_factories();
  void set_go_package_aliases();
  go_codegen_data data_;
};

class mstch_go_program : public mstch_program {
 public:
  mstch_go_program(
      const t_program* p,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_program(p, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"program:go_pkg_name", &mstch_go_program::go_pkg_name},
            {"program:thirft_source_path",
             &mstch_go_program::thirft_source_path},
            {"program:go_import_path", &mstch_go_program::go_import_path},
            {"program:docs?", &mstch_go_program::go_has_docs},
            {"program:docs", &mstch_go_program::go_doc_comment},
            {"program:compat?", &mstch_go_program::go_gen_compat},
            {"program:thrift_imports", &mstch_go_program::thrift_imports},
            {"program:has_thrift_imports",
             &mstch_go_program::has_thrift_imports},
            {"program:thrift_lib_import", &mstch_go_program::thrift_lib_import},
            {"program:go_package_alias", &mstch_go_program::go_package_alias},
        });
  }
  mstch::node go_pkg_name() {
    auto pkg_name =
        go::get_go_package_base_name(program_, data_.package_override);
    if (data_.compat) {
      return pkg_name;
    } else {
      return go::snakecase(pkg_name);
    }
  }
  mstch::node thirft_source_path() { return program_->path(); }
  mstch::node go_has_docs() { return program_->has_doc(); }
  mstch::node go_doc_comment() { return doc_comment(program_); }
  mstch::node go_gen_compat() { return data_.compat; }
  mstch::node thrift_imports() {
    mstch::array a;
    for (const auto* program : program_->get_included_programs()) {
      a.push_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node has_thrift_imports() {
    return !program_->get_included_programs().empty();
  }
  mstch::node go_import_path() {
    return go::get_go_package_dir(program_, data_.package_override);
  }
  mstch::node thrift_lib_import() { return data_.thrift_lib_import; }
  mstch::node go_package_alias() {
    return get_go_package_alias(program_, data_);
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_enum : public mstch_enum {
 public:
  mstch_go_enum(
      const t_enum* e,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_enum(e, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"enum:go_name", &mstch_go_enum::go_name},
            {"enum:go_qualified_name", &mstch_go_enum::go_qualified_name},
        });
  }

  mstch::node go_name() { return go::munge_ident(enum_->name()); }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(enum_->program(), data_);
    auto name = go::munge_ident(enum_->name());
    return prefix + name;
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_enum_value : public mstch_enum_value {
 public:
  mstch_go_enum_value(
      const t_enum_value* v,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_enum_value(v, ctx, pos), data_(*data) {
    (void)data_;
    register_methods(this, {});
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_const : public mstch_const {
 public:
  mstch_go_const(
      const t_const* c,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      const t_field* field,
      go_codegen_data* data)
      : mstch_const(c, ctx, pos, current_const, expected_type, field),
        data_(*data) {
    register_methods(
        this,
        {
            {"constant:go_name", &mstch_go_const::go_name},
            {"constant:docs?", &mstch_go_const::go_has_docs},
            {"constant:docs", &mstch_go_const::go_doc_comment},
            {"constant:var?", &mstch_go_const::go_is_var},
            {"constant:go_qualified_name", &mstch_go_const::go_qualified_name},
        });
  }
  mstch::node go_name() { return go::munge_ident(const_->name()); }
  mstch::node go_has_docs() { return const_->has_doc(); }
  mstch::node go_doc_comment() { return doc_comment(const_); }

  // go_var returns true to use a var instead of a const in Go for the thrift
  // const definition (e.g for structs, maps, or lists which cannot be const in
  // go)
  mstch::node go_is_var() {
    auto type = const_->get_type()->get_true_type();
    return type->is_list() || type->is_map() || type->is_set() ||
        type->is_struct();
  }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(const_->program(), data_);
    auto name = go::munge_ident(const_->name());
    return prefix + name;
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_const_value : public mstch_const_value {
 public:
  mstch_go_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      go_codegen_data* data)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type),
        data_(*data) {
    (void)data_;
    register_methods(
        this,
        {
            {"value:go_quoted_value", &mstch_go_const_value::go_quoted_value},
        });
  }

  mstch::node go_quoted_value() {
    return go::quote(const_value_->get_string());
  }
  bool same_type_as_expected() const override { return true; }

 private:
  go_codegen_data& data_;
};

class mstch_go_field : public mstch_field {
 public:
  mstch_go_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context,
      go_codegen_data* data)
      : mstch_field(f, ctx, pos, field_context), data_(*data) {
    (void)data_;
    register_methods(
        this,
        {
            {"field:go_name", &mstch_go_field::go_name},
            {"field:go_arg_name", &mstch_go_field::go_arg_name},
            {"field:pointer?", &mstch_go_field::is_pointer},
            {"field:nilable?", &mstch_go_field::is_nilable},
            {"field:dereference?", &mstch_go_field::should_dereference},
            {"field:key_str", &mstch_go_field::key_str},
            {"field:go_tag?", &mstch_go_field::has_go_tag},
            {"field:go_tag", &mstch_go_field::go_tag},
        });
  }

  mstch::node go_name() {
    auto name = go::munge_ident(field_->name());
    if (reserved_field_names.count(name) > 0) {
      name += "_";
    }
    return name;
  }
  mstch::node go_arg_name() { return go::munge_arg(field_->name()); }
  mstch::node is_pointer() {
    // Whether this field is a pointer '*' in Go:
    //  * Struct type fields must be pointers
    //  * Fields inside a union must be pointers
    //  * Optional fields must be pointers
    //     * Except (!!!) when the underlying type itself is nilable (map/slice)
    auto real_type = field_->type()->get_true_type();
    return (real_type->is_struct() || is_inside_union_() || is_optional_()) &&
        !go::is_type_nilable(real_type);
  }
  mstch::node is_nilable() {
    // Whether this field can be set to 'nil' in Go:
    //  * Struct type fields can be set to 'nil' (see 'is_pointer' above)
    //  * Fields inside a union can be set to 'nil' ('is_pointer' above)
    //  * Optional fields can be set to 'nil' (see 'is_pointer' above)
    //  * Fields represented by nilable Go types can be set to 'nil' (map/slice)
    auto real_type = field_->type()->get_true_type();
    return real_type->is_struct() || is_inside_union_() || is_optional_() ||
        go::is_type_nilable(real_type);
  }
  mstch::node should_dereference() {
    // Whether this field should be dereferenced when encoding/decoding.
    // Not all pointer fields need to be dereferenced when they are
    // processed by the encoder/decoder logic.
    auto real_type = field_->type()->get_true_type();
    return (is_inside_union_() || is_optional_()) &&
        !go::is_type_nilable(real_type) && !real_type->is_struct();
  }
  mstch::node key_str() {
    // Legacy schemas may have negative tags - replace minus with an underscore.
    if (field_->get_key() < 0) {
      return "_" + std::to_string(-field_->get_key());
    } else {
      return std::to_string(field_->get_key());
    }
  }
  mstch::node has_go_tag() {
    return field_->find_annotation_or_null("go.tag") != nullptr;
  }
  mstch::node go_tag() {
    auto tag = field_->find_annotation_or_null("go.tag");
    if (tag != nullptr) {
      return *tag;
    }
    return std::string();
  }

 private:
  go_codegen_data& data_;

  bool is_inside_union_() {
    // Whether field is part of a union
    return field_context_ != nullptr && field_context_->strct != nullptr &&
        field_context_->strct->is_union();
  }
};

class mstch_go_struct : public mstch_struct {
 public:
  mstch_go_struct(
      const t_struct* s,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_struct(s, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"struct:go_name", &mstch_go_struct::go_name},
            {"struct:go_qualified_name", &mstch_go_struct::go_qualified_name},
            {"struct:go_qualified_new_func",
             &mstch_go_struct::go_qualified_new_func},
        });
  }

  mstch::node go_name() { return go_name_(); }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(struct_->program(), data_);
    return prefix + go_name_();
  }
  mstch::node go_qualified_new_func() {
    auto prefix = go_package_alias_prefix(struct_->program(), data_);
    return prefix + go_new_func_();
  }

 private:
  go_codegen_data& data_;

  std::string go_name_() {
    auto name = struct_->name();
    if (data_.req_resp_struct_names.count(name) > 0) {
      // Unexported/lowercase
      return go::munge_ident(name, false);
    } else {
      // Exported/uppercase
      return go::munge_ident(name, true);
    }
  }

  std::string go_new_func_() {
    auto name = struct_->name();
    auto go_name = go::munge_ident(struct_->name(), true);
    if (data_.req_resp_struct_names.count(name) > 0) {
      // Unexported/lowercase
      return "new" + go_name;
    } else {
      // Exported/uppercase
      return "New" + go_name;
    }
  }
};

class mstch_go_service : public mstch_service {
 public:
  mstch_go_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_service(s, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"service:go_name", &mstch_go_service::go_name},
            {"service:use_context?", &mstch_go_service::is_use_context_enabled},
            {"service:proc_func_interface",
             &mstch_go_service::proc_func_interface},
            {"service:go_qualified_name", &mstch_go_service::go_qualified_name},
            {"service:go_package_alias_prefix",
             &mstch_go_service::go_package_alias_prefix_},
            {"service:req_resp_structs", &mstch_go_service::req_resp_structs},
        });
  }

  mstch::node go_name() { return go::munge_ident(service_->name()); }
  mstch::node is_use_context_enabled() { return data_.use_context; }
  mstch::node proc_func_interface() {
    if (data_.use_context) {
      return std::string("thrift.ProcessorFunctionContext");
    } else {
      return std::string("thrift.ProcessorFunction");
    }
  }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(service_->program(), data_);
    auto name = go::munge_ident(service_->name());
    return prefix + name;
  }
  mstch::node go_package_alias_prefix_() {
    return go_package_alias_prefix(service_->program(), data_);
  }

  mstch::node req_resp_structs() {
    std::vector<t_struct*> req_resp_structs;
    for (auto func : get_functions()) {
      if (!go::is_func_go_supported(func)) {
        continue;
      }
      auto svcGoName = go::munge_ident(service_->name());
      auto funcGoName = go::munge_ident(func->name());

      auto req_struct_name =
          go::munge_ident("req" + svcGoName + funcGoName, false);
      auto req_struct = new t_struct(service_->program(), req_struct_name);
      for (auto member : func->params().get_members()) {
        req_struct->append_field(std::unique_ptr<t_field>(member));
      }
      req_resp_structs.push_back(req_struct);
      data_.req_resp_struct_names.insert(req_struct_name);

      auto resp_struct_name =
          go::munge_ident("resp" + svcGoName + funcGoName, false);
      auto resp_struct = new t_struct(service_->program(), resp_struct_name);
      if (!func->get_return_type()->is_void()) {
        auto resp_field =
            std::make_unique<t_field>(func->get_return_type(), "value", 0);
        resp_field->set_req(t_field::e_req::required);
        resp_struct->append_field(std::move(resp_field));
      }
      req_resp_structs.push_back(resp_struct);
      data_.req_resp_struct_names.insert(resp_struct_name);
    }
    return make_mstch_array(req_resp_structs, *context_.struct_factory);
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_function : public mstch_function {
 public:
  mstch_go_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_function(f, ctx, pos), data_(*data) {
    (void)data_;
    register_methods(
        this,
        {
            {"function:go_name", &mstch_go_function::go_name},
            {"function:go_supported?", &mstch_go_function::is_go_supported},
        });
  }
  mstch::node go_name() {
    auto name_override = function_->find_annotation_or_null("go.name");
    if (name_override != nullptr) {
      return *name_override;
    } else {
      return go::munge_ident(function_->name());
    }
  }

  mstch::node is_go_supported() { return go::is_func_go_supported(function_); }

 private:
  go_codegen_data& data_;
};

class mstch_go_type : public mstch_type {
 public:
  mstch_go_type(
      const t_type* t,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_type(t, ctx, pos), data_(*data) {
    (void)data_;
    register_methods(this, {});
  }

 private:
  go_codegen_data& data_;
};

class mstch_go_typedef : public mstch_typedef {
 public:
  mstch_go_typedef(
      const t_typedef* t,
      mstch_context& ctx,
      mstch_element_position pos,
      go_codegen_data* data)
      : mstch_typedef(t, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"typedef:go_name", &mstch_go_typedef::go_name},
            {"typedef:go_newtype?", &mstch_go_typedef::go_newtype},
            {"typedef:go_qualified_name", &mstch_go_typedef::go_qualified_name},
            {"typedef:go_qualified_new_func",
             &mstch_go_typedef::go_qualified_new_func},
        });
  }
  mstch::node go_name() {
    if (typedef_->has_annotation("go.name")) {
      return typedef_->get_annotation("go.name");
    }
    return go::munge_ident(typedef_->name());
  }
  mstch::node go_newtype() { return typedef_->has_annotation("go.newtype"); }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(typedef_->program(), data_);
    auto name = go_name_();
    return prefix + name;
  }
  mstch::node go_qualified_new_func() {
    auto prefix = go_package_alias_prefix(typedef_->program(), data_);
    auto name = go_name_();
    return prefix + "New" + name;
  }

 private:
  go_codegen_data& data_;

  std::string go_name_() {
    if (typedef_->has_annotation("go.name")) {
      return typedef_->get_annotation("go.name");
    } else {
      return go::munge_ident(typedef_->name());
    }
  }
};

void t_mstch_go_generator::generate_program() {
  out_dir_base_ = "gen-go_mstch";
  set_mstch_factories();
  set_go_package_aliases();
  data_.current_program = program_;

  if (auto thrift_lib_import = get_option("thrift_import")) {
    data_.thrift_lib_import = *thrift_lib_import;
  }
  if (auto package_override = get_option("package")) {
    data_.package_override = *package_override;
  }
  data_.use_context = has_option("use_context");

  const auto* program = get_program();
  const auto& prog = cached_program(program);
  auto package_dir = boost::filesystem::path{
      go::get_go_package_dir(program, data_.package_override)};

  render_to_file(prog, "const.go", package_dir / "const.go");
  render_to_file(prog, "types.go", package_dir / "types.go");
  render_to_file(prog, "svcs.go", package_dir / "svcs.go");
  if (program->has_doc()) {
    render_to_file(prog, "doc.go", package_dir / "doc.go");
  }
}

void t_mstch_go_generator::set_mstch_factories() {
  mstch_context_.add<mstch_go_program>(&data_);
  mstch_context_.add<mstch_go_service>(&data_);
  mstch_context_.add<mstch_go_function>(&data_);
  mstch_context_.add<mstch_go_type>(&data_);
  mstch_context_.add<mstch_go_typedef>(&data_);
  mstch_context_.add<mstch_go_struct>(&data_);
  mstch_context_.add<mstch_go_field>(&data_);
  mstch_context_.add<mstch_go_enum>(&data_);
  mstch_context_.add<mstch_go_enum_value>(&data_);
  mstch_context_.add<mstch_go_const>(&data_);
  mstch_context_.add<mstch_go_const_value>(&data_);
}

void t_mstch_go_generator::set_go_package_aliases() {
  auto program = get_program();
  auto includes = program->get_included_programs();

  for (auto include : includes) {
    auto package = go::get_go_package_name(include, data_.package_override);
    auto package_base_name =
        go::get_go_package_base_name(include, data_.package_override);
    auto unique_package_name = go::make_unique_name(
        data_.go_package_name_collisions, go::munge_arg(package_base_name));

    data_.go_package_map.emplace(package, unique_package_name);
  }
}
} // namespace

THRIFT_REGISTER_GENERATOR(mstch_go, "Go", "");

} // namespace compiler
} // namespace thrift
} // namespace apache
