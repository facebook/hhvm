// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::serde_cache::CacheOpts;
use crate::SerializingCache;
use dashmap::DashMap;
use hackrs::{
    cache::{Cache, LocalCache},
    decl_parser::DeclParser,
    shallow_decl_provider::ShallowDeclCache,
};
use hash::HashMap;
use indicatif::ParallelProgressIterator;
use pos::{RelativePath, TypeName};
use rayon::iter::{IntoParallelRefIterator, ParallelIterator};
use std::cmp::Eq;
use std::fmt::Debug;
use std::hash::Hash;
use std::sync::Arc;
use ty::reason::Reason;

pub struct NonEvictingCache<K: Hash + Eq, V> {
    cache: DashMap<K, V>,
}

pub struct NonEvictingLocalCache<K: Hash + Eq, V> {
    cache: HashMap<K, V>,
}

impl<K: Hash + Eq, V> Default for NonEvictingCache<K, V> {
    fn default() -> Self {
        Self {
            cache: Default::default(),
        }
    }
}

impl<K: Hash + Eq, V> NonEvictingCache<K, V> {
    pub fn new() -> Self {
        Default::default()
    }
}

impl<K, V> Cache<K, V> for NonEvictingCache<K, V>
where
    K: Copy + Send + Sync + Hash + Eq,
    V: Clone + Send + Sync,
{
    fn get(&self, key: K) -> Option<V> {
        self.cache.get(&key).map(|x| V::clone(&*x))
    }

    fn insert(&self, key: K, val: V) {
        self.cache.insert(key, val);
    }
}

impl<K: Hash + Eq, V> Debug for NonEvictingCache<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NonEvictingCache").finish()
    }
}

impl<K: Hash + Eq, V> Default for NonEvictingLocalCache<K, V> {
    fn default() -> Self {
        Self {
            cache: Default::default(),
        }
    }
}

impl<K: Hash + Eq, V> NonEvictingLocalCache<K, V> {
    pub fn new() -> Self {
        Default::default()
    }
}

impl<K, V> LocalCache<K, V> for NonEvictingLocalCache<K, V>
where
    K: Copy + Hash + Eq,
    V: Clone,
{
    fn get(&self, key: K) -> Option<V> {
        self.cache.get(&key).map(|x| V::clone(&*x))
    }

    fn insert(&mut self, key: K, val: V) {
        self.cache.insert(key, val);
    }
}

impl<K: Hash + Eq, V> Debug for NonEvictingLocalCache<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NonEvictingLocalCache").finish()
    }
}

pub fn make_shallow_decl_cache<R: Reason>(opts: CacheOpts) -> ShallowDeclCache<R> {
    match opts {
        CacheOpts::Serialized(compression_type) => {
            ShallowDeclCache::new(
                Arc::new(SerializingCache::with_compression(compression_type)), // types
                Box::new(SerializingCache::with_compression(compression_type)), // funs
                Box::new(SerializingCache::with_compression(compression_type)), // consts
                Box::new(SerializingCache::with_compression(compression_type)), // modules
                Box::new(SerializingCache::with_compression(compression_type)), // properties
                Box::new(SerializingCache::with_compression(compression_type)), // static_properties
                Box::new(SerializingCache::with_compression(compression_type)), // methods
                Box::new(SerializingCache::with_compression(compression_type)), // static_methods
                Box::new(SerializingCache::with_compression(compression_type)), // constructors
            )
        }
        CacheOpts::Unserialized => ShallowDeclCache::with_no_member_caches(
            Arc::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
            Box::new(NonEvictingCache::default()),
        ),
    }
}

pub fn make_non_evicting_shallow_decl_cache<R: Reason>() -> ShallowDeclCache<R> {
    make_shallow_decl_cache(CacheOpts::Unserialized)
}

pub fn populate_shallow_decl_cache<R: Reason>(
    shallow_decl_cache: &ShallowDeclCache<R>,
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
            shallow_decl_cache.add_decls(decls);
            summary
                .classes()
                .map(|(class, _hash)| TypeName::new(class))
                .collect::<Vec<_>>()
                .into_iter()
        })
        .collect()
}
