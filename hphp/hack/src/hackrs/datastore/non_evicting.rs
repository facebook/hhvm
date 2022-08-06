// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;

use anyhow::Result;

pub struct NonEvictingStore<K: Hash + Eq, V> {
    store: dashmap::DashMap<K, V>,
}

pub struct NonEvictingLocalStore<K: Hash + Eq, V> {
    store: hash::HashMap<K, V>,
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

impl<K, V> crate::Store<K, V> for NonEvictingStore<K, V>
where
    K: Copy + Send + Sync + Hash + Eq,
    V: Clone + Send + Sync,
{
    fn get(&self, key: K) -> Result<Option<V>> {
        Ok(self.store.get(&key).map(|x| V::clone(&*x)))
    }

    fn insert(&self, key: K, val: V) -> Result<()> {
        self.store.insert(key, val);
        Ok(())
    }

    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        for key in keys {
            self.store.remove(&key);
        }
        Ok(())
    }
}

impl<K: Hash + Eq, V> std::fmt::Debug for NonEvictingStore<K, V> {
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

impl<K, V> crate::LocalStore<K, V> for NonEvictingLocalStore<K, V>
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

    fn remove_batch(&mut self, keys: &mut dyn Iterator<Item = K>) {
        for key in keys {
            self.store.remove(&key);
        }
    }
}

impl<K: Hash + Eq, V> std::fmt::Debug for NonEvictingLocalStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("NonEvictingLocalStore").finish()
    }
}
