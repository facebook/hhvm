// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::Deserialize;
use serde::Serialize;

use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    FromOcamlRep,
    FromOcamlRepIn,
    ToOcamlRep,
    Serialize
)]
pub struct OpaqueDigest(());
