// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use serde::{Deserialize, Serialize};

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

use crate::typing_defs_core::TshapeFieldName;

#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    Hash,
    EqModuloPos,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
pub struct TShapeField(pub TshapeFieldName);

impl Ord for TShapeField {
    fn cmp(&self, _other: &Self) -> Ordering {
        unimplemented!()
    }
}

impl PartialOrd for TShapeField {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for TShapeField {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl Eq for TShapeField {}

pub type TShapeMap<T> = std::collections::BTreeMap<TShapeField, T>;
