// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<875756161141535232a03946a68cf13a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
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
pub struct SubstContext<'a> {
    pub subst: s_map::SMap<'a, &'a Ty<'a>>,
    pub class_context: &'a str,
    pub from_req_extends: bool,
}
impl<'a> TrivialDrop for SubstContext<'a> {}

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
pub enum LinearizationKind {
    MemberResolution,
    AncestorTypes,
}
impl TrivialDrop for LinearizationKind {}

/// name of condition type for conditional reactivity of methods.
/// If None - method is unconditionally reactive
pub type ConditionTypeName<'a> = Option<&'a str>;

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
pub enum MethodReactivity<'a> {
    MethodPure(ConditionTypeName<'a>),
    MethodReactive(ConditionTypeName<'a>),
    MethodShallow(ConditionTypeName<'a>),
    MethodLocal(ConditionTypeName<'a>),
}
impl<'a> TrivialDrop for MethodReactivity<'a> {}

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
pub struct DeclClassType<'a> {
    pub need_init: bool,
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub is_disposable: bool,
    pub const_: bool,
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub name: &'a str,
    pub pos: &'a pos::Pos<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    /// class name to the subst_context that must be applied to that class
    pub substs: s_map::SMap<'a, &'a SubstContext<'a>>,
    pub consts: s_map::SMap<'a, &'a ClassConst<'a>>,
    pub typeconsts: s_map::SMap<'a, &'a TypeconstType<'a>>,
    pub pu_enums: s_map::SMap<'a, &'a PuEnumType<'a>>,
    pub props: s_map::SMap<'a, &'a Element<'a>>,
    pub sprops: s_map::SMap<'a, &'a Element<'a>>,
    pub methods: s_map::SMap<'a, &'a Element<'a>>,
    pub smethods: s_map::SMap<'a, &'a Element<'a>>,
    pub construct: (Option<&'a Element<'a>>, ConsistentKind),
    pub ancestors: s_map::SMap<'a, &'a Ty<'a>>,
    pub req_ancestors: &'a [&'a Requirement<'a>],
    pub req_ancestors_extends: s_set::SSet<'a>,
    pub extends: s_set::SSet<'a>,
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    pub xhp_attr_deps: s_set::SSet<'a>,
    pub enum_type: Option<&'a EnumType<'a>>,
    pub decl_errors: Option<&'a errors::Errors<'a>>,
    /// this field is used to prevent condition types being filtered
    /// in Decl_redecl_service.is_dependent_class_of_any
    pub condition_types: s_set::SSet<'a>,
}
impl<'a> TrivialDrop for DeclClassType<'a> {}

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
pub struct Element<'a> {
    pub flags: isize,
    pub reactivity: Option<MethodReactivity<'a>>,
    pub origin: &'a str,
    pub visibility: CeVisibility<'a>,
    pub deprecated: Option<&'a str>,
}
impl<'a> TrivialDrop for Element<'a> {}
