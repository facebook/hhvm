// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::error::Error;
use std::fmt;

use serde::Deserialize;
use serde::Serialize;

/// Returned by a failed integrity-check on a slab, indicating one way in which
/// the given bytes form a corrupt or otherwise invalid slab.
#[derive(Debug, Deserialize, PartialEq, Serialize)]
pub enum SlabIntegrityError {
    InvalidBasePointer(usize),
    InvalidBlockSize(usize),
    InvalidPointer(usize),
    InvalidRootValueOffset(usize),
    NotInitialized,
    TooSmall(usize),
}

impl fmt::Display for SlabIntegrityError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{:?}", self)
    }
}

impl Error for SlabIntegrityError {}
