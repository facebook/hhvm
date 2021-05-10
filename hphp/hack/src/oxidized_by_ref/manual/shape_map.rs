// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use serde::{Deserialize, Serialize};

use no_pos_hash::NoPosHash;
use ocamlrep_derive::{FromOcamlRepIn, ToOcamlRep};

use crate::ast_defs::{Id, ShapeFieldName};

#[derive(
    Copy,
    Clone,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
pub struct ShapeField<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub ShapeFieldName<'a>,
);
arena_deserializer::impl_deserialize_in_arena!(ShapeField<'arena>);

impl arena_trait::TrivialDrop for ShapeField<'_> {}

impl<'a> Ord for ShapeField<'a> {
    fn cmp(&self, other: &Self) -> Ordering {
        use ShapeFieldName::*;
        match (&self.0, &other.0) {
            (SFlitInt((_, s1)), SFlitInt((_, s2))) => s1.cmp(&s2),
            (SFlitStr((_, s1)), SFlitStr((_, s2))) => s1.cmp(&s2),
            (SFclassConst((Id(_, c1), (_, m1))), SFclassConst((Id(_, c2), (_, m2)))) => {
                (c1, m1).cmp(&(c2, m2))
            }
            (SFlitInt(_), _) => Ordering::Less,
            (SFlitStr(_), SFlitInt(_)) => Ordering::Greater,
            (SFlitStr(_), _) => Ordering::Less,
            (SFclassConst(_), _) => Ordering::Greater,
        }
    }
}

impl<'a> PartialOrd for ShapeField<'a> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl<'a> PartialEq for ShapeField<'a> {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl<'a> Eq for ShapeField<'a> {}

pub type ShapeMap<'a, T> = arena_collections::SortedAssocList<'a, ShapeField<'a>, T>;
