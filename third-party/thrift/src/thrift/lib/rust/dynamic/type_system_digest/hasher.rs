/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use std::collections::BTreeMap;

use ring::digest::Context;
use ring::digest::SHA256;

use crate::Digest;
use crate::DigestMode;
use crate::TypeSystemDigest;

pub trait DigestHasher: Sized {
    fn with_mode(mode: DigestMode) -> Self;
    fn mode(&self) -> DigestMode;
    fn hash_u32(&mut self, value: u32);
    fn update(&mut self, bytes: &[u8]);
    fn finalize(self) -> Digest;
}

pub trait OrderedByKey {
    type Key;
    type Value;

    fn for_each_ordered_by_key(&self, visit: impl FnMut(&Self::Key, &Self::Value));
}

impl<K, V> OrderedByKey for BTreeMap<K, V>
where
    K: Ord,
{
    type Key = K;
    type Value = V;

    fn for_each_ordered_by_key(&self, mut visit: impl FnMut(&Self::Key, &Self::Value)) {
        for (key, value) in self {
            visit(key, value);
        }
    }
}

pub trait DigestHasherExt: DigestHasher {
    /// Hash elements in canonical key order.
    ///
    /// This mirrors the C++ `forEachSortedByKey` helper: keys are extracted
    /// once, entries are visited by sorted key, and duplicate keys retain the
    /// first entry.
    fn hash_ordered_by_key<T, K>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        key_fn: impl Fn(&T) -> K,
        hash_fn: impl Fn(&mut Self, &K, &T),
    ) where
        K: Ord,
    {
        let mut sorted = BTreeMap::new();
        for item in items {
            sorted.entry(key_fn(&item)).or_insert(item);
        }
        for (key, item) in &sorted {
            hash_fn(self, key, item);
        }
    }

    /// Hash map entries whose container already guarantees key order.
    fn hash_in_key_order<M>(
        &mut self,
        items: &M,
        hash_fn: impl FnMut(&mut Self, &M::Key, &M::Value),
    ) where
        M: OrderedByKey + ?Sized,
    {
        let mut hash_fn = hash_fn;
        items.for_each_ordered_by_key(|key, value| hash_fn(self, key, value));
    }

    /// Hash elements in an order-independent way by sorting by element digest.
    fn hash_unordered_by_digest<T>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        hash_fn: impl Fn(&mut Self, &T),
    ) {
        let mut digests: Vec<Digest> = items
            .into_iter()
            .map(|item| {
                let mut h = Self::with_mode(self.mode());
                hash_fn(&mut h, &item);
                h.finalize()
            })
            .collect();

        self.hash_u32(digests.len() as u32);
        digests.sort();
        for digest in &digests {
            self.update(digest);
        }
    }

    /// Hash map entries in order-independent way by sorting by key digest.
    fn hash_map_by_key_digest<T>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        key_hash_fn: impl Fn(&mut Self, &T),
        entry_hash_fn: impl Fn(&mut Self, &T),
    ) {
        let mut sorted: Vec<(Digest, T)> = items
            .into_iter()
            .map(|item| {
                let mut h = Self::with_mode(self.mode());
                key_hash_fn(&mut h, &item);
                (h.finalize(), item)
            })
            .collect();

        self.hash_u32(sorted.len() as u32);
        sorted.sort_by_key(|entry| entry.0);
        for (_, item) in &sorted {
            entry_hash_fn(self, item);
        }
    }
}

impl<T: DigestHasher> DigestHasherExt for T {}

/// SHA-256 streaming hasher for building type system digests.
pub struct Hasher {
    context: Context,
    mode: DigestMode,
}

impl Hasher {
    pub fn new() -> Self {
        Self::with_mode(DigestMode::Full)
    }

    pub fn with_mode(mode: DigestMode) -> Self {
        Self {
            context: Context::new(&SHA256),
            mode,
        }
    }

    pub fn mode(&self) -> DigestMode {
        self.mode
    }

    pub fn include_annotations(&self) -> bool {
        self.mode == DigestMode::Full
    }

    pub fn include_custom_default_values(&self) -> bool {
        self.mode == DigestMode::Full
    }

    /// Feed a value implementing [`TypeSystemDigest`] into this hasher.
    pub fn hash<T: TypeSystemDigest + ?Sized>(&mut self, value: &T) {
        value.hash_into(self);
    }

    /// Feed raw bytes into the digest (no length prefix).
    pub fn update(&mut self, bytes: &[u8]) {
        self.context.update(bytes);
    }

    pub fn finalize(self) -> [u8; 32] {
        self.context
            .finish()
            .as_ref()
            .try_into()
            .expect("SHA-256 digest should be 32 bytes")
    }

    pub fn hash_ordered_by_key<T, K>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        key_fn: impl Fn(&T) -> K,
        hash_fn: impl Fn(&mut Hasher, &K, &T),
    ) where
        K: Ord,
    {
        <Self as DigestHasherExt>::hash_ordered_by_key(self, items, key_fn, hash_fn);
    }

    /// Hash map entries whose container already guarantees key order.
    pub fn hash_in_key_order<M>(
        &mut self,
        items: &M,
        hash_fn: impl FnMut(&mut Hasher, &M::Key, &M::Value),
    ) where
        M: OrderedByKey + ?Sized,
    {
        <Self as DigestHasherExt>::hash_in_key_order(self, items, hash_fn);
    }

    /// Hash elements in an order-independent way by sorting by element digest.
    ///
    /// Each element is hashed independently to produce a digest, then elements
    /// are sorted lexicographically by digest and fed into this hasher.
    /// A u32 count prefix is included for disambiguation.
    pub fn hash_unordered_by_digest<T>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        hash_fn: impl Fn(&mut Hasher, &T),
    ) {
        <Self as DigestHasherExt>::hash_unordered_by_digest(self, items, hash_fn);
    }

    /// Hash map entries in order-independent way by sorting by key digest.
    ///
    /// `key_hash_fn` hashes only the key (for sorting), `entry_hash_fn` hashes
    /// the full key+value pair (for the final digest). A u32 count prefix is
    /// included.
    pub fn hash_map_by_key_digest<T>(
        &mut self,
        items: impl IntoIterator<Item = T>,
        key_hash_fn: impl Fn(&mut Hasher, &T),
        entry_hash_fn: impl Fn(&mut Hasher, &T),
    ) {
        <Self as DigestHasherExt>::hash_map_by_key_digest(self, items, key_hash_fn, entry_hash_fn);
    }
}

impl DigestHasher for Hasher {
    fn with_mode(mode: DigestMode) -> Self {
        Self {
            context: Context::new(&SHA256),
            mode,
        }
    }

    fn mode(&self) -> DigestMode {
        self.mode
    }

    fn hash_u32(&mut self, value: u32) {
        self.hash(&value);
    }

    fn update(&mut self, bytes: &[u8]) {
        self.context.update(bytes);
    }

    fn finalize(self) -> Digest {
        self.context
            .finish()
            .as_ref()
            .try_into()
            .expect("SHA-256 digest should be 32 bytes")
    }
}
