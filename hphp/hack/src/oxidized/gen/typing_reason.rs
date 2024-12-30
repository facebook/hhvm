// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d166261debd93682ed77ea152fba947d>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRep;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
pub type PosId = (pos_or_decl::PosOrDecl, ast_defs::Id_);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(u8)]
pub enum ArgPosition {
    Aonly,
    Afirst,
    Asecond,
}
impl TrivialDrop for ArgPosition {}
arena_deserializer::impl_deserialize_in_arena!(ArgPosition);

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
pub enum ExprDepTypeReason {
    ERexpr(isize),
    ERstatic,
    ERclass(String),
    ERparent(String),
    ERself(String),
    ERpu(String),
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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(u8)]
pub enum PessimiseReason {
    PRabstract,
    #[rust_to_ocaml(name = "PRgeneric_param")]
    PRgenericParam,
    PRthis,
    #[rust_to_ocaml(name = "PRgeneric_apply")]
    PRgenericApply,
    #[rust_to_ocaml(name = "PRtuple_or_shape")]
    PRtupleOrShape,
    PRtypeconst,
    PRcase,
    PRenum,
    PRopaque,
    PRdynamic,
    PRfun,
    PRclassptr,
    #[rust_to_ocaml(name = "PRvoid_or_noreturn")]
    PRvoidOrNoreturn,
    PRrefinement,
    #[rust_to_ocaml(name = "PRunion_or_intersection")]
    PRunionOrIntersection,
    PRxhp,
}
impl TrivialDrop for PessimiseReason {}
arena_deserializer::impl_deserialize_in_arena!(PessimiseReason);

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(u8)]
pub enum BlameSource {
    BScall,
    BSlambda,
    BSassignment,
    #[rust_to_ocaml(name = "BSout_of_scope")]
    BSoutOfScope,
}
impl TrivialDrop for BlameSource {}
arena_deserializer::impl_deserialize_in_arena!(BlameSource);

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
pub enum Blame {
    Blame(pos::Pos, BlameSource),
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(u8)]
pub enum VarianceDir {
    Co,
    Contra,
}
impl TrivialDrop for VarianceDir {}
arena_deserializer::impl_deserialize_in_arena!(VarianceDir);

/// When recording the decomposition of a type during inference we want to keep
/// track of variance so we can give intuition about the direction of 'flow'.
/// In the case of invariant type paramters, we record both the fact that it was
/// invariant and the direction in which the error occurred
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum CstrVariance {
    Dir(VarianceDir),
    Inv(VarianceDir),
}

/// Shape field kinds
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
#[rust_to_ocaml(attr = "deriving (eq, hash)")]
#[repr(u8)]
pub enum FieldKind {
    Absent,
    Optional,
    Required,
}
impl TrivialDrop for FieldKind {}
arena_deserializer::impl_deserialize_in_arena!(FieldKind);

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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(u8)]
pub enum CtorKind {
    #[rust_to_ocaml(name = "Ctor_class")]
    CtorClass,
    #[rust_to_ocaml(name = "Ctor_newtype")]
    CtorNewtype,
}
impl TrivialDrop for CtorKind {}
arena_deserializer::impl_deserialize_in_arena!(CtorKind);

/// Symmetric projections are those in which the same decomposition is applied
/// to both sub- and supertype during inference
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum PrjSymm {
    #[rust_to_ocaml(name = "Prj_symm_neg")]
    PrjSymmNeg,
    #[rust_to_ocaml(name = "Prj_symm_nullable")]
    PrjSymmNullable,
    #[rust_to_ocaml(name = "Prj_symm_ctor")]
    PrjSymmCtor(CtorKind, String, isize, CstrVariance),
    #[rust_to_ocaml(name = "Prj_symm_tuple")]
    PrjSymmTuple(isize),
    #[rust_to_ocaml(name = "Prj_symm_shape")]
    PrjSymmShape(String, FieldKind, FieldKind),
    #[rust_to_ocaml(name = "Prj_symm_fn_param")]
    PrjSymmFnParam(isize, isize),
    #[rust_to_ocaml(name = "Prj_symm_fn_param_inout")]
    PrjSymmFnParamInout(isize, isize, VarianceDir),
    #[rust_to_ocaml(name = "Prj_symm_fn_ret")]
    PrjSymmFnRet,
    #[rust_to_ocaml(name = "Prj_symm_supportdyn")]
    PrjSymmSupportdyn,
}

