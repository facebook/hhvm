// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<49884b7db749cb8c355c913f14bd1f6c>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_braces)]
#![allow(unused_imports)]
#![allow(unused_variables)]
#![allow(clippy::all)]
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
                value: ref __binding_4,
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
                sort_text: ref __binding_6,
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
impl<'a> Node<'a> for DeclConstraintRequirement<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_decl_constraint_requirement(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            DeclConstraintRequirement::DCREqual(ref __binding_0) => __binding_0.accept(v),
            DeclConstraintRequirement::DCRSubtype(ref __binding_0) => __binding_0.accept(v),
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
                extends: ref __binding_10,
                uses: ref __binding_11,
                xhp_attr_uses: ref __binding_12,
                xhp_enum_values: ref __binding_13,
                xhp_marked_empty: ref __binding_14,
                req_extends: ref __binding_15,
                req_implements: ref __binding_16,
                req_constraints: ref __binding_17,
                implements: ref __binding_18,
                support_dynamic_type: ref __binding_19,
                consts: ref __binding_20,
                typeconsts: ref __binding_21,
                props: ref __binding_22,
                sprops: ref __binding_23,
                constructor: ref __binding_24,
                static_methods: ref __binding_25,
                methods: ref __binding_26,
                user_attributes: ref __binding_27,
                enum_type: ref __binding_28,
                docs_url: ref __binding_29,
                package: ref __binding_30,
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
            TshapeFieldName::TSFregexGroup(ref __binding_0) => __binding_0.accept(v),
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
                raw_val: ref __binding_2,
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
                def_value: ref __binding_4,
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
                tparams: ref __binding_0,
                where_constraints: ref __binding_1,
                params: ref __binding_2,
                implicit_params: ref __binding_3,
                ret: ref __binding_4,
                flags: ref __binding_5,
                cross_package: ref __binding_6,
                instantiated: ref __binding_7,
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
impl<'a> Node<'a> for TypeTag<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_type_tag(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypeTag::BoolTag => {}
            TypeTag::IntTag => {}
            TypeTag::StringTag => {}
            TypeTag::ArraykeyTag => {}
            TypeTag::FloatTag => {}
            TypeTag::NumTag => {}
            TypeTag::ResourceTag => {}
            TypeTag::NullTag => {}
            TypeTag::ClassTag(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ShapeFieldPredicate<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_field_predicate(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeFieldPredicate {
                sfp_predicate: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ShapePredicate<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_predicate(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapePredicate {
                sp_fields: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for TuplePredicate<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tuple_predicate(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TuplePredicate {
                tp_required: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for TypePredicate_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_type_predicate_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypePredicate_::IsTag(ref __binding_0) => __binding_0.accept(v),
            TypePredicate_::IsTupleOf(ref __binding_0) => __binding_0.accept(v),
            TypePredicate_::IsShapeOf(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for TypePredicate<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_type_predicate(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypePredicate(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
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
            Ty_::TclassPtr(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tvar(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tnewtype(ref __binding_0) => __binding_0.accept(v),
            Ty_::TunappliedAlias(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tdependent(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tclass(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tneg(ref __binding_0) => __binding_0.accept(v),
            Ty_::Tlabel(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for TupleType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tuple_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TupleType {
                required: ref __binding_0,
                extra: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for TupleExtra<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tuple_extra(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TupleExtra::Textra {
                optional: ref __binding_0,
                variadic: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
            TupleExtra::Tsplat(ref __binding_0) => __binding_0.accept(v),
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
                value: ref __binding_2,
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
impl<'a> Node<'a> for FunElt<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_elt(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunElt {
                deprecated: ref __binding_0,
                module: ref __binding_1,
                package: ref __binding_2,
                internal: ref __binding_3,
                type_: ref __binding_4,
                pos: ref __binding_5,
                php_std_lib: ref __binding_6,
                support_dynamic_type: ref __binding_7,
                no_auto_dynamic: ref __binding_8,
                no_auto_likes: ref __binding_9,
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
impl<'a> Node<'a> for ModuleDefType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_module_def_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ModuleDefType {
                mdt_pos: ref __binding_0,
            } => __binding_0.accept(v),
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
impl<'a> Node<'a> for TypedefCaseTypeVariant<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_case_type_variant(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefCaseTypeVariant(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for TypedefTypeAssignment<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef_type_assignment(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypedefTypeAssignment::SimpleTypeDef(ref __binding_0) => __binding_0.accept(v),
            TypedefTypeAssignment::CaseType(ref __binding_0) => __binding_0.accept(v),
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
                tparams: ref __binding_2,
                as_constraint: ref __binding_3,
                super_constraint: ref __binding_4,
                type_assignment: ref __binding_5,
                is_ctx: ref __binding_6,
                attributes: ref __binding_7,
                internal: ref __binding_8,
                docs_url: ref __binding_9,
                package: ref __binding_10,
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
impl<'a> Node<'a> for CstrVariance<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_cstr_variance(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            CstrVariance::Dir(ref __binding_0) => __binding_0.accept(v),
            CstrVariance::Inv(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for PrjSymm<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_prj_symm(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PrjSymm::PrjSymmNeg => {}
            PrjSymm::PrjSymmNullable => {}
            PrjSymm::PrjSymmCtor(ref __binding_0) => __binding_0.accept(v),
            PrjSymm::PrjSymmTuple(ref __binding_0) => __binding_0.accept(v),
            PrjSymm::PrjSymmShape(ref __binding_0) => __binding_0.accept(v),
            PrjSymm::PrjSymmFnParam(ref __binding_0) => __binding_0.accept(v),
            PrjSymm::PrjSymmFnParamInout(ref __binding_0) => __binding_0.accept(v),
            PrjSymm::PrjSymmFnRet => {}
            PrjSymm::PrjSymmSupportdyn => {}
        }
    }
}
impl<'a> Node<'a> for FlowKind<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_flow_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FlowKind::FlowAssign => {}
            FlowKind::FlowCall => {}
            FlowKind::FlowPropAccess => {}
            FlowKind::FlowConstAccess => {}
            FlowKind::FlowLocal => {}
            FlowKind::FlowFunReturn => {}
            FlowKind::FlowParamHint => {}
            FlowKind::FlowReturnExpr => {}
            FlowKind::FlowInstantiate(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for WitnessLocl<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_witness_locl(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            WitnessLocl::Witness(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::IdxVector(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Foreach(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Asyncforeach(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Arith(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ArithRet(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ArithRetInt(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ArithDynamic(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::BitwiseDynamic(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::IncdecDynamic(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Comp(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ConcatRet(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::LogicRet(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Bitwise(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::BitwiseRet(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::NoReturn(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::NoReturnAsync(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::RetFunKind(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Throw(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Placeholder(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::RetDiv(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::YieldGen(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::YieldAsyncgen(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::YieldAsyncnull(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::YieldSend(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::UnknownClass(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::VarParam(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::UnpackParam(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::NullsafeOp(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Predicated(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::IsRefinement(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::AsRefinement(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Equal(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Using(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::DynamicProp(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::DynamicCall(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::DynamicConstruct(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::IdxDict(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::IdxSetElement(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::UnsetField(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Regex(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::TypeVariable(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::TypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::TypeVariableError(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Shape(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ShapeLiteral(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Destructure(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::KeyValueCollectionKey(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Splice(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::EtBoolean(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ConcatOperand(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::InterpOperand(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::MissingClass(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::CapturedLike(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::UnsafeCast(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::Pattern(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::JoinPoint(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::StaticPropertyAccess(ref __binding_0) => __binding_0.accept(v),
            WitnessLocl::ClassConstantAccess(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for WitnessDecl<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_witness_decl(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            WitnessDecl::WitnessFromDecl(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::IdxVectorFromDecl(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::Hint(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::ClassClass(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::VarParamFromDecl(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::TupleFromSplat(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::VecOrDictKey(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::RetFunKindFromDecl(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::InoutParam(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::TconstNoCstr(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::VarrayOrDarrayKey(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::MissingOptionalField(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::ImplicitUpperBound(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::GlobalTypeVariableGenerics(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::SolveFail(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::CstrOnGenerics(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::Enforceable(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::GlobalClassProp(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::GlobalFunParam(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::GlobalFunRet(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::DefaultCapability(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::SupportDynamicType(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::PessimisedInout(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::PessimisedReturn(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::PessimisedProp(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::PessimisedThis(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::IllegalRecursiveType(ref __binding_0) => __binding_0.accept(v),
            WitnessDecl::SupportDynamicTypeAssume(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for T_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_t_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            T_::FromWitnessDecl(ref __binding_0) => __binding_0.accept(v),
            T_::Instantiate(ref __binding_0) => __binding_0.accept(v),
            T_::NoReason => {}
            T_::FromWitnessLocl(ref __binding_0) => __binding_0.accept(v),
            T_::LowerBound {
                bound: ref __binding_0,
                of__: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
            T_::Flow {
                from: ref __binding_0,
                kind: ref __binding_1,
                into: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
            T_::PrjBoth {
                sub_prj: ref __binding_0,
                prj: ref __binding_1,
                sub: ref __binding_2,
                super_: ref __binding_3,
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
            T_::PrjOne {
                part: ref __binding_0,
                whole: ref __binding_1,
                prj: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
            T_::Axiom {
                next: ref __binding_0,
                prev: ref __binding_1,
                axiom: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
            T_::Def(ref __binding_0) => __binding_0.accept(v),
            T_::Solved {
                solution: ref __binding_0,
                of__: ref __binding_1,
                in__: ref __binding_2,
            } => {
                {
                    __binding_0.accept(v)
                }
                {
                    __binding_1.accept(v)
                }
                { __binding_2.accept(v) }
            }
            T_::Invalid => {}
            T_::MissingField => {}
            T_::Idx(ref __binding_0) => __binding_0.accept(v),
            T_::ArithRetFloat(ref __binding_0) => __binding_0.accept(v),
            T_::ArithRetNum(ref __binding_0) => __binding_0.accept(v),
            T_::LostInfo(ref __binding_0) => __binding_0.accept(v),
            T_::Format(ref __binding_0) => __binding_0.accept(v),
            T_::Typeconst(ref __binding_0) => __binding_0.accept(v),
            T_::TypeAccess(ref __binding_0) => __binding_0.accept(v),
            T_::ExprDepType(ref __binding_0) => __binding_0.accept(v),
            T_::ContravariantGeneric(ref __binding_0) => __binding_0.accept(v),
            T_::InvariantGeneric(ref __binding_0) => __binding_0.accept(v),
            T_::LambdaParam(ref __binding_0) => __binding_0.accept(v),
            T_::DynamicCoercion(ref __binding_0) => __binding_0.accept(v),
            T_::DynamicPartialEnforcement(ref __binding_0) => __binding_0.accept(v),
            T_::RigidTvarEscape(ref __binding_0) => __binding_0.accept(v),
            T_::OpaqueTypeFromModule(ref __binding_0) => __binding_0.accept(v),
            T_::SDTCall(ref __binding_0) => __binding_0.accept(v),
            T_::LikeCall(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for PessimiseReason {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_pessimise_reason(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PessimiseReason::PRabstract => {}
            PessimiseReason::PRgenericParam => {}
            PessimiseReason::PRthis => {}
            PessimiseReason::PRgenericApply => {}
            PessimiseReason::PRtupleOrShape => {}
            PessimiseReason::PRtypeconst => {}
            PessimiseReason::PRcase => {}
            PessimiseReason::PRenum => {}
            PessimiseReason::PRopaque => {}
            PessimiseReason::PRdynamic => {}
            PessimiseReason::PRfun => {}
            PessimiseReason::PRclassptr => {}
            PessimiseReason::PRvoidOrNoreturn => {}
            PessimiseReason::PRrefinement => {}
            PessimiseReason::PRunionOrIntersection => {}
            PessimiseReason::PRxhp => {}
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
impl<'a> Node<'a> for VarianceDir {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_variance_dir(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            VarianceDir::Co => {}
            VarianceDir::Contra => {}
        }
    }
}
impl<'a> Node<'a> for FieldKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_field_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FieldKind::Absent => {}
            FieldKind::Optional => {}
            FieldKind::Required => {}
        }
    }
}
impl<'a> Node<'a> for CtorKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ctor_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            CtorKind::CtorClass => {}
            CtorKind::CtorNewtype => {}
        }
    }
}
impl<'a> Node<'a> for PrjAsymm {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_prj_asymm(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            PrjAsymm::PrjAsymmUnion => {}
            PrjAsymm::PrjAsymmInter => {}
            PrjAsymm::PrjAsymmNeg => {}
            PrjAsymm::PrjAsymmNullable => {}
            PrjAsymm::PrjAsymmArraykey => {}
            PrjAsymm::PrjAsymmNum => {}
            PrjAsymm::PrjRewriteClassname => {}
        }
    }
}
impl<'a> Node<'a> for Axiom {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_axiom(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Axiom::Extends => {}
            Axiom::UpperBound => {}
            Axiom::LowerBound => {}
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
