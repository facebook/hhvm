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

#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <fmt/format.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/generate/go/util.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache::thrift::compiler {

namespace {

std::string doc_comment(const t_named* named_node) {
  std::istringstream in(named_node->doc());

  std::string line;
  std::ostringstream out;
  while (std::getline(in, line)) {
    out << "// " << line << std::endl;
  }
  return out.str();
}

class t_mstch_go_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  whisker_options render_options() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {
        "service:autogen_path",
    };
    return opts;
  }

  std::string template_prefix() const override { return "go"; }

  void generate_program() override;

 private:
  void set_mstch_factories();

  go::codegen_data data_;
};

class mstch_go_program : public mstch_program {
 public:
  mstch_go_program(
      const t_program* p,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
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
            {"program:compat_setters?",
             &mstch_go_program::go_gen_compat_setters},
            {"program:thrift_imports", &mstch_go_program::thrift_imports},
            {"program:thrift_lib_import", &mstch_go_program::thrift_lib_import},
            {"program:thrift_metadata_import",
             &mstch_go_program::thrift_metadata_import},
            {"program:go_package_alias", &mstch_go_program::go_package_alias},
            {"program:gen_metadata?", &mstch_go_program::should_gen_metadata},
            {"program:gen_default_get?",
             &mstch_go_program::should_gen_default_get},
            {"program:import_metadata_package?",
             &mstch_go_program::should_import_metadata_package},
            {"program:metadata_qualifier",
             &mstch_go_program::metadata_qualifier},
            {"program:thrift_metadata_types",
             &mstch_go_program::thrift_metadata_types},
            {"program:req_resp_structs", &mstch_go_program::req_resp_structs},
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
  mstch::node go_gen_compat_setters() { return data_.compat_setters; }
  mstch::node thrift_imports() {
    mstch::array a;
    for (const auto* program : program_->get_includes_for_codegen()) {
      a.push_back(make_mstch_program_cached(program, context_));
    }
    return a;
  }
  mstch::node go_import_path() { return get_go_import_path_(); }
  mstch::node thrift_lib_import() { return data_.thrift_lib_import; }
  mstch::node thrift_metadata_import() { return data_.thrift_metadata_import; }
  mstch::node go_package_alias() {
    return data_.get_go_package_alias(program_);
  }
  mstch::node should_gen_metadata() { return data_.gen_metadata; }
  mstch::node should_gen_default_get() { return data_.gen_default_get; }
  mstch::node should_import_metadata_package() {
    // We don't need to import the metadata package if we are
    // generating metadata inside the metadata package itself. Duh.
    return !is_metadata_package_();
  }
  mstch::node metadata_qualifier() {
    // We don't need to use "metadata." qualifier when generating
    // metadata inside the metadata package itself.
    if (!is_metadata_package_()) {
      return std::string("metadata.");
    } else {
      return std::string("");
    }
  }
  mstch::node thrift_metadata_types() {
    return make_mstch_array(
        data_.thrift_metadata_types, *context_.type_factory);
  }
  mstch::node req_resp_structs() {
    return make_mstch_array(data_.req_resp_structs, *context_.struct_factory);
  }

 private:
  go::codegen_data& data_;

  std::string get_go_import_path_() {
    return go::get_go_package_dir(program_, data_.package_override);
  }
  bool is_metadata_package_() {
    return get_go_import_path_() == data_.thrift_metadata_import;
  }
};

class mstch_go_enum : public mstch_enum {
 public:
  mstch_go_enum(
      const t_enum* e,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
      : mstch_enum(e, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"enum:go_name", &mstch_go_enum::go_name},
            {"enum:go_qualified_name", &mstch_go_enum::go_qualified_name},
            {"enum:scoped_name", &mstch_go_enum::scoped_name},
        });
  }

  mstch::node go_name() { return go::munge_ident(enum_->name()); }
  mstch::node go_qualified_name() {
    auto prefix = data_.go_package_alias_prefix(enum_->program());
    auto name = go::munge_ident(enum_->name());
    return prefix + name;
  }
  mstch::node scoped_name() { return enum_->get_scoped_name(); }

 private:
  go::codegen_data& data_;
};

