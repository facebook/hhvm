// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<683b38e409958f970711c0b52aac6024>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

use ocamlrep_derive::OcamlRep;
use serde::Deserialize;
use serde::Serialize;

use crate::aast;
use crate::ast_defs;
use crate::pos;

/// The reason why something is expected to have a certain type
#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Reason {
    Rnone,
    Rwitness(pos::Pos),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(pos::Pos, Box<Reason>),
    /// Used as an index, in the Vector case
    RidxVector(pos::Pos),
    /// Used to append element to an array
    Rappend(pos::Pos),
    /// Array accessed with a static string index
    Rfield(pos::Pos),
    /// Because it is iterated in a foreach loop
    Rforeach(pos::Pos),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(pos::Pos),
    Raccess(pos::Pos),
    Rarith(pos::Pos),
    RarithInt(pos::Pos),
    RarithRet(pos::Pos),
    /// pos, arg float typing reason, arg position
    RarithRetFloat(pos::Pos, Box<Reason>, ArgPosition),
    /// pos, arg num typing reason, arg position
    RarithRetNum(pos::Pos, Box<Reason>, ArgPosition),
    RarithRetInt(pos::Pos),
    RarithDynamic(pos::Pos),
    RbitwiseDynamic(pos::Pos),
    RincdecDynamic(pos::Pos),
    Rstring2(pos::Pos),
    Rcomp(pos::Pos),
    Rconcat(pos::Pos),
    RconcatRet(pos::Pos),
    Rlogic(pos::Pos),
    RlogicRet(pos::Pos),
    Rbitwise(pos::Pos),
    RbitwiseRet(pos::Pos),
    Rstmt(pos::Pos),
    RnoReturn(pos::Pos),
    RnoReturnAsync(pos::Pos),
    RretFunKind(pos::Pos, ast_defs::FunKind),
    Rhint(pos::Pos),
    RnullCheck(pos::Pos),
    RnotInCstr(pos::Pos),
    Rthrow(pos::Pos),
    Rplaceholder(pos::Pos),
    Rattr(pos::Pos),
    Rxhp(pos::Pos),
    RretDiv(pos::Pos),
    RyieldGen(pos::Pos),
    RyieldAsyncgen(pos::Pos),
    RyieldAsyncnull(pos::Pos),
    RyieldSend(pos::Pos),
    /// true if due to lambda
    RlostInfo(String, Box<Reason>, pos::Pos, bool),
    Rcoerced(Box<Reason>, pos::Pos, String),
    Rformat(pos::Pos, String, Box<Reason>),
    RclassClass(pos::Pos, String),
    RunknownClass(pos::Pos),
    RdynamicYield(pos::Pos, pos::Pos, String, String),
    RmapAppend(pos::Pos),
    RvarParam(pos::Pos),
    /// splat pos, fun def pos, number of args before splat
    RunpackParam(pos::Pos, pos::Pos, isize),
    RinoutParam(pos::Pos),
    Rinstantiate(Box<Reason>, String, Box<Reason>),
    RarrayFilter(pos::Pos, Box<Reason>),
    Rtypeconst(Box<Reason>, (pos::Pos, String), String, Box<Reason>),
    RtypeAccess(Box<Reason>, Vec<(Box<Reason>, String)>),
    RexprDepType(Box<Reason>, pos::Pos, ExprDepTypeReason),
    /// ?-> operator is used
    RnullsafeOp(pos::Pos),
    RtconstNoCstr(aast::Sid),
    RusedAsMap(pos::Pos),
    RusedAsShape(pos::Pos),
    Rpredicated(pos::Pos, String),
    Ris(pos::Pos),
    Ras(pos::Pos),
    RfinalProperty(pos::Pos),
    RvarrayOrDarrayKey(pos::Pos),
    Rusing(pos::Pos),
    RdynamicProp(pos::Pos),
    RdynamicCall(pos::Pos),
    RidxDict(pos::Pos),
    RmissingRequiredField(pos::Pos, String),
    RmissingOptionalField(pos::Pos, String),
    RunsetField(pos::Pos, String),
    RcontravariantGeneric(Box<Reason>, String),
    RinvariantGeneric(Box<Reason>, String),
    Rregex(pos::Pos),
    RlambdaUse(pos::Pos),
    RimplicitUpperBound(pos::Pos, String),
    RtypeVariable(pos::Pos),
    RtypeVariableGenerics(pos::Pos, String, String),
    RsolveFail(pos::Pos),
    RcstrOnGenerics(pos::Pos, aast::Sid),
    RlambdaParam(pos::Pos, Box<Reason>),
    Rshape(pos::Pos, String),
    Renforceable(pos::Pos),
    Rdestructure(pos::Pos),
    RkeyValueCollectionKey(pos::Pos),
}

#[derive(Clone, Copy, Debug, Deserialize, Eq, OcamlRep, PartialEq, Serialize)]
pub enum ArgPosition {
    Aonly,
    Afirst,
    Asecond,
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum ExprDepTypeReason {
    ERexpr(isize),
    ERstatic,
    ERclass(String),
    ERparent(String),
    ERself(String),
}

#[derive(Clone, Debug, Deserialize, OcamlRep, Serialize)]
pub enum Ureason {
    URnone,
    URassign,
    URassignBranch,
    URassignInout,
    URhint,
    URreturn,
    URforeach,
    URthrow,
    URvector,
    URkey,
    URvalue,
    URif,
    URawait,
    URyield,
    URyieldFrom,
    /// Name of XHP class, Name of XHP attribute
    URxhp(String, String),
    URxhpSpread,
    URindex(String),
    URparam,
    URparamInout,
    URarrayValue,
    URarrayKey,
    URtupleAccess,
    URpairAccess,
    URdynamicYield,
    URnewtypeCstr,
    URclassReq,
    URenum,
    URenumCstr,
    URenumUnderlying,
    URenumIncompatibleCstr,
    URtypeconstCstr,
    URsubsumeTconstCstr,
    URsubsumeTconstAssign,
    URfinalProperty,
    URclone,
    URusing,
}
