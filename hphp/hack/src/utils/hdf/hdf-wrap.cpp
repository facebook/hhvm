// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#include "hphp/hack/src/utils/hdf/hdf-wrap.h"

namespace HPHP {

std::unique_ptr<Hdf> hdf_new() {
  return std::make_unique<Hdf>();
}

std::unique_ptr<Hdf> hdf_new_child(const Hdf& hdf, const std::string& name) {
  return std::make_unique<Hdf>(&hdf, name.c_str());
}

std::unique_ptr<Hdf> hdf_first_child(const Hdf& hdf) {
  return std::make_unique<Hdf>(hdf.firstChild());
}

std::unique_ptr<Hdf> hdf_next(const Hdf& hdf) {
  return std::make_unique<Hdf>(hdf.next());
}

rust::String hdf_name(const Hdf& hdf) {
  return rust::String{hdf.getName()};
}

rust::Vec<rust::String> hdf_child_names(const Hdf& hdf) {
  auto keys = rust::Vec<rust::String>{};
  for (auto child = hdf.firstChild(false); child.exists(); child = child.next(false)) {
    keys.push_back(rust::String(child.getName()));
  }
  return keys;
}

rust::Vec<rust::String> hdf_get_string_vec(const Hdf& hdf) {
  auto values = std::vector<std::string>{};
  hdf.configGet(values);
  auto ret = rust::Vec<rust::String>{};
  for (auto value : values) {
    ret.push_back(rust::String(value));
  }
  return ret;
}

} // namespace HPHP