/// Asymmetric projections are those in which the same decomposition is applied
/// to only one of the sub- or supertype during inference
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(u8)]
pub enum PrjAsymm {
    #[rust_to_ocaml(name = "Prj_asymm_union")]
    PrjAsymmUnion,
    #[rust_to_ocaml(name = "Prj_asymm_inter")]
    PrjAsymmInter,
    #[rust_to_ocaml(name = "Prj_asymm_neg")]
    PrjAsymmNeg,
    #[rust_to_ocaml(name = "Prj_asymm_nullable")]
    PrjAsymmNullable,
    #[rust_to_ocaml(name = "Prj_asymm_arraykey")]
    PrjAsymmArraykey,
    #[rust_to_ocaml(name = "Prj_asymm_num")]
    PrjAsymmNum,
    #[rust_to_ocaml(name = "Prj_rewrite_classname")]
    PrjRewriteClassname,
}
impl TrivialDrop for PrjAsymm {}
arena_deserializer::impl_deserialize_in_arena!(PrjAsymm);

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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum FlowKind {
    #[rust_to_ocaml(name = "Flow_assign")]
    FlowAssign,
    #[rust_to_ocaml(name = "Flow_call")]
    FlowCall,
    #[rust_to_ocaml(name = "Flow_prop_access")]
    FlowPropAccess,
    #[rust_to_ocaml(name = "Flow_const_access")]
    FlowConstAccess,
    #[rust_to_ocaml(name = "Flow_local")]
    FlowLocal,
    #[rust_to_ocaml(name = "Flow_fun_return")]
    FlowFunReturn,
    #[rust_to_ocaml(name = "Flow_param_hint")]
    FlowParamHint,
    #[rust_to_ocaml(name = "Flow_return_expr")]
    FlowReturnExpr,
    #[rust_to_ocaml(name = "Flow_instantiate")]
    FlowInstantiate(String),
}

