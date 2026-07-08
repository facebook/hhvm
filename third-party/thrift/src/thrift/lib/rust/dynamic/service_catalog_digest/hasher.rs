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

use type_system_digest::DigestMode;
use type_system_digest::TypeSystemDigest;
use type_system_digest::hasher::DigestHasher;
pub use type_system_digest::hasher::DigestHasherExt;
pub use type_system_digest::hasher::OrderedByKey;

use crate::Digest;
use crate::ServiceCatalogDigest;

/// SHA-256 streaming hasher for building service catalog digests.
pub struct Hasher {
    inner: type_system_digest::hasher::Hasher,
}

impl Hasher {
    pub fn new() -> Self {
        Self::with_mode(DigestMode::Full)
    }

    pub fn with_mode(mode: DigestMode) -> Self {
        Self {
            inner: type_system_digest::hasher::Hasher::with_mode(mode),
        }
    }

    pub fn mode(&self) -> DigestMode {
        self.inner.mode()
    }

    pub fn include_annotations(&self) -> bool {
        self.mode() == DigestMode::Full
    }

    pub fn hash<T: ServiceCatalogDigest + ?Sized>(&mut self, value: &T) {
        value.hash_into(self);
    }

    pub fn hash_type<T: TypeSystemDigest + ?Sized>(&mut self, value: &T) {
        self.inner.hash(value);
    }

    pub fn update(&mut self, bytes: &[u8]) {
        self.inner.update(bytes);
    }

    pub fn finalize(self) -> Digest {
        self.inner.finalize()
    }
}

impl DigestHasher for Hasher {
    fn with_mode(mode: DigestMode) -> Self {
        Self {
            inner: type_system_digest::hasher::Hasher::with_mode(mode),
        }
    }

    fn mode(&self) -> DigestMode {
        self.inner.mode()
    }

    fn hash_u32(&mut self, value: u32) {
        self.hash(&value);
    }

    fn update(&mut self, bytes: &[u8]) {
        self.inner.update(bytes);
    }

    fn finalize(self) -> Digest {
        self.inner.finalize()
    }
}

impl Default for Hasher {
    fn default() -> Self {
        Self::new()
    }
}
