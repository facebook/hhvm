// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<cfe4776107c2527bd92f2a0fc9302a52>>
//
// To regenerate this file, run:
//   buck run @fbcode//mode/dev-nosan-lg fbcode//hphp/hack/src:oxidized_regen

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
    #[rust_to_ocaml(name = "Vprotected_internal")]
    VprotectedInternal {
        class_id: String,
        module__: String,
    },
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
#[rust_to_ocaml(attr = "deriving eq")]
#[repr(u8)]
pub enum ValKind {
    Lval,
    LvalSubexpr,
    Other,
}
impl TrivialDrop for ValKind {}
arena_deserializer::impl_deserialize_in_arena!(ValKind);

/// The origin of a type is a succinct key that is unique to the
/// type containing it. Consequently, two types with the same
/// origin are necessarily identical. Any change to a type with
/// origin needs to come with a *reset* of its origin. For example,
/// all type mappers have to reset origins to [Missing_origin].
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
    /// When we do not have any origin for the type. It is always
    /// correct to use [Missing_origin]; so when in doubt, use it.
    #[rust_to_ocaml(name = "Missing_origin")]
    MissingOrigin,
    /// A type with origin [From_alias orig] is equivalent to
    /// the expansion of the alias [orig].
    #[rust_to_ocaml(name = "From_alias")]
    FromAlias(String, Option<pos_or_decl::PosOrDecl>),
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

/// This is similar to Aast.shape_field_name, but contains Pos_or_decl.t
/// instead of Pos.t. Aast.shape_field_name is used in shape expressions,
/// while this is used in shape types.
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
    #[rust_to_ocaml(name = "TSFregex_group")]
    TSFregexGroup(PosString),
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
    pub raw_val: Option<String>,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show, map)")]
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam {
    pub variance: ast_defs::Variance,
    pub name: PosId,
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show, map)")]
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
    /// The consumer doesn't enforce the type at runtime
    Unenforced,
    /// The consumer enforces the type at runtime
    Enforced,
}
impl TrivialDrop for Enforcement {}
arena_deserializer::impl_deserialize_in_arena!(Enforcement);

/// Because Tfun is currently used as both a decl and locl ty, without this,
/// the HH\Contexts\defaults alias must be stored in shared memory for a
/// decl Tfun record. We can eliminate this if the majority of usages end up
/// explicit or if we separate decl and locl Tfuns.
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }), map)")]
#[repr(C, u8)]
pub enum Capability {
    /// Should not be used for lambda inference
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }), map)")]
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }), map)")]
#[rust_to_ocaml(prefix = "fp_")]
#[repr(C)]
pub struct FunParam {
    #[rust_to_ocaml(attr = "hash.ignore")]
    #[rust_to_ocaml(attr = "equal fun _ _ -> true")]
    pub pos: pos_or_decl::PosOrDecl,
    /// Only type-relevant when
    ///  `Option.is_some (Typing_defs.Named_params.name_of_named_param fp_name)`
    /// used:
    /// - for IDE features
    /// - for named params (use accessor above)
    pub name: Option<String>,
    pub type_: Ty,
    pub flags: typing_defs_flags::fun_param::FunParam,
    pub def_value: Option<String>,
}

#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }), map)")]
pub type FunParams = Vec<FunParam>;

/// The type of a function AND a method
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
#[rust_to_ocaml(attr = "deriving (eq, hash, (show { with_path = false }), map)")]
#[rust_to_ocaml(prefix = "ft_")]
#[repr(C)]
pub struct FunType {
    pub tparams: Vec<Tparam>,
    pub where_constraints: Vec<WhereConstraint>,
    pub params: FunParams,
    pub implicit_params: FunImplicitParams,
    /// Carries through the sync/async information from the aast
    pub ret: Ty,
    pub flags: typing_defs_flags::fun::Fun,
    pub instantiated: bool,
}

