// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::ast_defs::Id;
use crate::pos::Pos;
use crate::typing_reason::*;

const RNONE: &Reason<'_> = &Reason::Rnone;

impl<'a> Reason<'a> {
    pub const fn none() -> &'static Reason<'static> {
        RNONE
    }

    pub fn hint(pos: &'a Pos<'a>) -> Self {
        Reason::Rhint(pos)
    }

    pub fn witness(pos: &'a Pos<'a>) -> Self {
        Reason::Rwitness(pos)
    }

    pub fn instantiate(r1: &'a Reason<'a>, x: &'a str, r2: &'a Reason<'a>) -> Self {
        Reason::Rinstantiate(r1, x, r2)
    }

    pub fn pos(&self) -> Option<&'a Pos<'a>> {
        use Reason::*;
        match self {
            Rnone => None,
            Rwitness(p)
            | Ridx(p, _)
            | RidxVector(p)
            | Rforeach(p)
            | Rasyncforeach(p)
            | Rarith(p)
            | RarithRet(p)
            | RarithDynamic(p)
            | Rcomp(p)
            | RconcatRet(p)
            | RlogicRet(p)
            | Rbitwise(p)
            | RbitwiseRet(p)
            | RnoReturn(p)
            | RnoReturnAsync(p)
            | RretFunKind(p, _)
            | Rhint(p)
            | Rthrow(p)
            | Rplaceholder(p)
            | RretDiv(p)
            | RyieldGen(p)
            | RyieldAsyncgen(p)
            | RyieldAsyncnull(p)
            | RyieldSend(p)
            | Rformat(p, _, _)
            | RclassClass(p, _)
            | RunknownClass(p)
            | RvarParam(p)
            | RunpackParam(p, _, _)
            | RinoutParam(p)
            | Rtypeconst(Rnone, (p, _), _, _)
            | RarrayFilter(p, _)
            | RnullsafeOp(p)
            | RtconstNoCstr(Id(p, _))
            | Rpredicated(p, _)
            | Ris(p)
            | Ras(p)
            | RvarrayOrDarrayKey(p)
            | Rusing(p)
            | RdynamicProp(p)
            | RdynamicCall(p)
            | RidxDict(p)
            | RmissingRequiredField(p, _)
            | RmissingOptionalField(p, _)
            | RunsetField(p, _)
            | Rregex(p)
            | RimplicitUpperBound(p, _)
            | RarithRetFloat(p, _, _)
            | RarithRetNum(p, _, _)
            | RarithRetInt(p)
            | RbitwiseDynamic(p)
            | RincdecDynamic(p)
            | RtypeVariable(p)
            | RtypeVariableGenerics(p, _, _)
            | RsolveFail(p)
            | RcstrOnGenerics(p, _)
            | RlambdaParam(p, _)
            | Rshape(p, _)
            | Renforceable(p)
            | Rdestructure(p)
            | RkeyValueCollectionKey(p)
            | RglobalClassProp(p)
            | RglobalFunParam(p)
            | RglobalFunRet(p) => Some(p),
            RlostInfo(_, r, _)
            | Rinstantiate(_, _, r)
            | Rtypeconst(r, _, _, _)
            | RtypeAccess(r, _)
            | RexprDepType(r, _, _)
            | RcontravariantGeneric(r, _)
            | RinvariantGeneric(r, _) => r.pos(),
        }
    }
}