/// Witness the reason for a type during typing using the position of a hint or
/// expression
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum WitnessLocl {
    Witness(pos::Pos),
    #[rust_to_ocaml(name = "Idx_vector")]
    IdxVector(pos::Pos),
    Foreach(pos::Pos),
    Asyncforeach(pos::Pos),
    Arith(pos::Pos),
    #[rust_to_ocaml(name = "Arith_ret")]
    ArithRet(pos::Pos),
    #[rust_to_ocaml(name = "Arith_ret_int")]
    ArithRetInt(pos::Pos),
    #[rust_to_ocaml(name = "Arith_dynamic")]
    ArithDynamic(pos::Pos),
    #[rust_to_ocaml(name = "Bitwise_dynamic")]
    BitwiseDynamic(pos::Pos),
    #[rust_to_ocaml(name = "Incdec_dynamic")]
    IncdecDynamic(pos::Pos),
    Comp(pos::Pos),
    #[rust_to_ocaml(name = "Concat_ret")]
    ConcatRet(pos::Pos),
    #[rust_to_ocaml(name = "Logic_ret")]
    LogicRet(pos::Pos),
    Bitwise(pos::Pos),
    #[rust_to_ocaml(name = "Bitwise_ret")]
    BitwiseRet(pos::Pos),
    #[rust_to_ocaml(name = "No_return")]
    NoReturn(pos::Pos),
    #[rust_to_ocaml(name = "No_return_async")]
    NoReturnAsync(pos::Pos),
    #[rust_to_ocaml(name = "Ret_fun_kind")]
    RetFunKind(pos::Pos, ast_defs::FunKind),
    Throw(pos::Pos),
    Placeholder(pos::Pos),
    #[rust_to_ocaml(name = "Ret_div")]
    RetDiv(pos::Pos),
    #[rust_to_ocaml(name = "Yield_gen")]
    YieldGen(pos::Pos),
    #[rust_to_ocaml(name = "Yield_asyncgen")]
    YieldAsyncgen(pos::Pos),
    #[rust_to_ocaml(name = "Yield_asyncnull")]
    YieldAsyncnull(pos::Pos),
    #[rust_to_ocaml(name = "Yield_send")]
    YieldSend(pos::Pos),
    #[rust_to_ocaml(name = "Unknown_class")]
    UnknownClass(pos::Pos),
    #[rust_to_ocaml(name = "Var_param")]
    VarParam(pos::Pos),
    #[rust_to_ocaml(name = "Unpack_param")]
    UnpackParam(pos::Pos, pos_or_decl::PosOrDecl, isize),
    #[rust_to_ocaml(name = "Nullsafe_op")]
    NullsafeOp(pos::Pos),
    Predicated(pos::Pos, String),
    #[rust_to_ocaml(name = "Is_refinement")]
    IsRefinement(pos::Pos),
    #[rust_to_ocaml(name = "As_refinement")]
    AsRefinement(pos::Pos),
    Equal(pos::Pos),
    Using(pos::Pos),
    #[rust_to_ocaml(name = "Dynamic_prop")]
    DynamicProp(pos::Pos),
    #[rust_to_ocaml(name = "Dynamic_call")]
    DynamicCall(pos::Pos),
    #[rust_to_ocaml(name = "Dynamic_construct")]
    DynamicConstruct(pos::Pos),
    #[rust_to_ocaml(name = "Idx_dict")]
    IdxDict(pos::Pos),
    #[rust_to_ocaml(name = "Idx_set_element")]
    IdxSetElement(pos::Pos),
    #[rust_to_ocaml(name = "Unset_field")]
    UnsetField(pos::Pos, String),
    Regex(pos::Pos),
    #[rust_to_ocaml(name = "Type_variable")]
    TypeVariable(pos::Pos, isize),
    #[rust_to_ocaml(name = "Type_variable_generics")]
    TypeVariableGenerics(pos::Pos, String, String, isize),
    #[rust_to_ocaml(name = "Type_variable_error")]
    TypeVariableError(pos::Pos, isize),
    Shape(pos::Pos, String),
    #[rust_to_ocaml(name = "Shape_literal")]
    ShapeLiteral(pos::Pos),
    Destructure(pos::Pos),
    #[rust_to_ocaml(name = "Key_value_collection_key")]
    KeyValueCollectionKey(pos::Pos),
    Splice(pos::Pos),
    #[rust_to_ocaml(name = "Et_boolean")]
    EtBoolean(pos::Pos),
    #[rust_to_ocaml(name = "Concat_operand")]
    ConcatOperand(pos::Pos),
    #[rust_to_ocaml(name = "Interp_operand")]
    InterpOperand(pos::Pos),
    #[rust_to_ocaml(name = "Missing_class")]
    MissingClass(pos::Pos),
    #[rust_to_ocaml(name = "Captured_like")]
    CapturedLike(pos::Pos),
    #[rust_to_ocaml(name = "Unsafe_cast")]
    UnsafeCast(pos::Pos),
    Pattern(pos::Pos),
    #[rust_to_ocaml(name = "Join_point")]
    JoinPoint(pos::Pos),
    #[rust_to_ocaml(name = "Static_property_access")]
    StaticPropertyAccess(pos::Pos),
    #[rust_to_ocaml(name = "Class_constant_access")]
    ClassConstantAccess(pos::Pos),
}

