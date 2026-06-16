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

//! Canonical, deterministic SHA-256 digest for Thrift type systems.
//!
//! This crate produces digests that are byte-identical to the C++ implementation
//! in `thrift/lib/cpp2/dynamic/TypeSystemDigest.h`. This enables cross-language
//! cache invalidation, version compatibility checks, and deduplication.
//!
//! # Properties
//!
//! - Equivalent type systems always produce the same digest
//! - Order-independent: URI ordering, field ordering, annotation ordering
//!   do not affect the digest
//! - Excludes `sourceInfo`: file paths are not semantically significant
//!
//! # Floating-point values
//!
//! Float/double values (in custom defaults and annotations) are hashed by
//! their IEEE 754 bit representation. This relies on `SerializableRecord`
//! rejecting NaN and negative zero at construction time to guarantee
//! determinism.

pub mod hasher;
mod impls;

use crate::hasher::Hasher;

/// SHA-256 digest (32 bytes).
pub type Digest = [u8; 32];

/// Current hash algorithm version.
pub const TYPE_SYSTEM_DIGEST_VERSION: u8 = 2;

/// Controls which parts of the type system are included in the digest.
///
/// Matches the C++ `DigestMode` enum in `TypeSystemDigest.h`.
#[derive(Copy, Clone, Debug, Default, PartialEq, Eq)]
pub enum DigestMode {
    /// Hash everything: structure, annotations, and custom defaults.
    #[default]
    Full,
    /// Hash only structure: skip annotations and custom defaults.
    Structural,
}

/// Trait for types that can produce a type system digest.
///
/// Implementors feed their content into a [`Hasher`] in a deterministic way
/// that matches the C++ `TypeSystemDigest` implementation byte-for-byte.
pub trait TypeSystemDigest {
    fn digest(&self) -> Digest {
        self.digest_with_mode(DigestMode::Full)
    }

    fn digest_with_mode(&self, mode: DigestMode) -> Digest {
        let mut h = Hasher::with_mode(mode);
        self.hash_into(&mut h);
        h.finalize()
    }

    #[doc(hidden)]
    fn hash_into(&self, hasher: &mut Hasher);
}

impl TypeSystemDigest for bool {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&[u8::from(*self)]);
    }
}

impl TypeSystemDigest for u8 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

impl TypeSystemDigest for i8 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

impl TypeSystemDigest for i16 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

impl TypeSystemDigest for u32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

impl TypeSystemDigest for i32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

impl TypeSystemDigest for i64 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_le_bytes());
    }
}

/// Hashes the IEEE 754 bit representation in little-endian.
///
/// This hashes the raw bit pattern, so distinct NaN representations would
/// produce different digests. This is safe because `SerializableRecord`
/// rejects NaN and negative zero at construction time.
impl TypeSystemDigest for f32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_bits().to_le_bytes());
    }
}

/// See [`f32`] implementation for NaN safety notes.
impl TypeSystemDigest for f64 {
    fn hash_into(&self, h: &mut Hasher) {
        h.update(&self.to_bits().to_le_bytes());
    }
}

/// Hashed as a size-prefixed envelope (u32 length + bytes).
impl TypeSystemDigest for str {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&(self.len() as u32));
        h.update(self.as_bytes());
    }
}

impl TypeSystemDigest for String {
    fn hash_into(&self, h: &mut Hasher) {
        self.as_str().hash_into(h);
    }
}

/// Hashed as a size-prefixed envelope (u32 length + bytes).
impl TypeSystemDigest for [u8] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&(self.len() as u32));
        h.update(self);
    }
}

impl TypeSystemDigest for Vec<u8> {
    fn hash_into(&self, h: &mut Hasher) {
        self.as_slice().hash_into(h);
    }
}
