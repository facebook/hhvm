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

#include <boost/algorithm/string.hpp>
#include <thrift/compiler/ast/t_program.h>

#include <cctype>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace apache {
namespace thrift {
namespace compiler {

void t_program::add_definition(std::unique_ptr<t_named> definition) {
  assert(definition != nullptr);

  scope_->add_definition(scope_name(*definition), definition.get());

  if (!definition->explicit_uri()) {
    // Resolve Thrift URI.
    if (auto* uri = definition->find_annotation_or_null("thrift.uri")) {
      definition->set_uri(*uri); // Explicit from annotation.
    } else { // Inherit from package.
      definition->set_uri(
          package_.get_uri(definition->name()), /*is_explicit=*/false);
    }
  }

  // Add to scope
  if (const t_named* existing = scope_->add_def(*definition)) {
    // TODO(afuller): Propagate the existing diff up, and
    // report a diagnostic immediately, instead of doing it after
    // in `validate_uri_uniqueness`.
    // return existing; // Confliciting definition!.
  }

  // Index the node.
  if (auto* node = dynamic_cast<t_exception*>(definition.get())) {
    structured_definitions_.push_back(node);
    exceptions_.push_back(node);
  } else if (auto* node = dynamic_cast<t_union*>(definition.get())) {
    structured_definitions_.push_back(node);
    structs_and_unions_.push_back(node);
  } else if (auto* node = dynamic_cast<t_struct*>(definition.get())) {
    structured_definitions_.push_back(node);
    structs_and_unions_.push_back(node);
  } else if (auto* node = dynamic_cast<t_interaction*>(definition.get())) {
    interactions_.push_back(node);
  } else if (auto* node = dynamic_cast<t_service*>(definition.get())) {
    services_.push_back(node);
  } else if (auto* node = dynamic_cast<t_enum*>(definition.get())) {
    enums_.push_back(node);
  } else if (auto* node = dynamic_cast<t_typedef*>(definition.get())) {
    typedefs_.push_back(node);
  } else if (auto* node = dynamic_cast<t_const*>(definition.get())) {
    consts_.push_back(node);
  }

  // Transfer ownership of the definition.
  definitions_.push_back(std::move(definition));
}

const std::string& t_program::get_namespace(const std::string& language) const {
  auto pos = namespaces_.find(language);
  static const auto& kEmpty = *new std::string();
  return (pos != namespaces_.end() ? pos->second : kEmpty);
}

std::vector<std::string> t_program::gen_namespace_or_default(
    const std::string& language, namespace_config config) const {
  std::vector<std::string> ret;

  auto pos = namespaces_.find(language);
  if (pos != namespaces_.end()) {
    if (!pos->second.empty()) {
      split(ret, pos->second, boost::algorithm::is_any_of("."));
    }
    return ret;
  }

  // namespace is not defined explicitly. Generating it from package name

  if (package().empty()) {
    return ret;
  }

  if (!config.no_domain) {
    ret = package().domain();

    if (config.no_top_level_domain && !ret.empty()) {
      ret.pop_back();
    }

    // We use reverse domain name notation
    std::reverse(ret.begin(), ret.end());
  }

  const auto& path = package().path();
  ret.insert(ret.end(), path.begin(), path.end());
  if (config.no_filename && !ret.empty() && name() == ret.back()) {
    ret.pop_back();
  }

  return ret;
}

void t_program::set_include_prefix(std::string include_prefix) {
  include_prefix_ = std::move(include_prefix);

  int len = include_prefix_.size();
  if (len > 0 && include_prefix_[len - 1] != '/') {
    include_prefix_ += '/';
  }
}

std::string t_program::compute_name_from_file_path(std::string path) {
  std::string::size_type slash = path.find_last_of("/\\");
  if (slash != std::string::npos) {
    path = path.substr(slash + 1);
  }
  std::string::size_type dot = path.rfind('.');
  if (dot != std::string::npos) {
    path = path.substr(0, dot);
  }
  return path;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
