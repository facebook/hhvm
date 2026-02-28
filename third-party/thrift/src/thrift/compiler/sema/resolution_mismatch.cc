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
  auto target_include = std::ranges::find_if(
      includes,
      [&](const t_include* incl) { return incl->get_program() == &target; });

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

// Counts the number of includes in `program` that have the same scope name
// (i.e., same program name) as `scope_name` and are not aliased.
unsigned int count_same_scope_includes(
    const t_program& program, std::string_view scope_name) {
  return std::ranges::count_if(program.includes(), [&](const t_include* incl) {
    return incl->get_program()->name() == scope_name &&
        !incl->alias().has_value();
  });
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

        const auto same_scope_include_count = count_same_scope_includes(
            *mismatch.program, mismatch.program->name());

        bool use_alias = false;
        if (same_scope_include_count > 1) {
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
      [&](scope::scoped_id scoped_id) {
        // A scoped_id like `foo.Bar` is ambiguous because it could mean:
        // 1. Definition `Bar` from a program with scope `foo`
        // 2. Enum value `Bar` from a local enum named `foo`
        // 3. A definition from the local program if the program is named `foo`

        // Case 1: Local resolution found a local enum value, but global
        // resolution found an external definition with the same scoped name.
        if (mismatch.local_node &&
            mismatch.local_node->program() == mismatch.program) {
          if (const auto* enum_val = try_as_enum_value(*mismatch.local_node)) {
            // The local resolution found an enum value (e.g., `Foo.Bar` where
            // `Foo` is a local enum). The global resolution found something
            // different (e.g., a definition from `Foo.thrift`).
            //
            // The identifier is already correctly referencing the local enum
            // value. Warn about the ambiguity but don't suggest changing
            // the identifier since the local resolution is correct.
            if (const auto* enum_ = find_enum_of_value(
                    *mismatch.local_node->program(), *enum_val)) {
              ctx.report(
                  mismatch.id_loc.begin,
                  std::string{IMPLICIT_SCOPE_USAGE_LINT},
                  diagnostic_level::warning,
                  "Scoped identifier '{}' is ambiguous: it refers to local "
                  "enum value '{}.{}' but could be confused with a definition "
                  "from '{}.thrift'. Consider renaming the enum to avoid "
                  "confusion.",
                  mismatch.id,
                  enum_->name(),
                  scoped_id.name,
                  scoped_id.scope);
            }
            return;
          }

          // The local resolution found a local definition, but the global
          // resolution found something else. This happens when a program
          // `foo.thrift` has a local definition `Bar` and uses `foo.Bar` to
          // refer to it, but another included `foo.thrift` also has `Bar`.
          //
          // Recommend using the unscoped name for local definitions.
          ctx.report(
              mismatch.id_loc.begin,
              std::string{IMPLICIT_SCOPE_USAGE_LINT},
              fixit{
                  mismatch.id,
                  std::string{scoped_id.name},
                  mismatch.id_loc.begin},
              diagnostic_level::warning,
              "Scoped identifier '{}' refers to a local definition. Use the "
              "unscoped name '{}' instead to avoid ambiguity with definitions "
              "from other programs.",
              mismatch.id,
              scoped_id.name);
          return;
        }

        // Case 2: Local resolution found nothing or found something in an
        // included program, but global resolution found something different.
        // This typically happens when multiple programs share the same scope
        // name. Warn about using an ambiguous scoped identifier.
        if (mismatch.global_node) {
          const auto same_scope_includes =
              count_same_scope_includes(*mismatch.program, scoped_id.scope);

          if (same_scope_includes > 1) {
            // Multiple programs with the same scope name are included.
            // The ambiguous_include lint already warns on the includes,
            // so here we warn about using the ambiguous identifier.
            ctx.report(
                mismatch.id_loc.begin,
                std::string{IMPLICIT_SCOPE_USAGE_LINT},
                fixit{
                    mismatch.id,
                    fmt::format(
                        "{}.{}",
                        generate_alias(*mismatch.global_node->program()),
                        scoped_id.name),
                    mismatch.id_loc.begin},
                diagnostic_level::warning,
                "Scoped identifier '{}' is ambiguous due to multiple programs "
                "with the same name. Use an aliased scope.",
                mismatch.id);
          }
        }
      },
      [&](scope::enum_id enum_id) {
        // A fully qualified enum_id like `foo.Bar.BAZ` is ambiguous when
        // multiple programs with scope `foo` define an enum `Bar` with value
        // `BAZ`.
        if (mismatch.global_node) {
          const auto same_scope_includes =
              count_same_scope_includes(*mismatch.program, enum_id.scope);

          if (same_scope_includes > 1) {
            // Multiple programs with the same scope name are included.
            // Warn about using the ambiguous enum identifier.
            ctx.report(
                mismatch.id_loc.begin,
                std::string{IMPLICIT_SCOPE_USAGE_LINT},
                fixit{
                    mismatch.id,
                    fmt::format(
                        "{}.{}.{}",
                        generate_alias(*mismatch.global_node->program()),
                        enum_id.enum_name,
                        enum_id.value_name),
                    mismatch.id_loc.begin},
                diagnostic_level::warning,
                "Enum identifier '{}' is ambiguous due to multiple programs "
                "with the same name. Use an aliased scope.",
                mismatch.id);
          }
        }
      });
}

void report_resolution_mismatches(sema_context& ctx, const t_program& program) {
  for (const auto& mismatch : program.global_scope()->resolution_mismatches()) {
    report_resolution_mismatch(ctx, mismatch);
  }
}

} // namespace apache::thrift::compiler
