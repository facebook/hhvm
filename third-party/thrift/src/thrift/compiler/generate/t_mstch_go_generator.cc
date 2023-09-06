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

  // whether to generate code compatible with the old Go generator
  // (to make the migration easier)
  bool compat = true;
  // whether to generate "legacy" getters which do not properly support optional
  // fields (to make the migration easier)
  bool compat_getters = true;
  // whether to generate "legacy" setters which do not properly support optional
  // fields (to make the migration easier)
  bool compat_setters = true;

  // Key: package name according to Thrift.
  // Value: package name to use in generated code.
  std::map<std::string, std::string> go_package_map;
  std::map<std::string, int32_t> go_package_name_collisions = {
      {"thrift", 0},
      {"context", 0},
      {"fmt", 0},
  };
  // Records field names for every struct in the program.
  // This is needed to resolve some edge case name collisions.
  std::map<std::string, std::set<std::string>> struct_to_field_names = {};
  // Req/Resp structs are internal and must be unexported (i.e. lowercase)
  // This set will help us track these srtucts by name.
  std::set<std::string> req_resp_struct_names;
  // Mapping of service name to a vector of req/resp structs for that service.
  std::map<std::string, std::vector<t_struct*>> service_to_req_resp_structs =
      {};
  // The current program being generated.
  const t_program* current_program;
};

