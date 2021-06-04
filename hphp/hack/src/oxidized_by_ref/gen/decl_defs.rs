// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e8b6511be6269ea07e77e8342650ee94>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use typing_defs::*;

/// A substitution context contains all the information necessary for
/// changing the type of an inherited class element to the class that is
/// inheriting the class element. It's best illustrated via an example.
///
/// ```
/// class A<Ta1, Ta2> { public function test(Ta1 $x, Ta2 $y): void {} }
///
/// class B<Tb> extends A<Tb, int> {}
///
/// class C extends B<string> {}
/// ```
///
/// The method `A::test()` has the type (function(Ta1, Ta2): void) in the
/// context of class A. However in the context of class B, it will have type
/// (function(Tb, int): void).
///
/// The substitution that leads to this change is [Ta1 -> Tb, Ta2 -> int],
/// which will produce a new type in the context of class B. It's subst_context
/// would then be:
///
/// ```
/// { sc_subst            = [Ta1 -> Tb, Ta2 -> int];
///   sc_class_context    = 'B';
///   sc_from_req_extends = false;
/// }
/// ```
///
/// The `sc_from_req_extends` field is set to true if the context was inherited
/// via a require extends type. This information is relevant when folding
/// `dc_substs` during inheritance. See Decl_inherit module.
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
pub struct SubstContext<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub subst: s_map::SMap<'a, &'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub class_context: &'a str,
    pub from_req_extends: bool,
}
impl<'a> TrivialDrop for SubstContext<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SubstContext<'arena>);

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
pub enum SourceType {
    Child,
    Parent,
    Trait,
    XHPAttr,
    Interface,
    IncludedEnum,
    ReqImpl,
    ReqExtends,
}
impl TrivialDrop for SourceType {}
arena_deserializer::impl_deserialize_in_arena!(SourceType);

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
pub enum LinearizationKind {
    MemberResolution,
    AncestorTypes,
}
impl TrivialDrop for LinearizationKind {}
arena_deserializer::impl_deserialize_in_arena!(LinearizationKind);

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
pub struct DeclClassType<'a> {
    pub need_init: bool,
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub is_disposable: bool,
    pub const_: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    /// class name to the subst_context that must be applied to that class
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub substs: s_map::SMap<'a, &'a SubstContext<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub consts: s_map::SMap<'a, &'a ClassConst<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub typeconsts: s_map::SMap<'a, &'a TypeconstType<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub props: s_map::SMap<'a, &'a Element<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sprops: s_map::SMap<'a, &'a Element<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub methods: s_map::SMap<'a, &'a Element<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub smethods: s_map::SMap<'a, &'a Element<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub construct: (Option<&'a Element<'a>>, ConsistentKind),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ancestors: s_map::SMap<'a, &'a Ty<'a>>,
    pub support_dynamic_type: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_ancestors: &'a [&'a Requirement<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_ancestors_extends: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr_deps: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_type: Option<&'a EnumType<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decl_errors: Option<&'a errors::Errors<'a>>,
    /// this field is used to prevent condition types being filtered
    /// in Decl_redecl_service.is_dependent_class_of_any
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub condition_types: s_set::SSet<'a>,
}
impl<'a> TrivialDrop for DeclClassType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeclClassType<'arena>);

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
pub struct Element<'a> {
    pub flags: isize,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub origin: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub visibility: CeVisibility<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deprecated: Option<&'a str>,
}
impl<'a> TrivialDrop for Element<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Element<'arena>);
