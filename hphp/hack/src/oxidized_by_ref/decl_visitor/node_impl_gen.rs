// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<818c988502579bceff20743df01d00ce>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
#![allow(unused_braces)]
use super::node::Node;
use super::visitor::Visitor;
use crate::{
    aast_defs::*, ast_defs::*, direct_decl_parser::*, shallow_decl_defs::*, t_shape_map::*,
    typing_defs::*, typing_defs_core::*, typing_reason::*,
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
impl<'a> Node<'a> for Capability<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_capability(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Capability::CapDefaults(ref __binding_0) => __binding_0.accept(v),
            Capability::CapTy(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ClassConstFrom<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_const_from(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConstFrom::Self_ => {}
            ClassConstFrom::From(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ClassConstRef<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_const_ref(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConstRef(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
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
impl<'a> Node<'a> for Decl<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_decl(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Decl::Class(ref __binding_0) => __binding_0.accept(v),
            Decl::Fun(ref __binding_0) => __binding_0.accept(v),
            Decl::Record(ref __binding_0) => __binding_0.accept(v),
            Decl::Typedef(ref __binding_0) => __binding_0.accept(v),
            Decl::Const(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Decls<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_decls(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Decls(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Enforcement {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_enforcement(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Enforcement::Unenforced => {}
            Enforcement::Enforced => {}
            Enforcement::PartiallyEnforced => {}
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
                flags: ref __binding_3,
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
                flags: ref __binding_6,
                ifc_decl: ref __binding_7,
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
impl<'a> Node<'a> for IfcFunDecl<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ifc_fun_decl(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            IfcFunDecl::FDPolicied(ref __binding_0) => __binding_0.accept(v),
            IfcFunDecl::FDInferFlows => {}
        }
    }
}
impl<'a> Node<'a> for PosByteString<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_pos_byte_string(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PosByteString(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for PosString<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_pos_string(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PosString(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
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
impl<'a> Node<'a> for RecordDefType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_record_def_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RecordDefType {
                name: ref __binding_0,
                extends: ref __binding_1,
                fields: ref __binding_2,
                abstract_: ref __binding_3,
                pos: ref __binding_4,
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
impl<'a> Node<'a> for RecordFieldReq {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_record_field_req(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RecordFieldReq::ValueRequired => {}
            RecordFieldReq::HasDefaultValue => {}
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
                implements_dynamic: ref __binding_14,
                consts: ref __binding_15,
                typeconsts: ref __binding_16,
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
                refs: ref __binding_3,
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
impl<'a> Node<'a> for ShallowMethod<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_method(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowMethod {
                name: ref __binding_0,
                type_: ref __binding_1,
                visibility: ref __binding_2,
                deprecated: ref __binding_3,
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
impl<'a> Node<'a> for ShallowProp<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_prop(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowProp {
                name: ref __binding_0,
                xhp_attr: ref __binding_1,
                type_: ref __binding_2,
                visibility: ref __binding_3,
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
impl<'a> Node<'a> for ShallowTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowTypeconst {
                abstract_: ref __binding_0,
                as_constraint: ref __binding_1,
                super_constraint: ref __binding_2,
                name: ref __binding_3,
                type_: ref __binding_4,
                enforceable: ref __binding_5,
                reifiable: ref __binding_6,
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
                { __binding_6.accept(v) }
            }
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
impl<'a> Node<'a> for TShapeField<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tshape_field(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TShapeField(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for T_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_t_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            T_::Rnone => {}
            T_::Rwitness(ref __binding_0) => __binding_0.accept(v),
            T_::RwitnessFromDecl(ref __binding_0) => __binding_0.accept(v),
            T_::Ridx(ref __binding_0) => __binding_0.accept(v),
            T_::RidxVector(ref __binding_0) => __binding_0.accept(v),
            T_::RidxVectorFromDecl(ref __binding_0) => __binding_0.accept(v),
            T_::Rforeach(ref __binding_0) => __binding_0.accept(v),
            T_::Rasyncforeach(ref __binding_0) => __binding_0.accept(v),
            T_::Rarith(ref __binding_0) => __binding_0.accept(v),
            T_::RarithRet(ref __binding_0) => __binding_0.accept(v),
            T_::RarithRetFloat(ref __binding_0) => __binding_0.accept(v),
            T_::RarithRetNum(ref __binding_0) => __binding_0.accept(v),
            T_::RarithRetInt(ref __binding_0) => __binding_0.accept(v),
            T_::RarithDynamic(ref __binding_0) => __binding_0.accept(v),
            T_::RbitwiseDynamic(ref __binding_0) => __binding_0.accept(v),
            T_::RincdecDynamic(ref __binding_0) => __binding_0.accept(v),
            T_::Rcomp(ref __binding_0) => __binding_0.accept(v),
            T_::RconcatRet(ref __binding_0) => __binding_0.accept(v),
            T_::RlogicRet(ref __binding_0) => __binding_0.accept(v),
            T_::Rbitwise(ref __binding_0) => __binding_0.accept(v),
            T_::RbitwiseRet(ref __binding_0) => __binding_0.accept(v),
            T_::RnoReturn(ref __binding_0) => __binding_0.accept(v),
            T_::RnoReturnAsync(ref __binding_0) => __binding_0.accept(v),
            T_::RretFunKind(ref __binding_0) => __binding_0.accept(v),
            T_::RretFunKindFromDecl(ref __binding_0) => __binding_0.accept(v),
            T_::Rhint(ref __binding_0) => __binding_0.accept(v),
            T_::Rthrow(ref __binding_0) => __binding_0.accept(v),
            T_::Rplaceholder(ref __binding_0) => __binding_0.accept(v),
            T_::RretDiv(ref __binding_0) => __binding_0.accept(v),
            T_::RyieldGen(ref __binding_0) => __binding_0.accept(v),
            T_::RyieldAsyncgen(ref __binding_0) => __binding_0.accept(v),
            T_::RyieldAsyncnull(ref __binding_0) => __binding_0.accept(v),
            T_::RyieldSend(ref __binding_0) => __binding_0.accept(v),
            T_::RlostInfo(ref __binding_0) => __binding_0.accept(v),
            T_::Rformat(ref __binding_0) => __binding_0.accept(v),
            T_::RclassClass(ref __binding_0) => __binding_0.accept(v),
            T_::RunknownClass(ref __binding_0) => __binding_0.accept(v),
            T_::RvarParam(ref __binding_0) => __binding_0.accept(v),
            T_::RvarParamFromDecl(ref __binding_0) => __binding_0.accept(v),
            T_::RunpackParam(ref __binding_0) => __binding_0.accept(v),
            T_::RinoutParam(ref __binding_0) => __binding_0.accept(v),
            T_::Rinstantiate(ref __binding_0) => __binding_0.accept(v),
            T_::RarrayFilter(ref __binding_0) => __binding_0.accept(v),
            T_::Rtypeconst(ref __binding_0) => __binding_0.accept(v),
            T_::RtypeAccess(ref __binding_0) => __binding_0.accept(v),
            T_::RexprDepType(ref __binding_0) => __binding_0.accept(v),
            T_::RnullsafeOp(ref __binding_0) => __binding_0.accept(v),
            T_::RtconstNoCstr(ref __binding_0) => __binding_0.accept(v),
            T_::Rpredicated(ref __binding_0) => __binding_0.accept(v),
            T_::Ris(ref __binding_0) => __binding_0.accept(v),
            T_::Ras(ref __binding_0) => __binding_0.accept(v),
            T_::RvarrayOrDarrayKey(ref __binding_0) => __binding_0.accept(v),
            T_::RvecOrDictKey(ref __binding_0) => __binding_0.accept(v),
            T_::Rusing(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicProp(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicCall(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicConstruct(ref __binding_0) => __binding_0.accept(v),
            T_::RidxDict(ref __binding_0) => __binding_0.accept(v),
            T_::RsetElement(ref __binding_0) => __binding_0.accept(v),
            T_::RmissingOptionalField(ref __binding_0) => __binding_0.accept(v),
            T_::RunsetField(ref __binding_0) => __binding_0.accept(v),
            T_::RcontravariantGeneric(ref __binding_0) => __binding_0.accept(v),
            T_::RinvariantGeneric(ref __binding_0) => __binding_0.accept(v),
            T_::Rregex(ref __binding_0) => __binding_0.accept(v),
            T_::RimplicitUpperBound(ref __binding_0) => __binding_0.accept(v),
            T_::RtypeVariable(ref __binding_0) => __binding_0.accept(v),
            T_::RtypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalTypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            T_::RsolveFail(ref __binding_0) => __binding_0.accept(v),
            T_::RcstrOnGenerics(ref __binding_0) => __binding_0.accept(v),
            T_::RlambdaParam(ref __binding_0) => __binding_0.accept(v),
            T_::Rshape(ref __binding_0) => __binding_0.accept(v),
            T_::Renforceable(ref __binding_0) => __binding_0.accept(v),
            T_::Rdestructure(ref __binding_0) => __binding_0.accept(v),
            T_::RkeyValueCollectionKey(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalClassProp(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalFunParam(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalFunRet(ref __binding_0) => __binding_0.accept(v),
            T_::Rsplice(ref __binding_0) => __binding_0.accept(v),
            T_::RetBoolean(ref __binding_0) => __binding_0.accept(v),
            T_::RdefaultCapability(ref __binding_0) => __binding_0.accept(v),
            T_::RhackArrDvArrs(ref __binding_0) => __binding_0.accept(v),
            T_::RconcatOperand(ref __binding_0) => __binding_0.accept(v),
            T_::RinterpOperand(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicCoercion(ref __binding_0) => __binding_0.accept(v),
            T_::RsoundDynamicCallable(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Tprim {
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
        }
    }
}
impl<'a> Node<'a> for TshapeFieldName<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tshape_field_name(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TshapeFieldName::TSFlitInt(ref __binding_0) => __binding_0.accept(v),
            TshapeFieldName::TSFlitStr(ref __binding_0) => __binding_0.accept(v),
            TshapeFieldName::TSFclassConst(ref __binding_0) => __binding_0.accept(v),
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
            Ty_::Tmixed => {}
            Ty_::Tlike(ref __binding_0) => __binding_0.accept(v),
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
            Ty_::TvecOrDict(ref __binding_0) => __binding_0.accept(v),
            Ty_::Taccess(ref __binding_0) => __binding_0.accept(v),
            Ty_::TunappliedAlias(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tnewtype(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tdependent(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tobject => {}
            Ty_::Tclass(ref __binding_0) => __binding_0.accept(v),
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
