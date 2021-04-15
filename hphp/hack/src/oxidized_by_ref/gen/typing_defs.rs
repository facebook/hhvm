// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<2c61962a3037f3e2b93584705f2bb1ac>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs_flags::*;

pub use typing_defs_core::*;

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
pub enum ClassConstFrom<'a> {
    Self_,
    From(&'a str),
}
impl<'a> TrivialDrop for ClassConstFrom<'a> {}

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
pub struct ClassConstRef<'a>(pub ClassConstFrom<'a>, pub &'a str);
impl<'a> TrivialDrop for ClassConstRef<'a> {}

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
pub struct ConstDecl<'a> {
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for ConstDecl<'a> {}

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
pub struct ClassElt<'a> {
    pub visibility: CeVisibility<'a>,
    pub type_: &'a lazy::Lazy<&'a Ty<'a>>,
    /// identifies the class from which this elt originates
    pub origin: &'a str,
    pub deprecated: Option<&'a str>,
    /// pos of the type of the elt
    pub pos: &'a lazy::Lazy<&'a pos_or_decl::PosOrDecl<'a>>,
    pub flags: isize,
}
impl<'a> TrivialDrop for ClassElt<'a> {}

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
pub struct FunElt<'a> {
    pub deprecated: Option<&'a str>,
    pub type_: &'a Ty<'a>,
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub php_std_lib: bool,
}
impl<'a> TrivialDrop for FunElt<'a> {}

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
pub struct ClassConst<'a> {
    pub synthesized: bool,
    pub abstract_: bool,
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub type_: &'a Ty<'a>,
    /// identifies the class from which this const originates
    pub origin: &'a str,
    /// references to the constants used in the initializer
    pub refs: &'a [ClassConstRef<'a>],
}
impl<'a> TrivialDrop for ClassConst<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
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
pub enum RecordFieldReq {
    ValueRequired,
    HasDefaultValue,
}
impl TrivialDrop for RecordFieldReq {}

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
pub struct RecordDefType<'a> {
    pub name: PosId<'a>,
    pub extends: Option<PosId<'a>>,
    pub fields: &'a [(PosId<'a>, RecordFieldReq)],
    pub abstract_: bool,
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
}
impl<'a> TrivialDrop for RecordDefType<'a> {}

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
pub struct Requirement<'a>(pub &'a pos_or_decl::PosOrDecl<'a>, pub &'a Ty<'a>);
impl<'a> TrivialDrop for Requirement<'a> {}

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
pub struct ClassType<'a> {
    pub need_init: bool,
    /// Whether the typechecker knows of all (non-interface) ancestors
    /// and thus knows all accessible members of this class
    /// This is not the case if one ancestor at least could not be found.
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    /// When a class is abstract (or in a trait) the initialization of
    /// a protected member can be delayed
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub is_disposable: bool,
    pub name: &'a str,
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    pub consts: s_map::SMap<'a, &'a ClassConst<'a>>,
    pub typeconsts: s_map::SMap<'a, &'a TypeconstType<'a>>,
    pub props: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub sprops: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub methods: s_map::SMap<'a, &'a ClassElt<'a>>,
    pub smethods: s_map::SMap<'a, &'a ClassElt<'a>>,
    /// the consistent_kind represents final constructor or __ConsistentConstruct
    pub construct: (Option<&'a ClassElt<'a>>, ConsistentKind),
    /// This includes all the classes, interfaces and traits this class is
    /// using.
    pub ancestors: s_map::SMap<'a, &'a Ty<'a>>,
    /// Whether the class is coercible to dynamic
    pub implements_dynamic: bool,
    pub req_ancestors: &'a [&'a Requirement<'a>],
    /// the extends of req_ancestors
    pub req_ancestors_extends: s_set::SSet<'a>,
    pub extends: s_set::SSet<'a>,
    pub enum_type: Option<&'a EnumType<'a>>,
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    pub decl_errors: Option<&'a errors::Errors<'a>>,
}
impl<'a> TrivialDrop for ClassType<'a> {}

#[derive(
    Clone,
    Copy,
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
pub enum TypeconstAbstractKind<'a> {
    TCAbstract(Option<&'a Ty<'a>>),
    TCPartiallyAbstract,
    TCConcrete,
}
impl<'a> TrivialDrop for TypeconstAbstractKind<'a> {}

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
pub struct TypeconstType<'a> {
    pub abstract_: TypeconstAbstractKind<'a>,
    pub synthesized: bool,
    pub name: PosId<'a>,
    pub as_constraint: Option<&'a Ty<'a>>,
    pub super_constraint: Option<&'a Ty<'a>>,
    pub type_: Option<&'a Ty<'a>>,
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
    pub enforceable: (&'a pos_or_decl::PosOrDecl<'a>, bool),
    pub reifiable: Option<&'a pos_or_decl::PosOrDecl<'a>>,
    pub concretized: bool,
}
impl<'a> TrivialDrop for TypeconstType<'a> {}

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
pub struct EnumType<'a> {
    pub base: &'a Ty<'a>,
    pub constraint: Option<&'a Ty<'a>>,
    pub includes: &'a [&'a Ty<'a>],
    pub enum_class: bool,
}
impl<'a> TrivialDrop for EnumType<'a> {}

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
pub struct TypedefType<'a> {
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub vis: oxidized::aast::TypedefVisibility,
    pub tparams: &'a [&'a Tparam<'a>],
    pub constraint: Option<&'a Ty<'a>>,
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for TypedefType<'a> {}

#[derive(
    Clone,
    Copy,
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
pub enum DeserializationError<'a> {
    /// The type was valid, but some component thereof was a decl_ty when we
    /// expected a locl_phase ty, or vice versa.
    WrongPhase(&'a str),
    /// The specific type or some component thereof is not one that we support
    /// deserializing, usually because not enough information was serialized to be
    /// able to deserialize it again.
    NotSupported(&'a str),
    /// The input JSON was invalid for some reason.
    DeserializationError(&'a str),
}
impl<'a> TrivialDrop for DeserializationError<'a> {}
