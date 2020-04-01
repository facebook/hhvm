// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<c7983ed2cbef620692a8312ece86329a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
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
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
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
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum SourceType {
    Child,
    Parent,
    Trait,
    XHPAttr,
    Interface,
    ReqImpl,
    ReqExtends,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct MroElement {
    /// The class's name
    pub name: String,
    /// The position at which this element was directly included in the hierarchy.
    /// If C extends B extends A, the use_pos of A in C's linearization will be the
    /// position of the class name A in the line "class B extends A".
    pub use_pos: pos::Pos,
    /// Like mro_use_pos, but includes type arguments (if any).
    pub ty_pos: pos::Pos,
    /// The type arguments with which this ancestor class was instantiated. The
    /// first class in the linearization (the one which was linearized) will have
    /// an empty list here, even when it takes type parameters.
    pub type_args: Vec<Ty>,
    /// True if this element referred to a class whose definition could not be
    /// found. This is always indicative of an "Unbound name" error (emitted by
    /// Naming), so one could imagine omitting elements with this flag set from the
    /// linearization (since correct programs will not have them), but keeping
    /// track of them helps reduce cascading errors in the event of a typo.
    /// Additionally, it's helpful to do this (for now) to keep the behavior of
    /// shallow_class_decl equivalent to legacy decl.
    pub class_not_found: bool,
    /// When this is [Some], this mro_element represents an attempt to include a
    /// linearization within itself. We include these in the linearization for the
    /// sake of error reporting (they will not occur in correct programs). The
    /// SSet.t is the set of class names known to have been involved in the
    /// inclusion of this class in the linearization.
    pub cyclic: Option<s_set::SSet>,
    /// When this is [Some], this mro_element represents the use of a trait which
    /// was already used earlier in the linearization. Normally, we do not emit
    /// duplicate mro_elements at all--we include these in the linearization only
    /// for error detection. The string is the name of the class through which this
    /// trait was most recently included (as a duplicate).
    pub trait_reuse: Option<String>,
    /// If this element is included in the linearization because it was directly
    /// required by some ancestor, this will be [Some], and the position will be
    /// the location where this requirement was most recently included into the
    /// hierarchy.
    pub required_at: Option<pos::Pos>,
    /// True if this element is included in the linearization (directly or
    /// indirectly) because of a require extends relationship.
    pub via_req_extends: bool,
    /// True if this element is included in the linearization (directly or
    /// indirectly) because of a require implements relationship.
    pub via_req_impl: bool,
    /// True if this element is included in the linearization because of any
    /// XHP-attribute-inclusion relationship, and thus, the linearized class
    /// inherits only the XHP attributes from this element.
    pub xhp_attrs_only: bool,
    /// True if this element is included in the linearization because of a
    /// interface-implementation relationship, and thus, the linearized class
    /// inherits only the class constants and type constants from this element.
    pub consts_only: bool,
    /// True if this element is included in the linearization via an unbroken chain
    /// of trait-use relationships, and thus, the linearized class inherits the
    /// private members of this element (on account of the runtime behavior where
    /// they are effectively copied into the linearized class).
    pub copy_private_members: bool,
    /// True if this element is included in the linearization via an unbroken chain
    /// of abstract classes, and thus, abstract type constants with default values
    /// are inherited unchanged. When this flag is not set, a concrete class was
    /// present in the chain. Since we convert abstract type constants with
    /// defaults to concrete ones when they are included in a concrete class, any
    /// type constant which 1) is abstract, 2) has a default, and 3) was inherited
    /// from an ancestor with this flag not set, should be inherited as a concrete
    /// type constant instead.
    pub passthrough_abstract_typeconst: bool,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum LinearizationKind {
    MemberResolution,
    AncestorTypes,
}

/// name of condition type for conditional reactivity of methods.
/// If None - method is unconditionally reactive
pub type ConditionTypeName = Option<String>;

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub enum MethodReactivity {
    MethodReactive(ConditionTypeName),
    MethodShallow(ConditionTypeName),
    MethodLocal(ConditionTypeName),
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
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
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    /// class name to the subst_context that must be applied to that class
    pub substs: s_map::SMap<SubstContext>,
    pub consts: s_map::SMap<ClassConst>,
    pub typeconsts: s_map::SMap<TypeconstType>,
    pub pu_enums: s_map::SMap<PuEnumType>,
    pub props: s_map::SMap<Element>,
    pub sprops: s_map::SMap<Element>,
    pub methods: s_map::SMap<Element>,
    pub smethods: s_map::SMap<Element>,
    pub construct: (Option<Element>, ConsistentKind),
    pub ancestors: s_map::SMap<Ty>,
    pub req_ancestors: Vec<Requirement>,
    pub req_ancestors_extends: s_set::SSet,
    pub extends: s_set::SSet,
    pub sealed_whitelist: Option<s_set::SSet>,
    pub xhp_attr_deps: s_set::SSet,
    pub enum_type: Option<EnumType>,
    pub decl_errors: Option<errors::Errors>,
    /// this field is used to prevent condition types being filtered
    /// in Decl_redecl_service.is_dependent_class_of_any
    pub condition_types: s_set::SSet,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    Hash,
    OcamlRep,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize
)]
pub struct Element {
    pub final_: bool,
    pub synthesized: bool,
    /// Only relevant for methods
    pub override_: bool,
    /// Only relevant for methods
    pub dynamicallycallable: bool,
    pub memoizelsb: bool,
    pub abstract_: bool,
    pub reactivity: Option<MethodReactivity>,
    /// Only relevant for properties
    pub xhp_attr: Option<XhpAttr>,
    pub const_: bool,
    pub lateinit: bool,
    pub lsb: bool,
    pub origin: String,
    pub visibility: Visibility,
    pub fixme_codes: i_set::ISet,
    pub deprecated: Option<String>,
}
