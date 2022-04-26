// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
#pragma once
#include "hphp/util/hdf.h"
#include "rust/cxx.h"
#include <memory>

namespace HPHP {

std::unique_ptr<Hdf> hdf_new();
std::unique_ptr<Hdf> hdf_new_child(const Hdf& hdf, const std::string& name);
std::unique_ptr<Hdf> hdf_first_child(const Hdf& hdf);
std::unique_ptr<Hdf> hdf_next(const Hdf& hdf);
rust::String hdf_name(const Hdf& hdf);

}
