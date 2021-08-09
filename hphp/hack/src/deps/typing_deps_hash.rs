// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::hash::Hasher;

use fnv::FnvHasher;

use oxidized::{file_info::NameType, typing_deps_mode::HashMode};

/// Variant types used in the naming table.
///
/// NOTE: Keep in sync with the order of the fields in `Typing_deps.ml`.
#[derive(Copy, Clone, Debug)]
#[repr(u8)]
pub enum DepType {
    GConst = 0,
    Fun = 1,
    Type = 2,
    Extends = 3,
    Const = 5,
    Cstr = 6,
    Prop = 7,
    SProp = 8,
    Method = 9,
    SMethod = 10,
    AllMembers = 11,
    GConstName = 12,
}

impl DepType {
    pub fn as_u8(self) -> u8 {
        self as u8
    }

    pub fn from_u8(tag: u8) -> Option<Self> {
        match tag {
            0 => Some(DepType::GConst),
            1 => Some(DepType::Fun),
            2 => Some(DepType::Type),
            3 => Some(DepType::Extends),
            5 => Some(DepType::Const),
            6 => Some(DepType::Cstr),
            7 => Some(DepType::Prop),
            8 => Some(DepType::SProp),
            9 => Some(DepType::Method),
            10 => Some(DepType::SMethod),
            11 => Some(DepType::AllMembers),
            12 => Some(DepType::GConstName),
            _ => None,
        }
    }

    #[inline]
    pub fn is_toplevel_symbol(self) -> bool {
        match self {
            DepType::GConst | DepType::Fun | DepType::Type => true,
            _ => false,
        }
    }
}

impl From<NameType> for DepType {
    fn from(name_type: NameType) -> Self {
        match name_type {
            NameType::Fun => DepType::Fun,
            NameType::Const => DepType::GConst,
            NameType::Class | NameType::RecordDef | NameType::Typedef => DepType::Type,
        }
    }
}

/// Select the hashing algorithm to use for dependency hashes.
///
/// FnvHasher appears to produce better hashes (fewer collisions) than
/// `std::collections::hash_map::DefaultHasher` on our workloads. However, other
/// hashing algorithms may perform better still.
fn make_hasher() -> FnvHasher {
    Default::default()
}

fn postprocess_hash(mode: HashMode, dep_type: DepType, hash: u64) -> u64 {
    let hash: u64 = match dep_type {
        DepType::Type => {
            // For class dependencies, set the lowest bit to 1. For extends
            // dependencies, the lowest bit will be 0 (in the case below), so we'll
            // be able to convert from a class hash to its extends hash without
            // reversing the hash.
            (hash << 1) | 1
        }
        _ => {
            // Ensure that only classes have the lowest bit set to 1, so that we
            // don't try to transitively traverse the subclasses of non-class
            // dependencies.
            hash << 1
        }
    };

    match mode {
        HashMode::Hash32Bit => {
            // We are in the legacy dependency graph system:
            //
            // The shared-memory dependency graph stores edges as pairs of vertices.
            // Each vertex has 31 bits of actual content and 1 bit of OCaml bookkeeping.
            // Thus, we truncate the hash to 31 bits.
            hash & ((1 << 31) - 1)
        }
        HashMode::Hash64Bit => {
            // One bit is used for OCaml bookkeeping!
            hash & !(1 << 63)
        }
    }
}

fn get_dep_type_hash_key(dep_type: DepType) -> u8 {
    match dep_type {
        DepType::Type | DepType::Extends => {
            // Use the same tag for classes and extends dependencies, so that we can
            // convert between them without reversing the hash.
            DepType::Type as u8
        }
        _ => dep_type as u8,
    }
}

/// Hash a one-argument `Typing_deps.Dep.variant`'s fields.
pub fn hash1(mode: HashMode, dep_type: DepType, name1: &[u8]) -> u64 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    postprocess_hash(mode, dep_type, hasher.finish())
}

/// Hash a two-argument `Typing_deps.Dep.variant`'s fields.
pub fn hash2(mode: HashMode, dep_type: DepType, name1: &[u8], name2: &[u8]) -> u64 {
    let mut hasher = make_hasher();
    hasher.write_u8(get_dep_type_hash_key(dep_type));
    hasher.write(name1);
    hasher.write(name2);
    postprocess_hash(mode, dep_type, hasher.finish())
}
