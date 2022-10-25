// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e7d7d108fdb8797b0ff7a60cbf27e043>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum CeVisibility<'a> {
    Vpublic,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Vprivate(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Vprotected(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Vinternal(&'a str),
}
impl<'a> TrivialDrop for CeVisibility<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CeVisibility<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, ord)")]
#[repr(C, u8)]
pub enum IfcFunDecl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    FDPolicied(Option<&'a str>),
    FDInferFlows,
}
impl<'a> TrivialDrop for IfcFunDecl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(IfcFunDecl<'arena>);

pub use oxidized::typing_defs_core::FunTparamsKind;
pub use oxidized::typing_defs_core::ShapeKind;
pub use oxidized::typing_defs_core::ValKind;

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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C)]
pub struct PosString<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a str,
);
impl<'a> TrivialDrop for PosString<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PosString<'arena>);

#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
pub type TByteString<'a> = str;

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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C)]
pub struct PosByteString<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub  &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a bstr::BStr,
);
impl<'a> TrivialDrop for PosByteString<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PosByteString<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum TshapeFieldName<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "TSFlit_int")]
    TSFlitInt(&'a PosString<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "TSFlit_str")]
    TSFlitStr(&'a PosByteString<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "TSFclass_const")]
    #[rust_to_ocaml(inline_tuple)]
    TSFclassConst(&'a (PosId<'a>, PosString<'a>)),
}
impl<'a> TrivialDrop for TshapeFieldName<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TshapeFieldName<'arena>);

pub use oxidized::typing_defs_core::ParamMode;

#[rust_to_ocaml(attr = "deriving (eq, show)")]
pub type XhpAttr = oxidized::xhp_attribute::XhpAttribute;

pub use oxidized::typing_defs_core::ConsistentKind;

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
#[rust_to_ocaml(attr = "deriving (eq, ord, show)")]
#[repr(C, u8)]
pub enum DependentType {
    DTexpr(ident::Ident),
}
impl TrivialDrop for DependentType {}
arena_deserializer::impl_deserialize_in_arena!(DependentType);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "ua_")]
#[repr(C)]
pub struct UserAttribute<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub classname_params: &'a [&'a str],
}
impl<'a> TrivialDrop for UserAttribute<'a> {}
arena_deserializer::impl_deserialize_in_arena!(UserAttribute<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[rust_to_ocaml(prefix = "tp_")]
#[repr(C)]
pub struct Tparam<'a> {
    pub variance: oxidized::ast_defs::Variance,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: PosId<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub constraints: &'a [(oxidized::ast_defs::ConstraintKind, &'a Ty<'a>)],
    pub reified: oxidized::ast_defs::ReifyKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub user_attributes: &'a [&'a UserAttribute<'a>],
}
impl<'a> TrivialDrop for Tparam<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Tparam<'arena>);

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
#[rust_to_ocaml(attr = "deriving (eq, show)")]
#[repr(C)]
pub struct WhereConstraint<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Ty<'a>,
    pub oxidized::ast_defs::ConstraintKind,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Ty<'a>,
);
impl<'a> TrivialDrop for WhereConstraint<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WhereConstraint<'arena>);

pub use oxidized::typing_defs_core::Enforcement;

#[derive(
    Clone,
    Debug,
    Deserialize,
    EqModuloPos,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
#[repr(C)]
pub struct Ty<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a reason::T_<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub Ty_<'a>,
);
impl<'a> TrivialDrop for Ty<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ty<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum NegType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Neg_prim")]
    NegPrim(&'a oxidized::ast_defs::Tprim),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Neg_class")]
    NegClass(&'a PosId<'a>),
}
impl<'a> TrivialDrop for NegType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(NegType<'arena>);

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
    FromOcamlRepIn,
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
pub struct ShapeFieldType<'a> {
    pub optional: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ty: &'a Ty<'a>,
}
impl<'a> TrivialDrop for ShapeFieldType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ShapeFieldType<'arena>);

