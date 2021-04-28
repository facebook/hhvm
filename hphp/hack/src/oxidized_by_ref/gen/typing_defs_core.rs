// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<87bd3445e35cd37e0cab5437943449f3>>
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

pub use crate::t_shape_map;
pub use crate::typing_reason as reason;

pub use reason::PosId;

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
pub enum CeVisibility<'a> {
    Vpublic,
    Vprivate(&'a str),
    Vprotected(&'a str),
}
impl<'a> TrivialDrop for CeVisibility<'a> {}

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
pub enum IfcFunDecl<'a> {
    FDPolicied(Option<&'a str>),
    FDInferFlows,
}
impl<'a> TrivialDrop for IfcFunDecl<'a> {}

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
pub enum Exact {
    Exact,
    Nonexact,
}
impl TrivialDrop for Exact {}

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
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}
impl TrivialDrop for ValKind {}

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
impl TrivialDrop for FunTparamsKind {}

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
pub enum ShapeKind {
    ClosedShape,
    OpenShape,
}
impl TrivialDrop for ShapeKind {}

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
pub struct PosString<'a>(pub &'a pos_or_decl::PosOrDecl<'a>, pub &'a str);
impl<'a> TrivialDrop for PosString<'a> {}

pub type TByteString<'a> = str;

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
pub struct PosByteString<'a>(pub &'a pos_or_decl::PosOrDecl<'a>, pub &'a bstr::BStr);
impl<'a> TrivialDrop for PosByteString<'a> {}

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
pub enum TshapeFieldName<'a> {
    TSFlitInt(&'a PosString<'a>),
    TSFlitStr(&'a PosByteString<'a>),
    TSFclassConst(&'a (PosId<'a>, PosString<'a>)),
}
impl<'a> TrivialDrop for TshapeFieldName<'a> {}

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
pub enum ParamMode {
    FPnormal,
    FPinout,
}
impl TrivialDrop for ParamMode {}

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
pub enum XhpAttrTag {
    Required,
    Lateinit,
}
impl TrivialDrop for XhpAttrTag {}

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
pub struct XhpAttr {
    pub tag: Option<XhpAttrTag>,
    pub has_default: bool,
}
impl TrivialDrop for XhpAttr {}

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
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
}
impl TrivialDrop for ConsistentKind {}

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
pub enum DependentType {
    DTexpr(ident::Ident),
}
impl TrivialDrop for DependentType {}

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
pub struct UserAttribute<'a> {
    pub name: PosId<'a>,
    pub classname_params: &'a [&'a str],
}
impl<'a> TrivialDrop for UserAttribute<'a> {}

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
pub struct Tparam<'a> {
    pub variance: oxidized::ast_defs::Variance,
    pub name: PosId<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Ty<'a>)],
    pub reified: oxidized::aast::ReifyKind,
    pub user_attributes: &'a [&'a UserAttribute<'a>],
}
impl<'a> TrivialDrop for Tparam<'a> {}

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
pub struct WhereConstraint<'a>(
    pub &'a Ty<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    pub &'a Ty<'a>,
);
impl<'a> TrivialDrop for WhereConstraint<'a> {}

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
pub enum CollectionStyle {
    VecStyle,
    DictStyle,
    KeysetStyle,
}
impl TrivialDrop for CollectionStyle {}

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
pub enum Enforcement<'a> {
    Unenforced,
    Enforced,
    PartiallyEnforced(&'a (CollectionStyle, PosId<'a>)),
}
impl<'a> TrivialDrop for Enforcement<'a> {}

#[derive(Clone, Debug, FromOcamlRepIn, Hash, NoPosHash, Serialize, ToOcamlRep)]
pub struct Ty<'a>(pub &'a reason::T_<'a>, pub Ty_<'a>);
impl<'a> TrivialDrop for Ty<'a> {}

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
pub struct ShapeFieldType<'a> {
    pub optional: bool,
    pub ty: &'a Ty<'a>,
}
impl<'a> TrivialDrop for ShapeFieldType<'a> {}

