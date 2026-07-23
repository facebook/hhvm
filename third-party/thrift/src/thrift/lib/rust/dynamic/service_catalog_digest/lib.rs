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

//! Canonical, deterministic SHA-256 digest for Thrift service catalogs.
//!
//! This crate produces digests that are byte-identical to the C++ implementation
//! in `thrift/lib/cpp2/dynamic/ServiceCatalogDigest.h`.
//!
//! The serialized implementations cover generated `service_catalog-rust` types.
//! Runtime service catalog nodes live in `thrift_service_catalog`.

pub use type_system_digest::Digest;
pub use type_system_digest::DigestMode;

pub mod hasher;
mod impls;

use crate::hasher::Hasher;

/// Current hash algorithm version.
pub const SERVICE_CATALOG_DIGEST_VERSION: u8 = 1;

// TODO(sadroeck): Derive these from Rust Thrift reflection once generated
// Rust exposes union field metadata.
#[doc(hidden)]
pub const SERVICE_DEF_FIELD_ID: i32 = 1;
#[doc(hidden)]
pub const INTERACTION_DEF_FIELD_ID: i32 = 2;
#[doc(hidden)]
pub const SERVER_STREAM_FIELD_ID: i32 = 1;
#[doc(hidden)]
pub const CLIENT_SINK_FIELD_ID: i32 = 2;
#[doc(hidden)]
pub const BIDIRECTIONAL_STREAM_FIELD_ID: i32 = 3;

/// Trait for types that can produce a service catalog digest.
pub trait ServiceCatalogDigest {
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

impl ServiceCatalogDigest for bool {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for u8 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for i8 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for i16 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for u32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for i32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for i64 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for f32 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for f64 {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for str {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for String {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.as_str());
    }
}

impl ServiceCatalogDigest for [u8] {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash_type(self);
    }
}

impl ServiceCatalogDigest for Vec<u8> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(self.as_slice());
    }
}

impl<T: ServiceCatalogDigest> ServiceCatalogDigest for Option<T> {
    fn hash_into(&self, h: &mut Hasher) {
        h.hash(&self.is_some());
        if let Some(value) = self {
            h.hash(value);
        }
    }
}
