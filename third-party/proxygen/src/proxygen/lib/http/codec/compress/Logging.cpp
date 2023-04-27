/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/Logging.h>

#include <sstream>

using std::ostream;
using std::string;
using std::stringstream;
using std::vector;

namespace proxygen {

ostream& operator<<(ostream& os, const std::list<uint32_t>* refset) {
  os << std::endl << '[';
  for (auto& ref : *refset) {
    os << ref << ' ';
  }
  os << ']' << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<HPACKHeader>& v) {
  for (const auto& h : v) {
    os << h.name << ": " << h.value << std::endl;
  }
  os << std::endl;
  return os;
}

string printDelta(const vector<HPACKHeader>& v1,
                  const vector<HPACKHeader>& v2) {
  stringstream out;
  // similar with merge operation
  size_t i = 0;
  size_t j = 0;
  out << std::endl;
  while (i < v1.size() && j < v2.size()) {
    if (v1[i] < v2[j]) {
      if (i > 0 && v1[i - 1] == v1[i]) {
        out << " duplicate " << v1[i] << std::endl;
      } else {
        out << " + " << v1[i] << std::endl;
      }
      i++;
    } else if (v1[i] > v2[j]) {
      out << " - " << v2[j] << std::endl;
      j++;
    } else {
      i++;
      j++;
    }
  }
  while (i < v1.size()) {
    out << " + " << v1[i];
    if (i > 0 && v1[i - 1] == v1[i]) {
      out << " (duplicate)";
    }
    out << std::endl;
    i++;
  }
  while (j < v2.size()) {
    out << " - " << v2[j] << std::endl;
    j++;
  }
  return out.str();
}

} // namespace proxygen