#[derive(
    Clone,
    Copy,
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
pub enum Ty_<'a> {
    /// The late static bound type of a class
    Tthis,
    /// Either an object type or a type alias, ty list are the arguments
    Tapply(&'a (PosId<'a>, &'a [&'a Ty<'a>])),
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
    Tlike(&'a Ty<'a>),
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
    Toption(&'a Ty<'a>),
    /// All the primitive types: int, string, void, etc.
    Tprim(&'a aast::Tprim),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    Tfun(&'a FunType<'a>),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    Ttuple(&'a [&'a Ty<'a>]),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    Tshape(
        &'a (
            ShapeKind,
            t_shape_map::TShapeMap<'a, &'a ShapeFieldType<'a>>,
        ),
    ),
    Tvar(ident::Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    Tgeneric(&'a (&'a str, &'a [&'a Ty<'a>])),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    Tunion(&'a [&'a Ty<'a>]),
    Tintersection(&'a [&'a Ty<'a>]),
    /// Tdarray (ty1, ty2) => "darray<ty1, ty2>"
    Tdarray(&'a (&'a Ty<'a>, &'a Ty<'a>)),
    /// Tvarray (ty) => "varray<ty>"
    Tvarray(&'a Ty<'a>),
    /// Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>"
    TvarrayOrDarray(&'a (&'a Ty<'a>, &'a Ty<'a>)),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    TvecOrDict(&'a (&'a Ty<'a>, &'a Ty<'a>)),
    /// Name of class, name of type const, remaining names of type consts
    Taccess(&'a TaccessType<'a>),
    /// This represents a type alias that lacks necessary type arguments. Given
    /// type Foo<T1,T2> = ...
    /// Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
    /// further type arguments. In particular, Tunappliedalias always stands for
    /// a higher-kinded type. It is never used for an alias like
    /// type Foo2 = ...
    /// that simply doesn't require type arguments.
    TunappliedAlias(&'a str),
    /// The type of an opaque type or enum. Outside their defining files or
    /// when they represent enums, they are "opaque", which means that they
    /// only unify with themselves. Within a file, uses of newtypes are
    /// expanded to their definitions (unless the newtype is an enum).
    ///
    /// However, it is possible to have a constraint that allows us to relax
    /// opaqueness. For example:
    ///
    ///   newtype MyType as int = ...
    ///
    /// or
    ///
    ///   enum MyType: int as int { ... }
    ///
    /// Outside of the file where the type was defined, this translates to:
    ///
    ///   Tnewtype ((pos, "MyType"), [], Tprim Tint)
    ///
    /// which means that MyType is abstract, but is a subtype of int as well.
    /// When the constraint is omitted, the third parameter is set to mixed.
    ///
    /// The second parameter is the list of type arguments to the type.
    Tnewtype(&'a (&'a str, &'a [&'a Ty<'a>], &'a Ty<'a>)),
    /// see dependent_type
    Tdependent(&'a (DependentType, &'a Ty<'a>)),
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
    Tclass(&'a (PosId<'a>, Exact, &'a [&'a Ty<'a>])),
}
impl<'a> TrivialDrop for Ty_<'a> {}

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
pub struct TaccessType<'a>(pub &'a Ty<'a>, pub PosId<'a>);
impl<'a> TrivialDrop for TaccessType<'a> {}

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
pub enum Capability<'a> {
    CapDefaults(&'a pos_or_decl::PosOrDecl<'a>),
    CapTy(&'a Ty<'a>),
}
impl<'a> TrivialDrop for Capability<'a> {}

/// Companion to fun_params type, intended to consolidate checking of
/// implicit params for functions.
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
pub struct FunImplicitParams<'a> {
    pub capability: Capability<'a>,
}
impl<'a> TrivialDrop for FunImplicitParams<'a> {}

/// The type of a function AND a method.
/// A function has a min and max arity because of optional arguments
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
pub struct FunType<'a> {
    pub arity: FunArity<'a>,
    pub tparams: &'a [&'a Tparam<'a>],
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    pub params: &'a FunParams<'a>,
    pub implicit_params: &'a FunImplicitParams<'a>,
    /// Carries through the sync/async information from the aast
    pub ret: &'a PossiblyEnforcedTy<'a>,
    pub flags: typing_defs_flags::FunTypeFlags,
    pub ifc_decl: IfcFunDecl<'a>,
}
impl<'a> TrivialDrop for FunType<'a> {}

/// Arity information for a fun_type; indicating the minimum number of
/// args expected by the function and the maximum number of args for
/// standard, non-variadic functions or the type of variadic argument taken
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
pub enum FunArity<'a> {
    Fstandard,
    /// PHP5.6-style ...$args finishes the func declaration.
    /// min ; variadic param type
    Fvariadic(&'a FunParam<'a>),
}
impl<'a> TrivialDrop for FunArity<'a> {}

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
pub struct PossiblyEnforcedTy<'a> {
    /// True if consumer of this type enforces it at runtime
    pub enforced: Enforcement<'a>,
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for PossiblyEnforcedTy<'a> {}

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
pub struct FunParam<'a> {
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    pub name: Option<&'a str>,
    pub type_: &'a PossiblyEnforcedTy<'a>,
    pub flags: typing_defs_flags::FunParamFlags,
}
impl<'a> TrivialDrop for FunParam<'a> {}

pub type FunParams<'a> = [&'a FunParam<'a>];

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
pub enum DestructureKind {
    ListDestructure,
    SplatUnpack,
}
impl TrivialDrop for DestructureKind {}

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
pub struct Destructure<'a> {
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
    pub required: &'a [&'a Ty<'a>],
    /// Represents the optional parameters in a function, only used for splats
    pub optional: &'a [&'a Ty<'a>],
    /// Represents a function's variadic parameter, also only used for splats
    pub variadic: Option<&'a Ty<'a>>,
    /// list() destructuring allows for partial matches on lists, even when the operation
    /// might throw i.e. list($a) = vec[];
    pub kind: DestructureKind,
}
impl<'a> TrivialDrop for Destructure<'a> {}

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
pub struct HasMember<'a> {
    pub name: nast::Sid<'a>,
    pub type_: &'a Ty<'a>,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    pub class_id: &'a nast::ClassId_<'a>,
    pub explicit_targs: Option<&'a [&'a nast::Targ<'a>]>,
}
impl<'a> TrivialDrop for HasMember<'a> {}

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
pub enum ConstraintType_<'a> {
    ThasMember(&'a HasMember<'a>),
    /// The type of container destructuring via list() or splat `...`
    Tdestructure(&'a Destructure<'a>),
    TCunion(&'a (&'a Ty<'a>, ConstraintType<'a>)),
    TCintersection(&'a (&'a Ty<'a>, ConstraintType<'a>)),
}
impl<'a> TrivialDrop for ConstraintType_<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
pub struct ConstraintType<'a>(pub &'a reason::Reason<'a>, pub &'a ConstraintType_<'a>);
impl<'a> TrivialDrop for ConstraintType<'a> {}

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
pub enum InternalType<'a> {
    LoclType(&'a Ty<'a>),
    ConstraintType(ConstraintType<'a>),
}
impl<'a> TrivialDrop for InternalType<'a> {}
