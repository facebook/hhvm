// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::serde_store::StoreOpts;
use crate::SerializingStore;
use dashmap::DashMap;
use datastore::{LocalStore, Store};
use hackrs::{decl_parser::DeclParser, shallow_decl_provider::ShallowDeclStore};
use hash::HashMap;
use indicatif::ParallelProgressIterator;
use pos::{RelativePath, TypeName};
use rayon::iter::{IntoParallelRefIterator, ParallelIterator};
use std::cmp::Eq;
use std::fmt::Debug;
use std::hash::Hash;
use std::sync::Arc;
use ty::reason::Reason;

pub struct NonEvictingStore<K: Hash + Eq, V> {
    store: DashMap<K, V>,
}

pub struct NonEvictingLocalStore<K: Hash + Eq, V> {
    store: HashMap<K, V>,
}

impl<K: Hash + Eq, V> Default for NonEvictingStore<K, V> {
    fn default() -> Self {
        Self {
            store: Default::default(),
        }
    }
}

impl<K: Hash + Eq, V> NonEvictingStore<K, V> {
    pub fn new() -> Self {
        Default::default()
    }
}

impl<K, V> Store<K, V> for NonEvictingStore<K, V>
where
    K: Copy + Send + Sync + Hash + Eq,
    V: Clone + Send + Sync,
{
    fn get(&self, key: K) -> Option<V> {
        self.store.get(&key).map(|x| V::clone(&*x))
    }

    fn insert(&self, key: K, val: V) {
        self.store.insert(key, val);
    }
}

impl<K: Hash + Eq, V> Debug for NonEvictingStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NonEvictingStore").finish()
    }
}

impl<K: Hash + Eq, V> Default for NonEvictingLocalStore<K, V> {
    fn default() -> Self {
        Self {
            store: Default::default(),
        }
    }
}

impl<K: Hash + Eq, V> NonEvictingLocalStore<K, V> {
    pub fn new() -> Self {
        Default::default()
    }
}

impl<K, V> LocalStore<K, V> for NonEvictingLocalStore<K, V>
where
    K: Copy + Hash + Eq,
    V: Clone,
{
    fn get(&self, key: K) -> Option<V> {
        self.store.get(&key).map(|x| V::clone(&*x))
    }

    fn insert(&mut self, key: K, val: V) {
        self.store.insert(key, val);
    }
}

impl<K: Hash + Eq, V> Debug for NonEvictingLocalStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NonEvictingLocalStore").finish()
    }
}

pub fn make_shallow_decl_store<R: Reason>(opts: StoreOpts) -> ShallowDeclStore<R> {
    match opts {
        StoreOpts::Serialized(compression_type) => {
            ShallowDeclStore::new(
                Arc::new(SerializingStore::with_compression(compression_type)), // types
                Box::new(SerializingStore::with_compression(compression_type)), // funs
                Box::new(SerializingStore::with_compression(compression_type)), // consts
                Box::new(SerializingStore::with_compression(compression_type)), // modules
                Box::new(SerializingStore::with_compression(compression_type)), // properties
                Box::new(SerializingStore::with_compression(compression_type)), // static_properties
                Box::new(SerializingStore::with_compression(compression_type)), // methods
                Box::new(SerializingStore::with_compression(compression_type)), // static_methods
                Box::new(SerializingStore::with_compression(compression_type)), // constructors
            )
        }
        StoreOpts::Unserialized => ShallowDeclStore::with_no_member_stores(
            Arc::new(NonEvictingStore::default()),
            Box::new(NonEvictingStore::default()),
            Box::new(NonEvictingStore::default()),
            Box::new(NonEvictingStore::default()),
        ),
    }
}

pub fn make_non_evicting_shallow_decl_store<R: Reason>() -> ShallowDeclStore<R> {
    make_shallow_decl_store(StoreOpts::Unserialized)
}

pub fn populate_shallow_decl_store<R: Reason>(
    shallow_decl_store: &ShallowDeclStore<R>,
    decl_parser: DeclParser<R>,
    filenames: &[RelativePath],
) -> Vec<TypeName> {
    let len = filenames.len();
    filenames
        .par_iter()
        .progress_count(len as u64)
        .flat_map_iter(|path| {
            let (mut decls, summary) = decl_parser.parse_and_summarize(*path).unwrap();
            decls.reverse(); // To match OCaml behavior for name collisions
            shallow_decl_store.add_decls(decls);
            summary
                .classes()
                .map(|(class, _hash)| TypeName::new(class))
                .collect::<Vec<_>>()
                .into_iter()
        })
        .collect()
}
