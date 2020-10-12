// Copyright (c) 2019, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};
use serde::Serialize;

#[derive(
    Clone,
    Copy,
    Debug,
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
pub struct LocalIdMap<'a, Ex>(pub crate::local_id::map::Map<'a, Ex>);
