// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::typing_defs_core::PosByteString;
use crate::typing_defs_core::PosString;
use crate::typing_defs_core::TshapeFieldName;

#[derive(
    Copy,
    Clone,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    Hash,
    EqModuloPos,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
pub struct TShapeField<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub TshapeFieldName<'a>,
);
arena_deserializer::impl_deserialize_in_arena!(TShapeField<'arena>);

impl arena_trait::TrivialDrop for TShapeField<'_> {}

impl<'a> Ord for TShapeField<'a> {
    fn cmp(&self, other: &Self) -> Ordering {
        use TshapeFieldName::*;
        match (&self.0, &other.0) {
            (TSFlitInt(PosString(_, s1)), TSFlitInt(PosString(_, s2))) => s1.cmp(s2),
            (TSFlitStr(PosByteString(_, s1)), TSFlitStr(PosByteString(_, s2))) => s1.cmp(s2),
            (
                TSFclassConst(((_, c1), PosString(_, m1))),
                TSFclassConst(((_, c2), PosString(_, m2))),
            ) => (c1, m1).cmp(&(c2, m2)),
            (TSFlitInt(_), _) => Ordering::Less,
            (TSFlitStr(_), TSFlitInt(_)) => Ordering::Greater,
            (TSFlitStr(_), _) => Ordering::Less,
            (TSFclassConst(_), _) => Ordering::Greater,
        }
    }
}

impl<'a> PartialOrd for TShapeField<'a> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<'a> PartialEq for TShapeField<'a> {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl<'a> Eq for TShapeField<'a> {}

pub type TShapeMap<'a, T> = arena_collections::SortedAssocList<'a, TShapeField<'a>, T>;
