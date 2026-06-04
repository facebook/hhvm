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

#include <thrift/compiler/sema/schematizer.h>

#include <filesystem>
#include <string>
#include <string_view>
#include <fmt/core.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#include <thrift/compiler/ast/t_global_scope.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/source_location.h>

namespace apache::thrift::compiler::detail {

// Returns the `TypeUri` type & the corresponding Uri value for the given node
schematizer::resolved_uri schematizer::calculate_uri(
    const t_named& node, const bool use_hash) {
  if (use_hash) {
    return {"definitionKey", identify_definition(node)};
  }
  if (!node.uri().empty()) {
    return {"uri", node.uri()};
  }
  if (node.program()) {
    return {"scopedName", node.program()->scoped_name(node)};
  }
  return {"scopedName", node.name()};
}

std::string_view schematizer::program_checksum(const t_program& program) {
  if (auto it = program_checksums_.find(&program);
      it != program_checksums_.end()) {
    return it->second;
  }
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[MD5_DIGEST_LENGTH];
  auto val = sm_.get_file(program.path())->text;
  EVP_Digest(val.data(), val.size(), hash, nullptr, EVP_md5(), nullptr);
  return (
      program_checksums_[&program] =
          std::string(reinterpret_cast<const char*>(hash), sizeof(hash)));
}

size_t schematizer::definition_identifier_length() {
  return 16;
}

std::string schematizer::identify_definition(const t_named& node) {
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[SHA256_DIGEST_LENGTH];
  auto val = fmt::format(
      "{}{}{}",
      program_checksum(*node.program()),
      node.program()->path(),
      node.name());
  SHA256(reinterpret_cast<const unsigned char*>(val.c_str()), val.size(), hash);
  return std::string(
      reinterpret_cast<const char*>(hash), definition_identifier_length());
}

int64_t schematizer::identify_program(const t_program& node) {
  auto checksum = program_checksum(node);
  auto path =
      std::filesystem::path(node.path()).lexically_normal().generic_string();
  // @lint-ignore CLANGTIDY facebook-hte-CArray
  unsigned char hash[SHA256_DIGEST_LENGTH];
  auto val = fmt::format("{}{}", checksum, path);
  SHA256(reinterpret_cast<const unsigned char*>(val.c_str()), val.size(), hash);
  int64_t ret;
  memcpy(&ret, hash, sizeof(ret));
  return ret;
}

std::string schematizer::schema_const_name(
    source_manager& sm, const t_program& node) {
  schematizer s(*node.global_scope(), sm, {});
  return fmt::format(
      "_fbthrift_schema_{:x}", static_cast<uint64_t>(s.identify_program(node)));
}

} // namespace apache::thrift::compiler::detail
