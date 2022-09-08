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

#include <algorithm>
#include <memory>

#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/generate/json.h>
#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

int get_lineno(const t_node& node, source_manager& sm) {
  auto loc = node.src_range().begin;
  return loc != source_location()
      ? resolved_location(node.src_range().begin, sm).line()
      : 0;
}

class t_json_experimental_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "json"; }
  bool convert_delimiter() const override { return true; }

  void generate_program() override;

 private:
  void set_mstch_factories();
};

class json_experimental_program : public mstch_program {
 public:
  json_experimental_program(
      const t_program* p, mstch_context& ctx, mstch_element_position pos)
      : mstch_program(p, ctx, pos) {
    register_methods(
        this,
        {
            {"program:py_namespace",
             &json_experimental_program::get_py_namespace},
            {"program:includes?", &json_experimental_program::has_includes},
            {"program:includes", &json_experimental_program::includes},
            {"program:namespaces?", &json_experimental_program::has_namespaces},
            {"program:namespaces", &json_experimental_program::namespaces},
            {"program:docstring?", &json_experimental_program::has_docstring},
            {"program:docstring", &json_experimental_program::get_docstring},
            {"program:normalized_include_prefix",
             &json_experimental_program::include_prefix},
        });
  }
  mstch::node get_py_namespace() { return program_->get_namespace("py"); }
  mstch::node has_includes() { return !program_->includes().empty(); }
  mstch::node includes() {
    mstch::array includes;
    auto last = program_->includes().size();
    for (auto program : program_->get_included_programs()) {
      includes.push_back(mstch::map{
          {"name", program->get_name()},
          {"path", program->include_prefix() + program->name() + ".thrift"},
          {"last?", (--last) == 0}});
    }
    return includes;
  }
  mstch::node has_namespaces() { return !program_->namespaces().empty(); }
  mstch::node namespaces() {
    mstch::array result;
    auto last = program_->namespaces().size();
    for (auto it : program_->namespaces()) {
      result.push_back(mstch::map{
          {"key", it.first}, {"value", it.second}, {"last?", (--last) == 0}});
    }
    return result;
  }
  mstch::node has_docstring() { return !program_->get_doc().empty(); }
  mstch::node get_docstring() { return json_quote_ascii(program_->get_doc()); }

  mstch::node include_prefix() {
    auto prefix = program_->include_prefix();
    if (prefix.empty()) {
      return std::string();
    }
    return boost::filesystem::path(prefix).has_root_directory()
        ? context_.options["include_prefix"]
        : prefix;
  }
};

class json_experimental_service : public mstch_service {
 public:
  json_experimental_service(
      const t_service* s,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_service(s, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"service:lineno", &json_experimental_service::get_lineno},
            {"service:docstring?", &json_experimental_service::has_docstring},
            {"service:docstring", &json_experimental_service::get_docstring},
            {"service:path", &json_experimental_service::path},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*service_, source_mgr_);
  }
  mstch::node has_docstring() { return service_->has_doc(); }
  mstch::node get_docstring() { return json_quote_ascii(service_->get_doc()); }
  mstch::node path() {
    return service_->program()->include_prefix() + service_->program()->name() +
        ".thrift";
  }

 private:
  source_manager& source_mgr_;
};

class json_experimental_function : public mstch_function {
 public:
  json_experimental_function(
      const t_function* f,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_function(f, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"function:lineno", &json_experimental_function::get_lineno},
            {"function:docstring?", &json_experimental_function::has_docstring},
            {"function:docstring", &json_experimental_function::get_docstring},
            {"function:args?", &json_experimental_function::has_args},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*function_, source_mgr_);
  }
  mstch::node has_docstring() { return function_->has_doc(); }
  mstch::node get_docstring() { return json_quote_ascii(function_->get_doc()); }
  mstch::node has_args() {
    return !function_->get_paramlist()->get_members().empty();
  }

 private:
  source_manager& source_mgr_;
};

class json_experimental_struct : public mstch_struct {
 public:
  json_experimental_struct(
      const t_struct* s,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_struct(s, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"struct:lineno", &json_experimental_struct::get_lineno},
            {"struct:docstring?", &json_experimental_struct::has_docstring},
            {"struct:docstring", &json_experimental_struct::get_docstring},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*struct_, source_mgr_);
  }
  mstch::node has_docstring() { return struct_->has_doc(); }
  mstch::node get_docstring() { return json_quote_ascii(struct_->get_doc()); }

 private:
  source_manager& source_mgr_;
};

class json_experimental_field : public mstch_field {
 public:
  json_experimental_field(
      const t_field* f,
      mstch_context& ctx,
      mstch_element_position pos,
      const field_generator_context* field_context,
      source_manager* sm)
      : mstch_field(f, ctx, pos, field_context), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"field:lineno", &json_experimental_field::get_lineno},
            {"field:docstring?", &json_experimental_field::has_docstring},
            {"field:docstring", &json_experimental_field::get_docstring},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*field_, source_mgr_);
  }
  mstch::node has_docstring() { return field_->has_doc(); }
  mstch::node get_docstring() { return json_quote_ascii(field_->get_doc()); }

 private:
  source_manager& source_mgr_;
};

