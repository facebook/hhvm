// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c38ae7495566de02be84c83f9cf38f5a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Default,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct TypingTyvarOccurrences {
    /// A map to track where each type variable occurs,
    /// more precisely in the type of which other type variables.
    /// E.g. if #1 is bound to (#2 | int), then this map contains the entry
    /// #2 -> { #1 }
    /// This is based on shallow binding, i.e. in the example above, if #2
    /// is mapped to #3, then tyvar_occurrences would be:
    /// #2 -> { #1 }
    /// #3 -> { #2 }
    /// but we would not record that #3 occurs in #1.
    /// When a type variable v gets solved or the type bound to it gets simplified,
    /// we simplify the unions and intersections of the types bound to the
    /// type variables associated to v in this map.
    /// So in our example, if #2 gets solved to int,
    /// we simplify #1 to (int | int) = int.
    /// There are only entries for variables that are unsolved or contain
    /// other unsolved type variables. Variables that are solved and contain
    /// no other unsolved type variables get removed from this map.
    pub tyvar_occurrences: i_map::IMap<i_set::ISet>,
    /// Mapping of type variables to the type variables contained in their
    /// types which are either unsolved or themselves contain unsolved type
    /// variables.
    /// This is the dual of tyvar_occurrences.
    pub tyvars_in_tyvar: i_map::IMap<i_set::ISet>,
}
