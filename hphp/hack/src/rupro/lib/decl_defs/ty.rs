// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::{self, Reason};
use crate::utils::core::Ident;
use hcons::Hc;
use intern::string::BytesId;
use oxidized::{aast, ast_defs};
use pos::{ModuleName, Positioned, Symbol, TypeConstName, TypeName};
use std::collections::BTreeMap;
use std::fmt;

pub use oxidized::{
    aast_defs::{ReifyKind, Tprim as Prim},
    ast_defs::{Abstraction, ClassishKind, ConstraintKind, Visibility},
    typing_defs::ClassConstKind,
    typing_defs_core::{ConsistentKind, Enforcement, Exact, ParamMode, ShapeKind},
    typing_defs_flags::{self, ClassEltFlags, ClassEltFlagsArgs, FunParamFlags, FunTypeFlags},
    xhp_attribute::{Tag, XhpAttribute},
};

// c.f. ast_defs::XhpEnumValue
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum XhpEnumValue {
    XEVInt(isize),
    XEVString(Symbol),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum CeVisibility {
    Public,
    Private(TypeName),
    Protected(TypeName),
    Internal(ModuleName),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum IfcFunDecl {
    FDPolicied(Option<Symbol>),
    FDInferFlows,
}

// The OCaml type `tshape_field_name` includes positions, but ignores those
// positions in its `ord` implementation. We can't do the same, though: Rust
// hash tables require impls of Hash and Eq to agree, and our Hash impl must
// take positions into account (else hash-consing will produce bad results). We
// could write a custom Ord impl which disagrees with the Eq impl, but it would
// violate the [PartialOrd requirement][] that `a == b` if and only if
// `partial_cmp(a, b) == Some(Equal)`, and the [Ord requirement][] for a strict
// total order.
//
// [PartialOrd requirement]: https://doc.rust-lang.org/std/cmp/trait.PartialOrd.html
// [Ord requirement]: https://doc.rust-lang.org/std/cmp/trait.Ord.html#corollaries
//
// Instead, we omit the positions from these keys, and store the field name's
// position as part of the map's value (in a `ShapeFieldNamePos`).
#[derive(Copy, Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub enum TshapeFieldName {
    TSFlitInt(Symbol),
    TSFlitStr(BytesId),
    TSFclassConst(TypeName, Symbol),
}

walkable!(TshapeFieldName);

/// The position of a shape field name; e.g., the position of `'a'` in
/// `shape('a' => int)`, or the positions of `Foo` and `X` in
/// `shape(Foo::X => int)`.
#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub enum ShapeFieldNamePos<P> {
    Simple(P),
    ClassConst(P, P),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum DependentType {
    DTexpr(Ident),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct UserAttribute<P> {
    pub name: Positioned<TypeName, P>,
    pub classname_params: Box<[TypeName]>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Tparam<R: Reason, TY> {
    pub variance: ast_defs::Variance,
    pub name: Positioned<TypeName, R::Pos>,
    pub tparams: Box<[Tparam<R, TY>]>,
    pub constraints: Box<[(ConstraintKind, TY)]>,
    pub reified: ReifyKind,
    pub user_attributes: Box<[UserAttribute<R::Pos>]>,
}

walkable!(impl<R: Reason, TY> for Tparam<R, TY> => [tparams, constraints]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct WhereConstraint<TY>(pub TY, pub ast_defs::ConstraintKind, pub TY);

walkable!(impl<R: Reason, TY> for WhereConstraint<TY> => [0, 1, 2]);

#[derive(Clone, Eq, Hash, PartialEq)]
pub struct DeclTy<R: Reason>(R, Hc<DeclTy_<R>>);

walkable!(DeclTy<R> as visit_decl_ty => [0, 1]);

impl<R: Reason> DeclTy<R> {
    pub fn new(reason: R, ty: Hc<DeclTy_<R>>) -> Self {
        Self(reason, ty)
    }

    pub fn pos(&self) -> &R::Pos {
        self.0.pos()
    }

    pub fn reason(&self) -> &R {
        &self.0
    }

    pub fn node(&self) -> &Hc<DeclTy_<R>> {
        &self.1
    }

    pub fn unwrap_class_type(&self) -> Option<(&R, &Positioned<TypeName, R::Pos>, &[DeclTy<R>])> {
        use DeclTy_::*;
        let r = self.reason();
        match &**self.node() {
            DTapply(id_and_args) => {
                let (pos_id, args) = &**id_and_args;
                Some((r, pos_id, args))
            }
            _ => None,
        }
    }
}

/// A shape may specify whether or not fields are required. For example, consider
/// this typedef:
///
/// ```
/// type ShapeWithOptionalField = shape(?'a' => ?int);
/// ```
///
/// With this definition, the field 'a' may be unprovided in a shape. In this
/// case, the field 'a' would have sf_optional set to true.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ShapeFieldType<R: Reason> {
    pub field_name_pos: ShapeFieldNamePos<R::Pos>,
    pub optional: bool,
    pub ty: DeclTy<R>,
}

walkable!(ShapeFieldType<R> => [ty]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum DeclTy_<R: Reason> {
    /// The late static bound type of a class
    DTthis,
    /// Either an object type or a type alias, ty list are the arguments
    DTapply(Box<(Positioned<TypeName, R::Pos>, Box<[DeclTy<R>]>)>),
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
    DTmixed,
    DTlike(DeclTy<R>),
    DTany,
    DTerr,
    DTnonnull,
    /// A dynamic type is a special type which sometimes behaves as if it were a
    /// top type; roughly speaking, where a specific value of a particular type is
    /// expected and that type is dynamic, anything can be given. We call this
    /// behaviour "coercion", in that the types "coerce" to dynamic. In other ways it
    /// behaves like a bottom type; it can be used in any sort of binary expression
    /// or even have object methods called from it. However, it is in fact neither.
    ///
    /// it captures dynamicism within function scope.
    /// See tests in typecheck/dynamic/ for more examples.
    DTdynamic,
    /// Nullable, called "option" in the ML parlance.
    DToption(DeclTy<R>),
    /// All the primitive types: int, string, void, etc.
    DTprim(aast::Tprim),
    /// A wrapper around fun_type, which contains the full type information for a
    /// function, method, lambda, etc.
    DTfun(Box<FunType<R, DeclTy<R>>>),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    DTtuple(Box<[DeclTy<R>]>),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    DTshape(Box<(ShapeKind, BTreeMap<TshapeFieldName, ShapeFieldType<R>>)>),
    DTvar(Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    DTgeneric(Box<(TypeName, Box<[DeclTy<R>]>)>),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    DTunion(Box<[DeclTy<R>]>),
    DTintersection(Box<[DeclTy<R>]>),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    DTvecOrDict(Box<(DeclTy<R>, DeclTy<R>)>),
    DTaccess(Box<TaccessType<R, DeclTy<R>>>),
}

// We've boxed all variants of DeclTy_ which are larger than two usizes, so the
// total size should be equal to `[usize; 3]` (one more for the discriminant).
// This is important because all variants use the same amount of memory and are
// passed around by value, so adding a large unboxed variant can cause a large
// regression.
static_assertions::assert_eq_size!(DeclTy_<reason::NReason>, [usize; 3]);
static_assertions::assert_eq_size!(DeclTy_<reason::BReason>, [usize; 3]);

impl<R: Reason> crate::visitor::Walkable<R> for DeclTy_<R> {
    fn recurse(&self, v: &mut dyn crate::visitor::Visitor<R>) {
        use DeclTy_::*;
        match self {
            DTthis | DTmixed | DTany | DTerr | DTnonnull | DTdynamic | DTprim(_) | DTvar(_) => {}
            DTapply(id_and_args) => {
                let (_, args) = &**id_and_args;
                args.accept(v)
            }
            DTlike(ty) | DToption(ty) => ty.accept(v),
            DTfun(ft) => ft.accept(v),
            DTtuple(tys) | DTunion(tys) | DTintersection(tys) => tys.accept(v),
            DTshape(kind_and_fields) => {
                let (_, fields) = &**kind_and_fields;
                fields.accept(v)
            }
            DTgeneric(id_and_args) => {
                let (_, args) = &**id_and_args;
                args.accept(v)
            }
            DTvecOrDict(key_and_val_tys) => {
                let (kty, vty) = &**key_and_val_tys;
                kty.accept(v);
                vty.accept(v)
            }
            DTaccess(tt) => tt.accept(v),
        }
    }
}

/// A Type const access expression of the form <type expr>::C.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct TaccessType<R: Reason, TY> {
    /// Type expression to the left of `::`
    pub ty: TY,

    /// Name of type const to the right of `::`
    pub type_const: Positioned<TypeConstName, R::Pos>,
}

walkable!(impl<R: Reason, TY> for TaccessType<R, TY> => [ty]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum Capability<R: Reason, TY> {
    CapDefaults(R::Pos),
    CapTy(TY),
}

walkable!(impl<R: Reason, TY> for Capability<R, TY> => {
    Self::CapDefaults(..) => [],
    Self::CapTy(ty) => [ty],
});

/// Companion to fun_params type, intended to consolidate checking of
/// implicit params for functions.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunImplicitParams<R: Reason, TY> {
    pub capability: Capability<R, TY>,
}

walkable!(impl<R: Reason, TY> for FunImplicitParams<R, TY> => [capability]);

/// The type of a function AND a method.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunType<R: Reason, TY> {
    pub tparams: Box<[Tparam<R, TY>]>,
    pub where_constraints: Box<[WhereConstraint<TY>]>,
    pub params: FunParams<R, TY>,
    pub implicit_params: FunImplicitParams<R, TY>,
    /// Carries through the sync/async information from the aast
    pub ret: PossiblyEnforcedTy<TY>,
    pub flags: typing_defs_flags::FunTypeFlags,
    pub ifc_decl: IfcFunDecl,
}

walkable!(impl<R: Reason, TY> for FunType<R, TY> => [
    tparams, where_constraints, params, implicit_params, ret
]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct PossiblyEnforcedTy<TY> {
    /// True if consumer of this type enforces it at runtime
    pub enforced: Enforcement,
    pub ty: TY,
}

walkable!(impl<R: Reason, TY> for PossiblyEnforcedTy<TY> => [ty]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunParam<R: Reason, TY> {
    pub pos: R::Pos,
    pub name: Option<Symbol>,
    pub ty: PossiblyEnforcedTy<TY>,
    pub flags: FunParamFlags,
}

walkable!(impl<R: Reason, TY> for FunParam<R, TY> => [ty]);

pub type FunParams<R, TY> = Box<[FunParam<R, TY>]>;

/// Origin of Class Constant References:
/// In order to be able to detect cycle definitions like
/// class C {
/// const int A = D::A;
/// }
/// class D {
/// const int A = C::A;
/// }
/// we need to remember which constants were used during initialization.
///
/// Currently the syntax of constants allows direct references to another class
/// like D::A, or self references using self::A.
///
/// class_const_from encodes the origin (class vs self).
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum ClassConstFrom {
    Self_,
    From(TypeName),
}

/// Class Constant References:
/// In order to be able to detect cycle definitions like
/// class C {
/// const int A = D::A;
/// }
/// class D {
/// const int A = C::A;
/// }
/// we need to remember which constants were used during initialization.
///
/// Currently the syntax of constants allows direct references to another class
/// like D::A, or self references using self::A.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ClassConstRef(pub ClassConstFrom, pub Symbol);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ConstDecl<R: Reason> {
    pub pos: R::Pos,
    pub ty: DeclTy<R>,
}

walkable!(ConstDecl<R> => [ty]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunElt<R: Reason> {
    pub deprecated: Option<BytesId>,
    pub module: Option<Positioned<ModuleName, R::Pos>>,
    /// Top-level functions have limited visibilities
    pub internal: bool,
    pub ty: DeclTy<R>,
    pub pos: R::Pos,
    pub php_std_lib: bool,
    pub support_dynamic_type: bool,
}

walkable!(FunElt<R> => [ty]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct AbstractTypeconst<R: Reason> {
    pub as_constraint: Option<DeclTy<R>>,
    pub super_constraint: Option<DeclTy<R>>,
    pub default: Option<DeclTy<R>>,
}

walkable!(AbstractTypeconst<R> => [as_constraint, super_constraint, default]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ConcreteTypeconst<R: Reason> {
    pub ty: DeclTy<R>,
}

walkable!(ConcreteTypeconst<R> => [ty]);

#[derive(Clone, Eq, Hash, PartialEq)]
pub enum Typeconst<R: Reason> {
    TCAbstract(AbstractTypeconst<R>),
    TCConcrete(ConcreteTypeconst<R>),
}

walkable!(Typeconst<R> => {
    Self::TCAbstract(x) => [x],
    Self::TCConcrete(x) => [x],
});

impl<R: Reason> fmt::Debug for Typeconst<R> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::TCAbstract(x) => x.fmt(f),
            Self::TCConcrete(x) => x.fmt(f),
        }
    }
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct EnumType<R: Reason> {
    pub base: DeclTy<R>,
    pub constraint: Option<DeclTy<R>>,
    pub includes: Box<[DeclTy<R>]>,
}

walkable!(EnumType<R> => [base, constraint, includes]);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct TypedefType<R: Reason> {
    pub module: Option<Positioned<ModuleName, R::Pos>>,
    pub pos: R::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Box<[Tparam<R, DeclTy<R>>]>,
    pub constraint: Option<DeclTy<R>>,
    pub ty: DeclTy<R>,
    pub is_ctx: bool,
    pub attributes: Box<[UserAttribute<R::Pos>]>,
}

walkable!(TypedefType<R> => [tparams, constraint, ty]);

walkable!(ast_defs::ConstraintKind);
