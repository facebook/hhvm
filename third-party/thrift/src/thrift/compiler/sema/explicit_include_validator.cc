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

#include <set>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/sema/explicit_include_validator.h>

namespace apache::thrift::compiler {

namespace {

std::string concatenate_relative_include(
    const t_program* include, const t_program* this_program) {
  // `\\` for WiNdOwS cOmPaTiBiLiTy
  size_t last_slash = this_program->path().find_last_of("/\\");
  return fmt::format(
      "{}/{}", this_program->path().substr(0, last_slash), include->path());
}

bool program_has_include(sema_context& ctx, const t_program* type_program) {
  const t_program* this_program = &ctx.program();
  if (this_program == type_program) {
    return true;
  }
  return ctx.cache().get(*type_program, [this_program, type_program]() {
    for (const t_include* include : this_program->includes()) {
      // note: comparing pointers fails when the include is a relative path
      const t_program* include_program = include->get_program();
      if (include_program == type_program ||
          concatenate_relative_include(include_program, this_program) ==
              type_program->path()) {
        return std::make_unique<bool>(true);
      }
    }
    return std::make_unique<bool>(false);
  });
}

const t_node& get_include_insert_node(const t_program& program) {
  // Add the new include to the top of the includes list, if there are
  // includes. Otherwise, just add it to the top of the file ¯\_(ツ)_/¯.
  if (!program.includes().empty()) {
    return *program.includes().front();
  }
  return program;
}

void report_missing(sema_context& ctx, const t_program* include) {
  auto implicit_includes_ = ctx.cache().get(ctx.program(), []() {
    return std::make_unique<
        std::shared_ptr<std::set<const apache::thrift::compiler::t_program*>>>(
        std::make_shared<
            std::set<const apache::thrift::compiler::t_program*>>());
  });
  if (implicit_includes_->find(include) != implicit_includes_->end()) {
    return;
  }
  implicit_includes_->insert(include);
  ctx.report(
      get_include_insert_node(ctx.program()),
      std::string(implicit_include_rule_name),
      fixit("", fmt::format("include \"{}\"\n", include->path())),
      diagnostic_level::error,
      "Your thrift file depends on a type that it did not include. Please add the following include.");
}

void visit_type(sema_context& ctx, const t_named& src, const t_type& type) {
  if (type.program() != nullptr && !program_has_include(ctx, type.program())) {
    report_missing(ctx, type.program());
  } else if (type.is<t_list>()) {
    const auto& list = *dynamic_cast<const t_list*>(&type);
    visit_type(ctx, src, *list.get_elem_type());
  } else if (type.is<t_set>()) {
    const auto& set = *dynamic_cast<const t_set*>(&type);
    visit_type(ctx, src, *set.get_elem_type());
  } else if (type.is<t_map>()) {
    const auto& map = *dynamic_cast<const t_map*>(&type);
    visit_type(ctx, src, *map.get_key_type());
    visit_type(ctx, src, *map.get_val_type());
  }
}
} // namespace

void add_explicit_include_validators(
    ast_validator& validator,
    bool skip_annotations,
    bool skip_service_extends) {
  /*
   * This function adds visitors that collectively apply the validation logic to
   * each type reference in the IDL file.
   * A type reference is a use of a type's identifier other than
   * to define that type, and is usually but not always represented as
   * t_type_ref in the AST.
   */

  // Structured types: field types
  validator.add_field_visitor([](sema_context& ctx, const t_field& f) {
    if (f.is_injected()) {
      return;
    }
    visit_type(ctx, f, *f.get_type());
  });

  // Typedefs: underlying type
  validator.add_typedef_visitor([](sema_context& ctx, const t_typedef& td) {
    visit_type(ctx, td, *td.get_type());
  });

  // Functions: return types, exceptions, stream/sink types
  validator.add_function_visitor([](sema_context& ctx, const t_function& f) {
    visit_type(ctx, f, *f.return_type());
    if (const t_type_ref& interaction = f.interaction()) {
      visit_type(ctx, f, *interaction);
    }
  });
  validator.add_function_param_visitor(
      [](sema_context& ctx, const t_field& param) {
        visit_type(ctx, param, *param.get_type());
      });
  validator.add_thrown_exception_visitor(
      [](sema_context& ctx, const t_field& ex) {
        visit_type(ctx, ex, *ex.get_type());
      });
  validator.add_stream_visitor([](sema_context& ctx, const t_stream& s) {
    visit_type(ctx, static_cast<const t_named&>(*ctx.parent()), *s.elem_type());
  });
  validator.add_sink_visitor([](sema_context& ctx, const t_sink& s) {
    if (s.final_response_type()) {
      visit_type(
          ctx,
          static_cast<const t_named&>(*ctx.parent()),
          *s.final_response_type());
    }
    visit_type(ctx, static_cast<const t_named&>(*ctx.parent()), *s.elem_type());
  });

  // Services: base service
  if (!skip_service_extends) {
    validator.add_service_visitor([](sema_context& ctx, const t_service& svc) {
      if (const t_service* extends = svc.extends()) {
        visit_type(ctx, svc, *extends);
      }
    });
  }

  // Constants: value type
  validator.add_const_visitor([](sema_context& ctx, const t_const& c) {
    visit_type(ctx, c, *c.type());
  });

  // All definitions: annotations
  if (skip_annotations) {
    return;
  }
  validator.add_named_visitor([](sema_context& ctx, const t_named& n) {
    // Temporary workaround for patch generator
    if (n.generated() ||
        (dynamic_cast<const t_named*>(ctx.parent()) &&
         static_cast<const t_named&>(*ctx.parent()).generated())) {
      return;
    }

    for (const t_const& anno : n.structured_annotations()) {
      visit_type(ctx, n, *anno.type());
    }
  });
}

} // namespace apache::thrift::compiler
