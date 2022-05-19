// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::DashMap;
use parking_lot::RwLock;
use std::fmt::Debug;
use std::hash::Hash;

/// A threadsafe cache, intended for global decl storage. The key type is
/// intended to be a `Symbol` or tuple of `Symbol`s, and the value type is
/// intended to be a ref-counted pointer (like `Arc` or `Hc`).
pub trait Cache<K: Copy, V>: Debug + Send + Sync {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&self, key: K, val: V);
}

/// A thread-local cache, intended for decl caching in typechecker workers. The
/// key type is intended to be a `Symbol` or tuple of `Symbol`s, and the value
/// type is intended to be a ref-counted pointer (like `Rc`).
pub trait LocalCache<K: Copy, V>: Debug {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&mut self, key: K, val: V);
}

pub struct ChangesCache<K, V, F> {
    stack: RwLock<Vec<DashMap<K, Option<V>>>>,
    fallback: F,
}

impl<K: Copy + Hash + Eq, V: Clone, F: Cache<K, V>> ChangesCache<K, V, F> {
    pub fn new(fallback: F) -> Self {
        Self {
            stack: RwLock::new(vec![Default::default()]),
            fallback,
        }
    }

    pub fn get(&self, key: K) -> Option<V> {
        for cache in self.stack.read().iter() {
            if let Some(val_opt) = cache.get(&key) {
                return val_opt.clone();
            }
        }
        self.fallback.get(key)
    }

    pub fn insert(&self, key: K, val: V) {
        let stack = self.stack.read();
        let cache = stack.last().expect("empty stack");
        cache.insert(key, Some(val));
    }

    pub fn push_local_changes(&self) {
        self.stack.write().push(Default::default())
    }

    pub fn pop_local_changes(&self) {
        self.stack.write().pop();
    }

    pub fn remove_batch<I: Iterator<Item = K>>(&self, keys: I) {
        let stack = self.stack.read();
        let cache = stack.last().expect("empty stack");
        for key in keys {
            cache.insert(key, None);
        }
    }
}

impl<K: Copy, V, F> std::fmt::Debug for ChangesCache<K, V, F> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ChangesCache").finish()
    }
}