std::string doc_comment(const t_named* named_node) {
  std::istringstream in(named_node->doc());

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
  void set_struct_to_field_names();
  void set_service_to_req_resp_structs();
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
            {"program:compat_getters?",
             &mstch_go_program::go_gen_compat_getters},
            {"program:compat_setters?",
             &mstch_go_program::go_gen_compat_setters},
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
  mstch::node go_gen_compat_getters() { return data_.compat_getters; }
  mstch::node go_gen_compat_setters() { return data_.compat_setters; }
  mstch::node thrift_imports() {
    mstch::array a;
    for (const auto* program : program_->get_includes_for_codegen()) {
      a.push_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node has_thrift_imports() {
    return !program_->get_includes_for_codegen().empty();
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
    auto real_type = const_->get_type()->get_true_type();
    return real_type->is_list() || real_type->is_map() || real_type->is_set() ||
        go::is_type_go_struct(real_type);
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
            {"field:go_setter_name", &mstch_go_field::go_setter_name},
            {"field:pointer?", &mstch_go_field::is_pointer},
            {"field:non_struct_pointer?",
             &mstch_go_field::is_non_struct_pointer},
            {"field:compat_setter_pointer?",
             &mstch_go_field::is_compat_setter_pointer},
            {"field:compat_setter_value_op",
             &mstch_go_field::compat_setter_value_op},
            {"field:nilable?", &mstch_go_field::is_nilable},
            {"field:key_str", &mstch_go_field::key_str},
            {"field:go_tag?", &mstch_go_field::has_go_tag},
            {"field:go_tag", &mstch_go_field::go_tag},
            {"field:retval?", &mstch_go_field::is_retval},
            {"field:json_omitempty?", &mstch_go_field::is_json_omitempty},
        });
  }

  mstch::node go_name() { return go::get_go_field_name(field_); }
  mstch::node go_setter_name() {
    auto setter_name = "Set" + go::get_go_field_name(field_);
    // Setters which collide with existing field names should be suffixed with
    // an underscore.
    if (field_context_ != nullptr && field_context_->strct != nullptr) {
      auto stfn_iter =
          data_.struct_to_field_names.find(field_context_->strct->name());
      if (stfn_iter != data_.struct_to_field_names.end()) {
        while (stfn_iter->second.count(setter_name) > 0) {
          setter_name += "_";
        }
      }
    }

    return setter_name;
  }
  mstch::node go_arg_name() {
    return go::munge_ident(field_->name(), /*exported*/ false);
  }
  mstch::node is_pointer() {
    // See comment in the private method for details.
    return is_pointer_();
  }
  mstch::node is_nilable() {
    // Whether this field can be set to 'nil' in Go:
    //  * Struct type fields can be set to 'nil' (see 'is_pointer' above)
    //  * Fields inside a union can be set to 'nil' ('is_pointer' above)
    //  * Optional fields can be set to 'nil' (see 'is_pointer' above)
    //  * Fields represented by nilable Go types can be set to 'nil' (map/slice)
    auto real_type = field_->type()->get_true_type();
    return go::is_type_go_struct(real_type) || is_inside_union_() ||
        is_optional_() || go::is_type_nilable(real_type);
  }
  mstch::node is_non_struct_pointer() {
    // Whether this field is a non-struct pointer.
    auto real_type = field_->type()->get_true_type();
    return is_pointer_() && !go::is_type_go_struct(real_type);
  }
  mstch::node is_compat_setter_pointer() { return is_compat_setter_pointer_(); }
  mstch::node compat_setter_value_op() {
    if (is_pointer_() && is_compat_setter_pointer_()) {
      return std::string("");
    } else if (is_pointer_() && !is_compat_setter_pointer_()) {
      return std::string("&");
    } else if (!is_pointer_() && !is_compat_setter_pointer_()) {
      return std::string("");
    } else { // if (!is_pointer_() && is_compat_setter_pointer_())
      return std::string("*");
    }
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
    return go::get_go_tag_annotation(field_) != nullptr;
  }
  mstch::node go_tag() {
    auto tag = go::get_go_tag_annotation(field_);
    if (tag != nullptr) {
      return *tag;
    }
    return std::string();
  }
  mstch::node is_retval() {
    return field_->name() == go::DEFAULT_RETVAL_FIELD_NAME;
  }
  mstch::node is_json_omitempty() {
    // Whether this field should be tagged with 'json:"omitempty"'.
    // Optional and union fields should be tagged as such.
    return is_optional_() || is_inside_union_();
  }

 private:
  go_codegen_data& data_;

  bool is_pointer_() {
    // Whether this field is a pointer '*' in a Go struct definition:
    //  * Struct-type fields are pointers.
    //     * Union-type fields are pointers too - by extension.
    //  * Fields inside a union are pointers.
    //     * Except (!!!) when the underlying type itself is nilable (map/slice)
    //  * Optional fields are pointers.
    //     * Except (!!!) when the underlying type itself is nilable (map/slice)
    auto real_type = field_->type()->get_true_type();
    return (go::is_type_go_struct(real_type) || is_inside_union_() ||
            is_optional_()) &&
        !go::is_type_nilable(real_type);
  }

  bool is_compat_setter_pointer_() {
    // Whether this field's value should be a pointer in compat-setter.
    // This is needed for a seamless migration from the legacy generator.
    //
    // Legacy generator treats optional fields with default values differently
    // compared to the new generator (pointer vs non-pointer).
    // This method helps with backwards compatibility.
    bool has_default_value = (field_->default_value() != nullptr);
    if (is_optional_() && has_default_value) {
      auto real_type = field_->type()->get_true_type();
      bool is_container =
          (real_type->is_list() || real_type->is_map() || real_type->is_set());
      return is_container;
    }
    return is_pointer_();
  }

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
            {"struct:go_public_req_name", &mstch_go_struct::go_public_req_name},
            {"struct:req_resp?", &mstch_go_struct::is_req_resp_struct},
            {"struct:resp?", &mstch_go_struct::is_resp_struct},
            {"struct:req?", &mstch_go_struct::is_req_struct},
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
  mstch::node is_req_resp_struct() {
    // Whether this is a helper request or response struct.
    return is_req_resp_struct_();
  }
  mstch::node is_resp_struct() {
    // Whether this is a helper response struct.
    return is_req_resp_struct_() &&
        boost::algorithm::starts_with(struct_->name(), "resp");
  }
  mstch::node is_req_struct() {
    // Whether this is a helper request struct.
    return is_req_resp_struct_() &&
        boost::algorithm::starts_with(struct_->name(), "req");
  }
  mstch::node go_public_req_name() {
    return boost::algorithm::erase_first_copy(struct_->name(), "req") + "Args";
  }

 private:
  go_codegen_data& data_;

  std::string go_name_() {
    auto name = struct_->name();
    if (is_req_resp_struct_()) {
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
    if (is_req_resp_struct_()) {
      // Unexported/lowercase
      return "new" + go_name;
    } else {
      // Exported/uppercase
      return "New" + go_name;
    }
  }

  bool is_req_resp_struct_() {
    return (data_.req_resp_struct_names.count(struct_->name()) > 0);
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
            {"service:go_qualified_name", &mstch_go_service::go_qualified_name},
            {"service:go_package_alias_prefix",
             &mstch_go_service::go_package_alias_prefix_},
            {"service:req_resp_structs", &mstch_go_service::req_resp_structs},
        });
  }

  mstch::node go_name() { return go::munge_ident(service_->name()); }
  mstch::node go_qualified_name() {
    auto prefix = go_package_alias_prefix(service_->program(), data_);
    auto name = go::munge_ident(service_->name());
    return prefix + name;
  }
  mstch::node go_package_alias_prefix_() {
    return go_package_alias_prefix(service_->program(), data_);
  }

  mstch::node req_resp_structs() {
    auto req_resp_structs =
        data_.service_to_req_resp_structs.find(service_->name());
    if (req_resp_structs != data_.service_to_req_resp_structs.end()) {
      return make_mstch_array(
          req_resp_structs->second, *context_.struct_factory);
    }
    throw std::runtime_error(
        "unable to get req/resp structs for service " + service_->name());
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
            {"function:ctx_arg_name", &mstch_go_function::ctx_arg_name},
        });
  }
  mstch::node go_name() { return go::get_go_func_name(function_); }

  mstch::node is_go_supported() { return go::is_func_go_supported(function_); }

  mstch::node ctx_arg_name() {
    // This helper returns the Context object name to be used in the function
    // signature. "ctx" by default, "ctx<num>" in case of name collisions with
    // other function arguments. The name is guaranteed not to collide.
    std::set<std::string> arg_names;
    auto& members = function_->get_paramlist()->get_members();
    for (auto& member : members) {
      arg_names.insert(go::munge_ident(member->name(), /*exported*/ false));
    }

    std::string ctx_name = "ctx";
    auto current_num = 0;
    while (arg_names.count(ctx_name) > 0) {
      ctx_name = std::string("ctx") + std::to_string(++current_num);
    }
    return ctx_name;
  }

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
    register_methods(
        this,
        {
            {"type:go_comparable?", &mstch_go_type::is_go_comparable},
        });
  }

  mstch::node is_go_comparable() { return go::is_type_go_comparable(type_); }

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
            {"typedef:go_qualified_write_func",
             &mstch_go_typedef::go_qualified_write_func},
            {"typedef:go_qualified_read_func",
             &mstch_go_typedef::go_qualified_read_func},
            {"typedef:placeholder?", &mstch_go_typedef::is_placeholder},
        });
  }
  mstch::node go_name() { return go_name_(); }
  mstch::node go_newtype() {
    return typedef_->find_structured_annotation_or_null(kGoNewTypeUri) !=
        nullptr;
  }
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
  mstch::node go_qualified_write_func() {
    auto prefix = go_package_alias_prefix(typedef_->program(), data_);
    auto name = go_name_();
    return prefix + "Write" + name;
  }
  mstch::node go_qualified_read_func() {
    auto prefix = go_package_alias_prefix(typedef_->program(), data_);
    auto name = go_name_();
    return prefix + "Read" + name;
  }
  mstch::node is_placeholder() {
    // Special handling for the following two scenarios:
    //   1. t_placeholder_typedef is not an actual typedef, but a
    //   dummy hack/workaround in Thrift compiler AST.
    //   2. Unnamed typedef to hold unstructured annotations.
    //
    // In either case, we want to skip a few steps down the chain to the
    // "actual" types if order to generate code properly.
    return typedef_->typedef_kind() != t_typedef::kind::defined;
  }

 private:
  go_codegen_data& data_;

  std::string go_name_() {
    auto name_override = go::get_go_name_annotation(typedef_);
    if (name_override != nullptr) {
      return *name_override;
    }
    return go::munge_ident(typedef_->name());
  }
};

