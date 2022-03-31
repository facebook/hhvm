// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::DashMap;
use hackrs::{
    cache::{Cache, LocalCache},
    shallow_decl_provider::ShallowDeclCache,
};
use hash::HashMap;
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

pub fn make_non_eviction_shallow_decl_cache<R: Reason>() -> ShallowDeclCache<R> {
    ShallowDeclCache::with_no_member_caches(
        Arc::new(NonEvictingCache::default()),
        Box::new(NonEvictingCache::default()),
        Box::new(NonEvictingCache::default()),
        Box::new(NonEvictingCache::default()),
    )
}
