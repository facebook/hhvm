// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5e6ade87c87cfe65c688a954291d9cc4>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

use arena_trait::TrivialDrop;
use eq_modulo_pos::EqModuloPos;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Deserialize;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

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
#[repr(u8)]
pub enum BlameSource {
    BScall,
    BSlambda,
    BSassignment,
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
#[repr(C, u8)]
pub enum Blame {
    Blame(pos::Pos, BlameSource),
}

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
#[repr(C, u8)]
pub enum T_ {
    Rnone,
    Rwitness(pos::Pos),
    RwitnessFromDecl(pos_or_decl::PosOrDecl),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(pos::Pos, Box<T_>),
    RidxVector(pos::Pos),
    /// Used as an index, in the Vector case
    RidxVectorFromDecl(pos_or_decl::PosOrDecl),
    /// Because it is iterated in a foreach loop
    Rforeach(pos::Pos),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(pos::Pos),
    Rarith(pos::Pos),
    RarithRet(pos::Pos),
    /// pos, arg float typing reason, arg position
    RarithRetFloat(pos::Pos, Box<T_>, ArgPosition),
    /// pos, arg num typing reason, arg position
    RarithRetNum(pos::Pos, Box<T_>, ArgPosition),
    RarithRetInt(pos::Pos),
    RarithDynamic(pos::Pos),
    RbitwiseDynamic(pos::Pos),
    RincdecDynamic(pos::Pos),
    Rcomp(pos::Pos),
    RconcatRet(pos::Pos),
    RlogicRet(pos::Pos),
    Rbitwise(pos::Pos),
    RbitwiseRet(pos::Pos),
    RnoReturn(pos::Pos),
    RnoReturnAsync(pos::Pos),
    RretFunKind(pos::Pos, ast_defs::FunKind),
    RretFunKindFromDecl(pos_or_decl::PosOrDecl, ast_defs::FunKind),
    Rhint(pos_or_decl::PosOrDecl),
    Rthrow(pos::Pos),
    Rplaceholder(pos::Pos),
    RretDiv(pos::Pos),
    RyieldGen(pos::Pos),
    RyieldAsyncgen(pos::Pos),
    RyieldAsyncnull(pos::Pos),
    RyieldSend(pos::Pos),
    RlostInfo(String, Box<T_>, Blame),
    Rformat(pos::Pos, String, Box<T_>),
    RclassClass(pos_or_decl::PosOrDecl, String),
    RunknownClass(pos::Pos),
    RvarParam(pos::Pos),
    RvarParamFromDecl(pos_or_decl::PosOrDecl),
    /// splat pos, fun def pos, number of args before splat
    RunpackParam(pos::Pos, pos_or_decl::PosOrDecl, isize),
    RinoutParam(pos_or_decl::PosOrDecl),
    Rinstantiate(Box<T_>, String, Box<T_>),
    Rtypeconst(
        Box<T_>,
        (pos_or_decl::PosOrDecl, String),
        lazy::Lazy<String>,
        Box<T_>,
    ),
    RtypeAccess(Box<T_>, Vec<(Box<T_>, lazy::Lazy<String>)>),
    RexprDepType(Box<T_>, pos_or_decl::PosOrDecl, ExprDepTypeReason),
    /// ?-> operator is used
    RnullsafeOp(pos::Pos),
    RtconstNoCstr(PosId),
    Rpredicated(pos::Pos, String),
    Ris(pos::Pos),
    Ras(pos::Pos),
    RvarrayOrDarrayKey(pos_or_decl::PosOrDecl),
    RvecOrDictKey(pos_or_decl::PosOrDecl),
    Rusing(pos::Pos),
    RdynamicProp(pos::Pos),
    RdynamicCall(pos::Pos),
    RdynamicConstruct(pos::Pos),
    RidxDict(pos::Pos),
    RsetElement(pos::Pos),
    RmissingOptionalField(pos_or_decl::PosOrDecl, String),
    RunsetField(pos::Pos, String),
    RcontravariantGeneric(Box<T_>, String),
    RinvariantGeneric(Box<T_>, String),
    Rregex(pos::Pos),
    RimplicitUpperBound(pos_or_decl::PosOrDecl, String),
    RtypeVariable(pos::Pos),
    RtypeVariableGenerics(pos::Pos, String, String),
    RglobalTypeVariableGenerics(pos_or_decl::PosOrDecl, String, String),
    RsolveFail(pos_or_decl::PosOrDecl),
    RcstrOnGenerics(pos_or_decl::PosOrDecl, PosId),
    RlambdaParam(pos::Pos, Box<T_>),
    Rshape(pos::Pos, String),
    Renforceable(pos_or_decl::PosOrDecl),
    Rdestructure(pos::Pos),
    RkeyValueCollectionKey(pos::Pos),
    RglobalClassProp(pos_or_decl::PosOrDecl),
    RglobalFunParam(pos_or_decl::PosOrDecl),
    RglobalFunRet(pos_or_decl::PosOrDecl),
    Rsplice(pos::Pos),
    RetBoolean(pos::Pos),
    RdefaultCapability(pos_or_decl::PosOrDecl),
    RconcatOperand(pos::Pos),
    RinterpOperand(pos::Pos),
    RdynamicCoercion(Box<T_>),
    RsupportDynamicType(pos_or_decl::PosOrDecl),
    RdynamicPartialEnforcement(pos_or_decl::PosOrDecl, String, Box<T_>),
    RrigidTvarEscape(pos::Pos, String, String, Box<T_>),
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
#[repr(C, u8)]
pub enum Ureason {
    URnone,
    URassign,
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
    URxhpSpread,
    URindex(String),
    URelement(String),
    URparam,
    URparamInout,
    URarrayValue,
    URpairValue,
    URtupleAccess,
    URpairAccess,
    URnewtypeCstr,
    URclassReq,
    URenum,
    URenumInclude,
    URenumCstr,
    URenumUnderlying,
    URenumIncompatibleCstr,
    URtypeconstCstr,
    URsubsumeTconstCstr,
    URsubsumeTconstAssign,
    URclone,
    URusing,
    URstrConcat,
    URstrInterp,
}
