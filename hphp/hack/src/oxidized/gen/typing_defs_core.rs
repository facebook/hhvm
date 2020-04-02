// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ab8ae0f3afbf71280423d42cdbff779d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

pub use crate::typing_reason as reason;

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
pub enum Visibility {
    Vpublic,
    Vprivate(String),
    Vprotected(String),
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
pub enum Exact {
    Exact,
    Nonexact,
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
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
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
pub enum ParamMutability {
    ParamOwnedMutable,
    ParamBorrowedMutable,
    ParamMaybeMutable,
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
pub enum FunTparamsKind {
    /// If ft_tparams is empty, the containing fun_type is a concrete function type.
    /// Otherwise, it is a generic function and ft_tparams specifies its type parameters.
    FTKtparams,
    /// The containing fun_type is a concrete function type which is an
    /// instantiation of a generic function with at least one reified type
    /// parameter. This means that the function requires explicit type arguments
    /// at every invocation, and ft_tparams specifies the type arguments with
    /// which the generic function was instantiated, as well as whether each
    /// explicit type argument must be reified.
    FTKinstantiatedTargs,
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
pub enum ShapeKind {
    ClosedShape,
    OpenShape,
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
pub enum ParamMode {
    FPnormal,
    FPinout,
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
pub enum XhpAttrTag {
    Required,
    Lateinit,
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
pub struct XhpAttr {
    pub tag: Option<XhpAttrTag>,
    pub has_default: bool,
}

/// Denotes the categories of requirements we apply to constructor overrides.
///
/// In the default case, we use Inconsistent. If a class has <<__ConsistentConstruct>>,
/// or if it inherits a class that has <<__ConsistentConstruct>>, we use inherited.
/// If we have a new final class that doesn't extend from <<__ConsistentConstruct>>,
/// then we use Final. Only classes that are Inconsistent or Final can have reified
/// generics.
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
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
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
pub enum DependentType {
    DTthis,
    DTcls(String),
    DTexpr(ident::Ident),
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
pub enum DestructureKind {
    ListDestructure,
    SplatUnpack,
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
pub struct Tparam {
    pub variance: ast_defs::Variance,
    pub name: ast_defs::Id,
    pub constraints: Vec<(ast_defs::ConstraintKind, Ty)>,
    pub reified: aast::ReifyKind,
    pub user_attributes: Vec<nast::UserAttribute>,
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
pub struct WhereConstraint(pub Ty, pub ast_defs::ConstraintKind, pub Ty);

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
pub struct Ty(pub reason::Reason, pub Box<Ty_>);

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
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
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
    /// Tdarray (ty1, ty2) => "darray<ty1, ty2>"
    Tdarray(Ty, Ty),
    /// Tvarray (ty) => "varray<ty>"
    Tvarray(Ty),
    /// Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>"
    TvarrayOrDarray(Option<Ty>, Ty),
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
    /// denoted by Foo:@Bar, or Foo:@Bar:@Bli:@T.
    /// It might be unresolved at first (e.g. if Bli is a generic variable).
    /// Will be refined to Tpu, or to the actual type associated with an
    /// atom, once typechecking is successful.
    TpuAccess(Ty, nast::Sid, nast::PuLoc),
    Tany(tany_sentinel::TanySentinel),
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
    Tshape(ShapeKind, nast::shape_map::ShapeMap<ShapeFieldType>),
    Tvar(ident::Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.localize_generic_parameters_with_bounds.
    Tgeneric(String),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
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
    Tnewtype(String, Vec<Ty>, Ty),
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
    Tclass(nast::Sid, Exact, Vec<Ty>),
    /// Localized version of Tarray
    Tarraykind(ArrayKind),
    /// Typing of Pocket Universe Expressions
    /// - first parameter is the enclosing class
    /// - second parameter is the name of the Pocket Universe Enumeration
    Tpu(Ty, nast::Sid),
    /// Typing of Pocket Universes type projections
    /// - first parameter is the enclosing class
    /// - second parameter is the name of the Pocket Universe Enumeration
    /// - third parameter is the Tgeneric in place of the
    ///   member name
    /// - the fourth parameter is the name of the type to project
    TpuTypeAccess(Ty, nast::Sid, nast::Sid, nast::Sid),
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
pub enum ConstraintType_ {
    ThasMember(HasMember),
    /// The type of container destructuring via list() or splat `...`
    Tdestructure(Destructure),
    TCunion(Ty, ConstraintType),
    TCintersection(Ty, ConstraintType),
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
pub struct HasMember {
    pub name: nast::Sid,
    pub type_: Ty,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    pub class_id: nast::ClassId_,
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
    pub kind: DestructureKind,
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
pub struct ConstraintType(pub reason::Reason, pub Box<ConstraintType_>);

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
pub enum InternalType {
    LoclType(Ty),
    ConstraintType(ConstraintType),
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
pub enum ArrayKind {
    /// An array declared as a varray.
    AKvarray(Ty),
    /// An array declared as a darray.
    AKdarray(Ty, Ty),
    /// An array annotated as a varray_or_darray.
    AKvarrayOrDarray(Ty, Ty),
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
pub enum Reactivity {
    Nonreactive,
    Local(Option<Ty>),
    Shallow(Option<Ty>),
    Reactive(Option<Ty>),
    Pure(Option<Ty>),
    MaybeReactive(Box<Reactivity>),
    RxVar(Option<Box<Reactivity>>),
}

/// The type of a function AND a method.
/// A function has a min and max arity because of optional arguments
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
pub struct FunType {
    pub arity: FunArity,
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub params: FunParams,
    /// Carries through the sync/async information from the aast
    pub ret: PossiblyEnforcedTy,
    pub reactive: Reactivity,
    pub flags: isize,
}

/// Arity information for a fun_type; indicating the minimum number of
/// args expected by the function and the maximum number of args for
/// standard, non-variadic functions or the type of variadic argument taken
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
pub enum FunArity {
    /// min; max is List.length ft_params
    Fstandard(isize),
    /// PHP5.6-style ...$args finishes the func declaration.
    /// min ; variadic param type
    Fvariadic(isize, FunParam),
    /// HH-style ... anonymous variadic arg; body presumably uses func_get_args.
    /// min ; position of ...
    Fellipsis(isize, pos::Pos),
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
pub enum ParamRxAnnotation {
    ParamRxVar,
    ParamRxIfImpl(Ty),
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
pub struct PossiblyEnforcedTy {
    /// True if consumer of this type enforces it at runtime
    pub enforced: bool,
    pub type_: Ty,
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
pub struct FunParam {
    pub pos: pos::Pos,
    pub name: Option<String>,
    pub type_: PossiblyEnforcedTy,
    pub kind: ParamMode,
    pub accept_disposable: bool,
    pub mutability: Option<ParamMutability>,
    pub rx_annotation: Option<ParamRxAnnotation>,
}

pub type FunParams = Vec<FunParam>;