/// Witness the reason for a type during decling using the position of a hint
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum WitnessDecl {
    #[rust_to_ocaml(name = "Witness_from_decl")]
    WitnessFromDecl(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Idx_vector_from_decl")]
    IdxVectorFromDecl(pos_or_decl::PosOrDecl),
    Hint(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Class_class")]
    ClassClass(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Var_param_from_decl")]
    VarParamFromDecl(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Tuple_from_splat")]
    TupleFromSplat(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Vec_or_dict_key")]
    VecOrDictKey(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Ret_fun_kind_from_decl")]
    RetFunKindFromDecl(pos_or_decl::PosOrDecl, ast_defs::FunKind),
    #[rust_to_ocaml(name = "Inout_param")]
    InoutParam(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Tconst_no_cstr")]
    TconstNoCstr(PosId),
    #[rust_to_ocaml(name = "Varray_or_darray_key")]
    VarrayOrDarrayKey(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Missing_optional_field")]
    MissingOptionalField(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Implicit_upper_bound")]
    ImplicitUpperBound(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Global_type_variable_generics")]
    GlobalTypeVariableGenerics(pos_or_decl::PosOrDecl, String, String),
    #[rust_to_ocaml(name = "Solve_fail")]
    SolveFail(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Cstr_on_generics")]
    CstrOnGenerics(pos_or_decl::PosOrDecl, PosId),
    Enforceable(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Global_class_prop")]
    GlobalClassProp(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Global_fun_param")]
    GlobalFunParam(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Global_fun_ret")]
    GlobalFunRet(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Default_capability")]
    DefaultCapability(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Support_dynamic_type")]
    SupportDynamicType(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Pessimised_inout")]
    PessimisedInout(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Pessimised_return")]
    PessimisedReturn(pos_or_decl::PosOrDecl, PessimiseReason),
    #[rust_to_ocaml(name = "Pessimised_prop")]
    PessimisedProp(pos_or_decl::PosOrDecl, PessimiseReason),
    #[rust_to_ocaml(name = "Pessimised_this")]
    PessimisedThis(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Illegal_recursive_type")]
    IllegalRecursiveType(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Support_dynamic_type_assume")]
    SupportDynamicTypeAssume(pos_or_decl::PosOrDecl),
}

/// Axioms are information about types provided by the user in class or type
/// parameter declarations. We make use of this information during subtype
/// constraint simplification
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(u8)]
pub enum Axiom {
    Extends,
    #[rust_to_ocaml(name = "Upper_bound")]
    UpperBound,
    #[rust_to_ocaml(name = "Lower_bound")]
    LowerBound,
}
impl TrivialDrop for Axiom {}
arena_deserializer::impl_deserialize_in_arena!(Axiom);

/// The reason why something is expected to have a certain type
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum T_ {
    /// Lift a decl-time witness into a reason
    #[rust_to_ocaml(name = "From_witness_decl")]
    FromWitnessDecl(WitnessDecl),
    Instantiate(Box<T_>, String, Box<T_>),
    #[rust_to_ocaml(name = "No_reason")]
    NoReason,
    /// Lift a typing-time witness into a reason
    #[rust_to_ocaml(name = "From_witness_locl")]
    FromWitnessLocl(WitnessLocl),
    /// Records that a type with reason [bound] acted as a lower bound
    /// for the type with reason [of_]
    #[rust_to_ocaml(name = "Lower_bound")]
    LowerBound {
        bound: Box<T_>,
        of__: Box<T_>,
    },
    /// Records the flow of a type from an expression or hint into an
    /// expression during typing
    Flow {
        from: Box<T_>,
        kind: FlowKind,
        into: Box<T_>,
    },
    /// Represents the projection of the sub- and supertype during subtype
    /// constraints simplifiction. [sub_prj] is the subtype resulting from the
    /// projection whilst [sub] and [super] and the reasons for the parent
    /// types
    #[rust_to_ocaml(name = "Prj_both")]
    PrjBoth {
        sub_prj: Box<T_>,
        prj: PrjSymm,
        sub: Box<T_>,
        super_: Box<T_>,
    },
    /// Represents the projection of the sub- or supertype during subtype
    /// constraints simplifiction. [part] is the sub/supertype resulting from
    /// the projection whilst [whole] is the reason for the parent type.
    #[rust_to_ocaml(name = "Prj_one")]
    PrjOne {
        part: Box<T_>,
        whole: Box<T_>,
        prj: PrjAsymm,
    },
    /// Represents the use of a user-defined axiom about either the
    /// subtype or supertype during subtype constraints simplifiction.
    /// [next] is the sub/supertype resulting from the application of the
    /// axiom whilst [prev] is reason for original type.
    Axiom {
        next: Box<T_>,
        prev: Box<T_>,
        axiom: Axiom,
    },
    /// Records the definition site of type alongside the reason recording its
    /// use.
    Def(pos_or_decl::PosOrDecl, Box<T_>),
    Solved {
        solution: Box<T_>,
        of__: isize,
        in__: Box<T_>,
    },
    Invalid,
    #[rust_to_ocaml(name = "Missing_field")]
    MissingField,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Idx(pos::Pos, Box<T_>),
    /// pos, arg float typing reason, arg position
    #[rust_to_ocaml(name = "Arith_ret_float")]
    ArithRetFloat(pos::Pos, Box<T_>, ArgPosition),
    /// pos, arg num typing reason, arg position
    #[rust_to_ocaml(name = "Arith_ret_num")]
    ArithRetNum(pos::Pos, Box<T_>, ArgPosition),
    #[rust_to_ocaml(name = "Lost_info")]
    LostInfo(String, Box<T_>, Blame),
    Format(pos::Pos, String, Box<T_>),
    Typeconst(
        Box<T_>,
        (pos_or_decl::PosOrDecl, String),
        lazy::Lazy<String>,
        Box<T_>,
    ),
    #[rust_to_ocaml(name = "Type_access")]
    TypeAccess(Box<T_>, Vec<(Box<T_>, lazy::Lazy<String>)>),
    #[rust_to_ocaml(name = "Expr_dep_type")]
    ExprDepType(Box<T_>, pos_or_decl::PosOrDecl, ExprDepTypeReason),
    #[rust_to_ocaml(name = "Contravariant_generic")]
    ContravariantGeneric(Box<T_>, String),
    #[rust_to_ocaml(name = "Invariant_generic")]
    InvariantGeneric(Box<T_>, String),
    #[rust_to_ocaml(name = "Lambda_param")]
    LambdaParam(pos::Pos, Box<T_>),
    #[rust_to_ocaml(name = "Dynamic_coercion")]
    DynamicCoercion(Box<T_>),
    #[rust_to_ocaml(name = "Dynamic_partial_enforcement")]
    DynamicPartialEnforcement(pos_or_decl::PosOrDecl, String, Box<T_>),
    #[rust_to_ocaml(name = "Rigid_tvar_escape")]
    RigidTvarEscape(pos::Pos, String, String, Box<T_>),
    #[rust_to_ocaml(name = "Opaque_type_from_module")]
    OpaqueTypeFromModule(pos_or_decl::PosOrDecl, String, Box<T_>),
    #[rust_to_ocaml(name = "SDT_call")]
    SDTCall(pos_or_decl::PosOrDecl, Box<T_>),
    #[rust_to_ocaml(name = "Like_call")]
    LikeCall(pos_or_decl::PosOrDecl, Box<T_>),
}

pub type Reason = T_;

pub type DeclT = T_;

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
#[repr(C, u8)]
pub enum Ureason {
    URnone,
    URassign,
    #[rust_to_ocaml(name = "URassign_inout")]
    URassignInout,
    URhint,
    URreturn,
    URforeach,
    URthrow,
    URvector,
    URkey(String),
    URvalue(String),
    URawait,
    URyield,
    /// Name of XHP class, Name of XHP attribute
    URxhp(String, String),
    #[rust_to_ocaml(name = "URxhp_spread")]
    URxhpSpread,
    URindex(String),
    URelement(String),
    URparam,
    #[rust_to_ocaml(name = "URparam_inout")]
    URparamInout,
    #[rust_to_ocaml(name = "URarray_value")]
    URarrayValue,
    #[rust_to_ocaml(name = "URpair_value")]
    URpairValue,
    #[rust_to_ocaml(name = "URtuple_access")]
    URtupleAccess,
    #[rust_to_ocaml(name = "URpair_access")]
    URpairAccess,
    #[rust_to_ocaml(name = "URnewtype_cstr")]
    URnewtypeCstr,
    #[rust_to_ocaml(name = "URclass_req")]
    URclassReq,
    URenum,
    #[rust_to_ocaml(name = "URenum_include")]
    URenumInclude,
    #[rust_to_ocaml(name = "URenum_cstr")]
    URenumCstr,
    #[rust_to_ocaml(name = "URenum_underlying")]
    URenumUnderlying,
    #[rust_to_ocaml(name = "URenum_incompatible_cstr")]
    URenumIncompatibleCstr,
    #[rust_to_ocaml(name = "URtypeconst_cstr")]
    URtypeconstCstr,
    #[rust_to_ocaml(name = "URsubsume_tconst_cstr")]
    URsubsumeTconstCstr,
    #[rust_to_ocaml(name = "URsubsume_tconst_assign")]
    URsubsumeTconstAssign,
    URclone,
    URusing,
    #[rust_to_ocaml(name = "URstr_concat")]
    URstrConcat,
    #[rust_to_ocaml(name = "URstr_interp")]
    URstrInterp,
    #[rust_to_ocaml(name = "URdynamic_prop")]
    URdynamicProp,
    URlabel,
}