class json_experimental_enum : public mstch_enum {
 public:
  json_experimental_enum(
      const t_enum* e,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_enum(e, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"enum:empty?", &json_experimental_enum::is_empty},
            {"enum:lineno", &json_experimental_enum::get_lineno},
            {"enum:docstring?", &json_experimental_enum::has_docstring},
            {"enum:docstring", &json_experimental_enum::get_docstring},
        });
  }
  mstch::node is_empty() { return enum_->get_enum_values().empty(); }
  mstch::node get_lineno() { return compiler::get_lineno(*enum_, source_mgr_); }
  mstch::node has_docstring() { return enum_->has_doc(); }
  mstch::node get_docstring() { return json_quote_ascii(enum_->get_doc()); }

 private:
  source_manager& source_mgr_;
};

class json_experimental_enum_value : public mstch_enum_value {
 public:
  json_experimental_enum_value(
      const t_enum_value* ev,
      mstch_context& ctx,
      mstch_element_position pos,
      source_manager* sm)
      : mstch_enum_value(ev, ctx, pos), source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"enum_value:lineno", &json_experimental_enum_value::get_lineno},
            {"enum_value:docstring?",
             &json_experimental_enum_value::has_docstring},
            {"enum_value:docstring",
             &json_experimental_enum_value::get_docstring},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*enum_value_, source_mgr_);
  }
  mstch::node has_docstring() { return enum_value_->has_doc(); }
  mstch::node get_docstring() {
    return json_quote_ascii(enum_value_->get_doc());
  }

 private:
  source_manager& source_mgr_;
};

// Trim all white spaces and commas from end (in place).
void rtrim(std::string& s) {
  s.erase(
      std::find_if(
          s.rbegin(),
          s.rend(),
          [](int ch) { return !(ch == ' ' || ch == ','); })
          .base(),
      s.end());
}

std::string to_string(const t_const_value* value) {
  auto stringify_list = [](const auto& value) {
    std::string result;
    for (const auto& v : value) {
      result += to_string(v) + ", ";
    }
    rtrim(result);
    return "[" + result + "]";
  };

  auto stringify_map = [](const auto& value) {
    std::string result;
    for (const auto& v : value) {
      auto key = to_string(v.first);
      if (v.first->get_type() != mstch_const_value::cv::CV_STRING) {
        // map keys must be strings
        key = json_quote_ascii(key);
      }
      result += key + ": " + to_string(v.second) + ", ";
    }
    rtrim(result);
    return "{" + result + "}";
  };

  switch (value->get_type()) {
    case mstch_const_value::cv::CV_BOOL:
      return value->get_bool() ? "true" : "false";
    case mstch_const_value::cv::CV_INTEGER:
      return std::to_string(value->get_integer());
    case mstch_const_value::cv::CV_DOUBLE:
      return std::to_string(value->get_double());
    case mstch_const_value::cv::CV_STRING:
      return json_quote_ascii(value->get_string());
    case mstch_const_value::cv::CV_LIST:
      return stringify_list(value->get_list());
    case mstch_const_value::cv::CV_MAP:
      return stringify_map(value->get_map());
  }
  return "";
}

class json_experimental_const_value : public mstch_const_value {
 public:
  json_experimental_const_value(
      const t_const_value* cv,
      mstch_context& ctx,
      mstch_element_position pos,
      const t_const* current_const,
      const t_type* expected_type,
      source_manager* sm)
      : mstch_const_value(cv, ctx, pos, current_const, expected_type),
        source_mgr_(*sm) {
    register_methods(
        this,
        {
            {"value:lineno", &json_experimental_const_value::get_lineno},
            {"value:type_name", &json_experimental_const_value::get_type_name},
            {"value:string_value_any",
             &json_experimental_const_value::string_value_any},
            {"value:docstring?", &json_experimental_const_value::has_docstring},
            {"value:docstring", &json_experimental_const_value::get_docstring},
        });
  }
  mstch::node get_lineno() {
    return compiler::get_lineno(*current_const_, source_mgr_);
  }
  mstch::node has_docstring() { return current_const_->has_doc(); }
  mstch::node get_docstring() {
    return json_quote_ascii(current_const_->get_doc());
  }

  mstch::node get_type_name() {
    return current_const_->get_type()->get_true_type()->get_full_name();
  }

  mstch::node string_value_any() { return to_string(const_value_); }

 private:
  source_manager& source_mgr_;
};

void t_json_experimental_generator::generate_program() {
  out_dir_base_ = "gen-json_experimental";
  const auto* program = get_program();
  set_mstch_factories();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);
  render_to_file(mstch_program, "thrift_ast", program->name() + ".json");
}

void t_json_experimental_generator::set_mstch_factories() {
  mstch_context_.add<json_experimental_program>();
  mstch_context_.add<json_experimental_service>(&source_mgr_);
  mstch_context_.add<json_experimental_function>(&source_mgr_);
  mstch_context_.add<json_experimental_struct>(&source_mgr_);
  mstch_context_.add<json_experimental_field>(&source_mgr_);
  mstch_context_.add<json_experimental_enum>(&source_mgr_);
  mstch_context_.add<json_experimental_enum_value>(&source_mgr_);
  mstch_context_.add<json_experimental_const_value>(&source_mgr_);
}

THRIFT_REGISTER_GENERATOR(json_experimental, "JSON_EXPERIMENTAL", "");

} // namespace
} // namespace compiler
} // namespace thrift
} // namespace apache
