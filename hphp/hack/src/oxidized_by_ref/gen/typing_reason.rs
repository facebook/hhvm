// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<e3cbab9e93ac233bd663329042b5f23b>>
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

pub type PosId<'a> = (&'a pos_or_decl::PosOrDecl<'a>, &'a ast_defs::Id_<'a>);

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
pub enum ArgPosition {
    Aonly,
    Afirst,
    Asecond,
}
impl TrivialDrop for ArgPosition {}
arena_deserializer::impl_deserialize_in_arena!(ArgPosition);

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
pub enum Blame<'a> {
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Blame(&'a (&'a pos::Pos<'a>, BlameSource)),
}
impl<'a> TrivialDrop for Blame<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Blame<'arena>);

/// The reason why something is expected to have a certain type
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
pub enum T_<'a> {
    Rnone,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rwitness(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RwitnessFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    /// Used as an index into a vector-like
    /// array or string. Position of indexing,
    /// reason for the indexed type
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ridx(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RidxVector(&'a pos::Pos<'a>),
    /// Used as an index, in the Vector case
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
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
    RarithRet(&'a pos::Pos<'a>),
    /// pos, arg float typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RarithRetFloat(&'a (&'a pos::Pos<'a>, T_<'a>, ArgPosition)),
    /// pos, arg num typing reason, arg position
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RarithRetNum(&'a (&'a pos::Pos<'a>, T_<'a>, ArgPosition)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RarithRetInt(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RarithDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RbitwiseDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RincdecDynamic(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rcomp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RconcatRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RlogicRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rbitwise(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RbitwiseRet(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RnoReturn(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RnoReturnAsync(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RretFunKind(&'a (&'a pos::Pos<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RretFunKindFromDecl(&'a (&'a pos_or_decl::PosOrDecl<'a>, oxidized::ast_defs::FunKind)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rhint(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rthrow(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rplaceholder(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RretDiv(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RyieldGen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RyieldAsyncgen(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RyieldAsyncnull(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RyieldSend(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RlostInfo(&'a (&'a str, T_<'a>, Blame<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rformat(&'a (&'a pos::Pos<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RclassClass(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RunknownClass(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RvarParam(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RvarParamFromDecl(&'a pos_or_decl::PosOrDecl<'a>),
    /// splat pos, fun def pos, number of args before splat
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RunpackParam(&'a (&'a pos::Pos<'a>, &'a pos_or_decl::PosOrDecl<'a>, isize)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RinoutParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rinstantiate(&'a (T_<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RarrayFilter(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rtypeconst(
        &'a (
            T_<'a>,
            (&'a pos_or_decl::PosOrDecl<'a>, &'a str),
            &'a str,
            T_<'a>,
        ),
    ),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RtypeAccess(&'a (T_<'a>, &'a [(&'a T_<'a>, &'a str)])),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RexprDepType(
        &'a (
            T_<'a>,
            &'a pos_or_decl::PosOrDecl<'a>,
            ExprDepTypeReason<'a>,
        ),
    ),
    /// ?-> operator is used
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RnullsafeOp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RtconstNoCstr(&'a PosId<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rpredicated(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ris(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Ras(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RvarrayOrDarrayKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RvecOrDictKey(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rusing(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdynamicProp(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdynamicCall(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdynamicConstruct(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RidxDict(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RsetElement(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RmissingOptionalField(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RunsetField(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RcontravariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RinvariantGeneric(&'a (T_<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rregex(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RimplicitUpperBound(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RtypeVariable(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RtypeVariableGenerics(&'a (&'a pos::Pos<'a>, &'a str, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RglobalTypeVariableGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RsolveFail(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RcstrOnGenerics(&'a (&'a pos_or_decl::PosOrDecl<'a>, PosId<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RlambdaParam(&'a (&'a pos::Pos<'a>, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rshape(&'a (&'a pos::Pos<'a>, &'a str)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Renforceable(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rdestructure(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RkeyValueCollectionKey(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RglobalClassProp(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RglobalFunParam(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RglobalFunRet(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    Rsplice(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RetBoolean(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdefaultCapability(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RconcatOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RinterpOperand(&'a pos::Pos<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdynamicCoercion(&'a T_<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RsupportDynamicType(&'a pos_or_decl::PosOrDecl<'a>),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RdynamicPartialEnforcement(&'a (&'a pos_or_decl::PosOrDecl<'a>, &'a str, T_<'a>)),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    RrigidTvarEscape(&'a (&'a pos::Pos<'a>, &'a str, &'a str, T_<'a>)),
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
pub enum Ureason<'a> {
    URnone,
    URassign,
    URassignInout,
    URhint,
    URreturn,
    URforeach,
    URthrow,
    URvector,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URkey(&'a str),
    URvalue,
    URawait,
    URyield,
    /// Name of XHP class, Name of XHP attribute
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URxhp(&'a (&'a str, &'a str)),
    URxhpSpread,
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URindex(&'a str),
    #[serde(deserialize_with = "arena_deserializer::arena", borrow)]
    URelement(&'a str),
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
impl<'a> TrivialDrop for Ureason<'a> {}
arena_deserializer::impl_deserialize_in_arena!(Ureason<'arena>);
