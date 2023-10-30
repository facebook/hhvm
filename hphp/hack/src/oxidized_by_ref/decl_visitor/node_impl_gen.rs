// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<fdb13d9865a820f15c7fa56ac31ac05b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_braces)]
#![allow(unused_imports)]
#![allow(unused_variables)]
use super::node::Node;
use super::visitor::Visitor;
use crate::ast_defs::*;
use crate::ast_defs::{self};
use crate::direct_decl_parser::*;
use crate::direct_decl_parser::{self};
use crate::shallow_decl_defs::*;
use crate::shallow_decl_defs::{self};
use crate::t_shape_map::*;
use crate::t_shape_map::{self};
use crate::typing_defs::*;
use crate::typing_defs::{self};
use crate::typing_defs_core::*;
use crate::typing_defs_core::{self};
use crate::typing_reason::*;
use crate::typing_reason::{self};
use crate::xhp_attribute::*;
use crate::xhp_attribute::{self};
use crate::*;
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
impl<'a> Node<'a> for XhpEnumValue<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_enum_value(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpEnumValue::XEVInt(ref __binding_0) => __binding_0.accept(v),
            XhpEnumValue::XEVString(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for ShallowTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowTypeconst {
                name: ref __binding_0,
                kind: ref __binding_1,
                enforceable: ref __binding_2,
                reifiable: ref __binding_3,
                is_ctx: ref __binding_4,
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
                attributes: ref __binding_5,
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
impl<'a> Node<'a> for ShallowClass<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shallow_class(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShallowClass {
                mode: ref __binding_0,
                final_: ref __binding_1,
                abstract_: ref __binding_2,
                is_xhp: ref __binding_3,
                internal: ref __binding_4,
                has_xhp_keyword: ref __binding_5,
                kind: ref __binding_6,
                module: ref __binding_7,
                name: ref __binding_8,
                tparams: ref __binding_9,
                where_constraints: ref __binding_10,
                extends: ref __binding_11,
                uses: ref __binding_12,
                xhp_attr_uses: ref __binding_13,
                xhp_enum_values: ref __binding_14,
                xhp_marked_empty: ref __binding_15,
                req_extends: ref __binding_16,
                req_implements: ref __binding_17,
                req_class: ref __binding_18,
                implements: ref __binding_19,
                support_dynamic_type: ref __binding_20,
                consts: ref __binding_21,
                typeconsts: ref __binding_22,
                props: ref __binding_23,
                sprops: ref __binding_24,
                constructor: ref __binding_25,
                static_methods: ref __binding_26,
                methods: ref __binding_27,
                user_attributes: ref __binding_28,
                enum_type: ref __binding_29,
                docs_url: ref __binding_30,
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
                {
                    __binding_23.accept(v)
                }
                {
                    __binding_24.accept(v)
                }
                {
                    __binding_25.accept(v)
                }
                {
                    __binding_26.accept(v)
                }
                {
                    __binding_27.accept(v)
                }
                {
                    __binding_28.accept(v)
                }
                {
                    __binding_29.accept(v)
                }
                { __binding_30.accept(v) }
            }
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
            Decl::Typedef(ref __binding_0) => __binding_0.accept(v),
            Decl::Const(ref __binding_0) => __binding_0.accept(v),
            Decl::Module(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for TypeOrigin<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_type_origin(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypeOrigin::MissingOrigin => {}
            TypeOrigin::FromAlias(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for DependentType {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_dependent_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            DependentType::DTexpr(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for UserAttributeParam<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_user_attribute_param(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            UserAttributeParam::Classname(ref __binding_0) => __binding_0.accept(v),
            UserAttributeParam::EnumClassLabel(ref __binding_0) => __binding_0.accept(v),
            UserAttributeParam::String(ref __binding_0) => __binding_0.accept(v),
            UserAttributeParam::Int(ref __binding_0) => __binding_0.accept(v),
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
                params: ref __binding_1,
            } => {
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
                tparams: ref __binding_0,
                where_constraints: ref __binding_1,
                params: ref __binding_2,
                implicit_params: ref __binding_3,
                ret: ref __binding_4,
                flags: ref __binding_5,
                cross_package: ref __binding_6,
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
impl<'a> Node<'a> for NegType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_neg_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            NegType::NegPrim(ref __binding_0) => __binding_0.accept(v),
            NegType::NegClass(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Ty_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ty_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Ty_::Tthis => {}
            Ty_::Tapply(ref __binding_0) => __binding_0.accept(v),
            Ty_::Trefinement(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tmixed => {}
            Ty_::Twildcard => {}
            Ty_::Tlike(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tany(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tnonnull => {}
            Ty_::Tdynamic => {}
            Ty_::Toption(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tprim(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tfun(ref __binding_0) => __binding_0.accept(v),
            Ty_::Ttuple(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tshape(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tgeneric(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tunion(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tintersection(ref __binding_0) => __binding_0.accept(v),
            Ty_::TvecOrDict(ref __binding_0) => __binding_0.accept(v),
            Ty_::Taccess(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tnewtype(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tvar(ref __binding_0) => __binding_0.accept(v),
            Ty_::TunappliedAlias(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tdependent(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tclass(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tneg(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Exact<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_exact(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Exact::Exact => {}
            Exact::Nonexact(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ClassRefinement<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_refinement(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassRefinement {
                cr_consts: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for RefinedConst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_refined_const(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RefinedConst {
                bound: ref __binding_0,
                is_ctx: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for RefinedConstBound<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_refined_const_bound(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RefinedConstBound::TRexact(ref __binding_0) => __binding_0.accept(v),
            RefinedConstBound::TRloose(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for RefinedConstBounds<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_refined_const_bounds(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RefinedConstBounds {
                lower: ref __binding_0,
                upper: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ShapeType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeType {
                origin: ref __binding_0,
                unknown_value: ref __binding_1,
                fields: ref __binding_2,
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
impl<'a> Node<'a> for FunElt<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_elt(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunElt {
                deprecated: ref __binding_0,
                module: ref __binding_1,
                internal: ref __binding_2,
                type_: ref __binding_3,
                pos: ref __binding_4,
                php_std_lib: ref __binding_5,
                support_dynamic_type: ref __binding_6,
                no_auto_dynamic: ref __binding_7,
                no_auto_likes: ref __binding_8,
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
impl<'a> Node<'a> for ModuleReference<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_module_reference(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ModuleReference::MRGlobal => {}
            ModuleReference::MRPrefix(ref __binding_0) => __binding_0.accept(v),
            ModuleReference::MRExact(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ModuleDefType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_module_def_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ModuleDefType {
                pos: ref __binding_0,
                exports: ref __binding_1,
                imports: ref __binding_2,
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
impl<'a> Node<'a> for AbstractTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_abstract_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            AbstractTypeconst {
                as_constraint: ref __binding_0,
                super_constraint: ref __binding_1,
                default: ref __binding_2,
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
impl<'a> Node<'a> for ConcreteTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_concrete_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ConcreteTypeconst {
                tc_type: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Typeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Typeconst::TCAbstract(ref __binding_0) => __binding_0.accept(v),
            Typeconst::TCConcrete(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for TypedefType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefType {
                module: ref __binding_0,
                pos: ref __binding_1,
                vis: ref __binding_2,
                tparams: ref __binding_3,
                as_constraint: ref __binding_4,
                super_constraint: ref __binding_5,
                type_: ref __binding_6,
                is_ctx: ref __binding_7,
                attributes: ref __binding_8,
                internal: ref __binding_9,
                docs_url: ref __binding_10,
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
                { __binding_10.accept(v) }
            }
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
            T_::Rtypeconst(ref __binding_0) => __binding_0.accept(v),
            T_::RtypeAccess(ref __binding_0) => __binding_0.accept(v),
            T_::RexprDepType(ref __binding_0) => __binding_0.accept(v),
            T_::RnullsafeOp(ref __binding_0) => __binding_0.accept(v),
            T_::RtconstNoCstr(ref __binding_0) => __binding_0.accept(v),
            T_::Rpredicated(ref __binding_0) => __binding_0.accept(v),
            T_::Ris(ref __binding_0) => __binding_0.accept(v),
            T_::Ras(ref __binding_0) => __binding_0.accept(v),
            T_::Requal(ref __binding_0) => __binding_0.accept(v),
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
            T_::RtypeVariableError(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalTypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            T_::RsolveFail(ref __binding_0) => __binding_0.accept(v),
            T_::RcstrOnGenerics(ref __binding_0) => __binding_0.accept(v),
            T_::RlambdaParam(ref __binding_0) => __binding_0.accept(v),
            T_::Rshape(ref __binding_0) => __binding_0.accept(v),
            T_::RshapeLiteral(ref __binding_0) => __binding_0.accept(v),
            T_::Renforceable(ref __binding_0) => __binding_0.accept(v),
            T_::Rdestructure(ref __binding_0) => __binding_0.accept(v),
            T_::RkeyValueCollectionKey(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalClassProp(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalFunParam(ref __binding_0) => __binding_0.accept(v),
            T_::RglobalFunRet(ref __binding_0) => __binding_0.accept(v),
            T_::Rsplice(ref __binding_0) => __binding_0.accept(v),
            T_::RetBoolean(ref __binding_0) => __binding_0.accept(v),
            T_::RdefaultCapability(ref __binding_0) => __binding_0.accept(v),
            T_::RconcatOperand(ref __binding_0) => __binding_0.accept(v),
            T_::RinterpOperand(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicCoercion(ref __binding_0) => __binding_0.accept(v),
            T_::RsupportDynamicType(ref __binding_0) => __binding_0.accept(v),
            T_::RdynamicPartialEnforcement(ref __binding_0) => __binding_0.accept(v),
            T_::RrigidTvarEscape(ref __binding_0) => __binding_0.accept(v),
            T_::RopaqueTypeFromModule(ref __binding_0) => __binding_0.accept(v),
            T_::RmissingClass(ref __binding_0) => __binding_0.accept(v),
            T_::Rinvalid => {}
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
impl<'a> Node<'a> for Abstraction {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_abstraction(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Abstraction::Concrete => {}
            Abstraction::Abstract => {}
        }
    }
}
impl<'a> Node<'a> for ClassishKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_classish_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassishKind::Cclass(ref __binding_0) => __binding_0.accept(v),
            ClassishKind::Cinterface => {}
            ClassishKind::Ctrait => {}
            ClassishKind::Cenum => {}
            ClassishKind::CenumClass(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Visibility {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_visibility(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Visibility::Private => {}
            Visibility::Public => {}
            Visibility::Protected => {}
            Visibility::Internal => {}
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
impl<'a> Node<'a> for TypedefVisibility {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_visibility(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefVisibility::Transparent => {}
            TypedefVisibility::Opaque => {}
            TypedefVisibility::OpaqueModule => {}
            TypedefVisibility::CaseType => {}
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
impl<'a> Node<'a> for ClassConstKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_const_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConstKind::CCAbstract(ref __binding_0) => __binding_0.accept(v),
            ClassConstKind::CCConcrete => {}
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
        }
    }
}
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
impl<'a> Node<'a> for Tag {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tag(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Tag::Required => {}
            Tag::LateInit => {}
        }
    }
}
impl<'a> Node<'a> for XhpAttribute {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attribute(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttribute {
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
