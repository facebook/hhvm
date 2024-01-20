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

// For some reason we can't iterate children from the root node
// when we parse the file in rust without losing the ref count
// to the ptr, without the children nodes being destructed.
// This lets us get the names of the children nodes so we can
// iterate from the document (config) root.
rust::Vec<rust::String> hdf_child_names(const Hdf& hdf);

}
