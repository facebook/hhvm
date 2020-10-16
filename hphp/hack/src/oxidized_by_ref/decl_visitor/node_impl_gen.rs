// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<ff481e41b5da20513015d6ae6ff2d9aa>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

#![allow(unused_variables)]
#![allow(unused_braces)]
use super::node::Node;
use super::visitor::Visitor;
use crate::{
    aast_defs::*, ast_defs::*, decl_defs::*, direct_decl_parser::*, shallow_decl_defs::*,
    shape_map::*, typing_defs::*, typing_defs_core::*, typing_reason::*,
};
impl<'a> Node<'a> for ArgPosition {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_arg_position(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ArgPosition::Aonly => {}
            ArgPosition::Afirst => {}
            ArgPosition::Asecond => {}
        }
    }
}
impl<'a> Node<'a> for Blame<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_blame(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Blame::Blame(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for BlameSource {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_blame_source(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            BlameSource::BScall => {}
            BlameSource::BSlambda => {}
            BlameSource::BSassignment => {}
            BlameSource::BSoutOfScope => {}
        }
    }
}
impl<'a> Node<'a> for ClassKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassKind::Cabstract => {}
            ClassKind::Cnormal => {}
            ClassKind::Cinterface => {}
            ClassKind::Ctrait => {}
            ClassKind::Cenum => {}
        }
    }
}
impl<'a> Node<'a> for ConstDecl<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_const_decl(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ConstDecl {
                pos: ref __binding_0,
                type_: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ConstraintKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_constraint_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ConstraintKind::ConstraintAs => {}
            ConstraintKind::ConstraintEq => {}
            ConstraintKind::ConstraintSuper => {}
        }
    }
}
impl<'a> Node<'a> for Decls<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_decls(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Decls {
                classes: ref __binding_0,
                funs: ref __binding_1,
                typedefs: ref __binding_2,
                consts: ref __binding_3,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                { __binding_3.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for DependentType {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_dependent_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            DependentType::DTthis => {}
            DependentType::DTexpr(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for EnumType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_enum_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            EnumType {
                base: ref __binding_0,
                constraint: ref __binding_1,
                includes: ref __binding_2,
                enum_class: ref __binding_3,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                { __binding_3.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Exact {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_exact(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Exact::Exact => {}
            Exact::Nonexact => {}
        }
    }
}
impl<'a> Node<'a> for ExprDepTypeReason<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_expr_dep_type_reason(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ExprDepTypeReason::ERexpr(ref __binding_0) => __binding_0.accept(v),
            ExprDepTypeReason::ERstatic => {}
            ExprDepTypeReason::ERclass(ref __binding_0) => __binding_0.accept(v),
            ExprDepTypeReason::ERparent(ref __binding_0) => __binding_0.accept(v),
            ExprDepTypeReason::ERself(ref __binding_0) => __binding_0.accept(v),
            ExprDepTypeReason::ERpu(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for FunArity<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_arity(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunArity::Fstandard => {}
            FunArity::Fvariadic(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for FunElt<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_elt(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunElt {
                deprecated: ref __binding_0,
                type_: ref __binding_1,
                pos: ref __binding_2,
                php_std_lib: ref __binding_3,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                { __binding_3.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for FunImplicitParams<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_implicit_params(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunImplicitParams {
                capability: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for FunKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunKind::FSync => {}
            FunKind::FAsync => {}
            FunKind::FGenerator => {}
            FunKind::FAsyncGenerator => {}
            FunKind::FCoroutine => {}
        }
    }
}
impl<'a> Node<'a> for FunParam<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_param(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunParam {
                pos: ref __binding_0,
                name: ref __binding_1,
                type_: ref __binding_2,
                rx_annotation: ref __binding_3,
                flags: ref __binding_4,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                { __binding_4.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for FunType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunType {
                arity: ref __binding_0,
                tparams: ref __binding_1,
                where_constraints: ref __binding_2,
                params: ref __binding_3,
                implicit_params: ref __binding_4,
                ret: ref __binding_5,
                reactive: ref __binding_6,
                flags: ref __binding_7,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                {
                    __binding_5.accept(v)
                }
                {
                    __binding_6.accept(v)
                }
                { __binding_7.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Id<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_id(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Id(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for MethodReactivity<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_method_reactivity(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            MethodReactivity::MethodPure(ref __binding_0) => __binding_0.accept(v),
            MethodReactivity::MethodReactive(ref __binding_0) => __binding_0.accept(v),
            MethodReactivity::MethodShallow(ref __binding_0) => __binding_0.accept(v),
            MethodReactivity::MethodLocal(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ParamRxAnnotation<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_param_rx_annotation(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ParamRxAnnotation::ParamRxVar => {}
            ParamRxAnnotation::ParamRxIfImpl(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for PossiblyEnforcedTy<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_possibly_enforced_ty(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PossiblyEnforcedTy {
                enforced: ref __binding_0,
                type_: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Reactivity<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_reactivity(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Reactivity::Nonreactive => {}
            Reactivity::Local(ref __binding_0) => __binding_0.accept(v),
            Reactivity::Shallow(ref __binding_0) => __binding_0.accept(v),
            Reactivity::Reactive(ref __binding_0) => __binding_0.accept(v),
            Reactivity::Pure(ref __binding_0) => __binding_0.accept(v),
            Reactivity::MaybeReactive(ref __binding_0) => __binding_0.accept(v),
            Reactivity::RxVar(ref __binding_0) => __binding_0.accept(v),
            Reactivity::Cipp(ref __binding_0) => __binding_0.accept(v),
            Reactivity::CippLocal(ref __binding_0) => __binding_0.accept(v),
            Reactivity::CippGlobal => {}
            Reactivity::CippRx => {}
        }
    }
}
impl<'a> Node<'a> for Reason<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_reason(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Reason::Rnone => {}
            Reason::Rwitness(ref __binding_0) => __binding_0.accept(v),
            Reason::Ridx(ref __binding_0) => __binding_0.accept(v),
            Reason::RidxVector(ref __binding_0) => __binding_0.accept(v),
            Reason::Rforeach(ref __binding_0) => __binding_0.accept(v),
            Reason::Rasyncforeach(ref __binding_0) => __binding_0.accept(v),
            Reason::Rarith(ref __binding_0) => __binding_0.accept(v),
            Reason::RarithRet(ref __binding_0) => __binding_0.accept(v),
            Reason::RarithRetFloat(ref __binding_0) => __binding_0.accept(v),
            Reason::RarithRetNum(ref __binding_0) => __binding_0.accept(v),
            Reason::RarithRetInt(ref __binding_0) => __binding_0.accept(v),
            Reason::RarithDynamic(ref __binding_0) => __binding_0.accept(v),
            Reason::RbitwiseDynamic(ref __binding_0) => __binding_0.accept(v),
            Reason::RincdecDynamic(ref __binding_0) => __binding_0.accept(v),
            Reason::Rcomp(ref __binding_0) => __binding_0.accept(v),
            Reason::RconcatRet(ref __binding_0) => __binding_0.accept(v),
            Reason::RlogicRet(ref __binding_0) => __binding_0.accept(v),
            Reason::Rbitwise(ref __binding_0) => __binding_0.accept(v),
            Reason::RbitwiseRet(ref __binding_0) => __binding_0.accept(v),
            Reason::RnoReturn(ref __binding_0) => __binding_0.accept(v),
            Reason::RnoReturnAsync(ref __binding_0) => __binding_0.accept(v),
            Reason::RretFunKind(ref __binding_0) => __binding_0.accept(v),
            Reason::Rhint(ref __binding_0) => __binding_0.accept(v),
            Reason::Rthrow(ref __binding_0) => __binding_0.accept(v),
            Reason::Rplaceholder(ref __binding_0) => __binding_0.accept(v),
            Reason::RretDiv(ref __binding_0) => __binding_0.accept(v),
            Reason::RyieldGen(ref __binding_0) => __binding_0.accept(v),
            Reason::RyieldAsyncgen(ref __binding_0) => __binding_0.accept(v),
            Reason::RyieldAsyncnull(ref __binding_0) => __binding_0.accept(v),
            Reason::RyieldSend(ref __binding_0) => __binding_0.accept(v),
            Reason::RlostInfo(ref __binding_0) => __binding_0.accept(v),
            Reason::Rformat(ref __binding_0) => __binding_0.accept(v),
            Reason::RclassClass(ref __binding_0) => __binding_0.accept(v),
            Reason::RunknownClass(ref __binding_0) => __binding_0.accept(v),
            Reason::RvarParam(ref __binding_0) => __binding_0.accept(v),
            Reason::RunpackParam(ref __binding_0) => __binding_0.accept(v),
            Reason::RinoutParam(ref __binding_0) => __binding_0.accept(v),
            Reason::Rinstantiate(ref __binding_0) => __binding_0.accept(v),
            Reason::RarrayFilter(ref __binding_0) => __binding_0.accept(v),
            Reason::Rtypeconst(ref __binding_0) => __binding_0.accept(v),
            Reason::RtypeAccess(ref __binding_0) => __binding_0.accept(v),
            Reason::RexprDepType(ref __binding_0) => __binding_0.accept(v),
            Reason::RnullsafeOp(ref __binding_0) => __binding_0.accept(v),
            Reason::RtconstNoCstr(ref __binding_0) => __binding_0.accept(v),
            Reason::Rpredicated(ref __binding_0) => __binding_0.accept(v),
            Reason::Ris(ref __binding_0) => __binding_0.accept(v),
            Reason::Ras(ref __binding_0) => __binding_0.accept(v),
            Reason::RvarrayOrDarrayKey(ref __binding_0) => __binding_0.accept(v),
            Reason::Rusing(ref __binding_0) => __binding_0.accept(v),
            Reason::RdynamicProp(ref __binding_0) => __binding_0.accept(v),
            Reason::RdynamicCall(ref __binding_0) => __binding_0.accept(v),
            Reason::RidxDict(ref __binding_0) => __binding_0.accept(v),
            Reason::RmissingRequiredField(ref __binding_0) => __binding_0.accept(v),
            Reason::RmissingOptionalField(ref __binding_0) => __binding_0.accept(v),
            Reason::RunsetField(ref __binding_0) => __binding_0.accept(v),
            Reason::RcontravariantGeneric(ref __binding_0) => __binding_0.accept(v),
            Reason::RinvariantGeneric(ref __binding_0) => __binding_0.accept(v),
            Reason::Rregex(ref __binding_0) => __binding_0.accept(v),
            Reason::RimplicitUpperBound(ref __binding_0) => __binding_0.accept(v),
            Reason::RtypeVariable(ref __binding_0) => __binding_0.accept(v),
            Reason::RtypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            Reason::RsolveFail(ref __binding_0) => __binding_0.accept(v),
            Reason::RcstrOnGenerics(ref __binding_0) => __binding_0.accept(v),
            Reason::RlambdaParam(ref __binding_0) => __binding_0.accept(v),
            Reason::Rshape(ref __binding_0) => __binding_0.accept(v),
            Reason::Renforceable(ref __binding_0) => __binding_0.accept(v),
            Reason::Rdestructure(ref __binding_0) => __binding_0.accept(v),
            Reason::RkeyValueCollectionKey(ref __binding_0) => __binding_0.accept(v),
            Reason::RglobalClassProp(ref __binding_0) => __binding_0.accept(v),
            Reason::RglobalFunParam(ref __binding_0) => __binding_0.accept(v),
            Reason::RglobalFunRet(ref __binding_0) => __binding_0.accept(v),
            Reason::Rsplice(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ReifyKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_reify_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ReifyKind::Erased => {}
            ReifyKind::SoftReified => {}
            ReifyKind::Reified => {}
        }
    }
}
impl<'a> Node<'a> for ShallowClass<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_class(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowClass {
                mode: ref __binding_0,
                final_: ref __binding_1,
                is_xhp: ref __binding_2,
                has_xhp_keyword: ref __binding_3,
                kind: ref __binding_4,
                name: ref __binding_5,
                tparams: ref __binding_6,
                where_constraints: ref __binding_7,
                extends: ref __binding_8,
                uses: ref __binding_9,
                xhp_attr_uses: ref __binding_10,
                req_extends: ref __binding_11,
                req_implements: ref __binding_12,
                implements: ref __binding_13,
                consts: ref __binding_14,
                typeconsts: ref __binding_15,
                pu_enums: ref __binding_16,
                props: ref __binding_17,
                sprops: ref __binding_18,
                constructor: ref __binding_19,
                static_methods: ref __binding_20,
                methods: ref __binding_21,
                user_attributes: ref __binding_22,
                enum_type: ref __binding_23,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                {
                    __binding_5.accept(v)
                }
                {
                    __binding_6.accept(v)
                }
                {
                    __binding_7.accept(v)
                }
                {
                    __binding_8.accept(v)
                }
                {
                    __binding_9.accept(v)
                }
                {
                    __binding_10.accept(v)
                }
                {
                    __binding_11.accept(v)
                }
                {
                    __binding_12.accept(v)
                }
                {
                    __binding_13.accept(v)
                }
                {
                    __binding_14.accept(v)
                }
                {
                    __binding_15.accept(v)
                }
                {
                    __binding_16.accept(v)
                }
                {
                    __binding_17.accept(v)
                }
                {
                    __binding_18.accept(v)
                }
                {
                    __binding_19.accept(v)
                }
                {
                    __binding_20.accept(v)
                }
                {
                    __binding_21.accept(v)
                }
                {
                    __binding_22.accept(v)
                }
                { __binding_23.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowClassConst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_class_const(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowClassConst {
                abstract_: ref __binding_0,
                name: ref __binding_1,
                type_: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowMethod<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_method(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowMethod {
                abstract_: ref __binding_0,
                final_: ref __binding_1,
                memoizelsb: ref __binding_2,
                name: ref __binding_3,
                override_: ref __binding_4,
                dynamicallycallable: ref __binding_5,
                reactivity: ref __binding_6,
                type_: ref __binding_7,
                visibility: ref __binding_8,
                deprecated: ref __binding_9,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                {
                    __binding_5.accept(v)
                }
                {
                    __binding_6.accept(v)
                }
                {
                    __binding_7.accept(v)
                }
                {
                    __binding_8.accept(v)
                }
                { __binding_9.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowProp<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_prop(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowProp {
                const_: ref __binding_0,
                xhp_attr: ref __binding_1,
                lateinit: ref __binding_2,
                lsb: ref __binding_3,
                name: ref __binding_4,
                needs_init: ref __binding_5,
                type_: ref __binding_6,
                abstract_: ref __binding_7,
                visibility: ref __binding_8,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                {
                    __binding_5.accept(v)
                }
                {
                    __binding_6.accept(v)
                }
                {
                    __binding_7.accept(v)
                }
                { __binding_8.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowPuEnum<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_pu_enum(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowPuEnum {
                name: ref __binding_0,
                is_final: ref __binding_1,
                case_types: ref __binding_2,
                case_values: ref __binding_3,
                members: ref __binding_4,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                { __binding_4.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowPuMember<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_pu_member(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowPuMember {
                atom: ref __binding_0,
                types: ref __binding_1,
                exprs: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShallowTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowTypeconst {
                abstract_: ref __binding_0,
                constraint: ref __binding_1,
                name: ref __binding_2,
                type_: ref __binding_3,
                enforceable: ref __binding_4,
                reifiable: ref __binding_5,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                { __binding_5.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShapeField<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_field(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeField(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ShapeFieldName<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_field_name(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeFieldName::SFlitInt(ref __binding_0) => __binding_0.accept(v),
            ShapeFieldName::SFlitStr(ref __binding_0) => __binding_0.accept(v),
            ShapeFieldName::SFclassConst(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ShapeFieldType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_field_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeFieldType {
                optional: ref __binding_0,
                ty: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShapeKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeKind::ClosedShape => {}
            ShapeKind::OpenShape => {}
        }
    }
}
impl<'a> Node<'a> for TaccessType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_taccess_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TaccessType(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Tparam<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tparam(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Tparam {
                variance: ref __binding_0,
                name: ref __binding_1,
                tparams: ref __binding_2,
                constraints: ref __binding_3,
                reified: ref __binding_4,
                user_attributes: ref __binding_5,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                {
                    __binding_4.accept(v)
                }
                { __binding_5.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Tprim<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tprim(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Tprim::Tnull => {}
            Tprim::Tvoid => {}
            Tprim::Tint => {}
            Tprim::Tbool => {}
            Tprim::Tfloat => {}
            Tprim::Tstring => {}
            Tprim::Tresource => {}
            Tprim::Tnum => {}
            Tprim::Tarraykey => {}
            Tprim::Tnoreturn => {}
            Tprim::Tatom(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Ty<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ty(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Ty(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Ty_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ty_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Ty_::Tthis => {}
            Ty_::Tapply(ref __binding_0) => __binding_0.accept(v),
            Ty_::Taccess(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tarray(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tmixed => {}
            Ty_::Tlike(ref __binding_0) => __binding_0.accept(v),
            Ty_::TpuAccess(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tany(ref __binding_0) => __binding_0.accept(v),
            Ty_::Terr => {}
            Ty_::Tnonnull => {}
            Ty_::Tdynamic => {}
            Ty_::Toption(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tprim(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tfun(ref __binding_0) => __binding_0.accept(v),
            Ty_::Ttuple(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tshape(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tvar(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tgeneric(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tunion(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tintersection(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tdarray(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tvarray(ref __binding_0) => __binding_0.accept(v),
            Ty_::TvarrayOrDarray(ref __binding_0) => __binding_0.accept(v),
            Ty_::TunappliedAlias(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tnewtype(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tdependent(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tobject => {}
            Ty_::Tclass(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tpu(ref __binding_0) => __binding_0.accept(v),
            Ty_::TpuTypeAccess(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for TypeconstAbstractKind<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typeconst_abstract_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypeconstAbstractKind::TCAbstract(ref __binding_0) => __binding_0.accept(v),
            TypeconstAbstractKind::TCPartiallyAbstract => {}
            TypeconstAbstractKind::TCConcrete => {}
        }
    }
}
impl<'a> Node<'a> for TypedefType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefType {
                pos: ref __binding_0,
                vis: ref __binding_1,
                tparams: ref __binding_2,
                constraint: ref __binding_3,
                type_: ref __binding_4,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                {
                    __binding_2.accept(v)
                }
                {
                    __binding_3.accept(v)
                }
                { __binding_4.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for TypedefVisibility {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_visibility(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefVisibility::Transparent => {}
            TypedefVisibility::Opaque => {}
        }
    }
}
impl<'a> Node<'a> for UserAttribute<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_user_attribute(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            UserAttribute {
                name: ref __binding_0,
                classname_params: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Variance {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_variance(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Variance::Covariant => {}
            Variance::Contravariant => {}
            Variance::Invariant => {}
        }
    }
}
impl<'a> Node<'a> for Visibility {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_visibility(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Visibility::Private => {}
            Visibility::Public => {}
            Visibility::Protected => {}
        }
    }
}
impl<'a> Node<'a> for WhereConstraint<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_where_constraint(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            WhereConstraint(ref __binding_0, ref __binding_1, ref __binding_2) => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for XhpAttr {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttr {
                tag: ref __binding_0,
                has_default: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for XhpAttrTag {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attr_tag(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttrTag::Required => {}
            XhpAttrTag::Lateinit => {}
        }
    }
}