class mstch_go_enum_value : public mstch_enum_value {
 public:
  mstch_go_enum_value(
      const t_enum_value* v,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
      : mstch_enum_value(v, ctx, pos), data_(*data) {
    (void)data_;
  }

 private:
  go::codegen_data& data_;
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
      go::codegen_data* data)
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
    auto real_type = const_->type()->get_true_type();
    return go::is_type_go_nilable(real_type);
  }
  mstch::node go_qualified_name() {
    auto prefix = data_.go_package_alias_prefix(const_->program());
    auto name = go::munge_ident(const_->name());
    return prefix + name;
  }

 private:
  go::codegen_data& data_;
};

class mstch_go_const_value : public mstch_const_value {
 public:
  mstch_go_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      go::codegen_data* data)
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
  go::codegen_data& data_;
};

class mstch_go_field : public mstch_field {
 public:
  mstch_go_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context,
      go::codegen_data* data)
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
            {"field:nilable?", &mstch_go_field::is_nilable},
            {"field:must_be_set_to_serialize?",
             &mstch_go_field::must_be_set_to_serialize},
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
    auto arg_name = go::munge_ident(field_->name(), /*exported*/ false);
    // Avoid 'context' package import collision
    if (arg_name == "context") {
      arg_name += "_";
    }
    return arg_name;
  }
  mstch::node is_pointer() {
    // See comment in the private method for details.
    return is_pointer_();
  }
  mstch::node is_nilable() {
    // Whether this field can be set to 'nil' in Go:
    //  * Fields of nilable Go types can be set to 'nil' (map/slice/struct)
    //  * Fields inside a union can be set to 'nil' ('is_pointer' above)
    //  * Optional fields can be set to 'nil' (see 'is_pointer' above)
    auto real_type = field_->type()->get_true_type();
    return go::is_type_go_nilable(real_type) || is_inside_union_() ||
        is_optional_();
  }
  mstch::node must_be_set_to_serialize() {
    // Whether the field must be set (non-nil) in order to serialize:
    //  * Struct type fields must be set (to avoid nil pointer dereference)
    //  * Fields inside a union must be set (that's the point of a union)
    //  * Optional fields must be set ("unset" optional fields must not be
    //  serailized as per Thrift-spec)
    auto real_type = field_->type()->get_true_type();
    return go::is_type_go_struct(real_type) || is_inside_union_() ||
        is_optional_();
  }
  mstch::node is_non_struct_pointer() {
    // Whether this field is a non-struct pointer.
    auto real_type = field_->type()->get_true_type();
    return is_pointer_() && !go::is_type_go_struct(real_type);
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
  go::codegen_data& data_;

  bool is_pointer_() {
    // Whether this field is a pointer '*' in a Go struct definition:
    //  * Struct-type fields are pointers.
    //     * Union-type fields are pointers too - by extension.
    //  * Fields inside a union are pointers.
    //     * Except (!!!) when the underlying type itself is nilable (map/slice)
    //  * Optional fields are pointers.
    //     * Except (!!!) when the underlying type itself is nilable (map/slice)
    auto real_type = field_->type()->get_true_type();
    return go::is_type_go_struct(real_type) ||
        ((is_inside_union_() || is_optional_()) &&
         !go::is_type_go_nilable(real_type));
  }

  bool is_inside_union_() {
    // Whether field is part of a union
    return field_context_ != nullptr && field_context_->strct != nullptr &&
        field_context_->strct->is<t_union>();
  }
};