#[derive(
    Clone,
    Copy,
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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Ty_<'a> {
    /// The late static bound type of a class
    Tthis,
    /// Either an object type or a type alias, ty list are the arguments
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tapply(&'a (PosId<'a>, &'a [&'a Ty<'a>])),
    /// 'With' refinements of the form `_ with { type T as int; type TC = C; }`.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Trefinement(&'a (&'a Ty<'a>, ClassRefinement<'a>)),
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
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Toption(&'a Ty<'a>),
    /// All the primitive types: int, string, void, etc.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tprim(&'a oxidized::ast_defs::Tprim),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tfun(&'a FunType<'a>),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ttuple(&'a [&'a Ty<'a>]),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tshape(
        &'a (
            oxidized::typing_defs_core::ShapeKind,
            t_shape_map::TShapeMap<'a, &'a ShapeFieldType<'a>>,
        ),
    ),
    Tvar(ident::Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tgeneric(&'a (&'a str, &'a [&'a Ty<'a>])),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tunion(&'a [&'a Ty<'a>]),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tintersection(&'a [&'a Ty<'a>]),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tvec_or_dict")]
    #[rust_to_ocaml(inline_tuple)]
    TvecOrDict(&'a (&'a Ty<'a>, &'a Ty<'a>)),
    /// Name of class, name of type const, remaining names of type consts
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Taccess(&'a TaccessType<'a>),
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
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tnewtype(&'a (&'a str, &'a [&'a Ty<'a>], &'a Ty<'a>)),
    /// This represents a type alias that lacks necessary type arguments. Given
    /// type Foo<T1,T2> = ...
    /// Tunappliedalias "Foo" stands for usages of plain Foo, without supplying
    /// further type arguments. In particular, Tunappliedalias always stands for
    /// a higher-kinded type. It is never used for an alias like
    /// type Foo2 = ...
    /// that simply doesn't require type arguments.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tunapplied_alias")]
    TunappliedAlias(&'a str),
    /// see dependent_type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tdependent(&'a (DependentType, &'a Ty<'a>)),
    /// An instance of a class or interface, ty list are the arguments
    /// If exact=Exact, then this represents instances of *exactly* this class
    /// If exact=Nonexact, this also includes subclasses
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Tclass(&'a (PosId<'a>, Exact<'a>, &'a [&'a Ty<'a>])),
    /// The negation of the type in neg_type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tneg(&'a NegType<'a>),
}
impl<'a> TrivialDrop for Ty_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ty_<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct TaccessType<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub PosId<'a>,
);
impl<'a> TrivialDrop for TaccessType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(TaccessType<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Exact<'a> {
    Exact,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Nonexact(&'a ClassRefinement<'a>),
}
impl<'a> TrivialDrop for Exact<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Exact<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct ClassRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub cr_types: s_map::SMap<'a, ClassTypeRefinement<'a>>,
}
impl<'a> TrivialDrop for ClassRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassRefinement<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum ClassTypeRefinement<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRexact(&'a Ty<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    TRloose(&'a ClassTypeRefinementBounds<'a>),
}
impl<'a> TrivialDrop for ClassTypeRefinement<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassTypeRefinement<'arena>);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "tr_")]
#[repr(C)]
pub struct ClassTypeRefinementBounds<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub lower: &'a [&'a Ty<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub upper: &'a [&'a Ty<'a>],
}
impl<'a> TrivialDrop for ClassTypeRefinementBounds<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ClassTypeRefinementBounds<'arena>);

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
#[rust_to_ocaml(and)]
#[repr(C, u8)]
pub enum Capability<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CapDefaults(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    CapTy(&'a Ty<'a>),
}
impl<'a> TrivialDrop for Capability<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Capability<'arena>);

/// Companion to fun_params type, intended to consolidate checking of
/// implicit params for functions.
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
#[rust_to_ocaml(and)]
#[repr(C)]
pub struct FunImplicitParams<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub capability: Capability<'a>,
}
impl<'a> TrivialDrop for FunImplicitParams<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FunImplicitParams<'arena>);

