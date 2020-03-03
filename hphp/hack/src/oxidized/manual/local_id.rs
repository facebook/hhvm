// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

pub type LocalId = (isize, String);

pub fn make_unscoped(name: String) -> LocalId {
    (0, name)
}

pub mod map {
    pub type Map<T> = std::collections::BTreeMap<super::LocalId, T>;
}

pub mod set {
    pub type Set = std::collections::BTreeSet<super::LocalId>;
}

pub fn get_name((_, name): &LocalId) -> &String {
    name
}