class mstch_go_struct : public mstch_struct {
 public:
  mstch_go_struct(
      const t_structured* s,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
      : mstch_struct(s, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"struct:go_name", &mstch_go_struct::go_name},
            {"struct:go_qualified_name", &mstch_go_struct::go_qualified_name},
            {"struct:go_qualified_new_func",
             &mstch_go_struct::go_qualified_new_func},
            {"struct:go_package_alias_prefix",
             &mstch_go_struct::go_package_alias_prefix_},
            {"struct:go_public_req_name", &mstch_go_struct::go_public_req_name},
            {"struct:go_public_resp_name",
             &mstch_go_struct::go_public_resp_name},
            {"struct:struct_spec_name", &mstch_go_struct::struct_spec_name},
            {"struct:req_resp?", &mstch_go_struct::is_req_resp_struct},
            {"struct:resp?", &mstch_go_struct::is_resp_struct},
            {"struct:req?", &mstch_go_struct::is_req_struct},
            {"struct:stream?", &mstch_go_struct::is_stream_struct},
            {"struct:fields_sorted", &mstch_go_struct::fields_sorted},
            {"struct:scoped_name", &mstch_go_struct::scoped_name},
            {"struct:use_reflect_codec?",
             &mstch_go_struct::should_use_reflect_codec},
        });
  }

  mstch::node go_name() { return go_name_(); }
  mstch::node go_qualified_name() {
    auto prefix = data_.go_package_alias_prefix(struct_->program());
    return prefix + go_name_();
  }
  mstch::node go_qualified_new_func() {
    auto prefix = data_.go_package_alias_prefix(struct_->program());
    return prefix + go_new_func_();
  }
  mstch::node go_package_alias_prefix_() {
    return data_.go_package_alias_prefix(struct_->program());
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
  mstch::node is_stream_struct() {
    // Whether this is a helper stream struct.
    return is_req_resp_struct_() &&
        boost::algorithm::starts_with(struct_->name(), "stream");
  }
  mstch::node go_public_req_name() {
    return boost::algorithm::erase_first_copy(struct_->name(), "req") +
        "ArgsDeprecated";
  }
  mstch::node go_public_resp_name() {
    return boost::algorithm::erase_first_copy(struct_->name(), "resp") +
        "ResultDeprecated";
  }
  mstch::node struct_spec_name() {
    return "premadeStructSpec_" + struct_->name();
  }
  mstch::node fields_sorted() {
    auto fields_in_id_order = struct_->get_sorted_members();
    // Fields (optionally) in the most optimal (memory-saving) layout order.
    auto minimizePadding =
        struct_->has_structured_annotation(kGoMinimizePaddingUri);
    if (minimizePadding) {
      std::vector<t_field*> fields_in_layout_order;
      std::copy(
          fields_in_id_order.begin(),
          fields_in_id_order.end(),
          std::back_inserter(fields_in_layout_order));
      go::optimize_fields_layout(
          fields_in_layout_order, struct_->is<t_union>());
      return make_mstch_fields(fields_in_layout_order);
    }
    return make_mstch_fields(fields_in_id_order);
  }
  mstch::node scoped_name() { return struct_->get_scoped_name(); }
  mstch::node should_use_reflect_codec() {
    auto use_reflect_codec_annotation =
        struct_->find_structured_annotation_or_null(kGoUseReflectCodecUri);
    return data_.use_reflect_codec || use_reflect_codec_annotation != nullptr;
  }

 private:
  go::codegen_data& data_;

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
      go::codegen_data* data)
      : mstch_service(s, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"service:go_name", &mstch_go_service::go_name},
            {"service:go_qualified_name", &mstch_go_service::go_qualified_name},
            {"service:go_package_alias_prefix",
             &mstch_go_service::go_package_alias_prefix_},
            {"service:scoped_name", &mstch_go_service::scoped_name},
        });
  }

  mstch::node go_name() { return go::munge_ident(service_->name()); }
  mstch::node go_qualified_name() {
    auto prefix = data_.go_package_alias_prefix(service_->program());
    auto name = go::munge_ident(service_->name());
    return prefix + name;
  }
  mstch::node go_package_alias_prefix_() {
    return data_.go_package_alias_prefix(service_->program());
  }
  mstch::node scoped_name() { return service_->get_scoped_name(); }

 private:
  go::codegen_data& data_;
};

class mstch_go_function : public mstch_function {
 public:
  mstch_go_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_interface* iface,
      go::codegen_data* data)
      : mstch_function(f, ctx, pos, iface), data_(*data) {
    (void)data_;
    register_methods(
        this,
        {
            {"function:go_name", &mstch_go_function::go_name},
            {"function:go_client_supported?",
             &mstch_go_function::is_go_client_supported},
            {"function:go_server_supported?",
             &mstch_go_function::is_go_server_supported},
            {"function:ctx_arg_name", &mstch_go_function::ctx_arg_name},
            {"function:retval_field_name",
             &mstch_go_function::retval_field_name},
            {"function:retval_nilable?", &mstch_go_function::is_retval_nilable},
        });
  }
  mstch::node go_name() { return go::get_go_func_name(function_); }

  mstch::node is_go_client_supported() {
    return go::is_func_go_client_supported(function_);
  }
  mstch::node is_go_server_supported() {
    return go::is_func_go_server_supported(function_);
  }

  mstch::node ctx_arg_name() {
    // This helper returns the Context object name to be used in the function
    // signature. "ctx" by default, "ctx<num>" in case of name collisions with
    // other function arguments. The name is guaranteed not to collide.
    return get_unique_name("ctx");
  }

  std::string get_unique_name(std::string const& name) {
    auto& members = function_->params().get_members();

    std::vector<std::string_view> arg_names;
    arg_names.reserve(members.size());
    for (auto member : members) {
      arg_names.push_back(
          data_.maybe_munge_ident_and_cache(member, /* exported */ false));
    }

    std::string unique_name = name;
    auto current_num = 0;
    while ( //
        std::find(arg_names.begin(), arg_names.end(), unique_name) !=
        arg_names.end()) {
      unique_name = name + std::to_string(++current_num);
    }
    return unique_name;
  }

  mstch::node is_retval_nilable() {
    auto real_type = function_->return_type()->get_true_type();
    return go::is_type_go_nilable(real_type);
  }

  mstch::node retval_field_name() {
    // Field name for the return value.
    return go::munge_ident(go::DEFAULT_RETVAL_FIELD_NAME, /*exported*/ true);
  }

 private:
  go::codegen_data& data_;
};

