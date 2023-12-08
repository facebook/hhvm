// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9367bd9de031f10a0839cb9aa82eeb04>>
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
    FromOcamlRepIn,
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
pub struct SubstContext<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub subst: s_map::SMap<'a, &'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub class_context: &'a str,
    pub from_req_extends: bool,
}
impl<'a> TrivialDrop for SubstContext<'a> {}
arena_deserializer::impl_deserialize_in_arena!(SubstContext<'arena>);

pub use oxidized::decl_defs::SourceType;

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
pub enum DeclError<'a> {
    #[rust_to_ocaml(name = "Wrong_extend_kind")]
    WrongExtendKind {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        pos: &'a pos::Pos<'a>,
        kind: oxidized::ast_defs::ClassishKind,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: &'a str,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        parent_pos: &'a pos_or_decl::PosOrDecl<'a>,
        parent_kind: oxidized::ast_defs::ClassishKind,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        parent_name: &'a str,
    },
    #[rust_to_ocaml(name = "Wrong_use_kind")]
    WrongUseKind {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        pos: &'a pos::Pos<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        name: &'a str,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        parent_pos: &'a pos_or_decl::PosOrDecl<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        parent_name: &'a str,
    },
    #[rust_to_ocaml(name = "Cyclic_class_def")]
    CyclicClassDef {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        pos: &'a pos::Pos<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        stack: s_set::SSet<'a>,
    },
}
impl<'a> TrivialDrop for DeclError<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeclError<'arena>);

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
#[rust_to_ocaml(prefix = "elt_")]
#[repr(C)]
pub struct Element<'a> {
    pub flags: typing_defs_flags::class_elt::ClassElt,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub origin: &'a str,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub visibility: CeVisibility<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deprecated: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sort_text: Option<&'a str>,
}
impl<'a> TrivialDrop for Element<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Element<'arena>);

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
#[rust_to_ocaml(prefix = "dc_")]
#[repr(C)]
pub struct DeclClassType<'a> {
    pub need_init: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub const_: bool,
    pub internal: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassishKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub module: Option<ast_defs::Id<'a>>,
    pub is_module_level_trait: bool,
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
    /// dc_req_class_ancestors gathers all the `require class`
    /// requirements declared in ancestors.  Remark that `require class`
    /// requirements are _not_ stored in `dc_req_ancestors` or
    /// `dc_req_ancestors_extends` fields.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub req_class_ancestors: &'a [&'a Requirement<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub extends: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_attr_deps: s_set::SSet<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub xhp_enum_values: s_map::SMap<'a, &'a [ast_defs::XhpEnumValue<'a>]>,
    pub xhp_marked_empty: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub enum_type: Option<&'a EnumType<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub decl_errors: &'a [DeclError<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub docs_url: Option<&'a str>,
    /// Wether this interface has attribute __AllowMultipleInstantiations.
    pub allow_multiple_instantiations: bool,
}
impl<'a> TrivialDrop for DeclClassType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(DeclClassType<'arena>);
