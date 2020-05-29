// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<27b7848c073f9173227c95a69efe0439>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
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
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct SubstContext<'a> {
    pub subst: s_map::SMap<'a, Ty<'a>>,
    pub class_context: &'a str,
    pub from_req_extends: bool,
}
impl<'a> TrivialDrop for SubstContext<'a> {}

pub use oxidized::decl_defs::SourceType;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct MroElement<'a> {
    /// The class's name
    pub name: &'a str,
    /// The position at which this element was directly included in the hierarchy.
    /// If C extends B extends A, the use_pos of A in C's linearization will be the
    /// position of the class name A in the line "class B extends A".
    pub use_pos: &'a pos::Pos<'a>,
    /// Like mro_use_pos, but includes type arguments (if any).
    pub ty_pos: &'a pos::Pos<'a>,
    /// The type arguments with which this ancestor class was instantiated. The
    /// first class in the linearization (the one which was linearized) will have
    /// an empty list here, even when it takes type parameters.
    pub type_args: &'a [Ty<'a>],
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
    pub cyclic: Option<s_set::SSet<'a>>,
    /// When this is [Some], this mro_element represents the use of a trait which
    /// was already used earlier in the linearization. Normally, we do not emit
    /// duplicate mro_elements at all--we include these in the linearization only
    /// for error detection. The string is the name of the class through which this
    /// trait was most recently included (as a duplicate).
    pub trait_reuse: Option<&'a str>,
    /// If this element is included in the linearization because it was directly
    /// required by some ancestor, this will be [Some], and the position will be
    /// the location where this requirement was most recently included into the
    /// hierarchy.
    pub required_at: Option<&'a pos::Pos<'a>>,
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
impl<'a> TrivialDrop for MroElement<'a> {}

pub use oxidized::decl_defs::LinearizationKind;

/// name of condition type for conditional reactivity of methods.
/// If None - method is unconditionally reactive
pub type ConditionTypeName<'a> = Option<&'a str>;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum MethodReactivity<'a> {
    MethodPure(ConditionTypeName<'a>),
    MethodReactive(ConditionTypeName<'a>),
    MethodShallow(ConditionTypeName<'a>),
    MethodLocal(ConditionTypeName<'a>),
}
impl<'a> TrivialDrop for MethodReactivity<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct DeclClassType<'a> {
    pub need_init: bool,
    pub members_fully_known: bool,
    pub abstract_: bool,
    pub final_: bool,
    pub is_disposable: bool,
    pub const_: bool,
    pub ppl: bool,
    pub deferred_init_members: s_set::SSet<'a>,
    pub kind: oxidized::ast_defs::ClassKind,
    pub is_xhp: bool,
    pub has_xhp_keyword: bool,
    pub name: &'a str,
    pub pos: &'a pos::Pos<'a>,
    pub tparams: &'a [Tparam<'a>],
    pub where_constraints: &'a [WhereConstraint<'a>],
    /// class name to the subst_context that must be applied to that class
    pub substs: s_map::SMap<'a, SubstContext<'a>>,
    pub consts: s_map::SMap<'a, ClassConst<'a>>,
    pub typeconsts: s_map::SMap<'a, TypeconstType<'a>>,
    pub pu_enums: s_map::SMap<'a, PuEnumType<'a>>,
    pub props: s_map::SMap<'a, Element<'a>>,
    pub sprops: s_map::SMap<'a, Element<'a>>,
    pub methods: s_map::SMap<'a, Element<'a>>,
    pub smethods: s_map::SMap<'a, Element<'a>>,
    pub construct: (Option<Element<'a>>, oxidized::decl_defs::ConsistentKind),
    pub ancestors: s_map::SMap<'a, Ty<'a>>,
    pub req_ancestors: &'a [Requirement<'a>],
    pub req_ancestors_extends: s_set::SSet<'a>,
    pub extends: s_set::SSet<'a>,
    pub sealed_whitelist: Option<s_set::SSet<'a>>,
    pub xhp_attr_deps: s_set::SSet<'a>,
    pub enum_type: Option<EnumType<'a>>,
    pub decl_errors: Option<errors::Errors<'a>>,
    /// this field is used to prevent condition types being filtered
    /// in Decl_redecl_service.is_dependent_class_of_any
    pub condition_types: s_set::SSet<'a>,
}
impl<'a> TrivialDrop for DeclClassType<'a> {}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Element<'a> {
    pub flags: isize,
    pub reactivity: Option<MethodReactivity<'a>>,
    pub origin: &'a str,
    pub visibility: Visibility<'a>,
    pub fixme_codes: i_set::ISet<'a>,
    pub deprecated: Option<&'a str>,
}
impl<'a> TrivialDrop for Element<'a> {}
