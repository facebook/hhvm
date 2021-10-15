// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<6f2d9a86628fecc1f7c6f41b07077aa2>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// [module_] represents when a top level symbol is _definitely_ in a module.
/// The "root" of the module tree is the first element, with any children
/// in the second. This is to avoid [module_ option] having a weird state in
/// [Some []]: "we are in a module with no name."
#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
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
pub struct Module_<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a [&'a str],
);
impl<'a> TrivialDrop for Module_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Module_<'arena>);

pub type TypingModules<'a> = Option<&'a Module_<'a>>;
