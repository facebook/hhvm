// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<866c802f4c935c919f32e89987f6138b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// The reason why something is expected to have a certain type
#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum Reason<'a> {
    Rnone,
    Rwitness(pos::Pos<'a>),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(pos::Pos<'a>, &'a Reason<'a>),
    /// Used as an index, in the Vector case
    RidxVector(pos::Pos<'a>),
    /// Because it is iterated in a foreach loop
    Rforeach(pos::Pos<'a>),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(pos::Pos<'a>),
    Rarith(pos::Pos<'a>),
    RarithInt(pos::Pos<'a>),
    RarithRet(pos::Pos<'a>),
    /// pos, arg float typing reason, arg position
    RarithRetFloat(
        pos::Pos<'a>,
        &'a Reason<'a>,
        oxidized::typing_reason::ArgPosition,
    ),
    /// pos, arg num typing reason, arg position
    RarithRetNum(
        pos::Pos<'a>,
        &'a Reason<'a>,
        oxidized::typing_reason::ArgPosition,
    ),
    RarithRetInt(pos::Pos<'a>),
    RarithDynamic(pos::Pos<'a>),
    RbitwiseDynamic(pos::Pos<'a>),
    RincdecDynamic(pos::Pos<'a>),
    Rcomp(pos::Pos<'a>),
    RconcatRet(pos::Pos<'a>),
    RlogicRet(pos::Pos<'a>),
    Rbitwise(pos::Pos<'a>),
    RbitwiseRet(pos::Pos<'a>),
    RnoReturn(pos::Pos<'a>),
    RnoReturnAsync(pos::Pos<'a>),
    RretFunKind(pos::Pos<'a>, oxidized::ast_defs::FunKind),
    Rhint(pos::Pos<'a>),
    Rthrow(pos::Pos<'a>),
    Rplaceholder(pos::Pos<'a>),
    RretDiv(pos::Pos<'a>),
    RyieldGen(pos::Pos<'a>),
    RyieldAsyncgen(pos::Pos<'a>),
    RyieldAsyncnull(pos::Pos<'a>),
    RyieldSend(pos::Pos<'a>),
    /// true if due to lambda
    RlostInfo(&'a str, &'a Reason<'a>, pos::Pos<'a>, bool),
    Rformat(pos::Pos<'a>, &'a str, &'a Reason<'a>),
    RclassClass(pos::Pos<'a>, &'a str),
    RunknownClass(pos::Pos<'a>),
    RdynamicYield(pos::Pos<'a>, pos::Pos<'a>, &'a str, &'a str),
    RmapAppend(pos::Pos<'a>),
    RvarParam(pos::Pos<'a>),
    /// splat pos, fun def pos, number of args before splat
    RunpackParam(pos::Pos<'a>, pos::Pos<'a>, isize),
    RinoutParam(pos::Pos<'a>),
    Rinstantiate(&'a Reason<'a>, &'a str, &'a Reason<'a>),
    RarrayFilter(pos::Pos<'a>, &'a Reason<'a>),
    Rtypeconst(
        &'a Reason<'a>,
        (pos::Pos<'a>, &'a str),
        &'a str,
        &'a Reason<'a>,
    ),
    RtypeAccess(&'a Reason<'a>, &'a [(&'a Reason<'a>, &'a str)]),
    RexprDepType(&'a Reason<'a>, pos::Pos<'a>, ExprDepTypeReason<'a>),
    /// ?-> operator is used
    RnullsafeOp(pos::Pos<'a>),
    RtconstNoCstr(aast::Sid<'a>),
    Rpredicated(pos::Pos<'a>, &'a str),
    Ris(pos::Pos<'a>),
    Ras(pos::Pos<'a>),
    RvarrayOrDarrayKey(pos::Pos<'a>),
    Rusing(pos::Pos<'a>),
    RdynamicProp(pos::Pos<'a>),
    RdynamicCall(pos::Pos<'a>),
    RidxDict(pos::Pos<'a>),
    RmissingRequiredField(pos::Pos<'a>, &'a str),
    RmissingOptionalField(pos::Pos<'a>, &'a str),
    RunsetField(pos::Pos<'a>, &'a str),
    RcontravariantGeneric(&'a Reason<'a>, &'a str),
    RinvariantGeneric(&'a Reason<'a>, &'a str),
    Rregex(pos::Pos<'a>),
    RlambdaUse(pos::Pos<'a>),
    RimplicitUpperBound(pos::Pos<'a>, &'a str),
    RtypeVariable(pos::Pos<'a>),
    RtypeVariableGenerics(pos::Pos<'a>, &'a str, &'a str),
    RsolveFail(pos::Pos<'a>),
    RcstrOnGenerics(pos::Pos<'a>, aast::Sid<'a>),
    RlambdaParam(pos::Pos<'a>, &'a Reason<'a>),
    Rshape(pos::Pos<'a>, &'a str),
    Renforceable(pos::Pos<'a>),
    Rdestructure(pos::Pos<'a>),
    RkeyValueCollectionKey(pos::Pos<'a>),
    RglobalClassProp(pos::Pos<'a>),
    RglobalFunParam(pos::Pos<'a>),
    RglobalFunRet(pos::Pos<'a>),
}

pub use oxidized::typing_reason::ArgPosition;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    ERclass(&'a str),
    ERparent(&'a str),
    ERself(&'a str),
}

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize)]
pub enum Ureason<'a> {
    URnone,
    URassign,
    URassignInout,
    URhint,
    URreturn,
    URforeach,
    URthrow,
    URvector,
    URkey,
    URvalue,
    URawait,
    URyield,
    URyieldFrom,
    /// Name of XHP class, Name of XHP attribute
    URxhp(&'a str, &'a str),
    URxhpSpread,
    URindex(&'a str),
    URparam,
    URparamInout,
    URarrayValue,
    URtupleAccess,
    URpairAccess,
    URnewtypeCstr,
    URclassReq,
    URenum,
    URenumCstr,
    URenumUnderlying,
    URenumIncompatibleCstr,
    URtypeconstCstr,
    URsubsumeTconstCstr,
    URsubsumeTconstAssign,
    URclone,
    URusing,
}
