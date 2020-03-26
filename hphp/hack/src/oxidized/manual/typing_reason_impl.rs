use crate::gen::ast_defs::Id;
use crate::gen::typing_reason::Reason;
use crate::gen::typing_reason::Reason::*;
use crate::pos::Pos;

// Helper functions on Reason
impl Reason {
    // Position is owned by reason
    pub fn get_pos(&self) -> &Pos {
        match self {
            Rnone => {
                // TODO: not sure what to do about this
                // Maybe return an Option<_> instead
                let _p = Pos::make_none();
                panic!("NYI");
            }
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
            | RdynamicYield(p, _, _, _)
            | RmapAppend(p)
            | RvarParam(p)
            | RunpackParam(p, _, _)
            | RinoutParam(p)
            | RnullsafeOp(p)
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
            | RarrayFilter(p, _)
            | RunsetField(p, _)
            | Rregex(p)
            | RlambdaUse(p)
            | RimplicitUpperBound(p, _)
            | RarithInt(p)
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
            | RglobalFunRet(p) => p,
            RlostInfo(_, r, _, _)
            | Rinstantiate(_, _, r)
            | RtypeAccess(r, _)
            | RexprDepType(r, _, _)
            | RcontravariantGeneric(r, _)
            | RinvariantGeneric(r, _) => (*r).get_pos(),
            Rtypeconst(r, (p, _), _, _) => match r.as_ref() {
                Rnone => p,
                r => r.get_pos(),
            },
            // Need to get pos out of sid
            RtconstNoCstr(Id(p, _)) => p,
        }
    }
}
