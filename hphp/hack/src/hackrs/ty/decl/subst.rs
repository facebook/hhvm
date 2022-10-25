// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use eq_modulo_pos::EqModuloPos;
use hash::IndexMap;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use pos::TypeName;
use serde::Deserialize;
use serde::Serialize;

use crate::decl::Tparam;
use crate::decl::Ty;
use crate::reason::Reason;

/// Maps type names to types with which to replace them.
#[derive(Debug, Clone, Eq, EqModuloPos, PartialEq, Serialize, Deserialize)]
#[derive(ToOcamlRep, FromOcamlRep)]
#[serde(bound = "R: Reason")]
pub struct Subst<R: Reason>(pub IndexMap<TypeName, Ty<R>>);

impl<R: Reason> From<IndexMap<TypeName, Ty<R>>> for Subst<R> {
    fn from(map: IndexMap<TypeName, Ty<R>>) -> Self {
        Self(map)
    }
}

impl<R: Reason> From<Subst<R>> for IndexMap<TypeName, Ty<R>> {
    fn from(subst: Subst<R>) -> Self {
        subst.0
    }
}

impl<R: Reason> Subst<R> {
    pub fn new(tparams: &[Tparam<R, Ty<R>>], targs: &[Ty<R>]) -> Self {
        // If there are fewer type arguments than type parameters, we'll have
        // emitted an error elsewhere. We bind missing types to `Tany` (rather
        // than `Terr`) here to keep parity with the OCaml implementation, which
        // produces `Tany` because of a now-dead feature called "silent_mode".
        let targs = targs
            .iter()
            .cloned()
            .chain(std::iter::repeat(Ty::any(R::none())));
        Self(
            tparams
                .iter()
                .map(|tparam| tparam.name.id())
                .zip(targs)
                .collect(),
        )
    }
}
