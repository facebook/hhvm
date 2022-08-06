// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hash;
use std::sync::Arc;

use anyhow::Result;

use crate::ReadonlyStore;
use crate::Store;

/// A mutable set of changes on top of a readonly fallback data store.
pub struct DeltaStore<K, V> {
    delta: Arc<dyn Store<K, Option<V>>>,
    fallback: Arc<dyn ReadonlyStore<K, V>>,
}

impl<K: Copy + Hash + Eq, V> DeltaStore<K, V> {
    pub fn new(
        delta: Arc<dyn Store<K, Option<V>>>,
        fallback: Arc<dyn ReadonlyStore<K, V>>,
    ) -> Self {
        Self { delta, fallback }
    }

    pub fn get(&self, key: K) -> Result<Option<V>> {
        if let Some(val_opt) = self.delta.get(key)? {
            Ok(val_opt)
        } else {
            self.fallback.get(key)
        }
    }

    pub fn insert(&self, key: K, val: V) -> Result<()> {
        self.delta.insert(key, Some(val))
    }

    pub fn remove(&self, key: K) -> Result<()> {
        self.delta.insert(key, None)
    }

    pub fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        for key in keys {
            self.remove(key)?;
        }
        Ok(())
    }
}

impl<K: Copy + Hash + Eq, V> Store<K, V> for DeltaStore<K, V> {
    fn get(&self, key: K) -> Result<Option<V>> {
        DeltaStore::get(self, key)
    }

    fn insert(&self, key: K, val: V) -> Result<()> {
        DeltaStore::insert(self, key, val)
    }

    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        DeltaStore::remove_batch(self, keys)
    }
}

impl<K: Copy, V> std::fmt::Debug for DeltaStore<K, V> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("DeltaStore").finish()
    }
}
