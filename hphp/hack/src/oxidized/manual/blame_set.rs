// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::local_id::LocalId;
use crate::typing_reason::Blame;

#[derive(Clone, Debug, Eq, Hash, Ord)]
#[derive(Deserialize, FromOcamlRep, NoPosHash, Serialize, ToOcamlRep)]
pub struct IdWithBlame(pub LocalId, Blame);

impl PartialEq for IdWithBlame {
    fn eq(&self, other: &Self) -> bool {
        self.0.eq(&other.0)
    }
}

impl PartialOrd for IdWithBlame {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.0.partial_cmp(&other.0)
    }
}

/// A set of LocalIds which treats blame as metadata
pub type BlameSet = std::collections::BTreeSet<IdWithBlame>;
