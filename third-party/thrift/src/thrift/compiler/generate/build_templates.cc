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
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace {

constexpr std::size_t string_literal_max_size = 16380; // MSVC C2026

fs::path from_components(
    fs::path::const_iterator begin, fs::path::const_iterator end) {
  fs::path tmp;
  while (begin != end) {
    tmp /= *begin++;
  }
  return tmp;
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " templates-dir" << std::endl;
    return 1;
  }

  const auto path = fs::path(argv[1]);

  //  read the templates into memory

  //  using an ordered map to enable deterministic builds
  std::map<std::string, std::string> templates;
  for (const auto& e : fs::recursive_directory_iterator(path)) {
    const auto& p = e.path();
    if (p.extension() == ".mustache" && fs::is_regular_file(p)) {
      using buf_it = std::istreambuf_iterator<char>;
      auto mm = std::mismatch(p.begin(), p.end(), path.begin(), path.end());
      assert(mm.second == path.end());
      auto rel = from_components(mm.first, p.end());
      auto in = std::ifstream(p.string());
      auto buf = std::string(buf_it(in), buf_it());
      templates[rel.generic_string()] = std::move(buf);
    }
  }

  //  emit a c library to stdout

  const char* const tag = "__FBTHRIFT_TAG__";

  std::string const at = "@";
  std::string const endl = "\n";

  std::cout << "//  " << at << "generated" << endl;

  std::cout << endl;

  std::cout << "#include <thrift/compiler/generate/templates.h>" << endl;

  std::cout << endl;

  std::cout << "namespace apache {" << endl;
  std::cout << "namespace thrift {" << endl;
  std::cout << "namespace compiler {" << endl;

  std::cout << endl;

  std::cout << "std::size_t const templates_size = " << templates.size() << ";"
            << endl;

  std::cout << endl;

  std::cout << "std::size_t const templates_name_sizes[] = {" << endl;
  for (const auto& kvp : templates) {
    std::cout << "//  " << kvp.first << endl;
    std::cout << kvp.first.size() << "," << endl;
  }
  std::cout << "};" << endl;

  std::cout << endl;

  std::cout << "char const* const templates_name_datas[] = {" << endl;
  for (const auto& kvp : templates) {
    std::cout << "//  " << kvp.first << endl;
    std::cout << "\"" << kvp.first << "\"," << endl;
  }
  std::cout << "};" << endl;

  std::cout << endl;

  std::cout << "std::size_t const templates_content_sizes[] = {" << endl;
  for (const auto& kvp : templates) {
    std::cout << "//  " << kvp.first << endl;
    std::cout << kvp.second.size() << "," << endl;
  }
  std::cout << "};" << endl;

  std::cout << endl;

  std::cout << "char const* const templates_content_datas[] = {" << endl;
  for (const auto& kvp : templates) {
    std::cout << "//  " << kvp.first << endl;
    std::cout << "(" << std::endl;
    const auto max_size = string_literal_max_size;
    const auto num_pieces = (kvp.second.size() + max_size - 1u) / max_size;
    for (std::size_t i = 0; i < num_pieces; ++i) {
      if (i != 0) {
        std::cout << " /* stitch */ ";
      }
      const auto piece = kvp.second.substr(i * max_size, max_size);
      std::cout << "R\"" << tag << "(" << piece << ")" << tag << "\"";
    }
    std::cout << ")," << endl;
  }
  std::cout << "};" << endl;

  std::cout << endl;

  std::cout << "} // namespace compiler" << endl;
  std::cout << "} // namespace thrift" << endl;
  std::cout << "} // namespace apache" << endl;

  return 0;
}
