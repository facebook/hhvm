// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::Reason;
use crate::utils::core::Ident;
use hcons::Hc;
use intern::string::BytesId;
use oxidized::{aast, ast_defs};
use pos::{Positioned, Symbol, TypeName};
use std::collections::BTreeMap;

pub use oxidized::{
    aast_defs::Tprim as Prim,
    ast_defs::Abstraction,
    ast_defs::ClassishKind,
    ast_defs::Visibility,
    typing_defs::ClassConstKind,
    typing_defs_core::{
        ConsistentKind, DestructureKind, Enforcement, Exact, FunTparamsKind, ParamMode, ShapeKind,
        ValKind,
    },
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
pub enum CeVisibility<P> {
    Public,
    Private(TypeName),
    Protected(TypeName),

    // XXX are module names disjoint from class names?
    Internal(Positioned<Symbol, P>),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum IfcFunDecl {
    FDPolicied(Option<Symbol>),
    FDInferFlows,
}

// The OCaml type `tshape_field_name` includes positions, but ignores those
// positions in its `ord` implementation. We can't do the same, though: Rust
// hash tables require impls of Hash and Ord to agree, and our Hash impl must
// take positions into account (else hash-consing will produce bad results). We
// could write impls which disagree, and try to avoid using this type as a hash
// table key, but it'd be better not to lay a trap like that. Instead, just omit
// positions from this type. If we need the positions which OCaml stores in the
// key, we can insert them as part of the map's value instead.
#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd)]
pub enum TshapeFieldName {
    TSFlitInt(Symbol),
    TSFlitStr(BytesId),
    TSFclassConst(TypeName, Symbol),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum DependentType {
    DTexpr(Ident),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct UserAttribute<P> {
    pub name: Positioned<TypeName, P>,
    pub classname_params: Vec<TypeName>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Tparam<R: Reason, TY> {
    pub variance: ast_defs::Variance,
    pub name: Positioned<TypeName, R::Pos>,
    pub tparams: Vec<Tparam<R, TY>>,
    pub constraints: Vec<(ast_defs::ConstraintKind, TY)>,
    pub reified: aast::ReifyKind,
    pub user_attributes: Vec<UserAttribute<R::Pos>>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct WhereConstraint<TY>(pub TY, pub ast_defs::ConstraintKind, pub TY);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct DeclTy<R: Reason>(R, Hc<DeclTy_<R>>);

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
            DTapply(pos_id, tyl) => Some((r, pos_id, tyl)),
            _ => None,
        }
    }
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum NegType<P> {
    NegPrim(aast::Tprim),
    NegClass(Positioned<TypeName, P>),
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
    pub optional: bool,
    pub ty: DeclTy<R>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum DeclTy_<R: Reason> {
    /// The late static bound type of a class
    DTthis,
    /// Either an object type or a type alias, ty list are the arguments
    DTapply(Positioned<TypeName, R::Pos>, Vec<DeclTy<R>>),
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
    DTfun(FunType<R, DeclTy<R>>),
    /// Tuple, with ordered list of the types of the elements of the tuple.
    DTtuple(Vec<DeclTy<R>>),
    /// Whether all fields of this shape are known, types of each of the
    /// known arms.
    DTshape(ShapeKind, BTreeMap<TshapeFieldName, ShapeFieldType<R>>),
    DTvar(Ident),
    /// The type of a generic parameter. The constraints on a generic parameter
    /// are accessed through the lenv.tpenv component of the environment, which
    /// is set up when checking the body of a function or method. See uses of
    /// Typing_phase.add_generic_parameters_and_constraints. The list denotes
    /// type arguments.
    DTgeneric(TypeName, Vec<DeclTy<R>>),
    /// Union type.
    /// The values that are members of this type are the union of the values
    /// that are members of the components of the union.
    /// Some examples (writing | for binary union)
    ///   Tunion []  is the "nothing" type, with no values
    ///   Tunion [int;float] is the same as num
    ///   Tunion [null;t] is the same as Toption t
    DTunion(Vec<DeclTy<R>>),
    DTintersection(Vec<DeclTy<R>>),
    /// Tvec_or_dict (ty1, ty2) => "vec_or_dict<ty1, ty2>"
    DTvecOrDict(DeclTy<R>, DeclTy<R>),
    /// Name of class, name of type const, remaining names of type consts
    DTaccess(TaccessType<R, DeclTy<R>>),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct TaccessType<R: Reason, TY>(pub TY, pub Positioned<Symbol, R::Pos>);

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum Capability<R: Reason, TY> {
    CapDefaults(R::Pos),
    CapTy(TY),
}

/// Companion to fun_params type, intended to consolidate checking of
/// implicit params for functions.
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunImplicitParams<R: Reason, TY> {
    pub capability: Capability<R, TY>,
}

/// The type of a function AND a method.
/// A function has a min and max arity because of optional arguments
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunType<R: Reason, TY> {
    pub arity: FunArity<R, TY>,
    pub tparams: Vec<Tparam<R, TY>>,
    pub where_constraints: Vec<WhereConstraint<TY>>,
    pub params: FunParams<R, TY>,
    pub implicit_params: FunImplicitParams<R, TY>,
    /// Carries through the sync/async information from the aast
    pub ret: PossiblyEnforcedTy<TY>,
    pub flags: typing_defs_flags::FunTypeFlags,
    pub ifc_decl: IfcFunDecl,
}

/// Arity information for a fun_type; indicating the minimum number of
/// args expected by the function and the maximum number of args for
/// standard, non-variadic functions or the type of variadic argument taken
#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum FunArity<R: Reason, TY> {
    Fstandard,
    /// PHP5.6-style ...$args finishes the func declaration.
    /// min ; variadic param type
    Fvariadic(FunParam<R, TY>),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct PossiblyEnforcedTy<TY> {
    /// True if consumer of this type enforces it at runtime
    pub enforced: Enforcement,
    pub ty: TY,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunParam<R: Reason, TY> {
    pub pos: R::Pos,
    pub name: Option<Symbol>,
    pub ty: PossiblyEnforcedTy<TY>,
    pub flags: FunParamFlags,
}

pub type FunParams<R, TY> = Vec<FunParam<R, TY>>;

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

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct FunElt<R: Reason> {
    pub deprecated: Option<BytesId>,
    pub module: Option<Positioned<Symbol, R::Pos>>,
    /// Top-level functions have limited visibilities
    pub internal: bool,
    pub ty: DeclTy<R>,
    pub pos: R::Pos,
    pub php_std_lib: bool,
    pub support_dynamic_type: bool,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct AbstractTypeconst<R: Reason> {
    pub as_constraint: Option<DeclTy<R>>,
    pub super_constraint: Option<DeclTy<R>>,
    pub default: Option<DeclTy<R>>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct ConcreteTypeconst<R: Reason> {
    pub tc_type: DeclTy<R>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub enum Typeconst<R: Reason> {
    TCAbstract(AbstractTypeconst<R>),
    TCConcrete(ConcreteTypeconst<R>),
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct EnumType<R: Reason> {
    pub base: DeclTy<R>,
    pub constraint: Option<DeclTy<R>>,
    pub includes: Vec<DeclTy<R>>,
}

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct TypedefType<R: Reason> {
    pub module: Option<Positioned<Symbol, R::Pos>>,
    pub pos: R::Pos,
    pub vis: aast::TypedefVisibility,
    pub tparams: Vec<Tparam<R, DeclTy<R>>>,
    pub constraint: Option<DeclTy<R>>,
    pub ty: DeclTy<R>,
    pub is_ctx: bool,
    pub attributes: Vec<UserAttribute<R::Pos>>,
}