void t_mstch_go_generator::generate_program() {
  out_dir_base_ = "gen-go_mstch";
  set_mstch_factories();
  set_go_package_aliases();
  set_struct_to_field_names();
  set_service_to_req_resp_structs();
  data_.current_program = program_;

  if (auto thrift_lib_import = get_option("thrift_import")) {
    data_.thrift_lib_import = *thrift_lib_import;
  }
  if (auto package_override = get_option("package")) {
    data_.package_override = *package_override;
  }

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
  auto includes = program->get_includes_for_codegen();

  // Prevent collisions with *this* program's package name
  auto pkg_name = go::get_go_package_base_name(program, data_.package_override);
  data_.go_package_name_collisions[pkg_name] = 0;

  for (auto include : includes) {
    auto package = go::get_go_package_name(include);
    auto package_base_name = go::get_go_package_base_name(include);
    auto unique_package_name = go::make_unique_name(
        data_.go_package_name_collisions,
        go::munge_ident(package_base_name, /*exported*/ false));

    data_.go_package_map.emplace(package, unique_package_name);
  }
}

void t_mstch_go_generator::set_struct_to_field_names() {
  auto program = get_program();
  for (auto struct_ : program->structs()) {
    data_.struct_to_field_names[struct_->name()] =
        go::get_struct_go_field_names(struct_);
  }
}

void t_mstch_go_generator::set_service_to_req_resp_structs() {
  auto program = get_program();
  for (auto service : program->services()) {
    std::vector<t_struct*> req_resp_structs =
        go::get_service_req_resp_structs(service);
    data_.service_to_req_resp_structs[service->name()] = req_resp_structs;
    for (auto struct_ : req_resp_structs) {
      data_.req_resp_struct_names.insert(struct_->name());
      data_.struct_to_field_names[struct_->name()] =
          go::get_struct_go_field_names(struct_);
    }
  }
}
} // namespace

THRIFT_REGISTER_GENERATOR(mstch_go, "Go", "");

} // namespace compiler
} // namespace thrift
} // namespace apache
