// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b9ecbe89ca3be7d75d8823595e824e24>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::pos::map;

pub type PosOrDecl<'a> = pos::Pos<'a>;

/// The decl and file of a position.
#[derive(
    Clone,
    Debug,
    Deserialize,
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
#[repr(C)]
pub struct Ctx<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decl: Option<decl_reference::DeclReference<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub file: &'a relative_path::RelativePath<'a>,
}
impl<'a> TrivialDrop for Ctx<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ctx<'arena>);
