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

#include <thrift/compiler/generate/t_mstch_generator.h>
#include <thrift/compiler/generate/templates.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/mstch_compat.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/standard_library.h>

using namespace std;

namespace fs = std::filesystem;

namespace apache::thrift::compiler {

whisker::source_manager t_mstch_generator::template_source_manager() const {
  return whisker::source_manager{
      std::make_unique<in_memory_source_manager_backend>(
          create_templates_by_path())};
}

bool t_mstch_generator::has_option(const std::string& option) const {
  return has_compiler_option(option);
}

std::optional<std::string> t_mstch_generator::get_option(
    const std::string& option) const {
  if (std::optional<std::string_view> found = get_compiler_option(option)) {
    return std::string{*found};
  }
  return std::nullopt;
}

whisker::map::raw t_mstch_generator::globals(prototype_database& proto) const {
  auto options = render_options();
  whisker::map::raw result = t_whisker_generator::globals(proto);
  for (const auto& undefined_name : options.allowed_undefined_variables) {
    result.insert({undefined_name, whisker::make::null});
  }
  return result;
}

t_mstch_generator::strictness_options t_mstch_generator::strictness() const {
  strictness_options strict;
  // Our legacy code has a ton of non-boolean conditionals
  strict.boolean_conditional = false;
  // Our legacy code relies on printing null as empty string
  strict.printable_types = false;
  // Undefined variables are covered by globals(...)
  strict.undefined_variables = true;
  return strict;
}

std::string t_mstch_generator::render(
    const std::string& template_name, const mstch::node& context) {
  return render(
      template_name,
      whisker::from_mstch(context, render_state().diagnostic_engine));
}

void t_mstch_generator::render_to_file(
    const mstch::map& context,
    const std::string& template_name,
    const std::filesystem::path& path) {
  write_to_file(path, render(template_name, context));
}

const std::shared_ptr<mstch_base>& t_mstch_generator::cached_program(
    const t_program* program) {
  const auto& id = program->path();
  auto itr = mstch_context_.program_cache.find(id);
  if (itr == mstch_context_.program_cache.end()) {
    itr = mstch_context_.program_cache
              .emplace(
                  id,
                  mstch_context_.program_factory->make_mstch_object(
                      program, mstch_context_))
              .first;
  }
  return itr->second;
}

} // namespace apache::thrift::compiler
