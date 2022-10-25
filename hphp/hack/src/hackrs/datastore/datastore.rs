// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

mod changes_store;
mod delta_store;
mod empty;
mod non_evicting;

use std::fmt::Debug;

use anyhow::Result;
pub use changes_store::ChangesStore;
pub use delta_store::DeltaStore;
pub use empty::EmptyStore;
pub use non_evicting::NonEvictingLocalStore;
pub use non_evicting::NonEvictingStore;

/// A threadsafe datastore, intended for global decl storage. The key type is
/// intended to be a `Symbol` or tuple of `Symbol`s, and the value type is
/// intended to be a ref-counted pointer (like `Arc` or `Hc`).
pub trait Store<K: Copy, V>: Debug + Send + Sync {
    fn get(&self, key: K) -> Result<Option<V>>;
    fn insert(&self, key: K, val: V) -> Result<()>;

    /// Return `true` if the store contains a value for the given key (i.e.,
    /// `self.get(key)?.is_some()` would evaluate to `true`).
    ///
    /// A default implementation is provided which uses the store's
    /// implementation of `Store::get`, but some implementors will be able to
    /// implement `contains_key` more efficiently than `get`.
    fn contains_key(&self, key: K) -> Result<bool> {
        Ok(self.get(key)?.is_some())
    }

    /// Implementations are free to ignore a key to be removed if it does not
    /// exist.
    fn remove_batch(&self, keys: &mut dyn Iterator<Item = K>) -> Result<()>;

    /// Implementations should return an error if a key to be moved does not
    /// exist and well defined behavior is not a requirement if a key occurs in
    /// both an old and new position.
    fn move_batch(&self, keys: &mut dyn Iterator<Item = (K, K)>) -> Result<()> {
        for (old_key, new_key) in keys {
            if let Some(val) = self.get(old_key)? {
                self.remove_batch(&mut std::iter::once(old_key))?;
                self.insert(new_key, val)?;
            } else {
                anyhow::bail!("move_batch: Trying to remove a non-existent value");
            }
        }
        Ok(())
    }
}

/// A thread-local datastore, intended for decl caching in typechecker workers.
/// The key type is intended to be a `Symbol` or tuple of `Symbol`s, and the
/// value type is intended to be a ref-counted pointer (like `Rc`).
pub trait LocalStore<K: Copy, V>: Debug {
    fn get(&self, key: K) -> Option<V>;
    fn insert(&mut self, key: K, val: V);
    fn remove_batch(&mut self, keys: &mut dyn Iterator<Item = K>);
}

/// A readonly threadsafe datastore, intended to model readonly data sources
/// (like the filesystem in file_provider, or the naming SQLite database in
/// naming_provider) in terms of `datastore` traits (for purposes like
/// `DeltaStore`).
pub trait ReadonlyStore<K: Copy, V>: Send + Sync {
    fn get(&self, key: K) -> Result<Option<V>>;
    fn contains_key(&self, key: K) -> Result<bool> {
        Ok(self.get(key)?.is_some())
    }
}

impl<T: Store<K, V>, K: Copy, V> ReadonlyStore<K, V> for T {
    fn contains_key(&self, key: K) -> Result<bool> {
        Store::contains_key(self, key)
    }

    fn get(&self, key: K) -> Result<Option<V>> {
        Store::get(self, key)
    }
}
