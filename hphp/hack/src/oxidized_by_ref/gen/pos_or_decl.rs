// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<613763597e21d75faa1f8cfa1c1910ed>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::pos::map;

pub type PosOrDecl<'a> = pos::Pos<'a>;

/// The decl and file of a position.
#[derive(
    Clone,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub struct Ctx<'a> {
    pub decl: Option<decl_reference::DeclReference<'a>>,
    pub file: &'a relative_path::RelativePath<'a>,
}
impl<'a> TrivialDrop for Ctx<'a> {}