class mstch_go_type : public mstch_type {
 public:
  mstch_go_type(
      const t_type* t,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
      : mstch_type(t, ctx, pos), data_(*data) {
    (void)data_;
    register_methods(
        this,
        {
            {"type:go_comparable?", &mstch_go_type::is_go_comparable},
            {"type:metadata_primitive?", &mstch_go_type::is_metadata_primitive},
            {"type:named?", &mstch_go_type::has_name},
            {"type:full_name", &mstch_go_type::full_name},
            {"type:metadata_name", &mstch_go_type::metadata_name},
            {"type:metadata_thrift_type_getter",
             &mstch_go_type::metadata_thrift_type_getter},
            {"type:codec_type_spec_name", &mstch_go_type::codec_type_spec_name},
            {"type:codec_type_spec_getter",
             &mstch_go_type::codec_type_spec_getter},
        });
  }

  mstch::node is_go_comparable() { return go::is_type_go_comparable(type_); }
  mstch::node is_metadata_primitive() {
    // Whether this type is primitive from metadata.thrift perspective.
    // i.e. see ThriftPrimitiveType enum in metadata.thrift
    auto real_type = type_->get_true_type();
    return go::is_type_metadata_primitive(real_type);
  }
  mstch::node has_name() { return !type_->name().empty(); }
  mstch::node full_name() { return type_->get_full_name(); }
  mstch::node metadata_name() { return metadata_name_(); }
  mstch::node codec_type_spec_name() { return codec_type_spec_name_(); }
  mstch::node metadata_thrift_type_getter() {
    // Program will be null for primitive (base) types.
    // They should be treated as being from the current program.
    auto is_from_current_program = type_->program() == nullptr ||
        data_.is_current_program(type_->program());

    if (is_from_current_program) {
      // If the type is from the current program, we can simply use its
      // corresponding *ThriftType variable already present in the program.
      return metadata_name_();
    } else {
      // If the type is external, we must retrieve it from its corresponding
      // program/package using GetMetadataThriftType helper method.
      return fmt::format(
          "{}.GetMetadataThriftType(\"{}\")",
          data_.get_go_package_alias(type_->program()),
          type_->get_full_name());
    }
  }
  mstch::node codec_type_spec_getter() {
    // Program will be null for primitive (base) types.
    // They should be treated as being from the current program.
    auto is_from_current_program = type_->program() == nullptr ||
        data_.is_current_program(type_->program());

    if (is_from_current_program) {
      // If the type is from the current program, we can simply use its
      // corresponding *ThriftType variable already present in the program.
      return codec_type_spec_name_();
    } else {
      // If the type is external, we must retrieve it from its corresponding
      // program/package using GetMetadataThriftType helper method.
      return fmt::format(
          "{}.GetCodecTypeSpec(\"{}\")",
          data_.get_go_package_alias(type_->program()),
          type_->get_full_name());
    }
  }

 private:
  go::codegen_data& data_;

  std::string metadata_name_() {
    return "premadeThriftType_" + sanitized_full_name_();
  }
  std::string codec_type_spec_name_() {
    return "premadeCodecTypeSpec_" + sanitized_full_name_();
  }
  std::string sanitized_full_name_() {
    std::string full_name = type_->get_full_name();
    boost::replace_all(full_name, " ", "");
    boost::replace_all(full_name, ".", "_");
    boost::replace_all(full_name, ",", "_");
    boost::replace_all(full_name, "<", "_");
    boost::replace_all(full_name, ">", "");
    return full_name;
  }
};

class mstch_go_typedef : public mstch_typedef {
 public:
  mstch_go_typedef(
      const t_typedef* t,
      mstch_context& ctx,
      mstch_element_position pos,
      go::codegen_data* data)
      : mstch_typedef(t, ctx, pos), data_(*data) {
    register_methods(
        this,
        {
            {"typedef:go_name", &mstch_go_typedef::go_name},
            {"typedef:go_qualified_name", &mstch_go_typedef::go_qualified_name},
            {"typedef:go_qualified_new_func",
             &mstch_go_typedef::go_qualified_new_func},
            {"typedef:go_qualified_write_func",
             &mstch_go_typedef::go_qualified_write_func},
            {"typedef:go_qualified_read_func",
             &mstch_go_typedef::go_qualified_read_func},
            {"typedef:defined_kind?", &mstch_go_typedef::is_defined_kind},
            {"typedef:scoped_name", &mstch_go_typedef::scoped_name},
        });
  }
  mstch::node go_name() { return go_name_(); }
  mstch::node go_qualified_name() {
    auto prefix = data_.go_package_alias_prefix(typedef_->program());
    auto name = go_name_();
    return prefix + name;
  }
  mstch::node go_qualified_new_func() {
    auto prefix = data_.go_package_alias_prefix(typedef_->program());
    auto name = go_name_();
    return prefix + "New" + name;
  }
  mstch::node go_qualified_write_func() {
    auto prefix = data_.go_package_alias_prefix(typedef_->program());
    auto name = go_name_();
    return prefix + "Write" + name;
  }
  mstch::node go_qualified_read_func() {
    auto prefix = data_.go_package_alias_prefix(typedef_->program());
    auto name = go_name_();
    return prefix + "Read" + name;
  }
  mstch::node is_defined_kind() {
    // NOTE: there are multiple typedef "kinds":
    //  * defined - typedef actually defined in a Thrift schema by a human.
    //  * unnamed - typedef used for unstructured annotations.
    //  * placeholder - typedef used as a placeholder during AST parsing
    //    when not all type are fully known yet. During generation, when we
    //    encounter this kind fo typedef, we should skip it to the underlying
    //    "real" type or "defined" typedef to ensure code correctness.
    return typedef_->typedef_kind() == t_typedef::kind::defined;
  }
  mstch::node scoped_name() { return typedef_->get_scoped_name(); }

