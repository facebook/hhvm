// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<7a9b7bad36d15de37725b54c00f39e6a>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

use arena_trait::TrivialDrop;
use no_pos_hash::NoPosHash;
use ocamlrep_derive::FromOcamlRep;
use ocamlrep_derive::FromOcamlRepIn;
use ocamlrep_derive::ToOcamlRep;
use serde::Serialize;

#[allow(unused_imports)]
use crate::*;

/// The reason why something is expected to have a certain type
#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Reason<'a> {
    Rnone,
    Rwitness(&'a pos::Pos<'a>),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    Ridx(&'a (&'a pos::Pos<'a>, Reason<'a>)),
    /// Used as an index, in the Vector case
    RidxVector(&'a pos::Pos<'a>),
    /// Because it is iterated in a foreach loop
    Rforeach(&'a pos::Pos<'a>),
    /// Because it is iterated "await as" in foreach
    Rasyncforeach(&'a pos::Pos<'a>),
    Rarith(&'a pos::Pos<'a>),
    RarithRet(&'a pos::Pos<'a>),
    /// pos, arg float typing reason, arg position
    RarithRetFloat(&'a (&'a pos::Pos<'a>, Reason<'a>, ArgPosition)),
    /// pos, arg num typing reason, arg position
    RarithRetNum(&'a (&'a pos::Pos<'a>, Reason<'a>, ArgPosition)),
    RarithRetInt(&'a pos::Pos<'a>),
    RarithDynamic(&'a pos::Pos<'a>),
    RbitwiseDynamic(&'a pos::Pos<'a>),
    RincdecDynamic(&'a pos::Pos<'a>),
    Rcomp(&'a pos::Pos<'a>),
    RconcatRet(&'a pos::Pos<'a>),
    RlogicRet(&'a pos::Pos<'a>),
    Rbitwise(&'a pos::Pos<'a>),
    RbitwiseRet(&'a pos::Pos<'a>),
    RnoReturn(&'a pos::Pos<'a>),
    RnoReturnAsync(&'a pos::Pos<'a>),
    RretFunKind(&'a (&'a pos::Pos<'a>, oxidized::ast_defs::FunKind)),
    Rhint(&'a pos::Pos<'a>),
    Rthrow(&'a pos::Pos<'a>),
    Rplaceholder(&'a pos::Pos<'a>),
    RretDiv(&'a pos::Pos<'a>),
    RyieldGen(&'a pos::Pos<'a>),
    RyieldAsyncgen(&'a pos::Pos<'a>),
    RyieldAsyncnull(&'a pos::Pos<'a>),
    RyieldSend(&'a pos::Pos<'a>),
    RlostInfo(&'a (&'a str, Reason<'a>, Blame<'a>)),
    Rformat(&'a (&'a pos::Pos<'a>, &'a str, Reason<'a>)),
    RclassClass(&'a (&'a pos::Pos<'a>, &'a str)),
    RunknownClass(&'a pos::Pos<'a>),
    RvarParam(&'a pos::Pos<'a>),
    /// splat pos, fun def pos, number of args before splat
    RunpackParam(&'a (&'a pos::Pos<'a>, &'a pos::Pos<'a>, isize)),
    RinoutParam(&'a pos::Pos<'a>),
    Rinstantiate(&'a (Reason<'a>, &'a str, Reason<'a>)),
    RarrayFilter(&'a (&'a pos::Pos<'a>, Reason<'a>)),
    Rtypeconst(&'a (Reason<'a>, (&'a pos::Pos<'a>, &'a str), &'a str, Reason<'a>)),
    RtypeAccess(&'a (Reason<'a>, &'a [(&'a Reason<'a>, &'a str)])),
    RexprDepType(&'a (Reason<'a>, &'a pos::Pos<'a>, ExprDepTypeReason<'a>)),
    /// ?-> operator is used
    RnullsafeOp(&'a pos::Pos<'a>),
    RtconstNoCstr(&'a aast::Sid<'a>),
    Rpredicated(&'a (&'a pos::Pos<'a>, &'a str)),
    Ris(&'a pos::Pos<'a>),
    Ras(&'a pos::Pos<'a>),
    RvarrayOrDarrayKey(&'a pos::Pos<'a>),
    Rusing(&'a pos::Pos<'a>),
    RdynamicProp(&'a pos::Pos<'a>),
    RdynamicCall(&'a pos::Pos<'a>),
    RidxDict(&'a pos::Pos<'a>),
    RmissingRequiredField(&'a (&'a pos::Pos<'a>, &'a str)),
    RmissingOptionalField(&'a (&'a pos::Pos<'a>, &'a str)),
    RunsetField(&'a (&'a pos::Pos<'a>, &'a str)),
    RcontravariantGeneric(&'a (Reason<'a>, &'a str)),
    RinvariantGeneric(&'a (Reason<'a>, &'a str)),
    Rregex(&'a pos::Pos<'a>),
    RimplicitUpperBound(&'a (&'a pos::Pos<'a>, &'a str)),
    RtypeVariable(&'a pos::Pos<'a>),
    RtypeVariableGenerics(&'a (&'a pos::Pos<'a>, &'a str, &'a str)),
    RsolveFail(&'a pos::Pos<'a>),
    RcstrOnGenerics(&'a (&'a pos::Pos<'a>, aast::Sid<'a>)),
    RlambdaParam(&'a (&'a pos::Pos<'a>, Reason<'a>)),
    Rshape(&'a (&'a pos::Pos<'a>, &'a str)),
    Renforceable(&'a pos::Pos<'a>),
    Rdestructure(&'a pos::Pos<'a>),
    RkeyValueCollectionKey(&'a pos::Pos<'a>),
    RglobalClassProp(&'a pos::Pos<'a>),
    RglobalFunParam(&'a pos::Pos<'a>),
    RglobalFunRet(&'a pos::Pos<'a>),
    Rsplice(&'a pos::Pos<'a>),
}
impl<'a> TrivialDrop for Reason<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
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
pub enum ArgPosition {
    Aonly,
    Afirst,
    Asecond,
}
impl TrivialDrop for ArgPosition {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum ExprDepTypeReason<'a> {
    ERexpr(isize),
    ERstatic,
    ERclass(&'a str),
    ERparent(&'a str),
    ERself(&'a str),
    ERpu(&'a str),
}
impl<'a> TrivialDrop for ExprDepTypeReason<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
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
pub enum BlameSource {
    BScall,
    BSlambda,
    BSassignment,
    BSoutOfScope,
}
impl TrivialDrop for BlameSource {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
pub enum Blame<'a> {
    Blame(&'a (&'a pos::Pos<'a>, BlameSource)),
}
impl<'a> TrivialDrop for Blame<'a> {}

#[derive(
    Clone,
    Copy,
    Debug,
    Eq,
    FromOcamlRepIn,
    Hash,
    NoPosHash,
    Ord,
    PartialEq,
    PartialOrd,
    Serialize,
    ToOcamlRep
)]
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
    /// Name of XHP class, Name of XHP attribute
    URxhp(&'a (&'a str, &'a str)),
    URxhpSpread,
    URindex(&'a str),
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
}
impl<'a> TrivialDrop for Ureason<'a> {}
