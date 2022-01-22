// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

pub type Hasher = rustc_hash::FxHasher;
pub type BuildHasher = std::hash::BuildHasherDefault<Hasher>;
pub type HashMap<K, V> = rustc_hash::FxHashMap<K, V>;
pub type HashSet<K> = rustc_hash::FxHashSet<K>;
pub type IndexMap<K, V> = indexmap::map::IndexMap<K, V, BuildHasher>;
pub type IndexSet<K> = indexmap::set::IndexSet<K, BuildHasher>;
