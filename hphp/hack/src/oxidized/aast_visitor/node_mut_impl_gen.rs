// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<64ff959f246d2d21fea6c829d4dc13f9>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node_mut::NodeMut;
use super::visitor_mut::VisitorMut;
use crate::aast::*;
use crate::aast_defs::*;
use crate::ast_defs::*;
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Afield<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_afield(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Afield::AFvalue(a0) => {
                a0.accept(c, v);
            }
            Afield::AFkvalue(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for AsExpr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_as_expr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            AsExpr::AsV(a0) => {
                a0.accept(c, v);
            }
            AsExpr::AsKv(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            AsExpr::AwaitAsV(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            AsExpr::AwaitAsKv(a0, a1, a2) => {
                a0.accept(c, v);
                a1.accept(c, v);
                a2.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for AssertExpr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_assert_expr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            AssertExpr::AEAssert(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Bop {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_bop(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
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
            Bop::LogXor => {}
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
            Bop::Eq(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for CaField<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ca_field(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.type_.accept(c, v);
        self.id.accept(c, v);
        self.value.accept(c, v);
        self.required.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for CaType {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ca_type(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            CaType::CAHint(a0) => {
                a0.accept(c, v);
            }
            CaType::CAEnum(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for CallType {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_call_type(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            CallType::Cnormal => {}
            CallType::CuserFunc => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Case<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_case(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Case::Default(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Case::Case(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Catch<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_catch(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
        self.2.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassAttr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_attr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ClassAttr::CAName(a0) => {
                a0.accept(c, v);
            }
            ClassAttr::CAField(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassConst<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_const(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.type_.accept(c, v);
        self.id.accept(c, v);
        self.expr.accept(c, v);
        self.doc_comment.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassGetExpr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_get_expr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ClassGetExpr::CGstring(a0) => {
                a0.accept(c, v);
            }
            ClassGetExpr::CGexpr(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassId<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_id(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ex(c, &mut self.0);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassId_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_id_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ClassId_::CIparent => {}
            ClassId_::CIself => {}
            ClassId_::CIstatic => {}
            ClassId_::CIexpr(a0) => {
                a0.accept(c, v);
            }
            ClassId_::CI(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ClassKind::Cabstract => {}
            ClassKind::Cnormal => {}
            ClassKind::Cinterface => {}
            ClassKind::Ctrait => {}
            ClassKind::Cenum => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassTparams<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_tparams(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.list.accept(c, v);
        self.constraints.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassTypeconst<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_typeconst(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.abstract_.accept(c, v);
        self.name.accept(c, v);
        self.constraint.accept(c, v);
        self.type_.accept(c, v);
        self.user_attributes.accept(c, v);
        self.span.accept(c, v);
        self.doc_comment.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ClassVar<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_var(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.final_.accept(c, v);
        self.xhp_attr.accept(c, v);
        self.abstract_.accept(c, v);
        self.visibility.accept(c, v);
        self.type_.accept(c, v);
        self.id.accept(c, v);
        self.expr.accept(c, v);
        self.user_attributes.accept(c, v);
        self.doc_comment.accept(c, v);
        self.is_promoted_variadic.accept(c, v);
        self.is_static.accept(c, v);
        self.span.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Class_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_class_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.span.accept(c, v);
        v.visit_en(c, &mut self.annotation);
        self.mode.accept(c, v);
        self.final_.accept(c, v);
        self.is_xhp.accept(c, v);
        self.kind.accept(c, v);
        self.name.accept(c, v);
        self.tparams.accept(c, v);
        self.extends.accept(c, v);
        self.uses.accept(c, v);
        self.use_as_alias.accept(c, v);
        self.insteadof_alias.accept(c, v);
        self.method_redeclarations.accept(c, v);
        self.xhp_attr_uses.accept(c, v);
        self.xhp_category.accept(c, v);
        self.reqs.accept(c, v);
        self.implements.accept(c, v);
        self.where_constraints.accept(c, v);
        self.consts.accept(c, v);
        self.typeconsts.accept(c, v);
        self.vars.accept(c, v);
        self.methods.accept(c, v);
        self.attributes.accept(c, v);
        self.xhp_children.accept(c, v);
        self.xhp_attrs.accept(c, v);
        self.namespace.accept(c, v);
        self.user_attributes.accept(c, v);
        self.file_attributes.accept(c, v);
        self.enum_.accept(c, v);
        self.pu_enums.accept(c, v);
        self.doc_comment.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for CollectionTarg<Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_collection_targ(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            CollectionTarg::CollectionTV(a0) => {
                a0.accept(c, v);
            }
            CollectionTarg::CollectionTKV(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ConstraintKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_constraint_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ConstraintKind::ConstraintAs => {}
            ConstraintKind::ConstraintEq => {}
            ConstraintKind::ConstraintSuper => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Def<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_def(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Def::Fun(a0) => {
                a0.accept(c, v);
            }
            Def::Class(a0) => {
                a0.accept(c, v);
            }
            Def::RecordDef(a0) => {
                a0.accept(c, v);
            }
            Def::Stmt(a0) => {
                a0.accept(c, v);
            }
            Def::Typedef(a0) => {
                a0.accept(c, v);
            }
            Def::Constant(a0) => {
                a0.accept(c, v);
            }
            Def::Namespace(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Def::NamespaceUse(a0) => {
                a0.accept(c, v);
            }
            Def::SetNamespaceEnv(a0) => {
                a0.accept(c, v);
            }
            Def::FileAttributes(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Enum_ {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_enum_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.base.accept(c, v);
        self.constraint.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Expr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_expr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ex(c, &mut self.0);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Expr_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_expr_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Expr_::Array(a0) => {
                a0.accept(c, v);
            }
            Expr_::Darray(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Varray(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Shape(a0) => {
                a0.accept(c, v);
            }
            Expr_::ValCollection(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::KeyValCollection(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Null => {}
            Expr_::This => {}
            Expr_::True => {}
            Expr_::False => {}
            Expr_::Omitted => {}
            Expr_::Id(a0) => {
                a0.accept(c, v);
            }
            Expr_::Lvar(a0) => {
                a0.accept(c, v);
            }
            Expr_::ImmutableVar(a0) => {
                a0.accept(c, v);
            }
            Expr_::Dollardollar(a0) => {
                a0.accept(c, v);
            }
            Expr_::Clone(a0) => {
                a0.accept(c, v);
            }
            Expr_::ObjGet(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::ArrayGet(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::ClassGet(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::ClassConst(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Call(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
                a.3.accept(c, v);
                a.4.accept(c, v);
            }
            Expr_::Int(a0) => {
                a0.accept(c, v);
            }
            Expr_::Float(a0) => {
                a0.accept(c, v);
            }
            Expr_::String(a0) => {
                a0.accept(c, v);
            }
            Expr_::String2(a0) => {
                a0.accept(c, v);
            }
            Expr_::PrefixedString(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Yield(a0) => {
                a0.accept(c, v);
            }
            Expr_::YieldBreak => {}
            Expr_::YieldFrom(a0) => {
                a0.accept(c, v);
            }
            Expr_::Await(a0) => {
                a0.accept(c, v);
            }
            Expr_::Suspend(a0) => {
                a0.accept(c, v);
            }
            Expr_::List(a0) => {
                a0.accept(c, v);
            }
            Expr_::ExprList(a0) => {
                a0.accept(c, v);
            }
            Expr_::Cast(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Unop(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Binop(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Pipe(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Eif(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Is(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::As(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::New(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
                a.3.accept(c, v);
                v.visit_ex(c, &mut a.4);
            }
            Expr_::Record(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Efun(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Lfun(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Xml(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Callconv(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Import(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Collection(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::BracedExpr(a0) => {
                a0.accept(c, v);
            }
            Expr_::ParenthesizedExpr(a0) => {
                a0.accept(c, v);
            }
            Expr_::Lplaceholder(a0) => {
                a0.accept(c, v);
            }
            Expr_::FunId(a0) => {
                a0.accept(c, v);
            }
            Expr_::MethodId(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::MethodCaller(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::SmethodId(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::SpecialFunc(a0) => {
                a0.accept(c, v);
            }
            Expr_::Pair(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Expr_::Assert(a0) => {
                a0.accept(c, v);
            }
            Expr_::Typename(a0) => {
                a0.accept(c, v);
            }
            Expr_::PUAtom(a0) => {
                a0.accept(c, v);
            }
            Expr_::PUIdentifier(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Expr_::Any => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Field<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_field(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FileAttribute<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_file_attribute(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.user_attributes.accept(c, v);
        self.namespace.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FunKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_fun_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            FunKind::FSync => {}
            FunKind::FAsync => {}
            FunKind::FGenerator => {}
            FunKind::FAsyncGenerator => {}
            FunKind::FCoroutine => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FunParam<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_fun_param(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ex(c, &mut self.annotation);
        self.type_hint.accept(c, v);
        self.is_reference.accept(c, v);
        self.is_variadic.accept(c, v);
        self.pos.accept(c, v);
        self.name.accept(c, v);
        self.expr.accept(c, v);
        self.callconv.accept(c, v);
        self.user_attributes.accept(c, v);
        self.visibility.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FunVariadicity<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_fun_variadicity(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            FunVariadicity::FVvariadicArg(a0) => {
                a0.accept(c, v);
            }
            FunVariadicity::FVellipsis(a0) => {
                a0.accept(c, v);
            }
            FunVariadicity::FVnonVariadic => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Fun_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_fun_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.span.accept(c, v);
        v.visit_en(c, &mut self.annotation);
        self.mode.accept(c, v);
        self.ret.accept(c, v);
        self.name.accept(c, v);
        self.tparams.accept(c, v);
        self.where_constraints.accept(c, v);
        self.variadic.accept(c, v);
        self.params.accept(c, v);
        self.body.accept(c, v);
        self.fun_kind.accept(c, v);
        self.user_attributes.accept(c, v);
        self.file_attributes.accept(c, v);
        self.external.accept(c, v);
        self.namespace.accept(c, v);
        self.doc_comment.accept(c, v);
        self.static_.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FuncBody<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_func_body(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.ast.accept(c, v);
        v.visit_fb(c, &mut self.annotation);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for FuncReactive {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_func_reactive(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            FuncReactive::FReactive => {}
            FuncReactive::FLocal => {}
            FuncReactive::FShallow => {}
            FuncReactive::FNonreactive => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Gconst<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_gconst(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_en(c, &mut self.annotation);
        self.mode.accept(c, v);
        self.name.accept(c, v);
        self.type_.accept(c, v);
        self.value.accept(c, v);
        self.namespace.accept(c, v);
        self.span.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Hint {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_hint(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for HintFun {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_hint_fun(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.reactive_kind.accept(c, v);
        self.is_coroutine.accept(c, v);
        self.param_tys.accept(c, v);
        self.param_kinds.accept(c, v);
        self.param_mutability.accept(c, v);
        self.variadic_ty.accept(c, v);
        self.return_ty.accept(c, v);
        self.is_mutable_return.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Hint_ {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_hint_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Hint_::Hoption(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hlike(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hfun(a0) => {
                a0.accept(c, v);
            }
            Hint_::Htuple(a0) => {
                a0.accept(c, v);
            }
            Hint_::Happly(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Hint_::Hshape(a0) => {
                a0.accept(c, v);
            }
            Hint_::Haccess(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Hint_::Hsoft(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hany => {}
            Hint_::Herr => {}
            Hint_::Hmixed => {}
            Hint_::Hnonnull => {}
            Hint_::Habstr(a0) => {
                a0.accept(c, v);
            }
            Hint_::Harray(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Hint_::Hdarray(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Hint_::Hvarray(a0) => {
                a0.accept(c, v);
            }
            Hint_::HvarrayOrDarray(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hprim(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hthis => {}
            Hint_::Hdynamic => {}
            Hint_::Hnothing => {}
            Hint_::HpuAccess(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            Hint_::Hunion(a0) => {
                a0.accept(c, v);
            }
            Hint_::Hintersection(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Id {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_id(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ImportFlavor {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_import_flavor(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ImportFlavor::Include => {}
            ImportFlavor::Require => {}
            ImportFlavor::IncludeOnce => {}
            ImportFlavor::RequireOnce => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for InsteadofAlias {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_insteadof_alias(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
        self.2.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for KvcKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_kvc_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            KvcKind::Map => {}
            KvcKind::ImmMap => {}
            KvcKind::Dict => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Lid {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_lid(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi>
    for MethodRedeclaration<Ex, Fb, En, Hi>
{
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_method_redeclaration(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.final_.accept(c, v);
        self.abstract_.accept(c, v);
        self.static_.accept(c, v);
        self.visibility.accept(c, v);
        self.name.accept(c, v);
        self.tparams.accept(c, v);
        self.where_constraints.accept(c, v);
        self.variadic.accept(c, v);
        self.params.accept(c, v);
        self.fun_kind.accept(c, v);
        self.ret.accept(c, v);
        self.trait_.accept(c, v);
        self.method.accept(c, v);
        self.user_attributes.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Method_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_method_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.span.accept(c, v);
        v.visit_en(c, &mut self.annotation);
        self.final_.accept(c, v);
        self.abstract_.accept(c, v);
        self.static_.accept(c, v);
        self.visibility.accept(c, v);
        self.name.accept(c, v);
        self.tparams.accept(c, v);
        self.where_constraints.accept(c, v);
        self.variadic.accept(c, v);
        self.params.accept(c, v);
        self.body.accept(c, v);
        self.fun_kind.accept(c, v);
        self.user_attributes.accept(c, v);
        self.ret.accept(c, v);
        self.external.accept(c, v);
        self.doc_comment.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for NastShapeInfo {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_nast_shape_info(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.allows_unknown_fields.accept(c, v);
        self.field_map.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for NsKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_ns_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            NsKind::NSNamespace => {}
            NsKind::NSClass => {}
            NsKind::NSClassAndNamespace => {}
            NsKind::NSFun => {}
            NsKind::NSConst => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for OgNullFlavor {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_og_null_flavor(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            OgNullFlavor::OGNullthrows => {}
            OgNullFlavor::OGNullsafe => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ParamKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_param_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ParamKind::Pinout => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ParamMutability {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_param_mutability(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ParamMutability::PMutable => {}
            ParamMutability::POwnedMutable => {}
            ParamMutability::PMaybeMutable => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for PuEnum<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_pu_enum(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.name.accept(c, v);
        self.is_final.accept(c, v);
        self.case_types.accept(c, v);
        self.case_values.accept(c, v);
        self.members.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for PuMember<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_pu_member(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.atom.accept(c, v);
        self.types.accept(c, v);
        self.exprs.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for RecordDef<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_record_def(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_en(c, &mut self.annotation);
        self.name.accept(c, v);
        self.extends.accept(c, v);
        self.abstract_.accept(c, v);
        self.fields.accept(c, v);
        self.user_attributes.accept(c, v);
        self.namespace.accept(c, v);
        self.span.accept(c, v);
        self.doc_comment.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ReifyKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_reify_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ReifyKind::Erased => {}
            ReifyKind::SoftReified => {}
            ReifyKind::Reified => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ShapeFieldInfo {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_shape_field_info(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.optional.accept(c, v);
        self.hint.accept(c, v);
        self.name.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for ShapeFieldName {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_shape_field_name(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            ShapeFieldName::SFlitInt(a0) => {
                a0.accept(c, v);
            }
            ShapeFieldName::SFlitStr(a0) => {
                a0.accept(c, v);
            }
            ShapeFieldName::SFclassConst(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for SpecialFunc<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_special_func(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            SpecialFunc::Genva(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Stmt<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_stmt(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Stmt_<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_stmt_(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Stmt_::Fallthrough => {}
            Stmt_::Expr(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Break => {}
            Stmt_::Continue => {}
            Stmt_::Throw(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Return(a0) => {
                a0.accept(c, v);
            }
            Stmt_::GotoLabel(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Goto(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Awaitall(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Stmt_::If(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Stmt_::Do(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Stmt_::While(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Stmt_::Using(a0) => {
                a0.accept(c, v);
            }
            Stmt_::For(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
                a.3.accept(c, v);
            }
            Stmt_::Switch(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
            Stmt_::Foreach(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Stmt_::Try(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Stmt_::DefInline(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Let(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
                a.2.accept(c, v);
            }
            Stmt_::Noop => {}
            Stmt_::Block(a0) => {
                a0.accept(c, v);
            }
            Stmt_::Markup(a) => {
                a.0.accept(c, v);
                a.1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Targ<Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_targ(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_hi(c, &mut self.0);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Tparam<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_tparam(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.variance.accept(c, v);
        self.name.accept(c, v);
        self.constraints.accept(c, v);
        self.reified.accept(c, v);
        self.user_attributes.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Tprim {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_tprim(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
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
            Tprim::Tatom(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for TypeHint<Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_type_hint(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_hi(c, &mut self.0);
        self.1.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for TypeconstAbstractKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_typeconst_abstract_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            TypeconstAbstractKind::TCAbstract(a0) => {
                a0.accept(c, v);
            }
            TypeconstAbstractKind::TCPartiallyAbstract => {}
            TypeconstAbstractKind::TCConcrete => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Typedef<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_typedef(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_en(c, &mut self.annotation);
        self.name.accept(c, v);
        self.tparams.accept(c, v);
        self.constraint.accept(c, v);
        self.kind.accept(c, v);
        self.user_attributes.accept(c, v);
        self.mode.accept(c, v);
        self.vis.accept(c, v);
        self.namespace.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for TypedefVisibility {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_typedef_visibility(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            TypedefVisibility::Transparent => {}
            TypedefVisibility::Opaque => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Uop {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_uop(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
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
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for UseAsAlias {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_use_as_alias(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
        self.2.accept(c, v);
        self.3.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for UseAsVisibility {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_use_as_visibility(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            UseAsVisibility::UseAsPublic => {}
            UseAsVisibility::UseAsPrivate => {}
            UseAsVisibility::UseAsProtected => {}
            UseAsVisibility::UseAsFinal => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for UserAttribute<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_user_attribute(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.name.accept(c, v);
        self.params.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for UsingStmt<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_using_stmt(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.is_block_scoped.accept(c, v);
        self.has_await.accept(c, v);
        self.expr.accept(c, v);
        self.block.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Variance {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_variance(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Variance::Covariant => {}
            Variance::Contravariant => {}
            Variance::Invariant => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for VcKind {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_vc_kind(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            VcKind::Vector => {}
            VcKind::ImmVector => {}
            VcKind::Vec => {}
            VcKind::Set => {}
            VcKind::ImmSet => {}
            VcKind::Pair_ => {}
            VcKind::Keyset => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for Visibility {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_visibility(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            Visibility::Private => {}
            Visibility::Public => {}
            Visibility::Protected => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for WhereConstraint {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_where_constraint(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
        self.2.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpAttr<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_attr(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.0.accept(c, v);
        self.1.accept(c, v);
        self.2.accept(c, v);
        self.3.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpAttrInfo {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_attr_info(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        self.xai_tag.accept(c, v);
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpAttrTag {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_attr_tag(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            XhpAttrTag::Required => {}
            XhpAttrTag::LateInit => {}
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpAttribute<Ex, Fb, En, Hi> {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_attribute(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            XhpAttribute::XhpSimple(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            XhpAttribute::XhpSpread(a0) => {
                a0.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpChild {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_child(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            XhpChild::ChildName(a0) => {
                a0.accept(c, v);
            }
            XhpChild::ChildList(a0) => {
                a0.accept(c, v);
            }
            XhpChild::ChildUnary(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
            XhpChild::ChildBinary(a0, a1) => {
                a0.accept(c, v);
                a1.accept(c, v);
            }
        }
    }
}
impl<Context, Ex, Fb, En, Hi> NodeMut<Context, Ex, Fb, En, Hi> for XhpChildOp {
    fn accept(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        v.visit_xhp_child_op(c, self);
    }
    fn recurse(
        &mut self,
        c: &mut Context,
        v: &mut dyn VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) {
        match self {
            XhpChildOp::ChildStar => {}
            XhpChildOp::ChildPlus => {}
            XhpChildOp::ChildQuestion => {}
        }
    }
}
