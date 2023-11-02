// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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

    pub fn witness_from_decl(pos: &'a Pos<'a>) -> Self {
        Reason::RwitnessFromDecl(pos)
    }

    pub fn instantiate(args: &'a (Reason<'a>, &'a str, Reason<'a>)) -> Self {
        Reason::Rinstantiate(args)
    }

    pub fn pos(&self) -> Option<&'a Pos<'a>> {
        use T_::*;
        match self {
            Rnone => None,
            Rinvalid => None,
            Rwitness(p)
            | RwitnessFromDecl(p)
            | Ridx((p, _))
            | RidxVector(p)
            | RidxVectorFromDecl(p)
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
            | RretFunKind((p, _))
            | RretFunKindFromDecl((p, _))
            | Rhint(p)
            | Rthrow(p)
            | Rplaceholder(p)
            | RretDiv(p)
            | RyieldGen(p)
            | RyieldAsyncgen(p)
            | RyieldAsyncnull(p)
            | RyieldSend(p)
            | Rformat((p, _, _))
            | RclassClass((p, _))
            | RunknownClass(p)
            | RvarParam(p)
            | RvarParamFromDecl(p)
            | RunpackParam((p, _, _))
            | RinoutParam(p)
            | Rtypeconst((Rnone, (p, _), _, _))
            | RnullsafeOp(p)
            | RtconstNoCstr((p, _))
            | Rpredicated((p, _))
            | Ris(p)
            | Ras(p)
            | Requal(p)
            | RvarrayOrDarrayKey(p)
            | RvecOrDictKey(p)
            | Rusing(p)
            | RdynamicProp(p)
            | RdynamicCall(p)
            | RdynamicConstruct(p)
            | RidxDict(p)
            | RsetElement(p)
            | RmissingOptionalField((p, _))
            | RunsetField((p, _))
            | Rregex(p)
            | RimplicitUpperBound((p, _))
            | RarithRetFloat((p, _, _))
            | RarithRetNum((p, _, _))
            | RarithRetInt(p)
            | RbitwiseDynamic(p)
            | RincdecDynamic(p)
            | RtypeVariable(p)
            | RtypeVariableGenerics((p, _, _))
            | RtypeVariableError(p)
            | RglobalTypeVariableGenerics((p, _, _))
            | RsolveFail(p)
            | RcstrOnGenerics((p, _))
            | RlambdaParam((p, _))
            | Rshape((p, _))
            | RshapeLiteral(p)
            | Renforceable(p)
            | Rdestructure(p)
            | RkeyValueCollectionKey(p)
            | RglobalClassProp(p)
            | RglobalFunParam(p)
            | RglobalFunRet(p)
            | Rsplice(p)
            | RetBoolean(p)
            | RdefaultCapability(p)
            | RconcatOperand(p)
            | RinterpOperand(p)
            | RsupportDynamicType(p)
            | RdynamicPartialEnforcement((p, _, _))
            | RrigidTvarEscape((p, _, _, _))
            | RmissingClass(p)
            | RcapturedLike(p)
            | RpessimisedInout(p)
            | RpessimisedReturn(p)
            | RpessimisedProp(p) => Some(p),
            RlostInfo((_, r, _))
            | Rinstantiate((_, _, r))
            | Rtypeconst((r, _, _, _))
            | RtypeAccess((r, _))
            | RexprDepType((r, _, _))
            | RcontravariantGeneric((r, _))
            | RinvariantGeneric((r, _)) => r.pos(),
            RopaqueTypeFromModule((p, _, _)) => Some(p),
            RdynamicCoercion(r) => r.pos(),
        }
    }
}

