// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::cmp::Ordering;

use ocamlrep_derive::OcamlRep;
use serde::{Deserialize, Serialize};

use crate::ast_defs::{Id, ShapeFieldName};

#[derive(Clone, Debug, Deserialize, Hash, OcamlRep, Serialize)]
pub struct ShapeField(pub ShapeFieldName);

impl Ord for ShapeField {
    fn cmp(&self, other: &Self) -> Ordering {
        use ShapeFieldName::*;
        match (&self.0, &other.0) {
            (SFlitInt((_, s1)), SFlitInt((_, s2))) => s1.cmp(&s2),
            (SFlitStr((_, s1)), SFlitStr((_, s2))) => s1.cmp(&s2),
            (SFclassConst(Id(_, c1), (_, m1)), SFclassConst(Id(_, c2), (_, m2))) => {
                (c1, m1).cmp(&(c2, m2))
            }
            (SFlitInt(_), _) => Ordering::Less,
            (SFlitStr(_), SFlitInt(_)) => Ordering::Greater,
            (SFlitStr(_), _) => Ordering::Less,
            (SFclassConst(_, _), _) => Ordering::Greater,
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

pub type ShapeMap<T> = std::collections::BTreeMap<ShapeField, T>;
