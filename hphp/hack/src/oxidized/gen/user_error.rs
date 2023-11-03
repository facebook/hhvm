// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<1e89075be2f57b24e27b3aaea3944f2b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    PartialEq,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C)]
pub struct UserError<PrimPos, Pos> {
    pub code: isize,
    pub claim: message::Message<PrimPos>,
    pub reasons: Vec<message::Message<Pos>>,
    #[rust_to_ocaml(attr = "hash.ignore")]
    pub quickfixes: Vec<quickfix::Quickfix<PrimPos>>,
    pub custom_msgs: Vec<String>,
    pub is_fixmed: bool,
}
