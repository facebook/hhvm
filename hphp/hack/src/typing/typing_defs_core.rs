// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bumpalo::collections::Vec;
pub use oxidized::typing_defs_core::{DestructureKind, Exact, ParamMode};
use oxidized::{aast_defs, ident, nast, tany_sentinel, typing_defs as oxidized_defs};

use crate::typing_make_type::TypeBuilder;
use crate::typing_reason::*;

/// A type as used during type inference.
///
/// The type is parametrized by the lifetime 'a, as it is designed to live
/// in an arena.
///
/// This representation lives along side the oxidized representation of types,
/// and will so for the foreseeable future.
///
/// There is a mapping from this representation to the oxidized representation,
/// for consumers that require the oxidized representation.
#[derive(Debug)]
pub enum Ty_<'a> {
    /// The late static bound type of a class
    Tthis,
    /// Either an object type or a type alias, ty list are the arguments
    Tapply(&'a nast::Sid, Vec<'a, Ty<'a>>),
    /// Name of class, name of type const, remaining names of type consts
    Taccess(TaccessType<'a>),
    /// The type of the various forms of "array":
    ///
    /// ```
    /// Tarray (None, None)         => "array"
    /// Tarray (Some ty, None)      => "array<ty>"
    /// Tarray (Some ty1, Some ty2) => "array<ty1, ty2>"
    /// Tarray (None, Some ty)      => [invalid]
    /// ```
    Tarray(Option<Ty<'a>>, Option<Ty<'a>>),
    /// Tdarray (ty1, ty2) => "darray<ty1, ty2>"
    Tdarray(Ty<'a>, Ty<'a>),
    /// Tvarray (ty) => "varray<ty>"
    Tvarray(Ty<'a>),
    /// Tvarray_or_darray (ty1, ty2) => "varray_or_darray<ty1, ty2>"
    TvarrayOrDarray(Option<Ty<'a>>, Ty<'a>),
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
    Tlike(Ty<'a>),
    /// Access to a Pocket Universe Expression or Atom, denoted by
    /// Foo:@Bar or Foo:@Bar:@X.
    /// It might be unresolved at first (e.g. if Foo is a generic variable).
    /// Will be refined to Tpu once typechecking is successful
    TpuAccess, // TODO: TpuAccess(Ty, nast::Sid, nast::PuLoc),
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
    Toption(Ty<'a>),
    /// All the primitive types: int, string, void, etc.
    Tprim(PrimKind<'a>),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    Tfun, // TODO: Tfun(FunType),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    Ttuple(Vec<'a, Ty<'a>>),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    Tshape, // TODO: Tshape(ShapeKind, nast::shape_map::ShapeMap<ShapeFieldType>),
    Tvar(ident::Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.localize_generic_parameters_with_bounds.
    Tgeneric(&'a str),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    Tunion(Vec<'a, Ty<'a>>),
    Tintersection(Vec<'a, Ty<'a>>),
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
    Tnewtype(&'a str, Vec<'a, Ty<'a>>, Ty<'a>),
    /// see dependent_type
    Tdependent, // TODO: Tdependent(DependentType, Ty),
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
    Tclass(&'a nast::Sid, Exact, Vec<'a, Ty<'a>>),
    /// Localized version of Tarray
    Tarraykind(ArrayKind<'a>),
    /// Typing of Pocket Universe Expressions
    /// - first parameter is the enclosing class
    /// - second parameter is the name of the Pocket Universe Enumeration
    /// - third parameter is  either Pu_plain (the enumeration as the set of
    ///   all its atoms) or Pu_atom (a specific atom in the enumeration)
    Tpu, // TODO: Tpu(Ty, nast::Sid),
    /// Typing of Pocket Universes type projections
    /// - first parameter is the enclosing class
    /// - second parameter is the name of the Pocket Universe Enumeration
    /// - third parameter is the generic (tvar/tabstract) in place of the
    ///   member name
    /// - the fourth parameter is the name of the type to project
    TpuTypeAccess, // TODO: TpuTypeAccess(Ty, nast::Sid, Ty, nast::Sid),
}

/// The actual Ty is a (reason ptr, type ptr)-tuple.
///
/// This is very close to the OCaml representation. An alternative
/// is to have a pointer to a pointer to a
/// (reason ptr, inlined type). Which reduces the size of
/// Ty_ and decreases stack usage.
#[derive(Clone, Copy, Debug)]
pub struct Ty<'a>(pub PReason<'a>, pub &'a Ty_<'a>);

impl<'a> Ty<'a> {
    pub fn get_node(&self) -> &'a Ty_<'a> {
        let Ty(_r, t) = self;
        *t
    }
    pub fn get_reason(&self) -> PReason<'a> {
        let Ty(r, _t) = self;
        *r
    }
    pub fn get_pos(&self) -> Option<&'a oxidized::pos::Pos> {
        self.get_reason().pos
    }
    pub fn is_tyvar(&self) -> bool {
        match self.get_node() {
            Ty_::Tvar(_) => true,
            _ => false,
        }
    }
    pub fn is_generic(&self) -> bool {
        match self.get_node() {
            Ty_::Tgeneric(_) => true,
            _ => false,
        }
    }
    pub fn is_dynamic(&self) -> bool {
        match self.get_node() {
            Ty_::Tdynamic => true,
            _ => false,
        }
    }
    pub fn is_nonnull(&self) -> bool {
        match self.get_node() {
            Ty_::Tnonnull => true,
            _ => false,
        }
    }
    pub fn is_any(&self) -> bool {
        match self.get_node() {
            Ty_::Tany(_) => true,
            _ => false,
        }
    }
    pub fn is_prim(&self, kind: PrimKind) -> bool {
        match self.get_node() {
            Ty_::Tprim(k) => kind == *k,
            _ => false,
        }
    }
    pub fn is_union(&self) -> bool {
        match self.get_node() {
            Ty_::Tunion(_) => true,
            _ => false,
        }
    }
    pub fn is_intersection(&self) -> bool {
        match self.get_node() {
            Ty_::Tintersection(_) => true,
            _ => false,
        }
    }
}
/// This is a direct translation of oxidized::gen::typing_defs_core::TaccessType.
///
/// We need this because it wraps Tys
#[derive(Debug)]
pub struct TaccessType<'a>(pub Ty<'a>, pub Vec<'a, &'a nast::Sid>);

/// This is a direct translation of oxidized::gen::typing_defs_core::ArrayKind.
///
/// We need this, because it wraps Tys.
#[derive(Debug)]
pub enum ArrayKind<'a> {
    /// An array declared as a varray.
    AKvarray(Ty<'a>),
    /// An array declared as a darray.
    AKdarray(Ty<'a>, Ty<'a>),
    /// An array annotated as a varray_or_darray.
    AKvarrayOrDarray(Ty<'a>, Ty<'a>),
    /// This is a type created when we see array() literal
    AKempty,
}

/// This is a direct translation of aast_defs::Tprim.
///
/// Why don't we directly use a reference to aast_defs::Tprim in the
/// Tprim constructor?
///
///   1. Tatom contains a string, that we might want to allocate during
///      type checking.
///   2. Additionally, I (hverr) think the aast::Tprim -> Tprim conversion
///      is cheaper than dereferencing a pointer to aast::Tprim.
#[derive(Debug, Eq, PartialEq)]
pub enum PrimKind<'a> {
    Tnull,
    Tvoid,
    Tint,
    Tbool,
    Tfloat,
    Tstring,
    Tresource,
    Tnum,
    Tarraykey,
    Tnoreturn,
    /// plain Pocket Universe atom when we don't know which enum it is in.
    /// E.g. `:@MyAtom`
    Tatom(&'a str),
}

#[derive(Debug)]
pub struct PossiblyEnforcedTy<'a> {
    /// True if consumer of this type enforces it at runtime
    pub enforced: bool,
    pub type_: Ty<'a>,
}

#[derive(Debug)]
pub enum ConstraintType_<'a> {
    ThasMember(HasMember<'a>),
    /// The type of container destructuring via list() or splat `...`
    Tdestructure, // TODO: Tdestructure(Destructure),
    TCunion(Ty<'a>, ConstraintType<'a>),
    TCintersection(Ty<'a>, ConstraintType<'a>),
}

#[derive(Debug)]
pub struct ConstraintType<'a>(pub PReason<'a>, pub &'a ConstraintType_<'a>);

#[derive(Debug)]
pub struct HasMember<'a> {
    pub name: &'a nast::Sid,
    pub member_type: Ty<'a>,
    /// This is required to check ambiguous object access, where sometimes
    /// HHVM would access the private member of a parent class instead of the
    /// one from the current class.
    pub class_id: &'a nast::ClassId_,
}

#[derive(Debug)]
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
    pub required: Vec<'a, Ty<'a>>,
    /// Represents the optional parameters in a function, only used for splats
    pub optional: Vec<'a, Ty<'a>>,
    /// Represents a function's variadic parameter, also only used for splats
    pub variadic: Option<Ty<'a>>,
    /// list() destructuring allows for partial matches on lists, even when the operation
    /// might throw i.e. list($a) = vec[];
    pub kind: DestructureKind,
}

#[derive(Debug)]
pub enum InternalType_<'a> {
    LoclType(Ty<'a>),
    ConstraintType(ConstraintType<'a>),
}

pub type InternalType<'a> = &'a InternalType_<'a>;

impl<'a> Ty<'a> {
    pub fn from_oxidized(ty: &oxidized_defs::Ty, builder: &'a TypeBuilder<'a>) -> Ty<'a> {
        use oxidized_defs::Ty_;
        match ty.1.as_ref() {
            Ty_::Tprim(aast_defs::Tprim::Tint) => builder.prim(builder.mk_rnone(), PrimKind::Tint),
            _ => unimplemented!("{:#?}", ty),
        }
    }
}
