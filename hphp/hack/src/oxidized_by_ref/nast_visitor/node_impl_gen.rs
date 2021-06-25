// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<91dc43f57aa9f7a47b7550728f0a5aec>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
#![allow(unused_braces)]
use super::node::Node;
use super::visitor::Visitor;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*, namespace_env::*};
impl<'a> Node<'a> for Afield<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_afield(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Afield::AFvalue(ref __binding_0) => __binding_0.accept(v),
            Afield::AFkvalue(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for AsExpr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_as_expr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            AsExpr::AsV(ref __binding_0) => __binding_0.accept(v),
            AsExpr::AsKv(ref __binding_0) => __binding_0.accept(v),
            AsExpr::AwaitAsV(ref __binding_0) => __binding_0.accept(v),
            AsExpr::AwaitAsKv(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Bop<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_bop(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Bop::Plus => {}
            Bop::Minus => {}
            Bop::Star => {}
            Bop::Slash => {}
            Bop::Eqeq => {}
            Bop::Eqeqeq => {}
            Bop::Starstar => {}
            Bop::Diff => {}
            Bop::Diff2 => {}
            Bop::Ampamp => {}
            Bop::Barbar => {}
            Bop::Lt => {}
            Bop::Lte => {}
            Bop::Gt => {}
            Bop::Gte => {}
            Bop::Dot => {}
            Bop::Amp => {}
            Bop::Bar => {}
            Bop::Ltlt => {}
            Bop::Gtgt => {}
            Bop::Percent => {}
            Bop::Xor => {}
            Bop::Cmp => {}
            Bop::QuestionQuestion => {}
            Bop::Eq(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for CaField<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ca_field(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            CaField {
                type_: ref __binding_0,
                id: ref __binding_1,
                value: ref __binding_2,
                required: ref __binding_3,
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
impl<'a> Node<'a> for CaType<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ca_type(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            CaType::CAHint(ref __binding_0) => __binding_0.accept(v),
            CaType::CAEnum(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Case<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_case(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Case::Default(ref __binding_0) => __binding_0.accept(v),
            Case::Case(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Catch<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_catch(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Catch(ref __binding_0, ref __binding_1, ref __binding_2) => {
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
impl<'a> Node<'a> for ClassAbstractTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_abstract_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassAbstractTypeconst {
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
impl<'a> Node<'a> for ClassAttr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_attr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassAttr::CAName(ref __binding_0) => __binding_0.accept(v),
            ClassAttr::CAField(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ClassConcreteTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_concrete_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConcreteTypeconst {
                c_tc_type: ref __binding_0,
            } => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a>
    for ClassConst<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_const(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConst {
                type_: ref __binding_0,
                id: ref __binding_1,
                kind: ref __binding_2,
                doc_comment: ref __binding_3,
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
impl<'a> Node<'a>
    for ClassConstKind<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_const_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassConstKind::CCAbstract(ref __binding_0) => __binding_0.accept(v),
            ClassConstKind::CCConcrete(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a>
    for ClassGetExpr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_get_expr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassGetExpr::CGstring(ref __binding_0) => __binding_0.accept(v),
            ClassGetExpr::CGexpr(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for ClassId<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_id(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassId(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for ClassId_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_id_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassId_::CIparent => {}
            ClassId_::CIself => {}
            ClassId_::CIstatic => {}
            ClassId_::CIexpr(ref __binding_0) => __binding_0.accept(v),
            ClassId_::CI(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for ClassPartiallyAbstractTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_partially_abstract_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassPartiallyAbstractTypeconst {
                constraint: ref __binding_0,
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
impl<'a> Node<'a> for ClassTypeconst<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_typeconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassTypeconst::TCAbstract(ref __binding_0) => __binding_0.accept(v),
            ClassTypeconst::TCConcrete(ref __binding_0) => __binding_0.accept(v),
            ClassTypeconst::TCPartiallyAbstract(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a>
    for ClassTypeconstDef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_typeconst_def(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassTypeconstDef {
                user_attributes: ref __binding_0,
                name: ref __binding_1,
                kind: ref __binding_2,
                span: ref __binding_3,
                doc_comment: ref __binding_4,
                is_ctx: ref __binding_5,
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
impl<'a> Node<'a> for ClassVar<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_var(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ClassVar {
                final_: ref __binding_0,
                xhp_attr: ref __binding_1,
                abstract_: ref __binding_2,
                readonly: ref __binding_3,
                visibility: ref __binding_4,
                type_: ref __binding_5,
                id: ref __binding_6,
                expr: ref __binding_7,
                user_attributes: ref __binding_8,
                doc_comment: ref __binding_9,
                is_promoted_variadic: ref __binding_10,
                is_static: ref __binding_11,
                span: ref __binding_12,
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
                { __binding_12.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Class_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_class_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Class_ {
                span: ref __binding_0,
                annotation: ref __binding_1,
                mode: ref __binding_2,
                final_: ref __binding_3,
                is_xhp: ref __binding_4,
                has_xhp_keyword: ref __binding_5,
                kind: ref __binding_6,
                name: ref __binding_7,
                tparams: ref __binding_8,
                extends: ref __binding_9,
                uses: ref __binding_10,
                use_as_alias: ref __binding_11,
                insteadof_alias: ref __binding_12,
                xhp_attr_uses: ref __binding_13,
                xhp_category: ref __binding_14,
                reqs: ref __binding_15,
                implements: ref __binding_16,
                support_dynamic_type: ref __binding_17,
                where_constraints: ref __binding_18,
                consts: ref __binding_19,
                typeconsts: ref __binding_20,
                vars: ref __binding_21,
                methods: ref __binding_22,
                attributes: ref __binding_23,
                xhp_children: ref __binding_24,
                xhp_attrs: ref __binding_25,
                namespace: ref __binding_26,
                user_attributes: ref __binding_27,
                file_attributes: ref __binding_28,
                enum_: ref __binding_29,
                doc_comment: ref __binding_30,
                emit_id: ref __binding_31,
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
                {
                    __binding_30.accept(v)
                }
                { __binding_31.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for CollectionTarg<'a, ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_collection_targ(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            CollectionTarg::CollectionTV(ref __binding_0) => __binding_0.accept(v),
            CollectionTarg::CollectionTKV(ref __binding_0) => __binding_0.accept(v),
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
impl<'a> Node<'a> for Contexts<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_contexts(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Contexts(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Def<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_def(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Def::Fun(ref __binding_0) => __binding_0.accept(v),
            Def::Class(ref __binding_0) => __binding_0.accept(v),
            Def::RecordDef(ref __binding_0) => __binding_0.accept(v),
            Def::Stmt(ref __binding_0) => __binding_0.accept(v),
            Def::Typedef(ref __binding_0) => __binding_0.accept(v),
            Def::Constant(ref __binding_0) => __binding_0.accept(v),
            Def::Namespace(ref __binding_0) => __binding_0.accept(v),
            Def::NamespaceUse(ref __binding_0) => __binding_0.accept(v),
            Def::SetNamespaceEnv(ref __binding_0) => __binding_0.accept(v),
            Def::FileAttributes(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for DocComment<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_doc_comment(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            DocComment(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for EmitId {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_emit_id(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            EmitId::EmitId(ref __binding_0) => __binding_0.accept(v),
            EmitId::Anonymous => {}
        }
    }
}
impl<'a> Node<'a> for Enum_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_enum_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Enum_ {
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
impl<'a> Node<'a> for Env<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_env(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Env {
                ns_uses: ref __binding_0,
                class_uses: ref __binding_1,
                record_def_uses: ref __binding_2,
                fun_uses: ref __binding_3,
                const_uses: ref __binding_4,
                name: ref __binding_5,
                auto_ns_map: ref __binding_6,
                is_codegen: ref __binding_7,
                disable_xhp_element_mangling: ref __binding_8,
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
impl<'a> Node<'a> for EnvAnnot {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_env_annot(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            EnvAnnot::Join => {}
            EnvAnnot::Refinement => {}
        }
    }
}
impl<'a> Node<'a> for Expr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_expr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Expr(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Expr_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_expr_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Expr_::Darray(ref __binding_0) => __binding_0.accept(v),
            Expr_::Varray(ref __binding_0) => __binding_0.accept(v),
            Expr_::Shape(ref __binding_0) => __binding_0.accept(v),
            Expr_::ValCollection(ref __binding_0) => __binding_0.accept(v),
            Expr_::KeyValCollection(ref __binding_0) => __binding_0.accept(v),
            Expr_::Null => {}
            Expr_::This => {}
            Expr_::True => {}
            Expr_::False => {}
            Expr_::Omitted => {}
            Expr_::Id(ref __binding_0) => __binding_0.accept(v),
            Expr_::Lvar(ref __binding_0) => __binding_0.accept(v),
            Expr_::Dollardollar(ref __binding_0) => __binding_0.accept(v),
            Expr_::Clone(ref __binding_0) => __binding_0.accept(v),
            Expr_::ArrayGet(ref __binding_0) => __binding_0.accept(v),
            Expr_::ObjGet(ref __binding_0) => __binding_0.accept(v),
            Expr_::ClassGet(ref __binding_0) => __binding_0.accept(v),
            Expr_::ClassConst(ref __binding_0) => __binding_0.accept(v),
            Expr_::Call(ref __binding_0) => __binding_0.accept(v),
            Expr_::FunctionPointer(ref __binding_0) => __binding_0.accept(v),
            Expr_::Int(ref __binding_0) => __binding_0.accept(v),
            Expr_::Float(ref __binding_0) => __binding_0.accept(v),
            Expr_::String(ref __binding_0) => __binding_0.accept(v),
            Expr_::String2(ref __binding_0) => __binding_0.accept(v),
            Expr_::PrefixedString(ref __binding_0) => __binding_0.accept(v),
            Expr_::Yield(ref __binding_0) => __binding_0.accept(v),
            Expr_::Await(ref __binding_0) => __binding_0.accept(v),
            Expr_::ReadonlyExpr(ref __binding_0) => __binding_0.accept(v),
            Expr_::Tuple(ref __binding_0) => __binding_0.accept(v),
            Expr_::List(ref __binding_0) => __binding_0.accept(v),
            Expr_::Cast(ref __binding_0) => __binding_0.accept(v),
            Expr_::Unop(ref __binding_0) => __binding_0.accept(v),
            Expr_::Binop(ref __binding_0) => __binding_0.accept(v),
            Expr_::Pipe(ref __binding_0) => __binding_0.accept(v),
            Expr_::Eif(ref __binding_0) => __binding_0.accept(v),
            Expr_::Is(ref __binding_0) => __binding_0.accept(v),
            Expr_::As(ref __binding_0) => __binding_0.accept(v),
            Expr_::New(ref __binding_0) => __binding_0.accept(v),
            Expr_::Record(ref __binding_0) => __binding_0.accept(v),
            Expr_::Efun(ref __binding_0) => __binding_0.accept(v),
            Expr_::Lfun(ref __binding_0) => __binding_0.accept(v),
            Expr_::Xml(ref __binding_0) => __binding_0.accept(v),
            Expr_::Callconv(ref __binding_0) => __binding_0.accept(v),
            Expr_::Import(ref __binding_0) => __binding_0.accept(v),
            Expr_::Collection(ref __binding_0) => __binding_0.accept(v),
            Expr_::ExpressionTree(ref __binding_0) => __binding_0.accept(v),
            Expr_::Lplaceholder(ref __binding_0) => __binding_0.accept(v),
            Expr_::FunId(ref __binding_0) => __binding_0.accept(v),
            Expr_::MethodId(ref __binding_0) => __binding_0.accept(v),
            Expr_::MethodCaller(ref __binding_0) => __binding_0.accept(v),
            Expr_::SmethodId(ref __binding_0) => __binding_0.accept(v),
            Expr_::Pair(ref __binding_0) => __binding_0.accept(v),
            Expr_::ETSplice(ref __binding_0) => __binding_0.accept(v),
            Expr_::EnumClassLabel(ref __binding_0) => __binding_0.accept(v),
            Expr_::Hole(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a>
    for ExpressionTree<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_expression_tree(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ExpressionTree {
                hint: ref __binding_0,
                splices: ref __binding_1,
                virtualized_expr: ref __binding_2,
                runtime_expr: ref __binding_3,
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
impl<'a> Node<'a> for Field<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_field(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Field(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a>
    for FileAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_file_attribute(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FileAttribute {
                user_attributes: ref __binding_0,
                namespace: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for FunDef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_def(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunDef {
                namespace: ref __binding_0,
                file_attributes: ref __binding_1,
                mode: ref __binding_2,
                fun: ref __binding_3,
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
impl<'a> Node<'a> for FunParam<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_param(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunParam {
                annotation: ref __binding_0,
                type_hint: ref __binding_1,
                is_variadic: ref __binding_2,
                pos: ref __binding_3,
                name: ref __binding_4,
                expr: ref __binding_5,
                readonly: ref __binding_6,
                callconv: ref __binding_7,
                user_attributes: ref __binding_8,
                visibility: ref __binding_9,
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
impl<'a> Node<'a>
    for FunVariadicity<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_variadicity(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunVariadicity::FVvariadicArg(ref __binding_0) => __binding_0.accept(v),
            FunVariadicity::FVellipsis(ref __binding_0) => __binding_0.accept(v),
            FunVariadicity::FVnonVariadic => {}
        }
    }
}
impl<'a> Node<'a> for Fun_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_fun_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Fun_ {
                span: ref __binding_0,
                readonly_this: ref __binding_1,
                annotation: ref __binding_2,
                readonly_ret: ref __binding_3,
                ret: ref __binding_4,
                name: ref __binding_5,
                tparams: ref __binding_6,
                where_constraints: ref __binding_7,
                variadic: ref __binding_8,
                params: ref __binding_9,
                ctxs: ref __binding_10,
                unsafe_ctxs: ref __binding_11,
                body: ref __binding_12,
                fun_kind: ref __binding_13,
                user_attributes: ref __binding_14,
                external: ref __binding_15,
                doc_comment: ref __binding_16,
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
                { __binding_16.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for FuncBody<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_func_body(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FuncBody {
                ast: ref __binding_0,
                annotation: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a>
    for FunctionPtrId<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_function_ptr_id(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            FunctionPtrId::FPId(ref __binding_0) => __binding_0.accept(v),
            FunctionPtrId::FPClassConst(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Gconst<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_gconst(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Gconst {
                annotation: ref __binding_0,
                mode: ref __binding_1,
                name: ref __binding_2,
                type_: ref __binding_3,
                value: ref __binding_4,
                namespace: ref __binding_5,
                span: ref __binding_6,
                emit_id: ref __binding_7,
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
impl<'a> Node<'a> for HfParamInfo {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_hf_param_info(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            HfParamInfo {
                kind: ref __binding_0,
                readonlyness: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Hint<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_hint(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Hint(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for HintFun<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_hint_fun(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            HintFun {
                is_readonly: ref __binding_0,
                param_tys: ref __binding_1,
                param_info: ref __binding_2,
                variadic_ty: ref __binding_3,
                ctxs: ref __binding_4,
                return_ty: ref __binding_5,
                is_readonly_return: ref __binding_6,
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
impl<'a> Node<'a> for Hint_<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_hint_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Hint_::Hoption(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hlike(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hfun(ref __binding_0) => __binding_0.accept(v),
            Hint_::Htuple(ref __binding_0) => __binding_0.accept(v),
            Hint_::Happly(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hshape(ref __binding_0) => __binding_0.accept(v),
            Hint_::Haccess(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hsoft(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hany => {}
            Hint_::Herr => {}
            Hint_::Hmixed => {}
            Hint_::Hnonnull => {}
            Hint_::Habstr(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hdarray(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hvarray(ref __binding_0) => __binding_0.accept(v),
            Hint_::HvarrayOrDarray(ref __binding_0) => __binding_0.accept(v),
            Hint_::HvecOrDict(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hprim(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hthis => {}
            Hint_::Hdynamic => {}
            Hint_::Hnothing => {}
            Hint_::Hunion(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hintersection(ref __binding_0) => __binding_0.accept(v),
            Hint_::HfunContext(ref __binding_0) => __binding_0.accept(v),
            Hint_::Hvar(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for HoleSource {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_hole_source(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            HoleSource::Typing => {}
            HoleSource::UnsafeCast => {}
            HoleSource::EnforcedCast => {}
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
impl<'a> Node<'a> for ImportFlavor {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_import_flavor(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ImportFlavor::Include => {}
            ImportFlavor::Require => {}
            ImportFlavor::IncludeOnce => {}
            ImportFlavor::RequireOnce => {}
        }
    }
}
impl<'a> Node<'a> for InsteadofAlias<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_insteadof_alias(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            InsteadofAlias(ref __binding_0, ref __binding_1, ref __binding_2) => {
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
impl<'a> Node<'a> for KvcKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_kvc_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            KvcKind::Map => {}
            KvcKind::ImmMap => {}
            KvcKind::Dict => {}
        }
    }
}
impl<'a> Node<'a> for Lid<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_lid(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Lid(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Method_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_method_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Method_ {
                span: ref __binding_0,
                annotation: ref __binding_1,
                final_: ref __binding_2,
                abstract_: ref __binding_3,
                static_: ref __binding_4,
                readonly_this: ref __binding_5,
                visibility: ref __binding_6,
                name: ref __binding_7,
                tparams: ref __binding_8,
                where_constraints: ref __binding_9,
                variadic: ref __binding_10,
                params: ref __binding_11,
                ctxs: ref __binding_12,
                unsafe_ctxs: ref __binding_13,
                body: ref __binding_14,
                fun_kind: ref __binding_15,
                user_attributes: ref __binding_16,
                readonly_ret: ref __binding_17,
                ret: ref __binding_18,
                external: ref __binding_19,
                doc_comment: ref __binding_20,
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
                { __binding_20.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for NastShapeInfo<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_nast_shape_info(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            NastShapeInfo {
                allows_unknown_fields: ref __binding_0,
                field_map: ref __binding_1,
            } => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for NsKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_ns_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            NsKind::NSNamespace => {}
            NsKind::NSClass => {}
            NsKind::NSClassAndNamespace => {}
            NsKind::NSFun => {}
            NsKind::NSConst => {}
        }
    }
}
impl<'a> Node<'a> for OgNullFlavor {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_og_null_flavor(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            OgNullFlavor::OGNullthrows => {}
            OgNullFlavor::OGNullsafe => {}
        }
    }
}
impl<'a> Node<'a> for ParamKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_param_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ParamKind::Pinout => {}
        }
    }
}
impl<'a> Node<'a> for ReadonlyKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_readonly_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ReadonlyKind::Readonly => {}
        }
    }
}
impl<'a> Node<'a> for RecordDef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_record_def(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            RecordDef {
                annotation: ref __binding_0,
                name: ref __binding_1,
                extends: ref __binding_2,
                abstract_: ref __binding_3,
                fields: ref __binding_4,
                user_attributes: ref __binding_5,
                namespace: ref __binding_6,
                span: ref __binding_7,
                doc_comment: ref __binding_8,
                emit_id: ref __binding_9,
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
impl<'a> Node<'a> for ShapeFieldInfo<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_shape_field_info(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            ShapeFieldInfo {
                optional: ref __binding_0,
                hint: ref __binding_1,
                name: ref __binding_2,
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
impl<'a> Node<'a> for Stmt<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_stmt(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Stmt(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Stmt_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_stmt_(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Stmt_::Fallthrough => {}
            Stmt_::Expr(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Break => {}
            Stmt_::Continue => {}
            Stmt_::Throw(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Return(ref __binding_0) => __binding_0.accept(v),
            Stmt_::YieldBreak => {}
            Stmt_::Awaitall(ref __binding_0) => __binding_0.accept(v),
            Stmt_::If(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Do(ref __binding_0) => __binding_0.accept(v),
            Stmt_::While(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Using(ref __binding_0) => __binding_0.accept(v),
            Stmt_::For(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Switch(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Foreach(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Try(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Noop => {}
            Stmt_::Block(ref __binding_0) => __binding_0.accept(v),
            Stmt_::Markup(ref __binding_0) => __binding_0.accept(v),
            Stmt_::AssertEnv(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for Targ<'a, ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_targ(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Targ(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Tparam<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_tparam(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Tparam {
                variance: ref __binding_0,
                name: ref __binding_1,
                parameters: ref __binding_2,
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
impl<'a> Node<'a> for TypeHint<'a, ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_type_hint(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            TypeHint(ref __binding_0, ref __binding_1) => {
                {
                    __binding_0.accept(v)
                }
                { __binding_1.accept(v) }
            }
        }
    }
}
impl<'a> Node<'a> for Typedef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_typedef(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Typedef {
                annotation: ref __binding_0,
                name: ref __binding_1,
                tparams: ref __binding_2,
                constraint: ref __binding_3,
                kind: ref __binding_4,
                user_attributes: ref __binding_5,
                mode: ref __binding_6,
                vis: ref __binding_7,
                namespace: ref __binding_8,
                span: ref __binding_9,
                emit_id: ref __binding_10,
                is_ctx: ref __binding_11,
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
                { __binding_11.accept(v) }
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
            TypedefVisibility::Tinternal => {}
        }
    }
}
impl<'a> Node<'a> for Uop {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_uop(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            Uop::Utild => {}
            Uop::Unot => {}
            Uop::Uplus => {}
            Uop::Uminus => {}
            Uop::Uincr => {}
            Uop::Udecr => {}
            Uop::Upincr => {}
            Uop::Updecr => {}
            Uop::Usilence => {}
        }
    }
}
impl<'a> Node<'a> for UseAsAlias<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_use_as_alias(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            UseAsAlias(ref __binding_0, ref __binding_1, ref __binding_2, ref __binding_3) => {
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
impl<'a> Node<'a> for UseAsVisibility {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_use_as_visibility(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            UseAsVisibility::UseAsPublic => {}
            UseAsVisibility::UseAsPrivate => {}
            UseAsVisibility::UseAsProtected => {}
            UseAsVisibility::UseAsFinal => {}
        }
    }
}
impl<'a> Node<'a>
    for UserAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
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
impl<'a> Node<'a> for UsingStmt<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_using_stmt(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            UsingStmt {
                is_block_scoped: ref __binding_0,
                has_await: ref __binding_1,
                exprs: ref __binding_2,
                block: ref __binding_3,
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
impl<'a> Node<'a> for VcKind {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_vc_kind(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            VcKind::Vector => {}
            VcKind::ImmVector => {}
            VcKind::Vec => {}
            VcKind::Set => {}
            VcKind::ImmSet => {}
            VcKind::Keyset => {}
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
impl<'a> Node<'a> for WhereConstraintHint<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_where_constraint_hint(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            WhereConstraintHint(ref __binding_0, ref __binding_1, ref __binding_2) => {
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
impl<'a> Node<'a> for XhpAttr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attr(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttr(ref __binding_0, ref __binding_1, ref __binding_2, ref __binding_3) => {
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
impl<'a> Node<'a> for XhpAttrInfo<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attr_info(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttrInfo {
                tag: ref __binding_0,
                enum_values: ref __binding_1,
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
            XhpAttrTag::LateInit => {}
        }
    }
}
impl<'a> Node<'a>
    for XhpAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>
{
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_attribute(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpAttribute::XhpSimple(ref __binding_0) => __binding_0.accept(v),
            XhpAttribute::XhpSpread(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for XhpChild<'a> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_child(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpChild::ChildName(ref __binding_0) => __binding_0.accept(v),
            XhpChild::ChildList(ref __binding_0) => __binding_0.accept(v),
            XhpChild::ChildUnary(ref __binding_0) => __binding_0.accept(v),
            XhpChild::ChildBinary(ref __binding_0) => __binding_0.accept(v),
        }
    }
}
impl<'a> Node<'a> for XhpChildOp {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_child_op(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpChildOp::ChildStar => {}
            XhpChildOp::ChildPlus => {}
            XhpChildOp::ChildQuestion => {}
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
impl<'a> Node<'a> for XhpSimple<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()> {
    fn accept(&'a self, v: &mut dyn Visitor<'a>) {
        v.visit_xhp_simple(self)
    }
    fn recurse(&'a self, v: &mut dyn Visitor<'a>) {
        match self {
            XhpSimple {
                name: ref __binding_0,
                type_: ref __binding_1,
                expr: ref __binding_2,
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
