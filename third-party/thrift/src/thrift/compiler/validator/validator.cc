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

#include <thrift/compiler/validator/validator.h>

#include <unordered_set>

namespace apache {
namespace thrift {
namespace compiler {

// fill_validators - the validator registry
//
// This is where all concrete validator types must be registered.
static void fill_validators(validator_list& vs) {
  vs.add<struct_names_uniqueness_validator>();
  vs.add<interactions_validator>();
}

void validator_list::traverse(t_program* const program) {
  auto pointers = std::vector<visitor*>{};
  for (const auto& v : validators_) {
    pointers.push_back(v.get());
  }
  interleaved_visitor(pointers).traverse(program);
}

void validator::validate(t_program* program, diagnostics_engine& diags) {
  auto validators = validator_list(diags);
  fill_validators(validators);
  validators.traverse(program);
}

bool struct_names_uniqueness_validator::visit(t_program* p) {
  std::unordered_set<std::string> seen;
  for (auto* object : p->objects()) {
    if (!seen.emplace(object->name()).second) {
      report_error(*object, "Redefinition of type `{}`.", object->name());
    }
  }
  for (auto* interaction : p->interactions()) {
    if (!seen.emplace(interaction->name()).second) {
      report_error(
          *interaction, "Redefinition of type `{}`.", interaction->name());
    }
  }
  return true;
}

bool interactions_validator::visit(t_program* p) {
  for (auto* interaction : p->interactions()) {
    for (auto* func : interaction->get_functions()) {
      auto ret = func->get_returntype();
      if (ret->is_service() &&
          static_cast<const t_service*>(ret)->is_interaction()) {
        report_error(*func, "Nested interactions are forbidden.");
      }
    }
  }
  return true;
}

bool interactions_validator::visit(t_service* s) {
  std::unordered_set<std::string> seen;
  for (auto* func : s->get_functions()) {
    auto ret = func->get_returntype();
    if (func->is_interaction_constructor()) {
      if (!ret->is_service() ||
          !static_cast<const t_service*>(ret)->is_interaction()) {
        report_error(*func, "Only interactions can be performed.");
        continue;
      }
    }

    if (!func->is_interaction_constructor()) {
      continue;
    }

    if (!seen.emplace(ret->name()).second) {
      report_error(
          *func,
          "Service `{}` has multiple methods for creating interaction `{}`.",
          s->name(),
          ret->name());
    }
  }
  return true;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
