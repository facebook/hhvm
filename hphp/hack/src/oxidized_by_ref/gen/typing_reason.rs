// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e619681b2474e419669022f2f5d8d7c9>>
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

#[rust_to_ocaml(attr = "deriving show")]
pub type LazyString<'a> = lazy::Lazy<&'a str>;

#[rust_to_ocaml(attr = "deriving show")]
pub type LazyStringList<'a, A> = [(A, &'a lazy::Lazy<&'a str>)];

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
    Rnone,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rwitness(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rwitness_from_decl")]
    RwitnessFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Ridx(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ridx_vector")]
    RidxVector(&'a pos::Pos<'a>),
    /// Used as an index, in the Vector case
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ridx_vector_from_decl")]
    RidxVectorFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    /// Because it is iterated in a foreach loop
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rforeach(&'a pos::Pos<'a>),
    /// Because it is iterated "await as" in foreach
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rasyncforeach(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rarith(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rarith_ret")]
    RarithRet(&'a pos::Pos<'a>),
    /// pos, arg float typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rarith_ret_float")]
    #[rust_to_ocaml(inline_tuple)]
    RarithRetFloat(
        &'a (
            &'a pos::Pos<'a>,
            T_<'a>,
            oxidized::typing_reason::ArgPosition,
        ),
    ),
    /// pos, arg num typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rarith_ret_num")]
    #[rust_to_ocaml(inline_tuple)]
    RarithRetNum(
        &'a (
            &'a pos::Pos<'a>,
            T_<'a>,
            oxidized::typing_reason::ArgPosition,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rarith_ret_int")]
    RarithRetInt(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rarith_dynamic")]
    RarithDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rbitwise_dynamic")]
    RbitwiseDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rincdec_dynamic")]
    RincdecDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rcomp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rconcat_ret")]
    RconcatRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rlogic_ret")]
    RlogicRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rbitwise(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rbitwise_ret")]
    RbitwiseRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rno_return")]
    RnoReturn(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rno_return_async")]
    RnoReturnAsync(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rret_fun_kind")]
    #[rust_to_ocaml(inline_tuple)]
    RretFunKind(&'a (&'a pos::Pos<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rret_fun_kind_from_decl")]
    #[rust_to_ocaml(inline_tuple)]
    RretFunKindFromDecl(&'a (&'a pos_or_decl::PosOrDecl<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rhint(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rthrow(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rplaceholder(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rret_div")]
    RretDiv(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ryield_gen")]
    RyieldGen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ryield_asyncgen")]
    RyieldAsyncgen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ryield_asyncnull")]
    RyieldAsyncnull(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ryield_send")]
    RyieldSend(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rlost_info")]
    #[rust_to_ocaml(inline_tuple)]
    RlostInfo(&'a (&'a str, T_<'a>, Blame<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rformat(&'a (&'a pos::Pos<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rclass_class")]
    #[rust_to_ocaml(inline_tuple)]
    RclassClass(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Runknown_class")]
    RunknownClass(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rvar_param")]
    RvarParam(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rvar_param_from_decl")]
    RvarParamFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    /// splat pos, fun def pos, number of args before splat
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Runpack_param")]
    #[rust_to_ocaml(inline_tuple)]
    RunpackParam(&'a (&'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rinout_param")]
    RinoutParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rinstantiate(&'a (T_<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rtypeconst(
        &'a (
            T_<'a>,
            (&'a pos_or_decl::PosOrDecl<'a>, &'a str),
            &'a lazy::Lazy<&'a str>,
            T_<'a>,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rtype_access")]
    #[rust_to_ocaml(inline_tuple)]
    RtypeAccess(&'a (T_<'a>, &'a [(&'a T_<'a>, &'a lazy::Lazy<&'a str>)])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rexpr_dep_type")]
    #[rust_to_ocaml(inline_tuple)]
    RexprDepType(
        &'a (
            T_<'a>,
            &'a pos_or_decl::PosOrDecl<'a>,
            ExprDepTypeReason<'a>,
        ),
    ),
    /// ?-> operator is used
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rnullsafe_op")]
    RnullsafeOp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rtconst_no_cstr")]
    RtconstNoCstr(&'a PosId<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rpredicated(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ris(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ras(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Requal(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rvarray_or_darray_key")]
    RvarrayOrDarrayKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rvec_or_dict_key")]
    RvecOrDictKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rusing(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdynamic_prop")]
    RdynamicProp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdynamic_call")]
    RdynamicCall(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdynamic_construct")]
    RdynamicConstruct(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ridx_dict")]
    RidxDict(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rset_element")]
    RsetElement(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rmissing_optional_field")]
    #[rust_to_ocaml(inline_tuple)]
    RmissingOptionalField(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Runset_field")]
    #[rust_to_ocaml(inline_tuple)]
    RunsetField(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rcontravariant_generic")]
    #[rust_to_ocaml(inline_tuple)]
    RcontravariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rinvariant_generic")]
    #[rust_to_ocaml(inline_tuple)]
    RinvariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rregex(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rimplicit_upper_bound")]
    #[rust_to_ocaml(inline_tuple)]
    RimplicitUpperBound(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rtype_variable")]
    RtypeVariable(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rtype_variable_generics")]
    #[rust_to_ocaml(inline_tuple)]
    RtypeVariableGenerics(&'a (&'a pos::Pos<'a>, &'a str, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rtype_variable_error")]
    RtypeVariableError(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rglobal_type_variable_generics")]
    #[rust_to_ocaml(inline_tuple)]
    RglobalTypeVariableGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rsolve_fail")]
    RsolveFail(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rcstr_on_generics")]
    #[rust_to_ocaml(inline_tuple)]
    RcstrOnGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, PosId<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rlambda_param")]
    #[rust_to_ocaml(inline_tuple)]
    RlambdaParam(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(inline_tuple)]
    Rshape(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rshape_literal")]
    RshapeLiteral(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Renforceable(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rdestructure(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rkey_value_collection_key")]
    RkeyValueCollectionKey(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rglobal_class_prop")]
    RglobalClassProp(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rglobal_fun_param")]
    RglobalFunParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rglobal_fun_ret")]
    RglobalFunRet(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rsplice(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ret_boolean")]
    RetBoolean(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdefault_capability")]
    RdefaultCapability(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rconcat_operand")]
    RconcatOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rinterp_operand")]
    RinterpOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdynamic_coercion")]
    RdynamicCoercion(&'a T_<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rsupport_dynamic_type")]
    RsupportDynamicType(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rdynamic_partial_enforcement")]
    #[rust_to_ocaml(inline_tuple)]
    RdynamicPartialEnforcement(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rrigid_tvar_escape")]
    #[rust_to_ocaml(inline_tuple)]
    RrigidTvarEscape(&'a (&'a pos::Pos<'a>, &'a str, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Ropaque_type_from_module")]
    #[rust_to_ocaml(inline_tuple)]
    RopaqueTypeFromModule(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    #[rust_to_ocaml(name = "Rmissing_class")]
    RmissingClass(&'a pos::Pos<'a>),
    Rinvalid,
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
}
impl<'a> TrivialDrop for Ureason<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ureason<'arena>);
