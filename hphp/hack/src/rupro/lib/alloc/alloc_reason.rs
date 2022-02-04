// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use crate::reason::{Blame, ExprDepTypeReason, Reason};

use super::Allocator;

impl<R: Reason> Allocator<R> {
    pub fn reason(&self, reason: &oxidized_by_ref::typing_reason::T_<'_>) -> R {
        R::mk(|| {
            use crate::reason::ReasonImpl as RI;
            use oxidized_by_ref::typing_reason::Blame as OBlame;
            use oxidized_by_ref::typing_reason::T_ as OR;
            match reason {
                OR::Rnone => RI::Rnone,
                OR::Rwitness(pos) => RI::Rwitness(self.pos_from_decl(pos)),
                OR::RwitnessFromDecl(pos) => RI::RwitnessFromDecl(self.pos_from_decl(pos)),
                OR::Ridx((pos, r)) => RI::Ridx(self.pos_from_decl(pos), self.reason(r)),
                OR::RidxVector(pos) => RI::RidxVector(self.pos_from_decl(pos)),
                OR::RidxVectorFromDecl(pos) => RI::RidxVectorFromDecl(self.pos_from_decl(pos)),
                OR::Rforeach(pos) => RI::Rforeach(self.pos_from_decl(pos)),
                OR::Rasyncforeach(pos) => RI::Rasyncforeach(self.pos_from_decl(pos)),
                OR::Rarith(pos) => RI::Rarith(self.pos_from_decl(pos)),
                OR::RarithRet(pos) => RI::RarithRet(self.pos_from_decl(pos)),
                OR::RarithRetFloat((pos, r, arg_position)) => {
                    RI::RarithRetFloat(self.pos_from_decl(pos), self.reason(r), *arg_position)
                }
                OR::RarithRetNum((pos, r, arg_position)) => {
                    RI::RarithRetNum(self.pos_from_decl(pos), self.reason(r), *arg_position)
                }
                OR::RarithRetInt(pos) => RI::RarithRetInt(self.pos_from_decl(pos)),
                OR::RarithDynamic(pos) => RI::RarithDynamic(self.pos_from_decl(pos)),
                OR::RbitwiseDynamic(pos) => RI::RbitwiseDynamic(self.pos_from_decl(pos)),
                OR::RincdecDynamic(pos) => RI::RincdecDynamic(self.pos_from_decl(pos)),
                OR::Rcomp(pos) => RI::Rcomp(self.pos_from_decl(pos)),
                OR::RconcatRet(pos) => RI::RconcatRet(self.pos_from_decl(pos)),
                OR::RlogicRet(pos) => RI::RlogicRet(self.pos_from_decl(pos)),
                OR::Rbitwise(pos) => RI::Rbitwise(self.pos_from_decl(pos)),
                OR::RbitwiseRet(pos) => RI::RbitwiseRet(self.pos_from_decl(pos)),
                OR::RnoReturn(pos) => RI::RnoReturn(self.pos_from_decl(pos)),
                OR::RnoReturnAsync(pos) => RI::RnoReturnAsync(self.pos_from_decl(pos)),
                OR::RretFunKind(&(pos, fun_kind)) => {
                    RI::RretFunKind(self.pos_from_decl(pos), fun_kind)
                }
                OR::RretFunKindFromDecl(&(pos, fun_kind)) => {
                    RI::RretFunKindFromDecl(self.pos_from_decl(pos), fun_kind)
                }
                OR::Rhint(pos) => RI::Rhint(self.pos_from_decl(pos)),
                OR::Rthrow(pos) => RI::Rthrow(self.pos_from_decl(pos)),
                OR::Rplaceholder(pos) => RI::Rplaceholder(self.pos_from_decl(pos)),
                OR::RretDiv(pos) => RI::RretDiv(self.pos_from_decl(pos)),
                OR::RyieldGen(pos) => RI::RyieldGen(self.pos_from_decl(pos)),
                OR::RyieldAsyncgen(pos) => RI::RyieldAsyncgen(self.pos_from_decl(pos)),
                OR::RyieldAsyncnull(pos) => RI::RyieldAsyncnull(self.pos_from_decl(pos)),
                OR::RyieldSend(pos) => RI::RyieldSend(self.pos_from_decl(pos)),
                OR::RlostInfo((sym, r, OBlame::Blame(&(pos, blame_source)))) => RI::RlostInfo(
                    self.symbol(sym),
                    self.reason(r),
                    Blame(self.pos_from_decl(pos), blame_source),
                ),
                OR::Rformat((pos, sym, r)) => {
                    RI::Rformat(self.pos_from_decl(pos), self.symbol(sym), self.reason(r))
                }
                OR::RclassClass(&(pos, s)) => {
                    RI::RclassClass(self.pos_from_decl(pos), self.symbol(s))
                }
                OR::RunknownClass(pos) => RI::RunknownClass(self.pos_from_decl(pos)),
                OR::RvarParam(pos) => RI::RvarParam(self.pos_from_decl(pos)),
                OR::RvarParamFromDecl(pos) => RI::RvarParamFromDecl(self.pos_from_decl(pos)),
                OR::RunpackParam(&(pos1, pos2, i)) => {
                    RI::RunpackParam(self.pos_from_decl(pos1), self.pos_from_decl(pos2), i)
                }
                OR::RinoutParam(pos) => RI::RinoutParam(self.pos_from_decl(pos)),
                OR::Rinstantiate((r1, sym, r2)) => {
                    RI::Rinstantiate(self.reason(r1), self.symbol(sym), self.reason(r2))
                }
                OR::Rtypeconst((r1, pos_id, sym, r2)) => RI::Rtypeconst(
                    self.reason(r1),
                    self.pos_id_from_decl(*pos_id),
                    self.symbol(sym),
                    self.reason(r2),
                ),
                OR::RtypeAccess((r, list)) => RI::RtypeAccess(
                    self.reason(r),
                    list.iter()
                        .map(|(r, s)| (self.reason(r), self.symbol(s)))
                        .collect(),
                ),
                OR::RexprDepType((r, pos, edt_reason)) => RI::RexprDepType(
                    self.reason(r),
                    self.pos_from_decl(pos),
                    self.expr_dep_type_reason(*edt_reason),
                ),
                OR::RnullsafeOp(pos) => RI::RnullsafeOp(self.pos_from_decl(pos)),
                OR::RtconstNoCstr(&pos_id) => RI::RtconstNoCstr(self.pos_id_from_decl(pos_id)),
                OR::Rpredicated(&(pos, s)) => {
                    RI::Rpredicated(self.pos_from_decl(pos), self.symbol(s))
                }
                OR::Ris(pos) => RI::Ris(self.pos_from_decl(pos)),
                OR::Ras(pos) => RI::Ras(self.pos_from_decl(pos)),
                OR::RvarrayOrDarrayKey(pos) => RI::RvarrayOrDarrayKey(self.pos_from_decl(pos)),
                OR::RvecOrDictKey(pos) => RI::RvecOrDictKey(self.pos_from_decl(pos)),
                OR::Rusing(pos) => RI::Rusing(self.pos_from_decl(pos)),
                OR::RdynamicProp(pos) => RI::RdynamicProp(self.pos_from_decl(pos)),
                OR::RdynamicCall(pos) => RI::RdynamicCall(self.pos_from_decl(pos)),
                OR::RdynamicConstruct(pos) => RI::RdynamicConstruct(self.pos_from_decl(pos)),
                OR::RidxDict(pos) => RI::RidxDict(self.pos_from_decl(pos)),
                OR::RsetElement(pos) => RI::RsetElement(self.pos_from_decl(pos)),
                OR::RmissingOptionalField(&(pos, s)) => {
                    RI::RmissingOptionalField(self.pos_from_decl(pos), self.symbol(s))
                }
                OR::RunsetField(&(pos, s)) => {
                    RI::RunsetField(self.pos_from_decl(pos), self.symbol(s))
                }
                OR::RcontravariantGeneric((r, s)) => {
                    RI::RcontravariantGeneric(self.reason(r), self.symbol(s))
                }
                OR::RinvariantGeneric((r, s)) => {
                    RI::RinvariantGeneric(self.reason(r), self.symbol(s))
                }
                OR::Rregex(pos) => RI::Rregex(self.pos_from_decl(pos)),
                OR::RimplicitUpperBound(&(pos, s)) => {
                    RI::RimplicitUpperBound(self.pos_from_decl(pos), self.symbol(s))
                }
                OR::RtypeVariable(pos) => RI::RtypeVariable(self.pos_from_decl(pos)),
                OR::RtypeVariableGenerics(&(pos, s1, s2)) => RI::RtypeVariableGenerics(
                    self.pos_from_decl(pos),
                    self.symbol(s1),
                    self.symbol(s2),
                ),
                OR::RglobalTypeVariableGenerics(&(pos, s1, s2)) => RI::RglobalTypeVariableGenerics(
                    self.pos_from_decl(pos),
                    self.symbol(s1),
                    self.symbol(s2),
                ),
                OR::RsolveFail(pos) => RI::RsolveFail(self.pos_from_decl(pos)),
                OR::RcstrOnGenerics(&(pos, pos_id)) => {
                    RI::RcstrOnGenerics(self.pos_from_decl(pos), self.pos_id_from_decl(pos_id))
                }
                OR::RlambdaParam((pos, r)) => {
                    RI::RlambdaParam(self.pos_from_decl(pos), self.reason(r))
                }
                OR::Rshape(&(pos, s)) => RI::Rshape(self.pos_from_decl(pos), self.symbol(s)),
                OR::Renforceable(pos) => RI::Renforceable(self.pos_from_decl(pos)),
                OR::Rdestructure(pos) => RI::Rdestructure(self.pos_from_decl(pos)),
                OR::RkeyValueCollectionKey(pos) => {
                    RI::RkeyValueCollectionKey(self.pos_from_decl(pos))
                }
                OR::RglobalClassProp(pos) => RI::RglobalClassProp(self.pos_from_decl(pos)),
                OR::RglobalFunParam(pos) => RI::RglobalFunParam(self.pos_from_decl(pos)),
                OR::RglobalFunRet(pos) => RI::RglobalFunRet(self.pos_from_decl(pos)),
                OR::Rsplice(pos) => RI::Rsplice(self.pos_from_decl(pos)),
                OR::RetBoolean(pos) => RI::RetBoolean(self.pos_from_decl(pos)),
                OR::RdefaultCapability(pos) => RI::RdefaultCapability(self.pos_from_decl(pos)),
                OR::RconcatOperand(pos) => RI::RconcatOperand(self.pos_from_decl(pos)),
                OR::RinterpOperand(pos) => RI::RinterpOperand(self.pos_from_decl(pos)),
                OR::RdynamicCoercion(r) => RI::RdynamicCoercion(self.reason(r)),
                OR::RsupportDynamicType(pos) => RI::RsupportDynamicType(self.pos_from_decl(pos)),
                OR::RdynamicPartialEnforcement((pos, s, r)) => RI::RdynamicPartialEnforcement(
                    self.pos_from_decl(pos),
                    self.symbol(s),
                    self.reason(r),
                ),
                OR::RrigidTvarEscape((pos, s1, s2, r)) => RI::RrigidTvarEscape(
                    self.pos_from_decl(pos),
                    self.symbol(s1),
                    self.symbol(s2),
                    self.reason(r),
                ),
            }
        })
    }

    fn expr_dep_type_reason(
        &self,
        edtr: oxidized_by_ref::typing_reason::ExprDepTypeReason<'_>,
    ) -> ExprDepTypeReason {
        use oxidized_by_ref::typing_reason::ExprDepTypeReason as Obr;
        match edtr {
            Obr::ERexpr(i) => ExprDepTypeReason::ERexpr(i),
            Obr::ERstatic => ExprDepTypeReason::ERstatic,
            Obr::ERclass(s) => ExprDepTypeReason::ERclass(self.symbol(s)),
            Obr::ERparent(s) => ExprDepTypeReason::ERparent(self.symbol(s)),
            Obr::ERself(s) => ExprDepTypeReason::ERself(self.symbol(s)),
            Obr::ERpu(s) => ExprDepTypeReason::ERpu(self.symbol(s)),
        }
    }
}
