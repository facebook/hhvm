// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use dashmap::DashMap;
use parking_lot::RwLock;
use std::fmt::Debug;
use std::hash::Hash;

/// A threadsafe datastore, intended for global decl storage. The key type is
/// intended to be a `Symbol` or tuple of `Symbol`s, and the value type is
/// intended to be a ref-counted pointer (like `Arc` or `Hc`).
pub trait Store<K: Copy, V>: Debug + Send + Sync {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&self, key: K, val: V);
}

/// A thread-local datastore, intended for decl caching in typechecker workers.
/// The key type is intended to be a `Symbol` or tuple of `Symbol`s, and the
/// value type is intended to be a ref-counted pointer (like `Rc`).
pub trait LocalStore<K: Copy, V>: Debug {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&mut self, key: K, val: V);
}

/// A stack of temporary changes on top of a fallback data store.
pub struct ChangesStore<K, V, F> {
    stack: RwLock<Vec<DashMap<K, Option<V>>>>,
    fallback: F,
}

impl<K: Copy + Hash + Eq, V: Clone, F: Store<K, V>> ChangesStore<K, V, F> {
    pub fn new(fallback: F) -> Self {
        Self {
            stack: RwLock::new(vec![Default::default()]),
            fallback,
        }
    }

    pub fn get(&self, key: K) -> Option<V> {
        for store in self.stack.read().iter() {
            if let Some(val_opt) = store.get(&key) {
                return val_opt.clone();
            }
        }
        self.fallback.get(key)
    }

    pub fn insert(&self, key: K, val: V) {
        let stack = self.stack.read();
        let store = stack.last().expect("empty stack");
        store.insert(key, Some(val));
    }

    pub fn push_local_changes(&self) {
        self.stack.write().push(Default::default())
    }

    pub fn pop_local_changes(&self) {
        self.stack.write().pop();
    }

    pub fn remove_batch<I: Iterator<Item = K>>(&self, keys: I) {
        let stack = self.stack.read();
        let store = stack.last().expect("empty stack");
        for key in keys {
            store.insert(key, None);
        }
    }
}

impl<K, V, F> Store<K, V> for ChangesStore<K, V, F>
where
    K: Copy + Hash + Eq + Send + Sync,
    V: Clone + Send + Sync,
    F: Store<K, V>,
{
    fn get(&self, key: K) -> Option<V> {
        ChangesStore::get(self, key)
    }

    fn insert(&self, key: K, val: V) {
        ChangesStore::insert(self, key, val)
    }
}

impl<K: Copy, V, F> std::fmt::Debug for ChangesStore<K, V, F> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ChangesStore").finish()
    }
}
