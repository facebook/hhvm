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

pub enum Lookup<T> {
    Present(T),
    Absent,
    Unknown,
}

pub struct ChangesCache<K, V> {
    stack: RwLock<Vec<DashMap<K, Option<V>>>>,
}

impl<K: Copy + Hash + Eq, V: Clone> ChangesCache<K, V> {
    pub fn new() -> Self {
        Self {
            stack: RwLock::new(vec![Default::default()]),
        }
    }

    pub fn get(&self, key: K) -> Lookup<V> {
        for cache in self.stack.read().iter() {
            if let Some(val_opt) = cache.get(&key) {
                return match &*val_opt {
                    Some(v) => Lookup::Present(v.clone()),
                    None => Lookup::Absent,
                };
            }
        }
        Lookup::Unknown
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
        let mut stack = self.stack.write();
        stack.pop();
        assert!(stack.len() > 0);
    }

    pub fn remove_batch<I: Iterator<Item = K>>(&self, keys: I) {
        let stack = self.stack.read();
        let cache = stack.last().expect("empty stack");
        for key in keys {
            cache.insert(key, None);
        }
    }
}

impl<K: Copy, V> std::fmt::Debug for ChangesCache<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ChangesCache").finish()
    }
}
