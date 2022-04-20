// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#pragma once
#include "hphp/util/hdf.h"
#include "rust/cxx.h"
#include <memory>

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

}
