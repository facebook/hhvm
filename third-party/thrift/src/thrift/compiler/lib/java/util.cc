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

#include <thrift/compiler/lib/java/util.h>

#include <cassert>
#include <cctype>
#include <boost/algorithm/string.hpp>

namespace apache {
namespace thrift {
namespace compiler {
namespace java {

std::string mangle_java_name(const std::string& ref, bool capitalize) {
  std::ostringstream res;
  // When we are capitalizing, initialize upCase to true in order to force first
  // character to upper case
  bool upcase = capitalize;
  // On the other hand, force first character to lowercase, unless both first
  // AND second characters are uppercase (this is so we don't mess up any
  // poorly-named method that might start with acronyms)
  bool acronym = ref.size() > 1 && std::isupper(ref[0]) && std::isupper(ref[1]);
  bool downcase = !capitalize && !acronym;
  for (typename std::string::size_type i = 0; i < ref.size(); ++i) {
    if (ref[i] == '_' && i < ref.size() - 1) {
      upcase = true;
      continue;
    } else {
      char ch = ref[i];
      if (downcase) {
        ch = std::tolower(ch);
      }
      if (upcase) {
        ch = std::toupper(ch);
      }
      res << ch;
      upcase = false;
      downcase = false;
    }
  }
  return res.str();
}

std::string mangle_java_constant_name(const std::string& ref) {
  std::ostringstream res;
  bool lowercase = false;
  for (typename std::string::size_type i = 0; i < ref.size(); ++i) {
    char ch = ref[i];
    if (std::isupper(ch)) {
      if (lowercase) {
        res << '_';
      }
      res << static_cast<char>(std::toupper(ch));
      lowercase = false;
    } else if (std::islower(ch)) {
      res << static_cast<char>(std::toupper(ch));
      lowercase = true;
    } else {
      // Not a letter, just emit it
      res << ch;
    }
  }
  return res.str();
}

std::string quote_java_string(const std::string& unescaped) {
  std::ostringstream quoted;
  quoted << '\"';
  for (unsigned char c : unescaped) {
    switch (c) {
      case '\\':
        quoted << "\\\\";
        break;
      case '"':
        quoted << "\\\"";
        break;
      case '\n':
        quoted << "\\n";
        break;
      default: {
        if (c < 0x20) {
          quoted << fmt::format("\\x{:02x}", c);
        } else if (c >= 0xf8) {
          quoted << fmt::format("\\u{:04x}", c);
        } else {
          quoted << c;
        }
        break;
      }
    }
  }
  quoted << '\"';

  return quoted.str();
}

std::string package_to_path(std::string package) {
  if (boost::algorithm::contains(package, "/")) {
    std::ostringstream err;
    err << "\"" << package << "\" is not a valid Java package name";
    throw std::runtime_error{err.str()};
  }
  boost::algorithm::replace_all(package, ".", "/");
  return package;
}

} // namespace java
} // namespace compiler
} // namespace thrift
} // namespace apache
