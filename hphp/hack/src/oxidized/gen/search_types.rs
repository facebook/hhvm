// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<958bf8057ef06bb233e7115cc39353f7>>
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
pub enum SiKind {
    #[rust_to_ocaml(name = "SI_Class")]
    SIClass,
    #[rust_to_ocaml(name = "SI_Interface")]
    SIInterface,
    #[rust_to_ocaml(name = "SI_Enum")]
    SIEnum,
    #[rust_to_ocaml(name = "SI_Trait")]
    SITrait,
    #[rust_to_ocaml(name = "SI_Unknown")]
    SIUnknown,
    #[rust_to_ocaml(name = "SI_Mixed")]
    SIMixed,
    #[rust_to_ocaml(name = "SI_Function")]
    SIFunction,
    #[rust_to_ocaml(name = "SI_Typedef")]
    SITypedef,
    #[rust_to_ocaml(name = "SI_GlobalConstant")]
    SIGlobalConstant,
    #[rust_to_ocaml(name = "SI_XHP")]
    SIXHP,
    #[rust_to_ocaml(name = "SI_Namespace")]
    SINamespace,
    #[rust_to_ocaml(name = "SI_ClassMethod")]
    SIClassMethod,
    #[rust_to_ocaml(name = "SI_Literal")]
    SILiteral,
    #[rust_to_ocaml(name = "SI_ClassConstant")]
    SIClassConstant,
    #[rust_to_ocaml(name = "SI_Property")]
    SIProperty,
    #[rust_to_ocaml(name = "SI_LocalVariable")]
    SILocalVariable,
    #[rust_to_ocaml(name = "SI_Keyword")]
    SIKeyword,
    #[rust_to_ocaml(name = "SI_Constructor")]
    SIConstructor,
}
impl TrivialDrop for SiKind {}
arena_deserializer::impl_deserialize_in_arena!(SiKind);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "sia_")]
#[repr(C)]
pub struct SiAddendum {
    /// This is expected not to contain the leading namespace backslash! See [Utils.strip_ns].
    pub name: String,
    pub kind: SiKind,
    pub is_abstract: bool,
    pub is_final: bool,
}

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
    pub kind: SiKind,
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
