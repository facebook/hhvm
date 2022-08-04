// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use anyhow::Result;

/// A zero-sized store which retains no data and returns `None` for every `get`.
#[derive(Debug)]
pub struct EmptyStore;

impl<K: Copy, V> crate::Store<K, V> for EmptyStore {
    fn get(&self, _key: K) -> Result<Option<V>> {
        Ok(None)
    }
    fn insert(&self, _key: K, _val: V) -> Result<()> {
        Ok(())
    }
    fn remove_batch(&self, _keys: &mut dyn Iterator<Item = K>) -> Result<()> {
        Ok(())
    }
}
