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

use ring::digest::Context;
use ring::digest::SHA256;

use crate::TypeSystemDigest;

/// SHA-256 streaming hasher for building type system digests.
pub struct Hasher {
    context: Context,
}

impl Hasher {
    pub fn new() -> Self {
        Hasher {
            context: Context::new(&SHA256),
        }
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
        let mut digests: Vec<[u8; 32]> = items
            .into_iter()
            .map(|item| {
                let mut h = Hasher::new();
                hash_fn(&mut h, &item);
                h.finalize()
            })
            .collect();

        self.hash(&(digests.len() as u32));
        digests.sort();
        for d in &digests {
            self.update(d);
        }
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
        let mut sorted: Vec<([u8; 32], T)> = items
            .into_iter()
            .map(|item| {
                let mut h = Hasher::new();
                key_hash_fn(&mut h, &item);
                (h.finalize(), item)
            })
            .collect();

        self.hash(&(sorted.len() as u32));
        sorted.sort_by_key(|a| a.0);
        for (_, item) in &sorted {
            entry_hash_fn(self, item);
        }
    }
}
