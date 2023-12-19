// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<3a1227b45d5371f9c18f66a2e7d2519f>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
pub use reason::PosId;
use serde::Deserialize;
use serde::Serialize;

pub use crate::t_shape_map;
pub use crate::typing_reason as reason;
#[allow(unused_imports)]
use crate::*;

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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum CeVisibility {
    Vpublic,
    Vprivate(String),
    Vprotected(String),
    Vinternal(String),
}

#[rust_to_ocaml(attr = "deriving (eq, hash, ord, (show { with_path = false }))")]
pub type CrossPackageDecl = Option<String>;

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
#[rust_to_ocaml(attr = "deriving eq")]
#[repr(u8)]
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}
impl TrivialDrop for ValKind {}
arena_deserializer::impl_deserialize_in_arena!(ValKind);

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
#[rust_to_ocaml(attr = "deriving eq")]
#[repr(u8)]
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
    #[rust_to_ocaml(name = "FTKinstantiated_targs")]
    FTKinstantiatedTargs,
}
impl TrivialDrop for FunTparamsKind {}
arena_deserializer::impl_deserialize_in_arena!(FunTparamsKind);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C, u8)]
pub enum TypeOrigin {
    #[rust_to_ocaml(name = "Missing_origin")]
    MissingOrigin,
    #[rust_to_ocaml(name = "From_alias")]
    FromAlias(String),
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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C)]
pub struct PosString(pub pos_or_decl::PosOrDecl, pub String);

