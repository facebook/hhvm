// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<201047830fa71d2c0c68e46c28b32e9f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// This is used as a filter on top-level symbol searches, for both autocomplete and symbol-search.
#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (eq, (show { with_path = false }))")]
#[repr(u8)]
pub enum AutocompleteType {
    /// satisfies [valid_for_acid], e.g. in autocomplete contexts like `|` at the start of a statement
    Acid,
    /// satisfies [valid_for_acclassish], currently only used for `nameof`
    Acclassish,
    /// satisfies [valid_for_acnew] AND isn't an abstract class, e.g. in autocomplete contexts like `$x = new |`
    Acnew,
    /// satisfies [valid_for_actype], e.g. in autocomplete contexts like `Foo<|`
    Actype,
    /// is [SI_Trait], e.g. in autocomplete contexts like `uses |`
    #[rust_to_ocaml(name = "Actrait_only")]
    ActraitOnly,
    /// isn't [SI_Namespace]; repo-wide symbol search
    #[rust_to_ocaml(name = "Ac_workspace_symbol")]
    AcWorkspaceSymbol,
}
impl TrivialDrop for AutocompleteType {}
arena_deserializer::impl_deserialize_in_arena!(AutocompleteType);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum SiFile {
    /// string represent Int64
    #[rust_to_ocaml(name = "SI_Filehash")]
    SIFilehash(String),
    #[rust_to_ocaml(name = "SI_Path")]
    SIPath(relative_path::RelativePath),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(prefix = "si_")]
#[repr(C)]
pub struct SiItem {
    pub name: String,
    pub kind: file_info::SiKind,
    /// needed so that local file deletes can "tombstone" the item
    pub file: SiFile,
    pub fullname: String,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    EqModuloPos,
    FromOcamlRep,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving eq")]
#[repr(u8)]
pub enum SiComplete {
    Complete,
    Incomplete,
}
impl TrivialDrop for SiComplete {}
arena_deserializer::impl_deserialize_in_arena!(SiComplete);
