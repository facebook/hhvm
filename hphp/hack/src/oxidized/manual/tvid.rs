// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub mod map {
    pub type Map<T> = std::collections::BTreeMap<isize, T>;
}

pub mod set {
    pub type Set = std::collections::BTreeSet<isize>;
}
