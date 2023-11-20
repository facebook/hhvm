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

#include <thrift/compiler/generate/t_concat_generator.h>

#include <cinttypes>
#include <cstdio>
#include <fstream>
#include <iterator>

#include <openssl/sha.h>

#include <thrift/compiler/generate/t_generator.h>

using namespace std;

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Top level program generation function. Calls the generator subclass methods
 * for preparing file streams etc. then iterates over all the parts of the
 * program to perform the correct actions.
 *
 * @param program The thrift program to compile into C++ source
 */
void t_concat_generator::generate_program() {
  // Initialize the generator
  init_generator();

  // Generate enums
  for (const auto* tenum : program_->enums()) {
    generate_enum(tenum);
  }

  const std::vector<t_structured*>& structured_definitions =
      program_->structured_definitions();

  // Generate forward declarations. Typedefs may use these
  for (const t_structured* object : structured_definitions) {
    generate_forward_declaration(object);
  }

  // Generate typedefs
  for (const auto* ttypedef : program_->typedefs()) {
    generate_typedef(ttypedef);
  }

  // Generate constants
  auto consts = program_->consts();
  generate_consts(consts);

  // Generate structs, exceptions, and unions in declared order
  for (const t_structured* object : structured_definitions) {
    if (object->is_exception()) {
      generate_xception(object);
    } else {
      generate_struct(object);
    }
  }

  // Generate services
  auto services = program_->services();
  for (const auto* tservice : program_->services()) {
    service_name_ = get_service_name(tservice);
    generate_service(tservice);
  }

  // Close the generator
  close_generator();
}

string t_concat_generator::escape_string(const string& in) const {
  string result = "";
  for (string::const_iterator it = in.begin(); it < in.end(); it++) {
    std::map<char, std::string>::const_iterator res = escape_.find(*it);
    if (res != escape_.end()) {
      result.append(res->second);
    } else {
      result.push_back(*it);
    }
  }
  return result;
}

void t_concat_generator::generate_consts(vector<t_const*> consts) {
  for (const auto* tconst : consts) {
    generate_const(tconst);
  }
}

void t_concat_generator::generate_docstring_comment(
    ofstream& out,
    const string& comment_start,
    const string& line_prefix,
    const string& contents,
    const string& comment_end) {
  if (comment_start != "")
    indent(out) << comment_start;
  stringstream docs(contents, ios_base::in);
  while (!docs.eof()) {
    char line[1024];
    docs.getline(line, 1024);
    if (strlen(line) > 0 || !docs.eof()) { // skip the empty last line
      indent(out) << line_prefix << line << std::endl;
    }
  }
  if (comment_end != "")
    indent(out) << comment_end;
}

std::string t_concat_generator::generate_structural_id(
    const t_structured* tstruct) {
  // Generate a string that contains all the members' information:
  // key, name, type and req.
  vector<std::string> fields_str;
  constexpr auto delim = ",";
  for (const auto& field : tstruct->fields()) {
    std::stringstream ss_field;
    ss_field << field.id() << delim << field.name() << delim
             << field.type()->get_name() << delim << (int)(field.get_req());
    fields_str.push_back(ss_field.str());
  }

  // Sort the vector of keys.
  std::sort(fields_str.begin(), fields_str.end());

  // Generate a hashable string: each member key is delimited by ":".
  std::stringstream ss;
  copy(fields_str.begin(), fields_str.end(), ostream_iterator<string>(ss, ":"));
  std::string hashable_keys_list = ss.str();

  // Hash the string and generate a portable hash number.
  unsigned char buf[SHA_DIGEST_LENGTH] = {};
  SHA1(
      reinterpret_cast<const unsigned char*>(hashable_keys_list.data()),
      hashable_keys_list.size(),
      buf);
  uint64_t hash = 0;
  std::memcpy(&hash, &buf, sizeof(hash));
#if !defined(_WIN32) && __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
  hash = __builtin_bswap64(hash);
#endif
  hash &= 0x7FFFFFFFFFFFFFFFull; // 63 bits

  // Generate a readable number.
  char structural_id[21];
  snprintf(structural_id, sizeof(structural_id), "%" PRIu64, hash);

  return structural_id;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
