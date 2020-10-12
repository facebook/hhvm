// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

#[derive(
    Copy,
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct OpaqueDigest<'a>(pub &'a [u8]);
