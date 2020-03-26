// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
pub use arena_collections::map::Map;
pub use arena_collections::set::Set;

pub type ISet<'a> = Set<'a, isize>;

pub type SSet<'a> = Set<'a, &'a str>;

pub type IMap<'a, V> = Map<'a, isize, V>;

pub type SMap<'a, V> = Map<'a, &'a str, V>;
