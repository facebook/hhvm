// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cd71e1ed74a32b13cdd01ff292b2625e>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;

use crate::ast_defs;
use crate::errors;
use crate::i_set;
use crate::pos;
use crate::s_map;
use crate::s_set;
use crate::sequence;

use crate::typing_defs::*;

#[derive(Clone, Debug, OcamlRep)]
pub struct SubstContext {
    pub subst: s_map::SMap<DeclTy>,
    pub class_context: String,
    pub from_req_extends: bool,
}

#[derive(Clone, Copy, Debug, Eq, OcamlRep, PartialEq)]
pub enum SourceType {
    Child,
    Parent,
    Trait,
    XHPAttr,
    Interface,
    ReqImpl,
    ReqExtends,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct MroElement {
    pub name: String,
    pub use_pos: pos::Pos,
    pub ty_pos: pos::Pos,
    pub type_args: Vec<DeclTy>,
    pub class_not_found: bool,
    pub cyclic: Option<s_set::SSet>,
    pub trait_reuse: Option<String>,
    pub required_at: Option<pos::Pos>,
    pub via_req_extends: bool,
    pub via_req_impl: bool,
    pub xhp_attrs_only: bool,
    pub consts_only: bool,
    pub copy_private_members: bool,
    pub passthrough_abstract_typeconst: bool,
}

pub type Linearization = sequence::Sequence<MroElement>;

pub type ConditionTypeName = Option<String>;

#[derive(Clone, Debug, OcamlRep)]
pub enum MethodReactivity {
    MethodReactive(ConditionTypeName),
    MethodShallow(ConditionTypeName),
    MethodLocal(ConditionTypeName),
}

#[derive(Clone, Debug, OcamlRep)]
pub struct DeclClassType {
    pub need_init: bool,
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub is_disposable: bool,
    pub const_: bool,
    pub ppl: bool,
    pub deferred_init_members: s_set::SSet,
    pub kind: ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub name: String,
    pub pos: pos::Pos,
    pub tparams: Vec<DeclTparam>,
    pub where_constraints: Vec<DeclWhereConstraint>,
    pub substs: s_map::SMap<SubstContext>,
    pub consts: s_map::SMap<ClassConst>,
    pub typeconsts: s_map::SMap<TypeconstType>,
    pub pu_enums: s_map::SMap<PuEnumType>,
    pub props: s_map::SMap<Element>,
    pub sprops: s_map::SMap<Element>,
    pub methods: s_map::SMap<Element>,
    pub smethods: s_map::SMap<Element>,
    pub construct: (Option<Element>, ConsistentKind),
    pub ancestors: s_map::SMap<DeclTy>,
    pub req_ancestors: Vec<Requirement>,
    pub req_ancestors_extends: s_set::SSet,
    pub extends: s_set::SSet,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub xhp_attr_deps: s_set::SSet,
    pub enum_type: Option<EnumType>,
    pub decl_errors: Option<errors::Errors>,
    pub condition_types: s_set::SSet,
}

#[derive(Clone, Debug, OcamlRep)]
pub struct Element {
    pub final_: bool,
    pub synthesized: bool,
    pub override_: bool,
    pub memoizelsb: bool,
    pub abstract_: bool,
    pub reactivity: Option<MethodReactivity>,
    pub xhp_attr: Option<XhpAttr>,
    pub const_: bool,
    pub lateinit: bool,
    pub lsb: bool,
    pub origin: String,
    pub visibility: Visibility,
    pub fixme_codes: i_set::ISet,
    pub deprecated: Option<String>,
}
