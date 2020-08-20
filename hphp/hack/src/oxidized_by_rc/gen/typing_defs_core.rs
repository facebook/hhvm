// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<20f6f595b01549d89c1946e1e7929e92>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_rc/regen.sh

use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_reason as reason;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Visibility {
    Vpublic,
    Vprivate(std::rc::Rc<String>),
    Vprotected(std::rc::Rc<String>),
}

pub use oxidized_by_ref::typing_defs_core::Exact;

pub use oxidized_by_ref::typing_defs_core::ValKind;

pub use oxidized_by_ref::typing_defs_core::ParamMutability;

pub use oxidized_by_ref::typing_defs_core::FunTparamsKind;

pub use oxidized_by_ref::typing_defs_core::ShapeKind;

pub use oxidized_by_ref::typing_defs_core::ParamMode;

pub use oxidized_by_ref::typing_defs_core::XhpAttrTag;

pub use oxidized_by_ref::typing_defs_core::XhpAttr;

pub use oxidized_by_ref::typing_defs_core::ConsistentKind;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum DependentType {
    DTthis,
    DTcls(std::rc::Rc<String>),
    DTexpr(ident::Ident),
}

pub use oxidized_by_ref::typing_defs_core::DestructureKind;

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Tparam {
    pub variance: oxidized::ast_defs::Variance,
    pub name: ast_defs::Id,
    pub tparams: Vec<Tparam>,
    pub constraints: Vec<(oxidized::ast_defs::ConstraintKind, Ty)>,
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: Vec<nast::UserAttribute>,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct WhereConstraint(pub Ty, pub oxidized::ast_defs::ConstraintKind, pub Ty);

#[derive(Clone, Debug, Hash, Serialize, ToOcamlRep)]
pub struct Ty(pub std::rc::Rc<reason::Reason>, pub std::rc::Rc<Ty_>);

/// A shape may specify whether or not fields are required. For example, consider
/// this typedef:
///
/// ```
/// type ShapeWithOptionalField = shape(?'a' => ?int);
/// ```
///
/// With this definition, the field 'a' may be unprovided in a shape. In this
/// case, the field 'a' would have sf_optional set to true.
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Ty_ {
    /// The late static bound type of a class
    Tthis,
    /// Either an object type or a type alias, ty list are the arguments
    Tapply(nast::Sid, Vec<Ty>),
    /// Name of class, name of type const, remaining names of type consts
    Taccess(TaccessType),
    /// The type of the various forms of "array":
    ///
    /// ```
    /// Tarray (None, None)         => "array"
    /// Tarray (Some ty, None)      => "array<ty>"
    /// Tarray (Some ty1, Some ty2) => "array<ty1, ty2>"
    /// Tarray (None, Some ty)      => [invalid]
    /// ```
    Tarray(Option<Ty>, Option<Ty>),
    /// "Any" is the type of a variable with a missing annotation, and "mixed" is
    /// the type of a variable annotated as "mixed". THESE TWO ARE VERY DIFFERENT!
    /// Any unifies with anything, i.e., it is both a supertype and subtype of any
    /// other type. You can do literally anything to it; it's the "trust me" type.
    /// Mixed, on the other hand, is only a supertype of everything. You need to do
    /// a case analysis to figure out what it is (i.e., its elimination form).
    ///
    /// Here's an example to demonstrate:
    ///
    /// ```
    /// function f($x): int {
    ///   return $x + 1;
    /// }
    /// ```
    ///
    /// In that example, $x has type Tany. This unifies with anything, so adding
    /// one to it is allowed, and returning that as int is allowed.
    ///
    /// In contrast, if $x were annotated as mixed, adding one to that would be
    /// a type error -- mixed is not a subtype of int, and you must be a subtype
    /// of int to take part in addition. (The converse is true though -- int is a
    /// subtype of mixed.) A case analysis would need to be done on $x, via
    /// is_int or similar.
    ///
    /// mixed exists only in the decl_phase phase because it is desugared into ?nonnull
    /// during the localization phase.
    Tmixed,
    Tlike(Ty),
    /// Access to a Pocket Universe or Pocket Universes dependent type,
    /// denoted by Foo:@Bar.
    /// It might be unresolved at first (e.g. if Foo is a generic variable).
    /// Will be refined to Tpu, or to the actual type associated with an
    /// atom, once typechecking is successful.
    TpuAccess(Ty, nast::Sid),
    Tany(oxidized_by_ref::tany_sentinel::TanySentinel),
    Terr,
    Tnonnull,
    /// A dynamic type is a special type which sometimes behaves as if it were a
    /// top type; roughly speaking, where a specific value of a particular type is
    /// expected and that type is dynamic, anything can be given. We call this
    /// behaviour "coercion", in that the types "coerce" to dynamic. In other ways it
    /// behaves like a bottom type; it can be used in any sort of binary expression
    /// or even have object methods called from it. However, it is in fact neither.
    ///
    /// it captures dynamicism within function scope.
    /// See tests in typecheck/dynamic/ for more examples.
    Tdynamic,
    /// Nullable, called "option" in the ML parlance.
    Toption(Ty),
    /// All the primitive types: int, string, void, etc.
    Tprim(aast::Tprim),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    Tfun(FunType),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    Ttuple(Vec<Ty>),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    Tshape(
        oxidized_by_ref::typing_defs_core::ShapeKind,
        nast::shape_map::ShapeMap<ShapeFieldType>,
    ),
    Tvar(ident::Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    Tgeneric(std::rc::Rc<String>, Vec<Ty>),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
    /// Tdarray (ty1, ty2) => "darray<ty1, ty2>"
    Tdarray(Ty, Ty),
    /// Tvarray (ty) => "varray<ty>"
    Tvarray(Ty),
    /// Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>"
    TvarrayOrDarray(Ty, Ty),
    /// This represents a type alias that lacks necessary type arguments. Given
    /// type Foo<T1,T2> = ...
    /// Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
    /// further type arguments. In particular, Tunappliedalias always stands for
    /// a higher-kinded type. It is never used for an alias like
    /// type Foo2 = ...
    /// that simply doesn't require type arguments.
    TunappliedAlias(std::rc::Rc<String>),
    /// The type of an opaque type (e.g. a "newtype" outside of the file where it
    /// was defined) or enum. They are "opaque", which means that they only unify with
    /// themselves. However, it is possible to have a constraint that allows us to
    /// relax this. For example:
    ///
    ///   newtype my_type as int = ...
    ///
    /// Outside of the file where the type was defined, this translates to:
    ///
    ///   Tnewtype ((pos, "my_type"), [], Tprim Tint)
    ///
    /// Which means that my_type is abstract, but is subtype of int as well.
    Tnewtype(std::rc::Rc<String>, Vec<Ty>, Ty),
    /// see dependent_type
    Tdependent(DependentType, Ty),
    /// Tobject is an object type compatible with all objects. This type is also
    /// compatible with some string operations (since a class might implement
    /// __toString), but not with string type hints.
    ///
    /// Tobject is currently used to type code like:
    ///   ../test/typecheck/return_unknown_class.php
    Tobject,
    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    Tclass(nast::Sid, oxidized_by_ref::typing_defs_core::Exact, Vec<Ty>),
    /// Typing of Pocket Universe Expressions
    /// - first parameter is the enclosing class
    /// - second parameter is the name of the Pocket Universe Enumeration
    Tpu(Ty, nast::Sid),
    /// Typing of Pocket Universes type projections
    /// - first parameter is the Tgeneric in place of the member name
    /// - second parameter is the name of the type to project
    TpuTypeAccess(nast::Sid, nast::Sid),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ConstraintType_ {
    ThasMember(HasMember),
    /// The type of container destructuring via list() or splat `...`
    Tdestructure(Destructure),
    TCunion(Ty, ConstraintType),
    TCintersection(Ty, ConstraintType),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct HasMember {
    pub name: nast::Sid,
    pub type_: Ty,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    pub class_id: nast::ClassId_,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct Destructure {
    /// This represents the standard parameters of a function or the fields in a list
    /// destructuring assignment. Example:
    ///
    /// function take(bool $b, float $f = 3.14, arraykey ...$aks): void {}
    /// function f((bool, float, int, string) $tup): void {
    ///   take(...$tup);
    /// }
    ///
    /// corresponds to the subtyping assertion
    ///
    /// (bool, float, int, string) <: splat([#1], [opt#2], ...#3)
    pub required: Vec<Ty>,
    /// Represents the optional parameters in a function, only used for splats
    pub optional: Vec<Ty>,
    /// Represents a function's variadic parameter, also only used for splats
    pub variadic: Option<Ty>,
    /// list() destructuring allows for partial matches on lists, even when the operation
    /// might throw i.e. list($a) = vec[];
    pub kind: oxidized_by_ref::typing_defs_core::DestructureKind,
}

#[derive(Clone, Debug, Hash, Serialize, ToOcamlRep)]
pub struct ConstraintType(
    pub std::rc::Rc<reason::Reason>,
    pub std::rc::Rc<ConstraintType_>,
);

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum InternalType {
    LoclType(Ty),
    ConstraintType(ConstraintType),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct TaccessType(pub Ty, pub Vec<nast::Sid>);

/// represents reactivity of function
/// - None corresponds to non-reactive function
/// - Some reactivity - to reactive function with specified reactivity flavor
///
/// Nonreactive <: Local -t <: Shallow -t <: Reactive -t
///
/// MaybeReactive represents conditional reactivity of function that depends on
/// reactivity of function arguments
///
/// ```
///   <<__Rx>>
///   function f(<<__MaybeRx>> $g) { ... }
/// ```
///
/// call to function f will be treated as reactive only if $g is reactive
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum Reactivity {
    Nonreactive,
    Local(Option<Ty>),
    Shallow(Option<Ty>),
    Reactive(Option<Ty>),
    Pure(Option<Ty>),
    MaybeReactive(std::rc::Rc<Reactivity>),
    RxVar(Option<std::rc::Rc<Reactivity>>),
}

/// The type of a function AND a method.
/// A function has a min and max arity because of optional arguments
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FunType {
    pub arity: FunArity,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub params: FunParams,
    /// Carries through the sync/async information from the aast
    pub ret: PossiblyEnforcedTy,
    pub reactive: Reactivity,
    pub flags: oxidized_by_ref::typing_defs_flags::FunTypeFlags,
}

/// Arity information for a fun_type; indicating the minimum number of
/// args expected by the function and the maximum number of args for
/// standard, non-variadic functions or the type of variadic argument taken
#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum FunArity {
    Fstandard,
    /// PHP5.6-style ...$args finishes the func declaration.
    /// min ; variadic param type
    Fvariadic(FunParam),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub enum ParamRxAnnotation {
    ParamRxVar,
    ParamRxIfImpl(Ty),
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct PossiblyEnforcedTy {
    /// True if consumer of this type enforces it at runtime
    pub enforced: bool,
    pub type_: Ty,
}

#[derive(
    Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, ToOcamlRep
)]
pub struct FunParam {
    pub pos: std::rc::Rc<pos::Pos>,
    pub name: Option<std::rc::Rc<String>>,
    pub type_: PossiblyEnforcedTy,
    pub rx_annotation: Option<ParamRxAnnotation>,
    pub flags: oxidized_by_ref::typing_defs_flags::FunParamFlags,
}

pub type FunParams = Vec<FunParam>;
