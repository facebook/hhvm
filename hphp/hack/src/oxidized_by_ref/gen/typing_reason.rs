// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<20534e1077f6a68abb03a54b9f33e45a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep::FromOcamlRepIn;
use ocamlrep::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

#[rust_to_ocaml(attr = "deriving (eq, hash, ord, show)")]
pub type PosId<'a> = (&'a pos_or_decl::PosOrDecl<'a>, &'a ast_defs::Id_<'a>);

pub use oxidized::typing_reason::ArgPosition;

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(C, u8)]
pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ERclass(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ERparent(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ERself(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    ERpu(&'a str),
}
impl<'a> TrivialDrop for ExprDepTypeReason<'a> {}
arena_deserializer::impl_deserialize_in_arena!(ExprDepTypeReason<'arena>);

pub use oxidized::typing_reason::BlameSource;

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
#[rust_to_ocaml(attr = "deriving (eq, hash, show)")]
#[repr(C, u8)]
pub enum Blame<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Blame(&'a (&'a pos::Pos<'a>, oxidized::typing_reason::BlameSource)),
}
impl<'a> TrivialDrop for Blame<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Blame<'arena>);

pub use oxidized::typing_reason::VarianceDir;

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
pub enum CstrVariance<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Dir(&'a oxidized::typing_reason::VarianceDir),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Inv(&'a oxidized::typing_reason::VarianceDir),
}
impl<'a> TrivialDrop for CstrVariance<'a> {}
arena_deserializer::impl_deserialize_in_arena!(CstrVariance<'arena>);

pub use oxidized::typing_reason::CtorKind;
pub use oxidized::typing_reason::FieldKind;

/// Symmetric projections are those in which the same decomposition is applied
/// to both sub- and supertype during inference
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum PrjSymm<'a> {
    #[rust_to_ocaml(name = "Prj_symm_neg")]
    PrjSymmNeg,
    #[rust_to_ocaml(name = "Prj_symm_nullable")]
    PrjSymmNullable,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Prj_symm_ctor")]
    #[rust_to_ocaml(inline_tuple)]
    PrjSymmCtor(
        &'a (
            &'a oxidized::typing_reason::CtorKind,
            &'a str,
            isize,
            CstrVariance<'a>,
        ),
    ),
    #[rust_to_ocaml(name = "Prj_symm_tuple")]
    PrjSymmTuple(isize),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Prj_symm_shape")]
    #[rust_to_ocaml(inline_tuple)]
    PrjSymmShape(
        &'a (
            &'a str,
            &'a oxidized::typing_reason::FieldKind,
            &'a oxidized::typing_reason::FieldKind,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Prj_symm_fn_param")]
    #[rust_to_ocaml(inline_tuple)]
    PrjSymmFnParam(&'a (isize, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Prj_symm_fn_param_inout")]
    #[rust_to_ocaml(inline_tuple)]
    PrjSymmFnParamInout(&'a (isize, isize, &'a oxidized::typing_reason::VarianceDir)),
    #[rust_to_ocaml(name = "Prj_symm_fn_ret")]
    PrjSymmFnRet,
    #[rust_to_ocaml(name = "Prj_symm_supportdyn")]
    PrjSymmSupportdyn,
}
impl<'a> TrivialDrop for PrjSymm<'a> {}
arena_deserializer::impl_deserialize_in_arena!(PrjSymm<'arena>);

pub use oxidized::typing_reason::PrjAsymm;

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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum FlowKind<'a> {
    #[rust_to_ocaml(name = "Flow_assign")]
    FlowAssign,
    #[rust_to_ocaml(name = "Flow_call")]
    FlowCall,
    #[rust_to_ocaml(name = "Flow_prop_access")]
    FlowPropAccess,
    #[rust_to_ocaml(name = "Flow_local")]
    FlowLocal,
    #[rust_to_ocaml(name = "Flow_fun_return")]
    FlowFunReturn,
    #[rust_to_ocaml(name = "Flow_param_hint")]
    FlowParamHint,
    #[rust_to_ocaml(name = "Flow_return_expr")]
    FlowReturnExpr,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Flow_instantiate")]
    FlowInstantiate(&'a str),
}
impl<'a> TrivialDrop for FlowKind<'a> {}
arena_deserializer::impl_deserialize_in_arena!(FlowKind<'arena>);

/// Witness the reason for a type during typing using the position of a hint or
/// expression
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum WitnessLocl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Witness(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Idx_vector")]
    IdxVector(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Foreach(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Asyncforeach(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Arith(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Arith_ret")]
    ArithRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Arith_ret_int")]
    ArithRetInt(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Arith_dynamic")]
    ArithDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Bitwise_dynamic")]
    BitwiseDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Incdec_dynamic")]
    IncdecDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Comp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Concat_ret")]
    ConcatRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Logic_ret")]
    LogicRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Bitwise(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Bitwise_ret")]
    BitwiseRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "No_return")]
    NoReturn(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "No_return_async")]
    NoReturnAsync(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ret_fun_kind")]
    #[rust_to_ocaml(inline_tuple)]
    RetFunKind(&'a (&'a pos::Pos<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Throw(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Placeholder(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ret_div")]
    RetDiv(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Yield_gen")]
    YieldGen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Yield_asyncgen")]
    YieldAsyncgen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Yield_asyncnull")]
    YieldAsyncnull(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Yield_send")]
    YieldSend(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Unknown_class")]
    UnknownClass(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Var_param")]
    VarParam(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Unpack_param")]
    #[rust_to_ocaml(inline_tuple)]
    UnpackParam(&'a (&'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Nullsafe_op")]
    NullsafeOp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Predicated(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Is_refinement")]
    IsRefinement(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "As_refinement")]
    AsRefinement(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Equal(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Using(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Dynamic_prop")]
    DynamicProp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Dynamic_call")]
    DynamicCall(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Dynamic_construct")]
    DynamicConstruct(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Idx_dict")]
    IdxDict(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Idx_set_element")]
    IdxSetElement(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Unset_field")]
    #[rust_to_ocaml(inline_tuple)]
    UnsetField(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Regex(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Type_variable")]
    #[rust_to_ocaml(inline_tuple)]
    TypeVariable(&'a (&'a pos::Pos<'a>, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Type_variable_generics")]
    #[rust_to_ocaml(inline_tuple)]
    TypeVariableGenerics(&'a (&'a pos::Pos<'a>, &'a str, &'a str, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Type_variable_error")]
    #[rust_to_ocaml(inline_tuple)]
    TypeVariableError(&'a (&'a pos::Pos<'a>, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Shape(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Shape_literal")]
    ShapeLiteral(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Destructure(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Key_value_collection_key")]
    KeyValueCollectionKey(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Splice(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Et_boolean")]
    EtBoolean(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Concat_operand")]
    ConcatOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Interp_operand")]
    InterpOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Missing_class")]
    MissingClass(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Captured_like")]
    CapturedLike(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Unsafe_cast")]
    UnsafeCast(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Pattern(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Join_point")]
    JoinPoint(&'a pos::Pos<'a>),
}
impl<'a> TrivialDrop for WitnessLocl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WitnessLocl<'arena>);

/// Witness the reason for a type during decling using the position of a hint
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum WitnessDecl<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Witness_from_decl")]
    WitnessFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Idx_vector_from_decl")]
    IdxVectorFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Hint(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Class_class")]
    #[rust_to_ocaml(inline_tuple)]
    ClassClass(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Var_param_from_decl")]
    VarParamFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tuple_from_splat")]
    TupleFromSplat(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Vec_or_dict_key")]
    VecOrDictKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ret_fun_kind_from_decl")]
    #[rust_to_ocaml(inline_tuple)]
    RetFunKindFromDecl(&'a (&'a pos_or_decl::PosOrDecl<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Inout_param")]
    InoutParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Tconst_no_cstr")]
    TconstNoCstr(&'a PosId<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Varray_or_darray_key")]
    VarrayOrDarrayKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Missing_optional_field")]
    #[rust_to_ocaml(inline_tuple)]
    MissingOptionalField(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Implicit_upper_bound")]
    #[rust_to_ocaml(inline_tuple)]
    ImplicitUpperBound(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Global_type_variable_generics")]
    #[rust_to_ocaml(inline_tuple)]
    GlobalTypeVariableGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Solve_fail")]
    SolveFail(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Cstr_on_generics")]
    #[rust_to_ocaml(inline_tuple)]
    CstrOnGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, PosId<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Enforceable(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Global_class_prop")]
    GlobalClassProp(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Global_fun_param")]
    GlobalFunParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Global_fun_ret")]
    GlobalFunRet(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Default_capability")]
    DefaultCapability(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Support_dynamic_type")]
    SupportDynamicType(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Pessimised_inout")]
    PessimisedInout(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Pessimised_return")]
    PessimisedReturn(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Pessimised_prop")]
    PessimisedProp(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Pessimised_this")]
    PessimisedThis(&'a pos_or_decl::PosOrDecl<'a>),
}
impl<'a> TrivialDrop for WitnessDecl<'a> {}
arena_deserializer::impl_deserialize_in_arena!(WitnessDecl<'arena>);

pub use oxidized::typing_reason::Axiom;

/// The reason why something is expected to have a certain type
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
#[rust_to_ocaml(attr = "deriving hash")]
#[repr(C, u8)]
pub enum T_<'a> {
    /// Lift a decl-time witness into a reason
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "From_witness_decl")]
    FromWitnessDecl(&'a WitnessDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Instantiate(&'a (T_<'a>, &'a str, T_<'a>)),
    #[rust_to_ocaml(name = "No_reason")]
    NoReason,
    /// Lift a typing-time witness into a reason
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "From_witness_locl")]
    FromWitnessLocl(&'a WitnessLocl<'a>),
    /// Records that a type with reason [bound] acted as a lower bound
    /// for the type with reason [of_]
    #[rust_to_ocaml(name = "Lower_bound")]
    LowerBound {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        bound: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        of__: &'a T_<'a>,
    },
    /// Records the flow of a type from an expression or hint into an
    /// expression during typing
    Flow {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        from: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        kind: FlowKind<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        into: &'a T_<'a>,
    },
    /// Represents the projection of the sub- and supertype during subtype
    /// constraints simplifiction. [sub_prj] is the subtype resulting from the
    /// projection whilst [sub] and [super] and the reasons for the parent
    /// types
    #[rust_to_ocaml(name = "Prj_both")]
    PrjBoth {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        sub_prj: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        prj: PrjSymm<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        sub: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        super_: &'a T_<'a>,
    },
    /// Represents the projection of the sub- or supertype during subtype
    /// constraints simplifiction. [part] is the sub/supertype resulting from
    /// the projection whilst [whole] is the reason for the parent type.
    #[rust_to_ocaml(name = "Prj_one")]
    PrjOne {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        part: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        whole: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        prj: &'a oxidized::typing_reason::PrjAsymm,
    },
    /// Represents the use of a user-defined axiom about either the
    /// subtype or supertype during subtype constraints simplifiction.
    /// [next] is the sub/supertype resulting from the application of the
    /// axiom whilst [prev] is reason for original type.
    Axiom {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        next: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        prev: &'a T_<'a>,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        axiom: &'a oxidized::typing_reason::Axiom,
    },
    /// Records the definition site of type alongside the reason recording its
    /// use.
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Def(&'a (&'a pos_or_decl::PosOrDecl<'a>, T_<'a>)),
    Solved {
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        solution: &'a T_<'a>,
        of__: isize,
        #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
        in__: &'a T_<'a>,
    },
    Invalid,
    #[rust_to_ocaml(name = "Missing_field")]
    MissingField,
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Idx(&'a (&'a pos::Pos<'a>, T_<'a>)),
    /// pos, arg float typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Arith_ret_float")]
    #[rust_to_ocaml(inline_tuple)]
    ArithRetFloat(
        &'a (
            &'a pos::Pos<'a>,
            T_<'a>,
            oxidized::typing_reason::ArgPosition,
        ),
    ),
    /// pos, arg num typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Arith_ret_num")]
    #[rust_to_ocaml(inline_tuple)]
    ArithRetNum(
        &'a (
            &'a pos::Pos<'a>,
            T_<'a>,
            oxidized::typing_reason::ArgPosition,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Lost_info")]
    #[rust_to_ocaml(inline_tuple)]
    LostInfo(&'a (&'a str, T_<'a>, Blame<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Format(&'a (&'a pos::Pos<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Typeconst(
        &'a (
            T_<'a>,
            (&'a pos_or_decl::PosOrDecl<'a>, &'a str),
            &'a lazy::Lazy<&'a str>,
            T_<'a>,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Type_access")]
    #[rust_to_ocaml(inline_tuple)]
    TypeAccess(&'a (T_<'a>, &'a [(&'a T_<'a>, &'a lazy::Lazy<&'a str>)])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Expr_dep_type")]
    #[rust_to_ocaml(inline_tuple)]
    ExprDepType(
        &'a (
            T_<'a>,
            &'a pos_or_decl::PosOrDecl<'a>,
            ExprDepTypeReason<'a>,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Contravariant_generic")]
    #[rust_to_ocaml(inline_tuple)]
    ContravariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Invariant_generic")]
    #[rust_to_ocaml(inline_tuple)]
    InvariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Lambda_param")]
    #[rust_to_ocaml(inline_tuple)]
    LambdaParam(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Dynamic_coercion")]
    DynamicCoercion(&'a T_<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Dynamic_partial_enforcement")]
    #[rust_to_ocaml(inline_tuple)]
    DynamicPartialEnforcement(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rigid_tvar_escape")]
    #[rust_to_ocaml(inline_tuple)]
    RigidTvarEscape(&'a (&'a pos::Pos<'a>, &'a str, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Opaque_type_from_module")]
    #[rust_to_ocaml(inline_tuple)]
    OpaqueTypeFromModule(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, T_<'a>)),
}
impl<'a> TrivialDrop for T_<'a> {}
arena_deserializer::impl_deserialize_in_arena!(T_<'arena>);

pub type Reason<'a> = T_<'a>;

pub type DeclT<'a> = T_<'a>;

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
#[rust_to_ocaml(attr = "deriving show")]
#[repr(C, u8)]
pub enum Ureason<'a> {
    URnone,
    URassign,
    #[rust_to_ocaml(name = "URassign_inout")]
    URassignInout,
    URhint,
    URreturn,
    URforeach,
    URthrow,
    URvector,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URkey(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URvalue(&'a str),
    URawait,
    URyield,
    /// Name of XHP class, Name of XHP attribute
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    URxhp(&'a (&'a str, &'a str)),
    #[rust_to_ocaml(name = "URxhp_spread")]
    URxhpSpread,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URindex(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URelement(&'a str),
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
impl<'a> TrivialDrop for Ureason<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ureason<'arena>);
