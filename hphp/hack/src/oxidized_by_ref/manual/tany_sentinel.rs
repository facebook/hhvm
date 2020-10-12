// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRep, FromOcamlRepIn, ToOcamlRep};

#[derive(
    Copy,
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
pub struct TanySentinel;

impl arena_trait::TrivialDrop for TanySentinel {}
