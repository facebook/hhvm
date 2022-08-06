// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;
use std::hash::Hash;
use std::hash::Hasher;

use eq_modulo_pos::EqModuloPos;
use eq_modulo_pos::EqModuloPosAndReason;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::typing_defs_core::TshapeFieldName;

#[derive(
    Clone,
    Debug,
    Deserialize,
    FromOcamlRep,
    EqModuloPos,
    EqModuloPosAndReason,
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

// non-derived impl Hash because PartialEq and Eq are non-derived
impl Hash for TShapeField {
    fn hash<H: Hasher>(&self, hasher: &mut H) {
        Hash::hash(&self.0, hasher)
    }
}

pub type TShapeMap<T> = std::collections::BTreeMap<TShapeField, T>;