/// The type of a function AND a method.
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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "ft_")]
#[repr(C)]
pub struct FunType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub tparams: &'a [&'a Tparam<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub where_constraints: &'a [&'a WhereConstraint<'a>],
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub params: &'a FunParams<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub implicit_params: &'a FunImplicitParams<'a>,
    /// Carries through the sync/async information from the aast
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ret: &'a PossiblyEnforcedTy<'a>,
    pub flags: typing_defs_flags::FunTypeFlags,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub ifc_decl: IfcFunDecl<'a>,
}
impl<'a> TrivialDrop for FunType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FunType<'arena>);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "et_")]
#[repr(C)]
pub struct PossiblyEnforcedTy<'a> {
    /// True if consumer of this type enforces it at runtime
    pub enforced: oxidized::typing_defs_core::Enforcement,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
}
impl<'a> TrivialDrop for PossiblyEnforcedTy<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PossiblyEnforcedTy<'arena>);

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
#[rust_to_ocaml(and)]
#[rust_to_ocaml(prefix = "fp_")]
#[repr(C)]
pub struct FunParam<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub pos: &'a pos_or_decl::PosOrDecl<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: Option<&'a str>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a PossiblyEnforcedTy<'a>,
    pub flags: typing_defs_flags::FunParamFlags,
}
impl<'a> TrivialDrop for FunParam<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FunParam<'arena>);

#[rust_to_ocaml(and)]
pub type FunParams<'a> = [&'a FunParam<'a>];

pub use oxidized::typing_defs_core::DestructureKind;

#[derive(
    Clone,
    Debug,
    Deserialize,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "d_")]
#[repr(C)]
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
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub required: &'a [&'a Ty<'a>],
    /// Represents the optional parameters in a function, only used for splats
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub optional: &'a [&'a Ty<'a>],
    /// Represents a function's variadic parameter, also only used for splats
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub variadic: Option<&'a Ty<'a>>,
    /// list() destructuring allows for partial matches on lists, even when the operation
    /// might throw i.e. list($a) = vec[];
    pub kind: oxidized::typing_defs_core::DestructureKind,
}
impl<'a> TrivialDrop for Destructure<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Destructure<'arena>);

#[derive(
    Clone,
    Debug,
    Deserialize,
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
#[rust_to_ocaml(attr = "deriving show")]
#[rust_to_ocaml(prefix = "hm_")]
#[repr(C)]
pub struct HasMember<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub name: nast::Sid<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub type_: &'a Ty<'a>,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub class_id: &'a nast::ClassId_<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(attr = "opaque")]
    pub explicit_targs: Option<&'a [&'a nast::Targ<'a>]>,
}
impl<'a> TrivialDrop for HasMember<'a> {}
arena_deserializer::impl_deserialize_in_arena!(HasMember<'arena>);

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
#[rust_to_ocaml(prefix = "ci_")]
#[repr(C)]
pub struct CanIndex<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub key: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub shape: Option<TshapeFieldName<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub val: &'a Ty<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub expr_pos: &'a pos::Pos<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub index_pos: &'a pos::Pos<'a>,
}
impl<'a> TrivialDrop for CanIndex<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CanIndex<'arena>);

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
#[rust_to_ocaml(prefix = "ct_")]
#[repr(C)]
pub struct CanTraverse<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub key: Option<&'a Ty<'a>>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub val: &'a Ty<'a>,
    pub is_await: bool,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    pub reason: &'a reason::Reason<'a>,
}
impl<'a> TrivialDrop for CanTraverse<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CanTraverse<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
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
#[repr(C, u8)]
pub enum ConstraintType_<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Thas_member")]
    ThasMember(&'a HasMember<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Thas_type_member")]
    #[rust_to_ocaml(inline_tuple)]
    ThasTypeMember(&'a (&'a str, &'a Ty<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tcan_index")]
    TcanIndex(&'a CanIndex<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tcan_traverse")]
    TcanTraverse(&'a CanTraverse<'a>),
    /// The type of container destructuring via list() or splat `...`
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Tdestructure(&'a Destructure<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    TCunion(&'a (&'a Ty<'a>, ConstraintType<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    TCintersection(&'a (&'a Ty<'a>, ConstraintType<'a>)),
}
impl<'a> TrivialDrop for ConstraintType_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ConstraintType_<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Serialize,
    ToOcamlRep
)]
#[rust_to_ocaml(and)]
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C)]
pub struct ConstraintType<'a>(
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a reason::Reason<'a>,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)] pub &'a ConstraintType_<'a>,
);
impl<'a> TrivialDrop for ConstraintType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ConstraintType<'arena>);

#[derive(
    Clone,
    Copy,
    Debug,
    Deserialize,
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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum InternalType<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    LoclType(&'a Ty<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ConstraintType(ConstraintType<'a>),
}
impl<'a> TrivialDrop for InternalType<'a> {}
arena_deserializer::impl_deserialize_in_arena!(InternalType<'arena>);