/// = Reason.t * 'phase ty_
#[derive(
    Clone,
    Debug,
    Deserialize,
    EqModuloPos,
    FromOcamlRep,
    Hash,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Ty(pub reason::T_, pub Box<Ty_>);

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
pub enum TypeTagGeneric {
    Filled(Ty),
    Wildcard(isize),
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
pub enum TypeTag {
    BoolTag,
    IntTag,
    ArraykeyTag,
    FloatTag,
    NumTag,
    ResourceTag,
    NullTag,
    ClassTag(ast_defs::Id_, Vec<TypeTagGeneric>),
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
#[rust_to_ocaml(prefix = "sfp_")]
#[repr(C)]
pub struct ShapeFieldPredicate {
    pub optional: bool,
    pub predicate: TypePredicate,
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
#[rust_to_ocaml(prefix = "sp_")]
#[repr(C)]
pub struct ShapePredicate {
    pub allows_unknown_fields: bool,
    pub fields: t_shape_map::TShapeMap<ShapeFieldPredicate>,
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
pub struct TuplePredicate {
    pub tp_required: Vec<TypePredicate>,
}

/// Represents the predicate of a type switch, i.e. in the expression
/// ```
///      if ($x is Bool) { ... } else { ... }
/// ```
///
/// The predicate would be `is Bool`
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
pub enum TypePredicate_ {
    IsTag(TypeTag),
    IsTupleOf(TuplePredicate),
    IsShapeOf(ShapePredicate),
    IsUnionOf(Vec<TypePredicate>),
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
pub struct TypePredicate(pub reason::Reason, pub Box<TypePredicate_>);

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
    /// A wrapper around tuple_type, which contains information about tuple elements
    Ttuple(TupleType),
    Tshape(ShapeType),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints.
    Tgeneric(String),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    #[rust_to_ocaml(attr = "transform.explicit")]
    Tunion(Vec<Ty>),
    Tintersection(Vec<Ty>),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    #[rust_to_ocaml(name = "Tvec_or_dict")]
    TvecOrDict(Ty, Ty),
    /// Name of class, name of type const, remaining names of type consts
    Taccess(TaccessType),
    /// A type of a class pointer, class<T>. To be compatible with classname<T>,
    /// it takes an arbitrary type. In the future, it should only take a string
    /// that is a class name, and be named Tclass. The current Tclass would be
    /// renamed to Tinstance, where a Tinstance is an instantiation of a Tclass
    #[rust_to_ocaml(name = "Tclass_ptr")]
    TclassPtr(Ty),
    Tvar(isize),
    /// The type of an opaque type or enum. Outside their defining files or
    /// when they represent enums, they are "opaque", which means that they
    /// only unify with themselves. Within a file, uses of newtypes are
    /// expanded to their definitions (unless the newtype is an enum).
    ///
    /// However, it is possible to have a constraint that allows us to relax
    /// opaqueness. For example:
    ///
    /// newtype MyType as int = ...
    ///
    /// or
    ///
    /// enum MyType: int as int { ... }
    ///
    /// Outside of the file where the type was defined, this translates to:
    ///
    /// Tnewtype ((pos, "MyType"), [], Tprim Tint)
    ///
    /// which means that MyType is abstract, but is a subtype of int as well.
    /// When the constraint is omitted, the third parameter is set to mixed.
    ///
    /// The second parameter is the list of type arguments to the type.
    Tnewtype(String, Vec<Ty>, Ty),
    /// see dependent_type
    Tdependent(DependentType, Ty),
    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    /// TODO(T199606542) rename this to Tinstance
    Tclass(PosId, Exact, Vec<Ty>),
    /// The negation of the [type_predicate]
    Tneg(TypePredicate),
    /// The type of the label expression #ID
    Tlabel(String),
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

/// Class refinements are for type annotations like
///
/// Box with {type T = string}
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
    /// for `=` constraints
    TRexact(Ty),
    /// for `as` or `super` constraints
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
#[rust_to_ocaml(prefix = "s_")]
#[repr(C)]
pub struct ShapeType {
    #[rust_to_ocaml(attr = "transform.opaque")]
    pub origin: TypeOrigin,
    pub unknown_value: Ty,
    pub fields: t_shape_map::TShapeMap<ShapeFieldType>,
}

/// Required and extra components of a tuple. Extra components
/// are either optional + variadic, or a type splat.
/// Exmaple 1:
/// (string,bool,optional float,optional bool,int...)
/// has require components string, bool, optional components float, bool
/// and variadic component int.
/// Example 2:
/// (string,float,...T)
/// has required components string, float, and splat component T.
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
#[rust_to_ocaml(prefix = "t_")]
#[repr(C)]
pub struct TupleType {
    pub required: Vec<Ty>,
    pub optional: Vec<Ty>,
    pub extra: TupleExtra,
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
#[rust_to_ocaml(attr = "deriving (hash, transform)")]
#[repr(C, u8)]
pub enum TupleExtra {
    Tvariadic(Ty),
    Tsplat(Ty),
}
