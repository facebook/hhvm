// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;
use std::sync::Arc;

use anyhow::Result;
use dashmap::DashMap;
use parking_lot::RwLock;

use crate::Store;

/// A stack of temporary changes on top of a fallback data store.
pub struct ChangesStore<K, V> {
    stack: RwLock<Vec<DashMap<K, Option<V>>>>,
    fallback: Arc<dyn Store<K, V>>,
}

impl<K: Copy + Hash + Eq, V: Clone> ChangesStore<K, V> {
    pub fn new(fallback: Arc<dyn Store<K, V>>) -> Self {
        Self {
            stack: Default::default(),
            fallback,
        }
    }

    pub fn get(&self, key: K) -> Result<Option<V>> {
        for store in self.stack.read().iter() {
            if let Some(val_opt) = store.get(&key) {
                return Ok(val_opt.clone());
            }
        }
        self.fallback.get(key)
    }

    pub fn insert(&self, key: K, val: V) -> Result<()> {
        if let Some(store) = self.stack.read().last() {
            store.insert(key, Some(val));
        } else {
            self.fallback.insert(key, val)?;
        }
        Ok(())
    }

    pub fn push_local_changes(&self) {
        self.stack.write().push(Default::default());
    }

    pub fn pop_local_changes(&self) {
        self.stack.write().pop();
    }

    pub fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        if let Some(store) = self.stack.read().last() {
            for key in keys {
                store.insert(key, None);
            }
        } else {
            self.fallback.remove_batch(keys)?;
        }
        Ok(())
    }
}

impl<K, V> Store<K, V> for ChangesStore<K, V>
where
    K: Copy + Hash + Eq + Send + Sync,
    V: Clone + Send + Sync,
{
    fn get(&self, key: K) -> Result<Option<V>> {
        ChangesStore::get(self, key)
    }

    fn insert(&self, key: K, val: V) -> Result<()> {
        ChangesStore::insert(self, key, val)
    }

    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        ChangesStore::remove_batch(self, keys)
    }
}

impl<K: Copy, V> std::fmt::Debug for ChangesStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("ChangesStore").finish()
    }
}