impl<'a> std::fmt::Debug for T_<'a> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        use T_::*;
        match self {
            Rnone => f.debug_tuple("Rnone").finish(),
            Rwitness(p) => f.debug_tuple("Rwitness").field(p).finish(),
            RwitnessFromDecl(p) => f.debug_tuple("RwitnessFromDecl").field(p).finish(),
            Ridx((p, t)) => f.debug_tuple("Ridx").field(p).field(t).finish(),
            RidxVector(p) => f.debug_tuple("RidxVector").field(p).finish(),
            RidxVectorFromDecl(p) => f.debug_tuple("RidxVectorFromDecl").field(p).finish(),
            Rforeach(p) => f.debug_tuple("Rforeach").field(p).finish(),
            Rasyncforeach(p) => f.debug_tuple("Rasyncforeach").field(p).finish(),
            Rarith(p) => f.debug_tuple("Rarith").field(p).finish(),
            RarithRet(p) => f.debug_tuple("RarithRet").field(p).finish(),
            RarithRetFloat((p, t, ap)) => f
                .debug_tuple("RarithRetFloat")
                .field(p)
                .field(t)
                .field(ap)
                .finish(),
            RarithRetNum((p, t, ap)) => f
                .debug_tuple("RarithRetNum")
                .field(p)
                .field(t)
                .field(ap)
                .finish(),
            RarithRetInt(p) => f.debug_tuple("RarithRetInt").field(p).finish(),
            RarithDynamic(p) => f.debug_tuple("RarithDynamic").field(p).finish(),
            RbitwiseDynamic(p) => f.debug_tuple("RbitwiseDynamic").field(p).finish(),
            RincdecDynamic(p) => f.debug_tuple("RincdecDynamic").field(p).finish(),
            Rcomp(p) => f.debug_tuple("Rcomp").field(p).finish(),
            RconcatRet(p) => f.debug_tuple("RconcatRet").field(p).finish(),
            RlogicRet(p) => f.debug_tuple("RlogicRet").field(p).finish(),
            Rbitwise(p) => f.debug_tuple("Rbitwise").field(p).finish(),
            RbitwiseRet(p) => f.debug_tuple("RbitwiseRet").field(p).finish(),
            RnoReturn(p) => f.debug_tuple("RnoReturn").field(p).finish(),
            RnoReturnAsync(p) => f.debug_tuple("RnoReturnAsync").field(p).finish(),
            RretFunKind((p, fk)) => f.debug_tuple("RretFunKind").field(p).field(fk).finish(),
            RretFunKindFromDecl((p, fk)) => f
                .debug_tuple("RretFunKindFromDecl")
                .field(p)
                .field(fk)
                .finish(),
            Rhint(p) => f.debug_tuple("Rhint").field(p).finish(),
            Rthrow(p) => f.debug_tuple("Rthrow").field(p).finish(),
            Rplaceholder(p) => f.debug_tuple("Rplaceholder").field(p).finish(),
            RretDiv(p) => f.debug_tuple("RretDiv").field(p).finish(),
            RyieldGen(p) => f.debug_tuple("RyieldGen").field(p).finish(),
            RyieldAsyncgen(p) => f.debug_tuple("RyieldAsyncgen").field(p).finish(),
            RyieldAsyncnull(p) => f.debug_tuple("RieldAsyncnull").field(p).finish(),
            RyieldSend(p) => f.debug_tuple("RyieldSend").field(p).finish(),
            RlostInfo(p) => f.debug_tuple("RlostInfo").field(p).finish(),
            Rformat(p) => f.debug_tuple("Rformat").field(p).finish(),
            RclassClass(p) => f.debug_tuple("RclassClass").field(p).finish(),
            RunknownClass(p) => f.debug_tuple("RunknownClass").field(p).finish(),
            RvarParam(p) => f.debug_tuple("RvarParam").field(p).finish(),
            RvarParamFromDecl(p) => f.debug_tuple("RvarParamFromDecl").field(p).finish(),
            RunpackParam(p) => f.debug_tuple("RunpackParam").field(p).finish(),
            RinoutParam(p) => f.debug_tuple("RinoutParam").field(p).finish(),
            Rinstantiate(p) => f.debug_tuple("Rinstantiate").field(p).finish(),
            Rtypeconst(p) => f.debug_tuple("Rtypeconst").field(p).finish(),
            RtypeAccess(p) => f.debug_tuple("RtypeAccess").field(p).finish(),
            RexprDepType(p) => f.debug_tuple("RexprDepType").field(p).finish(),
            RnullsafeOp(p) => f.debug_tuple("RnullsafeOp").field(p).finish(),
            RtconstNoCstr(p) => f.debug_tuple("RtconstNoCstr").field(p).finish(),
            Rpredicated(p) => f.debug_tuple("Rpredicated").field(p).finish(),
            Ris(p) => f.debug_tuple("Ris").field(p).finish(),
            Ras(p) => f.debug_tuple("Ras").field(p).finish(),
            Requal(p) => f.debug_tuple("Requal").field(p).finish(),
            RvarrayOrDarrayKey(p) => f.debug_tuple("RvarrayOrDarrayKey").field(p).finish(),
            RvecOrDictKey(p) => f.debug_tuple("RvecOrDictKey").field(p).finish(),
            Rusing(p) => f.debug_tuple("Rusing").field(p).finish(),
            RdynamicProp(p) => f.debug_tuple("RdynamicProp").field(p).finish(),
            RdynamicCall(p) => f.debug_tuple("RdynamicCall").field(p).finish(),
            RdynamicConstruct(p) => f.debug_tuple("RdynamicConstruct").field(p).finish(),
            RidxDict(p) => f.debug_tuple("RidxDict").field(p).finish(),
            RsetElement(p) => f.debug_tuple("RsetElement").field(p).finish(),
            RmissingOptionalField(p) => f.debug_tuple("RmissingOptionalField").field(p).finish(),
            RunsetField(p) => f.debug_tuple("RunsetField").field(p).finish(),
            RcontravariantGeneric(p) => f.debug_tuple("RcontravariantGeneric").field(p).finish(),
            RinvariantGeneric(p) => f.debug_tuple("RinvariantGeneric").field(p).finish(),
            Rregex(p) => f.debug_tuple("Rregex").field(p).finish(),
            RimplicitUpperBound(p) => f.debug_tuple("RimplicitUpperBound").field(p).finish(),
            RtypeVariable(p) => f.debug_tuple("RtypeVariable").field(p).finish(),
            RtypeVariableGenerics(p) => f.debug_tuple("RtypeVariableGenerics").field(p).finish(),
            RtypeVariableError(p) => f.debug_tuple("RtypeVariableError").field(p).finish(),
            RglobalTypeVariableGenerics(p) => f
                .debug_tuple("RglobalTypeVariableGenerics")
                .field(p)
                .finish(),
            RsolveFail(p) => f.debug_tuple("RsolveFail").field(p).finish(),
            RcstrOnGenerics(p) => f.debug_tuple("RcstrOnGenerics").field(p).finish(),
            RlambdaParam(p) => f.debug_tuple("RlambdaParam").field(p).finish(),
            Rshape(p) => f.debug_tuple("Rshape").field(p).finish(),
            RshapeLiteral(p) => f.debug_tuple("RshapeLiteral").field(p).finish(),
            Renforceable(p) => f.debug_tuple("Renforceable").field(p).finish(),
            Rdestructure(p) => f.debug_tuple("Rdestructure").field(p).finish(),
            RkeyValueCollectionKey(p) => f.debug_tuple("RkeyValueCollectionKey").field(p).finish(),
            RglobalClassProp(p) => f.debug_tuple("RglobalClassProp").field(p).finish(),
            RglobalFunParam(p) => f.debug_tuple("RglobalFunParam").field(p).finish(),
            RglobalFunRet(p) => f.debug_tuple("RglobalFunRet").field(p).finish(),
            Rsplice(p) => f.debug_tuple("Rsplice").field(p).finish(),
            RetBoolean(p) => f.debug_tuple("RetBoolean").field(p).finish(),
            RdefaultCapability(p) => f.debug_tuple("RdefaultCapability").field(p).finish(),
            RconcatOperand(p) => f.debug_tuple("RconcatOperand").field(p).finish(),
            RinterpOperand(p) => f.debug_tuple("RinterpOperand").field(p).finish(),
            RdynamicCoercion(p) => f.debug_tuple("RdynamicCoercion").field(p).finish(),
            RsupportDynamicType(p) => f.debug_tuple("RsupportDynamicType").field(p).finish(),
            RdynamicPartialEnforcement((p, s, t)) => f
                .debug_tuple("RdynamicPartialEnforcement")
                .field(p)
                .field(s)
                .field(t)
                .finish(),
            RrigidTvarEscape((p, s1, s2, t)) => f
                .debug_tuple("RigidTvarEscape")
                .field(p)
                .field(s1)
                .field(s2)
                .field(t)
                .finish(),
            RopaqueTypeFromModule((p, s, t)) => f
                .debug_tuple("RopaqueTypeFromModule")
                .field(p)
                .field(s)
                .field(t)
                .finish(),
            RmissingClass(p) => f.debug_tuple("RmissingClass").field(p).finish(),
            RcapturedLike(p) => f.debug_tuple("RcapturedLike").field(p).finish(),
            RpessimisedInout(p) => f.debug_tuple("RpessimisedInout").field(p).finish(),
            RpessimisedReturn(p) => f.debug_tuple("RpessimisedReturn").field(p).finish(),
            RpessimisedProp(p) => f.debug_tuple("RpessimisedProp").field(p).finish(),
            Rinvalid => f.debug_tuple("Rinvalid").finish(),
        }
    }
}
