// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<9a65594b3b4ab1c8fd2cf6661743b78e>>
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

#[rust_to_ocaml(attr = "deriving show")]
pub type LazyString = lazy::Lazy<String>;

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
    Rnone,
    Rwitness(pos::Pos),
    #[rust_to_ocaml(name = "Rwitness_from_decl")]
    RwitnessFromDecl(pos_or_decl::PosOrDecl),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(pos::Pos, Box<T_>),
    #[rust_to_ocaml(name = "Ridx_vector")]
    RidxVector(pos::Pos),
    /// Used as an index, in the Vector case
    #[rust_to_ocaml(name = "Ridx_vector_from_decl")]
    RidxVectorFromDecl(pos_or_decl::PosOrDecl),
    /// Because it is iterated in a foreach loop
    Rforeach(pos::Pos),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(pos::Pos),
    Rarith(pos::Pos),
    #[rust_to_ocaml(name = "Rarith_ret")]
    RarithRet(pos::Pos),
    /// pos, arg float typing reason, arg position
    #[rust_to_ocaml(name = "Rarith_ret_float")]
    RarithRetFloat(pos::Pos, Box<T_>, ArgPosition),
    /// pos, arg num typing reason, arg position
    #[rust_to_ocaml(name = "Rarith_ret_num")]
    RarithRetNum(pos::Pos, Box<T_>, ArgPosition),
    #[rust_to_ocaml(name = "Rarith_ret_int")]
    RarithRetInt(pos::Pos),
    #[rust_to_ocaml(name = "Rarith_dynamic")]
    RarithDynamic(pos::Pos),
    #[rust_to_ocaml(name = "Rbitwise_dynamic")]
    RbitwiseDynamic(pos::Pos),
    #[rust_to_ocaml(name = "Rincdec_dynamic")]
    RincdecDynamic(pos::Pos),
    Rcomp(pos::Pos),
    #[rust_to_ocaml(name = "Rconcat_ret")]
    RconcatRet(pos::Pos),
    #[rust_to_ocaml(name = "Rlogic_ret")]
    RlogicRet(pos::Pos),
    Rbitwise(pos::Pos),
    #[rust_to_ocaml(name = "Rbitwise_ret")]
    RbitwiseRet(pos::Pos),
    #[rust_to_ocaml(name = "Rno_return")]
    RnoReturn(pos::Pos),
    #[rust_to_ocaml(name = "Rno_return_async")]
    RnoReturnAsync(pos::Pos),
    #[rust_to_ocaml(name = "Rret_fun_kind")]
    RretFunKind(pos::Pos, ast_defs::FunKind),
    #[rust_to_ocaml(name = "Rret_fun_kind_from_decl")]
    RretFunKindFromDecl(pos_or_decl::PosOrDecl, ast_defs::FunKind),
    Rhint(pos_or_decl::PosOrDecl),
    Rthrow(pos::Pos),
    Rplaceholder(pos::Pos),
    #[rust_to_ocaml(name = "Rret_div")]
    RretDiv(pos::Pos),
    #[rust_to_ocaml(name = "Ryield_gen")]
    RyieldGen(pos::Pos),
    #[rust_to_ocaml(name = "Ryield_asyncgen")]
    RyieldAsyncgen(pos::Pos),
    #[rust_to_ocaml(name = "Ryield_asyncnull")]
    RyieldAsyncnull(pos::Pos),
    #[rust_to_ocaml(name = "Ryield_send")]
    RyieldSend(pos::Pos),
    #[rust_to_ocaml(name = "Rlost_info")]
    RlostInfo(String, Box<T_>, Blame),
    Rformat(pos::Pos, String, Box<T_>),
    #[rust_to_ocaml(name = "Rclass_class")]
    RclassClass(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Runknown_class")]
    RunknownClass(pos::Pos),
    #[rust_to_ocaml(name = "Rvar_param")]
    RvarParam(pos::Pos),
    #[rust_to_ocaml(name = "Rvar_param_from_decl")]
    RvarParamFromDecl(pos_or_decl::PosOrDecl),
    /// splat pos, fun def pos, number of args before splat
    #[rust_to_ocaml(name = "Runpack_param")]
    RunpackParam(pos::Pos, pos_or_decl::PosOrDecl, isize),
    #[rust_to_ocaml(name = "Rinout_param")]
    RinoutParam(pos_or_decl::PosOrDecl),
    Rinstantiate(Box<T_>, String, Box<T_>),
    Rtypeconst(
        Box<T_>,
        (pos_or_decl::PosOrDecl, String),
        lazy::Lazy<String>,
        Box<T_>,
    ),
    #[rust_to_ocaml(name = "Rtype_access")]
    RtypeAccess(Box<T_>, Vec<(Box<T_>, lazy::Lazy<String>)>),
    #[rust_to_ocaml(name = "Rexpr_dep_type")]
    RexprDepType(Box<T_>, pos_or_decl::PosOrDecl, ExprDepTypeReason),
    /// ?-> operator is used
    #[rust_to_ocaml(name = "Rnullsafe_op")]
    RnullsafeOp(pos::Pos),
    #[rust_to_ocaml(name = "Rtconst_no_cstr")]
    RtconstNoCstr(PosId),
    Rpredicated(pos::Pos, String),
    Ris(pos::Pos),
    Ras(pos::Pos),
    Requal(pos::Pos),
    #[rust_to_ocaml(name = "Rvarray_or_darray_key")]
    RvarrayOrDarrayKey(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rvec_or_dict_key")]
    RvecOrDictKey(pos_or_decl::PosOrDecl),
    Rusing(pos::Pos),
    #[rust_to_ocaml(name = "Rdynamic_prop")]
    RdynamicProp(pos::Pos),
    #[rust_to_ocaml(name = "Rdynamic_call")]
    RdynamicCall(pos::Pos),
    #[rust_to_ocaml(name = "Rdynamic_construct")]
    RdynamicConstruct(pos::Pos),
    #[rust_to_ocaml(name = "Ridx_dict")]
    RidxDict(pos::Pos),
    #[rust_to_ocaml(name = "Rset_element")]
    RsetElement(pos::Pos),
    #[rust_to_ocaml(name = "Rmissing_optional_field")]
    RmissingOptionalField(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Runset_field")]
    RunsetField(pos::Pos, String),
    #[rust_to_ocaml(name = "Rcontravariant_generic")]
    RcontravariantGeneric(Box<T_>, String),
    #[rust_to_ocaml(name = "Rinvariant_generic")]
    RinvariantGeneric(Box<T_>, String),
    Rregex(pos::Pos),
    #[rust_to_ocaml(name = "Rimplicit_upper_bound")]
    RimplicitUpperBound(pos_or_decl::PosOrDecl, String),
    #[rust_to_ocaml(name = "Rtype_variable")]
    RtypeVariable(pos::Pos),
    #[rust_to_ocaml(name = "Rtype_variable_generics")]
    RtypeVariableGenerics(pos::Pos, String, String),
    #[rust_to_ocaml(name = "Rtype_variable_error")]
    RtypeVariableError(pos::Pos),
    #[rust_to_ocaml(name = "Rglobal_type_variable_generics")]
    RglobalTypeVariableGenerics(pos_or_decl::PosOrDecl, String, String),
    #[rust_to_ocaml(name = "Rsolve_fail")]
    RsolveFail(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rcstr_on_generics")]
    RcstrOnGenerics(pos_or_decl::PosOrDecl, PosId),
    #[rust_to_ocaml(name = "Rlambda_param")]
    RlambdaParam(pos::Pos, Box<T_>),
    Rshape(pos::Pos, String),
    #[rust_to_ocaml(name = "Rshape_literal")]
    RshapeLiteral(pos::Pos),
    Renforceable(pos_or_decl::PosOrDecl),
    Rdestructure(pos::Pos),
    #[rust_to_ocaml(name = "Rkey_value_collection_key")]
    RkeyValueCollectionKey(pos::Pos),
    #[rust_to_ocaml(name = "Rglobal_class_prop")]
    RglobalClassProp(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rglobal_fun_param")]
    RglobalFunParam(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rglobal_fun_ret")]
    RglobalFunRet(pos_or_decl::PosOrDecl),
    Rsplice(pos::Pos),
    #[rust_to_ocaml(name = "Ret_boolean")]
    RetBoolean(pos::Pos),
    #[rust_to_ocaml(name = "Rdefault_capability")]
    RdefaultCapability(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rconcat_operand")]
    RconcatOperand(pos::Pos),
    #[rust_to_ocaml(name = "Rinterp_operand")]
    RinterpOperand(pos::Pos),
    #[rust_to_ocaml(name = "Rdynamic_coercion")]
    RdynamicCoercion(Box<T_>),
    #[rust_to_ocaml(name = "Rsupport_dynamic_type")]
    RsupportDynamicType(pos_or_decl::PosOrDecl),
    #[rust_to_ocaml(name = "Rdynamic_partial_enforcement")]
    RdynamicPartialEnforcement(pos_or_decl::PosOrDecl, String, Box<T_>),
    #[rust_to_ocaml(name = "Rrigid_tvar_escape")]
    RrigidTvarEscape(pos::Pos, String, String, Box<T_>),
    #[rust_to_ocaml(name = "Ropaque_type_from_module")]
    RopaqueTypeFromModule(pos_or_decl::PosOrDecl, String, Box<T_>),
    #[rust_to_ocaml(name = "Rmissing_class")]
    RmissingClass(pos::Pos),
    Rinvalid,
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
}