#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
pub type TByteString = String;

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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C)]
pub struct PosByteString(pub pos_or_decl::PosOrDecl, pub bstr::BString);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C, u8)]
pub enum TshapeFieldName {
    #[rust_to_ocaml(name = "TSFlit_int")]
    TSFlitInt(PosString),
    #[rust_to_ocaml(name = "TSFlit_str")]
    TSFlitStr(PosByteString),
    #[rust_to_ocaml(name = "TSFclass_const")]
    TSFclassConst(PosId, PosString),
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
pub enum ParamMode {
    FPnormal,
    FPinout,
}
impl TrivialDrop for ParamMode {}
arena_deserializer::impl_deserialize_in_arena!(ParamMode);

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type XhpAttr = xhp_attribute::XhpAttribute;

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
pub enum ConsistentKind {
    Inconsistent,
    ConsistentConstruct,
    FinalClass,
}
impl TrivialDrop for ConsistentKind {}
arena_deserializer::impl_deserialize_in_arena!(ConsistentKind);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
#[repr(C, u8)]
pub enum DependentType {
    DTexpr(isize),
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(C, u8)]
pub enum UserAttributeParam {
    Classname(String),
    EnumClassLabel(String),
    String(bstr::BString),
    Int(String),
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[rust_to_ocaml(prefix = "ua_")]
#[repr(C)]
pub struct UserAttribute {
    pub name: PosId,
    pub params: Vec<UserAttributeParam>,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam {
    pub variance: ast_defs::Variance,
    pub name: PosId,
    pub tparams: Vec<Tparam>,
    pub constraints: Vec<(ast_defs::ConstraintKind, Ty)>,
    pub reified: ast_defs::ReifyKind,
    pub user_attributes: Vec<UserAttribute>,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(C)]
pub struct WhereConstraint(pub Ty, pub ast_defs::ConstraintKind, pub Ty);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show, ord)")]
#[repr(u8)]
pub enum Enforcement {
    Unenforced,
    Enforced,
}
impl TrivialDrop for Enforcement {}
arena_deserializer::impl_deserialize_in_arena!(Enforcement);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
#[repr(C, u8)]
pub enum Capability {
    CapDefaults(pos_or_decl::PosOrDecl),
    CapTy(Ty),
}

/// Companion to fun_params type, intended to consolidate checking of
/// implicit params for functions.
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
#[repr(C)]
pub struct FunImplicitParams {
    pub capability: Capability,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct PossiblyEnforcedTy {
    /// True if consumer of this type enforces it at runtime
    pub enforced: Enforcement,
    pub type_: Ty,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
#[rust_to_ocaml(prefix = "fp_")]
#[repr(C)]
pub struct FunParam {
    #[rust_to_ocaml(attr = "hash.ignore")]
    #[rust_to_ocaml(attr = "equal fun _ -> fun _ -> true")]
    pub pos: pos_or_decl::PosOrDecl,
    pub name: Option<String>,
    pub type_: PossiblyEnforcedTy,
    pub flags: typing_defs_flags::fun_param::FunParam,
}

#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
pub type FunParams = Vec<FunParam>;

/// The type of a function AND a method.
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }))")]
#[rust_to_ocaml(prefix = "ft_")]
#[repr(C)]
pub struct FunType {
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub params: FunParams,
    pub implicit_params: FunImplicitParams,
    /// Carries through the sync/async information from the aast
    pub ret: PossiblyEnforcedTy,
    pub flags: typing_defs_flags::fun::Fun,
    pub cross_package: CrossPackageDecl,
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
#[rust_to_ocaml(attr = "deriving (hash, (show { with_path = false }))")]
#[repr(C, u8)]
pub enum NegType {
    #[rust_to_ocaml(name = "Neg_prim")]
    NegPrim(ast_defs::Tprim),
    #[rust_to_ocaml(name = "Neg_class")]
    NegClass(PosId),
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
#[repr(C)]
pub struct Ty(pub reason::T_, pub Box<Ty_>);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "sft_")]
#[repr(C)]
pub struct ShapeFieldType {
    pub optional: bool,
    pub ty: Ty,
}

#[derive(
    Clone,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Ty_ {
    /// The late static bound type of a class
    Tthis,
    /// Either an object type or a type alias, ty list are the arguments
    Tapply(PosId, Vec<Ty>),
    /// 'With' refinements of the form `_ with { type T as int; type TC = C; }`.
    Trefinement(Ty, ClassRefinement),
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
    /// Various intepretations, depending on context.
    ///   inferred type e.g. (vec<_> $x) ==> $x[0]
    ///   placeholder in refinement e.g. $x as Vector<_>
    ///   placeholder for higher-kinded formal type parameter e.g. foo<T1<_>>(T1<int> $_)
    Twildcard,
    Tlike(Ty),
    Tany(tany_sentinel::TanySentinel),
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
    Tprim(ast_defs::Tprim),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    Tfun(FunType),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    Ttuple(Vec<Ty>),
    Tshape(ShapeType),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    Tgeneric(String, Vec<Ty>),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    #[rust_to_ocaml(name = "Tvec_or_dict")]
    TvecOrDict(Ty, Ty),
    /// Name of class, name of type const, remaining names of type consts
    Taccess(TaccessType),
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
    Tnewtype(String, Vec<Ty>, Ty),
    Tvar(isize),
    /// This represents a type alias that lacks necessary type arguments. Given
    /// type Foo<T1,T2> = ...
    /// Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
    /// further type arguments. In particular, Tunappliedalias always stands for
    /// a higher-kinded type. It is never used for an alias like
    /// type Foo2 = ...
    /// that simply doesn't require type arguments.
    #[rust_to_ocaml(name = "Tunapplied_alias")]
    TunappliedAlias(String),
    /// see dependent_type
    Tdependent(DependentType, Ty),
    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    Tclass(PosId, Exact, Vec<Ty>),
    /// The negation of the type in neg_type
    Tneg(NegType),
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct TaccessType(pub Ty, pub PosId);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Exact {
    Exact,
    Nonexact(ClassRefinement),
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassRefinement {
    pub cr_consts: s_map::SMap<RefinedConst>,
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "rc_")]
#[repr(C)]
pub struct RefinedConst {
    pub bound: RefinedConstBound,
    pub is_ctx: bool,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum RefinedConstBound {
    TRexact(Ty),
    TRloose(RefinedConstBounds),
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tr_")]
#[repr(C)]
pub struct RefinedConstBounds {
    pub lower: Vec<Ty>,
    pub upper: Vec<Ty>,
}

/// Whether all fields of this shape are known, types of each of the
/// known arms.
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving hash")]
#[rust_to_ocaml(prefix = "s_")]
#[repr(C)]
pub struct ShapeType {
    pub origin: TypeOrigin,
    pub unknown_value: Ty,
    pub fields: t_shape_map::TShapeMap<ShapeFieldType>,
}

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(u8)]
pub enum DestructureKind {
    ListDestructure,
    SplatUnpack,
}
impl TrivialDrop for DestructureKind {}
arena_deserializer::impl_deserialize_in_arena!(DestructureKind);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
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
#[rust_to_ocaml(prefix = "d_")]
#[repr(C)]
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
#[rust_to_ocaml(prefix = "hm_")]
#[repr(C)]
pub struct HasMember {
    pub name: nast::Sid,
    pub type_: Ty,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    #[rust_to_ocaml(attr = "opaque")]
    pub class_id: nast::ClassId_,
    #[rust_to_ocaml(attr = "opaque")]
    pub explicit_targs: Option<Vec<nast::Targ>>,
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
#[rust_to_ocaml(prefix = "ci_")]
#[repr(C)]
pub struct CanIndex {
    pub key: Ty,
    pub shape: Option<TshapeFieldName>,
    pub val: Ty,
    pub expr_pos: pos::Pos,
    pub index_pos: pos::Pos,
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
#[rust_to_ocaml(prefix = "ct_")]
#[repr(C)]
pub struct CanTraverse {
    pub key: Option<Ty>,
    pub val: Ty,
    pub is_await: bool,
    pub reason: reason::Reason,
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
#[rust_to_ocaml(prefix = "htm_")]
#[repr(C)]
pub struct HasTypeMember {
    pub id: String,
    pub lower: Ty,
    pub upper: Ty,
}

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[repr(C, u8)]
pub enum ConstraintType_ {
    #[rust_to_ocaml(name = "Thas_member")]
    ThasMember(HasMember),
    #[rust_to_ocaml(name = "Thas_type_member")]
    ThasTypeMember(HasTypeMember),
    #[rust_to_ocaml(name = "Tcan_index")]
    TcanIndex(CanIndex),
    #[rust_to_ocaml(name = "Tcan_traverse")]
    TcanTraverse(CanTraverse),
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
    FromOcamlRep,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct ConstraintType(pub reason::Reason, pub Box<ConstraintType_>);

#[derive(
    Clone,
    Debug,
    Deserialize,
    Eq,
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
pub enum InternalType {
    LoclType(Ty),
    ConstraintType(ConstraintType),
}
