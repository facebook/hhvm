// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<74c9757bbea9838dce84c7722c98ea9b>>
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
pub use typing_defs::*;

#[allow(unused_imports)]
use crate::*;

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
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(attr = "deriving (show, ord)")]
#[rust_to_ocaml(prefix = "sc_")]
#[repr(C)]
pub struct SubstContext {
    pub subst: s_map::SMap<Ty>,
    pub class_context: String,
    pub from_req_extends: bool,
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
#[repr(u8)]
pub enum SourceType {
    Child,
    Parent,
    Trait,
    XHPAttr,
    Interface,
    IncludedEnum,
    ReqImpl,
    ReqExtends,
    ReqClass,
}
impl TrivialDrop for SourceType {}
arena_deserializer::impl_deserialize_in_arena!(SourceType);

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
pub enum DeclError {
    #[rust_to_ocaml(name = "Wrong_extend_kind")]
    WrongExtendKind {
        pos: pos::Pos,
        kind: ast_defs::ClassishKind,
        name: String,
        parent_pos: pos_or_decl::PosOrDecl,
        parent_kind: ast_defs::ClassishKind,
        parent_name: String,
    },
    #[rust_to_ocaml(name = "Wrong_use_kind")]
    WrongUseKind {
        pos: pos::Pos,
        name: String,
        parent_pos: pos_or_decl::PosOrDecl,
        parent_name: String,
    },
    #[rust_to_ocaml(name = "Cyclic_class_def")]
    CyclicClassDef { pos: pos::Pos, stack: s_set::SSet },
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
#[rust_to_ocaml(prefix = "elt_")]
#[repr(C)]
pub struct Element {
    pub flags: typing_defs_flags::class_elt::ClassElt,
    pub origin: String,
    pub visibility: CeVisibility,
    pub deprecated: Option<String>,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "dc_")]
#[repr(C)]
pub struct DeclClassType {
    pub need_init: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    pub internal: bool,
    pub deferred_init_members: s_set::SSet,
    pub kind: ast_defs::ClassishKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub module: Option<ast_defs::Id>,
    pub is_module_level_trait: bool,
    pub name: String,
    pub pos: pos_or_decl::PosOrDecl,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    /// class name to the subst_context that must be applied to that class
    pub substs: s_map::SMap<SubstContext>,
    pub consts: s_map::SMap<ClassConst>,
    pub typeconsts: s_map::SMap<TypeconstType>,
    pub props: s_map::SMap<Element>,
    pub sprops: s_map::SMap<Element>,
    pub methods: s_map::SMap<Element>,
    pub smethods: s_map::SMap<Element>,
    pub construct: (Option<Element>, ConsistentKind),
    pub ancestors: s_map::SMap<Ty>,
    pub support_dynamic_type: bool,
    pub req_ancestors: Vec<Requirement>,
    pub req_ancestors_extends: s_set::SSet,
    /// dc_req_class_ancestors gathers all the `require class`
    /// requirements declared in ancestors.  Remark that `require class`
    /// requirements are _not_ stored in `dc_req_ancestors` or
    /// `dc_req_ancestors_extends` fields.
    pub req_class_ancestors: Vec<Requirement>,
    pub extends: s_set::SSet,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub xhp_attr_deps: s_set::SSet,
    pub xhp_enum_values: s_map::SMap<Vec<ast_defs::XhpEnumValue>>,
    pub xhp_marked_empty: bool,
    pub enum_type: Option<EnumType>,
    pub decl_errors: Vec<DeclError>,
    pub docs_url: Option<String>,
    /// Wether this interface has attribute __AllowMultipleInstantiations.
    pub allow_multiple_instantiations: bool,
}