 private:
  go::codegen_data& data_;

  std::string go_name_() {
    auto name_override = go::get_go_name_annotation(typedef_);
    if (name_override != nullptr) {
      return *name_override;
    }
    return go::munge_ident(typedef_->name());
  }
};

void t_mstch_go_generator::generate_program() {
  out_dir_base_ = "gen-go";
  set_mstch_factories();

  const t_program* program = get_program();

  data_.set_current_program(program);
  data_.compute_go_package_aliases();
  data_.compute_struct_to_field_names();
  data_.compute_req_resp_structs();
  data_.compute_thrift_metadata_types();

  if (auto thrift_lib_import = get_option("thrift_import")) {
    data_.thrift_lib_import = *thrift_lib_import;
  }
  if (auto thrift_metadata_import = get_option("thrift_metadata_import")) {
    data_.thrift_metadata_import = *thrift_metadata_import;
  }
  if (auto package_override = get_option("package")) {
    data_.package_override = *package_override;
  }
  if (auto gen_metadata = get_option("gen_metadata")) {
    data_.gen_metadata = (gen_metadata.value() == "true");
  }
  if (auto gen_default_get = get_option("gen_default_get")) {
    data_.gen_default_get = (gen_default_get.value() == "true");
  }
  if (auto use_reflect_codec = get_option("use_reflect_codec")) {
    data_.use_reflect_codec = (use_reflect_codec.value() == "true");
  }

  const auto& prog = cached_program(program);

  render_to_file(prog, "const.go", "const.go");
  render_to_file(prog, "types.go", "types.go");
  render_to_file(prog, "svcs.go", "svcs.go");
  render_to_file(prog, "codec.go", "codec.go");
  if (data_.gen_metadata) {
    render_to_file(prog, "metadata.go", "metadata.go");
  }
  if (program->has_doc()) {
    render_to_file(prog, "doc.go", "doc.go");
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

} // namespace

THRIFT_REGISTER_GENERATOR(mstch_go, "Go", "");

} // namespace apache::thrift::compiler
