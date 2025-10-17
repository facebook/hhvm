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

#include <thrift/compiler/sema/resolution_mismatch.h>

namespace apache::thrift::compiler {

const std::string_view IMPLICIT_SCOPE_USAGE_LINT = "implicit_scope_usage";
const std::string_view UNSCOPED_ENUM_VALUE_LINT = "unscoped_enum_value";

// Generate an alias for the given thrift program, based on the program's path
std::string generate_alias(const t_program& program) {
  // Use the parent directory name as the alias prefix, e.g.
  // "foo/bar/baz.thrift" -> bar_baz
  std::string::size_type pos = program.path().find_last_of('/');
  std::string parent_dir = "other";
  if (pos != std::string::npos) {
    auto parent_path = program.path().substr(0, pos);
    const auto parent_pos = parent_path.find_last_of('/');
    parent_dir = (parent_pos == std::string::npos)
        ? parent_path
        : parent_path.substr(parent_pos + 1);
  }
  return fmt::format("{}_{}", parent_dir, program.name());
}

// Find an include in `program`, matching the `target` program
const t_include* find_include_for_program(
    const t_program& program, const t_program& target) {
  const auto& includes = program.includes();
  auto target_include = std::find_if(
      includes.begin(), includes.end(), [&](const t_include* incl) {
        return incl->get_program() == &target;
      });
  if (target_include != includes.end()) {
    return *target_include;
  } else {
    return nullptr;
  }
}

const t_enum_value* try_as_enum_value(const t_named& named) {
  if (const auto* enum_val = dynamic_cast<const t_enum_value*>(&named)) {
    return enum_val;
  }
  if (const auto* const_node = dynamic_cast<const t_const*>(&named)) {
    return const_node->value() && const_node->value()->is_enum_value()
        ? const_node->value()->get_enum_value()
        : nullptr;
  }
  return nullptr;
}

const t_enum* find_enum_of_value(
    const t_program& prog, const t_enum_value& val) {
  for (const auto* enum_ : prog.enums()) {
    for (const auto& enum_val : enum_->values()) {
      if (&enum_val == &val) {
        return enum_;
      }
    }
  }
  return nullptr;
}

void report_resolution_mismatch(
    sema_context& ctx, const ResolutionMismatch& mismatch) {
  auto id = scope::identifier{mismatch.id, mismatch.id_loc};

  id.visit(
      [&](scope::unscoped_id unscoped_id) {
        // If `<name>` is ambiguous in a program called <scope>.thrift, that
        // means there's at least one other Thrift program called <scope>.thrift
        // which exports a `<name>` structured definition. Duplicate unscoped
        // enum values being used without their Enum name will be reported by a
        // different validator.
        //
        // The ability to implicitly refer to remote definitions using the
        // same scope as the local program should be codemodded & the
        // resolution logic in the compiler should be updated to only resolve
        // locally for unscoped identifiers.

        if (mismatch.local_node &&
            mismatch.local_node->program() == mismatch.program) {
          // Identifier `<scope>.<definition>` is being overwritten by another
          // program called `<scope>.thrift`. This is non-actionable by the
          // user. This will be resolved by switching the resolution to
          // local-only.

          if (const auto* enum_val = try_as_enum_value(*mismatch.local_node)) {
            // An unscoped reference to an enum value is used, which is being
            // overriden by another program's Enum value of the same (unscoped)
            // name. The local qualified name should be used, i.e.
            // `<enum_name>.<value_name>`.

            if (const auto* enum_ = find_enum_of_value(
                    *mismatch.local_node->program(), *enum_val)) {
              ctx.report(
                  mismatch.id_loc.begin,
                  std::string{UNSCOPED_ENUM_VALUE_LINT},
                  fixit{
                      std::string{unscoped_id.name},
                      fmt::format("{}.{}", enum_->name(), unscoped_id.name),
                      mismatch.id_loc.begin},
                  diagnostic_level::warning,
                  "Using an enum value without the enum name is deprecated. Use a fully qualified name");
            }
          }

          return;
        }

        const auto& includes = mismatch.program->includes();
        const auto same_scope_includes = std::count_if(
            includes.begin(), includes.end(), [&](const t_include* incl) {
              return incl->get_program()->name() == mismatch.program->name() &&
                  !incl->alias().has_value();
            });

        bool use_alias = false;
        if (same_scope_includes > 1) {
          // We have multiple direct includes with the required scope name,
          // so we need to alias the correct one.
          const t_include* target_include = find_include_for_program(
              *mismatch.program, *mismatch.global_node->program());

          if (target_include && !target_include->alias().has_value()) {
            // The include exists without an alias, so we need to add one.
            use_alias = true;
            ctx.report(
                target_include->src_range().end,
                std::string{IMPLICIT_SCOPE_USAGE_LINT},
                fixit{
                    "",
                    fmt::format(
                        "as {}",
                        generate_alias(*mismatch.global_node->program())),
                    target_include->src_range().end},
                diagnostic_level::warning,
                "Multiple programs named '{}', please specify an appropriate include alias to disambiguate.",
                mismatch.program->name());
          } else {
            // The include does not exist. If an include existed with an alias,
            // that should always have priority over global resolution. Using a
            // symbol without a direct include should be caught by another lint.
          }
        }

        // The identifier should use the scoped name explicitly.
        ctx.report(
            id.src_range().begin,
            std::string{IMPLICIT_SCOPE_USAGE_LINT},
            fixit{
                std::string{unscoped_id.name},
                fmt::format(
                    "{}.{}",
                    use_alias ? generate_alias(*mismatch.global_node->program())
                              : mismatch.program->name(),
                    unscoped_id.name),
                id.src_range().begin},

            diagnostic_level::warning,
            "Identifier '{}' refers to a definition in another thrift program. Use an explicit scoped identifier.",
            mismatch.id);
      },
      [&](scope::scoped_id) {
        // TODO (sadroeck): Handle scoped_id mismatch
      },
      [&](scope::enum_id) {
        // TODO (sadroeck): Handle enum_id mismatch
      });
}

void report_resolution_mismatches(sema_context& ctx, const t_program& program) {
  for (const auto& mismatch : program.global_scope()->resolution_mismatches()) {
    report_resolution_mismatch(ctx, mismatch);
  }
}

} // namespace apache::thrift::compiler
