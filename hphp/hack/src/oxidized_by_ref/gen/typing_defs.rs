// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2e1de6a2c79566a1def75d6587b8e227>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;
pub use typing_defs_core::*;
pub use typing_defs_flags::*;

#[allow(unused_imports)]
use crate::*;

/// Origin of Class Constant References:
/// In order to be able to detect cycle definitions like
/// class C {
/// const int A = D::A;
/// }
/// class D {
/// const int A = C::A;
/// }
/// we need to remember which constants were used during initialization.
///
/// Currently the syntax of constants allows direct references to another class
/// like D::A, or self references using self::A.
///
/// class_const_from encodes the origin (class vs self).
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum ClassConstFrom<'a> {
    #[rust_to_ocaml(name = "Self")]
    Self_,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    From(&'a str),
}
impl<'a> TrivialDrop for ClassConstFrom<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConstFrom<'arena>);

/// Class Constant References:
/// In order to be able to detect cycle definitions like
/// class C {
/// const int A = D::A;
/// }
/// class D {
/// const int A = C::A;
/// }
/// we need to remember which constants were used during initialization.
///
/// Currently the syntax of constants allows direct references to another class
/// like D::A, or self references using self::A.
#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct ClassConstRef<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub ClassConstFrom<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a str,
);
impl<'a> TrivialDrop for ClassConstRef<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConstRef<'arena>);

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
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[rust_to_ocaml(prefix = "cd_")]
#[repr(C)]
pub struct ConstDecl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub value: Option<&'a str>,
}
impl<'a> TrivialDrop for ConstDecl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ConstDecl<'arena>);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "ce_")]
#[repr(C)]
pub struct ClassElt<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub visibility: CeVisibility<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a lazy::Lazy<&'a Ty<'a>>,
    /// identifies the class from which this elt originates
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub origin: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deprecated: Option<&'a str>,
    /// pos of the type of the elt
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a lazy::Lazy<&'a pos_or_decl::PosOrDecl<'a>>,
    pub flags: typing_defs_flags::class_elt::ClassElt,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sort_text: Option<&'a str>,
}
impl<'a> TrivialDrop for ClassElt<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassElt<'arena>);

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
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[rust_to_ocaml(prefix = "fe_")]
#[repr(C)]
pub struct FunElt<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deprecated: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<ast_defs::Id<'a>>,
    /// Top-level functions have limited visibilities
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub php_std_lib: bool,
    pub support_dynamic_type: bool,
    pub no_auto_dynamic: bool,
    pub no_auto_likes: bool,
}
impl<'a> TrivialDrop for FunElt<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FunElt<'arena>);

pub use oxidized::typing_defs::ClassConstKind;

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst<'a> {
    pub synthesized: bool,
    pub abstract_: oxidized::typing_defs::ClassConstKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    /// identifies the class from which this const originates
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub origin: &'a str,
    /// references to the constants used in the initializer
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub refs: &'a [ClassConstRef<'a>],
}
impl<'a> TrivialDrop for ClassConst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassConst<'arena>);

#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum ModuleReference<'a> {
    MRGlobal,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MRPrefix(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    MRExact(&'a str),
}
impl<'a> TrivialDrop for ModuleReference<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ModuleReference<'arena>);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "mdt_")]
#[repr(C)]
pub struct ModuleDefType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub exports: Option<&'a [ModuleReference<'a>]>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub imports: Option<&'a [ModuleReference<'a>]>,
}
impl<'a> TrivialDrop for ModuleDefType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ModuleDefType<'arena>);

/// The position is that of the hint in the `use` / `implements` AST node
/// that causes a class to have this requirement applied to it. E.g.
///
/// ```
/// class Foo {}
///
/// interface Bar {
///   require extends Foo; <- position of the decl_phase ty
/// }
///
/// class Baz extends Foo implements Bar { <- position of the `implements`
/// }
/// ```
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct Requirement<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Ty<'a>,
);
impl<'a> TrivialDrop for Requirement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Requirement<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "atc_")]
#[repr(C)]
pub struct AbstractTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub as_constraint: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub super_constraint: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub default: Option<&'a Ty<'a>>,
}
impl<'a> TrivialDrop for AbstractTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(AbstractTypeconst<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct ConcreteTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tc_type: &'a Ty<'a>,
}
impl<'a> TrivialDrop for ConcreteTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ConcreteTypeconst<'arena>);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "patc_")]
#[repr(C)]
pub struct PartiallyAbstractTypeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for PartiallyAbstractTypeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PartiallyAbstractTypeconst<'arena>);

#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum Typeconst<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCAbstract(&'a AbstractTypeconst<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TCConcrete(&'a ConcreteTypeconst<'a>),
}
impl<'a> TrivialDrop for Typeconst<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Typeconst<'arena>);

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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "ttc_")]
#[repr(C)]
pub struct TypeconstType<'a> {
    pub synthesized: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub kind: Typeconst<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub origin: &'a str,
    /// If the typeconst had the <<__Enforceable>> attribute on its
    /// declaration, this will be [(position_of_declaration, true)].
    ///
    /// In legacy decl, the second element of the tuple will also be true if
    /// the typeconst overrides some parent typeconst which had the
    /// <<__Enforceable>> attribute. In that case, the position will point to
    /// the declaration of the parent typeconst.
    ///
    /// In shallow decl, this is not the case--there is no overriding behavior
    /// modeled here, and the second element will only be true when the
    /// declaration of this typeconst had the attribute.
    ///
    /// When the second element of the tuple is false, the position will be
    /// [Pos_or_decl.none].
    ///
    /// To manage the difference between legacy and shallow decl, use
    /// [Typing_classes_heap.Api.get_typeconst_enforceability] rather than
    /// accessing this field directly.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enforceable: (&'a pos_or_decl::PosOrDecl<'a>, bool),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reifiable: Option<&'a pos_or_decl::PosOrDecl<'a>>,
    pub concretized: bool,
    pub is_ctx: bool,
}
impl<'a> TrivialDrop for TypeconstType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypeconstType<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "te_")]
#[repr(C)]
pub struct EnumType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub base: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraint: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub includes: &'a [&'a Ty<'a>],
}
impl<'a> TrivialDrop for EnumType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(EnumType<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "td_")]
#[repr(C)]
pub struct TypedefType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<ast_defs::Id<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub vis: oxidized::ast_defs::TypedefVisibility,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub as_constraint: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub super_constraint: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    pub is_ctx: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub attributes: &'a [&'a UserAttribute<'a>],
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
}
impl<'a> TrivialDrop for TypedefType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TypedefType<'arena>);

#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum DeserializationError<'a> {
    /// The type was valid, but some component thereof was a decl_ty when we
    /// expected a locl_phase ty, or vice versa.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Wrong_phase")]
    WrongPhase(&'a str),
    /// The specific type or some component thereof is not one that we support
    /// deserializing, usually because not enough information was serialized to be
    /// able to deserialize it again.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Not_supported")]
    NotSupported(&'a str),
    /// The input JSON was invalid for some reason.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Deserialization_error")]
    DeserializationError(&'a str),
}
impl<'a> TrivialDrop for DeserializationError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeserializationError<'arena>);
