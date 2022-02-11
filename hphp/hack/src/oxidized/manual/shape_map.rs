// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};
use serde::{Deserialize, Serialize};

use crate::ast_defs::{Id, ShapeFieldName};

#[derive(Clone, Debug, Deserialize, FromOcamlRep, ToOcamlRep, Serialize)]
pub struct ShapeField(pub ShapeFieldName);

impl Ord for ShapeField {
    fn cmp(&self, other: &Self) -> Ordering {
        use ShapeFieldName as SFN;
        match (&self.0, &other.0) {
            (SFN::SFlitInt((_, s1)), SFN::SFlitInt((_, s2))) => s1.cmp(s2),
            (SFN::SFlitStr((_, s1)), SFN::SFlitStr((_, s2))) => s1.cmp(s2),
            (SFN::SFclassConst(Id(_, c1), (_, m1)), SFN::SFclassConst(Id(_, c2), (_, m2))) => {
                (c1, m1).cmp(&(c2, m2))
            }
            (SFN::SFlitInt(_), _) => Ordering::Less,
            (SFN::SFlitStr(_), SFN::SFlitInt(_)) => Ordering::Greater,
            (SFN::SFlitStr(_), _) => Ordering::Less,
            (SFN::SFclassConst(_, _), _) => Ordering::Greater,
        }
    }
}

impl PartialOrd for ShapeField {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl PartialEq for ShapeField {
    fn eq(&self, other: &Self) -> bool {
        self.cmp(other) == Ordering::Equal
    }
}

impl Eq for ShapeField {}

// non-derived impl Hash because PartialEq and Eq are non-derived
impl std::hash::Hash for ShapeField {
    fn hash<H: std::hash::Hasher>(&self, hasher: &mut H) {
        self.0.hash(hasher)
    }
}

pub type ShapeMap<T> = std::collections::BTreeMap<ShapeField, T>;
