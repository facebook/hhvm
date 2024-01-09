// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<b08d45e342feb031f7cceeb2fa66525b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum ClassConstFrom {
    #[rust_to_ocaml(name = "Self")]
    Self_,
    From(String),
}

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct ClassConstRef(pub ClassConstFrom, pub String);

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
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[rust_to_ocaml(prefix = "cd_")]
#[repr(C)]
pub struct ConstDecl {
    pub pos: pos_or_decl::PosOrDecl,
    pub type_: Ty,
    pub value: Option<String>,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "ce_")]
#[repr(C)]
pub struct ClassElt {
    pub visibility: CeVisibility,
    pub type_: lazy::Lazy<Ty>,
    /// identifies the class from which this elt originates
    pub origin: String,
    pub deprecated: Option<String>,
    /// pos of the type of the elt
    pub pos: lazy::Lazy<pos_or_decl::PosOrDecl>,
    pub flags: typing_defs_flags::class_elt::ClassElt,
    pub sort_text: Option<String>,
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
#[rust_to_ocaml(attr = "deriving (show, eq)")]
#[rust_to_ocaml(prefix = "fe_")]
#[repr(C)]
pub struct FunElt {
    pub deprecated: Option<String>,
    pub module: Option<ast_defs::Id>,
    /// Top-level functions have limited visibilities
    pub internal: bool,
    pub type_: Ty,
    pub pos: pos_or_decl::PosOrDecl,
    pub php_std_lib: bool,
    pub support_dynamic_type: bool,
    pub no_auto_dynamic: bool,
    pub no_auto_likes: bool,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum ClassConstKind {
    CCAbstract(bool),
    CCConcrete,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "cc_")]
#[repr(C)]
pub struct ClassConst {
    pub synthesized: bool,
    pub abstract_: ClassConstKind,
    pub pos: pos_or_decl::PosOrDecl,
    pub type_: Ty,
    /// identifies the class from which this const originates
    pub origin: String,
    /// references to the constants used in the initializer
    pub refs: Vec<ClassConstRef>,
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum ModuleReference {
    MRGlobal,
    MRPrefix(String),
    MRExact(String),
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "mdt_")]
#[repr(C)]
pub struct ModuleDefType {
    pub pos: pos_or_decl::PosOrDecl,
    pub exports: Option<Vec<ModuleReference>>,
    pub imports: Option<Vec<ModuleReference>>,
}

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
#[repr(C)]
pub struct Requirement(pub pos_or_decl::PosOrDecl, pub Ty);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "atc_")]
#[repr(C)]
pub struct AbstractTypeconst {
    pub as_constraint: Option<Ty>,
    pub super_constraint: Option<Ty>,
    pub default: Option<Ty>,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct ConcreteTypeconst {
    pub tc_type: Ty,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "patc_")]
#[repr(C)]
pub struct PartiallyAbstractTypeconst {
    pub constraint: Ty,
    pub type_: Ty,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C, u8)]
pub enum Typeconst {
    TCAbstract(AbstractTypeconst),
    TCConcrete(ConcreteTypeconst),
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "ttc_")]
#[repr(C)]
pub struct TypeconstType {
    pub synthesized: bool,
    pub name: PosId,
    pub kind: Typeconst,
    pub origin: String,
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
    pub enforceable: (pos_or_decl::PosOrDecl, bool),
    pub reifiable: Option<pos_or_decl::PosOrDecl>,
    pub concretized: bool,
    pub is_ctx: bool,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "te_")]
#[repr(C)]
pub struct EnumType {
    pub base: Ty,
    pub constraint: Option<Ty>,
    pub includes: Vec<Ty>,
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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "td_")]
#[repr(C)]
pub struct TypedefType {
    pub module: Option<ast_defs::Id>,
    pub pos: pos_or_decl::PosOrDecl,
    pub vis: ast_defs::TypedefVisibility,
    pub tparams: Vec<Tparam>,
    pub as_constraint: Option<Ty>,
    pub super_constraint: Option<Ty>,
    pub type_: Ty,
    pub is_ctx: bool,
    pub attributes: Vec<UserAttribute>,
    pub internal: bool,
    pub docs_url: Option<String>,
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum DeserializationError {
    /// The type was valid, but some component thereof was a decl_ty when we
    /// expected a locl_phase ty, or vice versa.
    #[rust_to_ocaml(name = "Wrong_phase")]
    WrongPhase(String),
    /// The specific type or some component thereof is not one that we support
    /// deserializing, usually because not enough information was serialized to be
    /// able to deserialize it again.
    #[rust_to_ocaml(name = "Not_supported")]
    NotSupported(String),
    /// The input JSON was invalid for some reason.
    #[rust_to_ocaml(name = "Deserialization_error")]
    DeserializationError(String),
}
