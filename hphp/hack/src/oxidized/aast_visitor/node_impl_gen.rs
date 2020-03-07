// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<a73c5ab47fb26bd64b2239d7b8acd475>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node::Node;
use super::visitor::Visitor;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Afield<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_afield(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Afield::AFvalue(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Afield::AFkvalue(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for AsExpr<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_as_expr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            AsExpr::AsV(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            AsExpr::AsKv(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            AsExpr::AwaitAsV(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            AsExpr::AwaitAsKv(a0, a1, a2) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                a2.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for AssertExpr<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_assert_expr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            AssertExpr::AEAssert(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Bop {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_bop(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Bop::Plus => Ok(()),
            Bop::Minus => Ok(()),
            Bop::Star => Ok(()),
            Bop::Slash => Ok(()),
            Bop::Eqeq => Ok(()),
            Bop::Eqeqeq => Ok(()),
            Bop::Starstar => Ok(()),
            Bop::Diff => Ok(()),
            Bop::Diff2 => Ok(()),
            Bop::Ampamp => Ok(()),
            Bop::Barbar => Ok(()),
            Bop::LogXor => Ok(()),
            Bop::Lt => Ok(()),
            Bop::Lte => Ok(()),
            Bop::Gt => Ok(()),
            Bop::Gte => Ok(()),
            Bop::Dot => Ok(()),
            Bop::Amp => Ok(()),
            Bop::Bar => Ok(()),
            Bop::Ltlt => Ok(()),
            Bop::Gtgt => Ok(()),
            Bop::Percent => Ok(()),
            Bop::Xor => Ok(()),
            Bop::Cmp => Ok(()),
            Bop::QuestionQuestion => Ok(()),
            Bop::Eq(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for CaField<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ca_field(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.value.accept(c, v)?;
        self.required.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for CaType {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ca_type(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            CaType::CAHint(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            CaType::CAEnum(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for CallType {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_call_type(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            CallType::Cnormal => Ok(()),
            CallType::CuserFunc => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Case<Ex, Fb, En, Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_case(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Case::Default(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Case::Case(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Catch<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_catch(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassAttr<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_attr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ClassAttr::CAName(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ClassAttr::CAField(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassConst<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_const(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassGetExpr<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_get_expr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ClassGetExpr::CGstring(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ClassGetExpr::CGexpr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassId<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_id(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassId_<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_id_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ClassId_::CIparent => Ok(()),
            ClassId_::CIself => Ok(()),
            ClassId_::CIstatic => Ok(()),
            ClassId_::CIexpr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ClassId_::CI(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ClassKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ClassKind::Cabstract => Ok(()),
            ClassKind::Cnormal => Ok(()),
            ClassKind::Cinterface => Ok(()),
            ClassKind::Ctrait => Ok(()),
            ClassKind::Cenum => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassTparams<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_tparams(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.list.accept(c, v)?;
        self.constraints.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassTypeconst<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_typeconst(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.abstract_.accept(c, v)?;
        self.name.accept(c, v)?;
        self.constraint.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for ClassVar<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_var(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.final_.accept(c, v)?;
        self.xhp_attr.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.visibility.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.is_promoted_variadic.accept(c, v)?;
        self.is_static.accept(c, v)?;
        self.span.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Class_<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_class_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &self.annotation)?;
        self.mode.accept(c, v)?;
        self.final_.accept(c, v)?;
        self.is_xhp.accept(c, v)?;
        self.has_xhp_keyword.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.extends.accept(c, v)?;
        self.uses.accept(c, v)?;
        self.use_as_alias.accept(c, v)?;
        self.insteadof_alias.accept(c, v)?;
        self.method_redeclarations.accept(c, v)?;
        self.xhp_attr_uses.accept(c, v)?;
        self.xhp_category.accept(c, v)?;
        self.reqs.accept(c, v)?;
        self.implements.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.consts.accept(c, v)?;
        self.typeconsts.accept(c, v)?;
        self.vars.accept(c, v)?;
        self.methods.accept(c, v)?;
        self.attributes.accept(c, v)?;
        self.xhp_children.accept(c, v)?;
        self.xhp_attrs.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.enum_.accept(c, v)?;
        self.pu_enums.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for CollectionTarg<Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_collection_targ(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            CollectionTarg::CollectionTV(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            CollectionTarg::CollectionTKV(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ConstraintKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_constraint_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ConstraintKind::ConstraintAs => Ok(()),
            ConstraintKind::ConstraintEq => Ok(()),
            ConstraintKind::ConstraintSuper => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Def<Ex, Fb, En, Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_def(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Def::Fun(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::Class(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::RecordDef(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::Stmt(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::Typedef(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::Constant(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::Namespace(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Def::NamespaceUse(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::SetNamespaceEnv(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Def::FileAttributes(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for DocComment {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_doc_comment(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Enum_ {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_enum_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.base.accept(c, v)?;
        self.constraint.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Expr<Ex, Fb, En, Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_expr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Expr_<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_expr_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Expr_::Array(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Darray(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Varray(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Shape(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::ValCollection(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::KeyValCollection(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Null => Ok(()),
            Expr_::This => Ok(()),
            Expr_::True => Ok(()),
            Expr_::False => Ok(()),
            Expr_::Omitted => Ok(()),
            Expr_::Id(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Lvar(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Dollardollar(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Clone(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::ObjGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::ArrayGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::ClassGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::ClassConst(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Call(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                a.3.accept(c, v)?;
                a.4.accept(c, v)?;
                Ok(())
            }
            Expr_::FunctionPointer(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Int(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Float(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::String(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::String2(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::PrefixedString(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Yield(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::YieldBreak => Ok(()),
            Expr_::YieldFrom(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Await(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Suspend(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::List(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::ExprList(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Cast(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Unop(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Binop(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Pipe(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Eif(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Is(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::As(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::New(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                a.3.accept(c, v)?;
                v.visit_ex(c, &a.4)?;
                Ok(())
            }
            Expr_::Record(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Efun(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Lfun(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Xml(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Callconv(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Import(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Collection(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::BracedExpr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::ParenthesizedExpr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Lplaceholder(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::FunId(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::MethodId(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::MethodCaller(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::SmethodId(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Pair(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::Assert(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Typename(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::PUAtom(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::PUIdentifier(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::Any => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Field<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_field(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for FileAttribute<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_file_attribute(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.user_attributes.accept(c, v)?;
        self.namespace.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for FunKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_fun_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            FunKind::FSync => Ok(()),
            FunKind::FAsync => Ok(()),
            FunKind::FGenerator => Ok(()),
            FunKind::FAsyncGenerator => Ok(()),
            FunKind::FCoroutine => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for FunParam<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_fun_param(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ex(c, &self.annotation)?;
        self.type_hint.accept(c, v)?;
        self.is_variadic.accept(c, v)?;
        self.pos.accept(c, v)?;
        self.name.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.callconv.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.visibility.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for FunVariadicity<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_fun_variadicity(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            FunVariadicity::FVvariadicArg(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            FunVariadicity::FVellipsis(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            FunVariadicity::FVnonVariadic => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Fun_<Ex, Fb, En, Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_fun_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &self.annotation)?;
        self.mode.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.variadic.accept(c, v)?;
        self.params.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.external.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.static_.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for FuncBody<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_func_body(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.ast.accept(c, v)?;
        v.visit_fb(c, &self.annotation)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for FuncReactive {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_func_reactive(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            FuncReactive::FReactive => Ok(()),
            FuncReactive::FLocal => Ok(()),
            FuncReactive::FShallow => Ok(()),
            FuncReactive::FNonreactive => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Gconst<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_gconst(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_en(c, &self.annotation)?;
        self.mode.accept(c, v)?;
        self.name.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.value.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Hint {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_hint(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for HintFun {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_hint_fun(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.reactive_kind.accept(c, v)?;
        self.is_coroutine.accept(c, v)?;
        self.param_tys.accept(c, v)?;
        self.param_kinds.accept(c, v)?;
        self.param_mutability.accept(c, v)?;
        self.variadic_ty.accept(c, v)?;
        self.return_ty.accept(c, v)?;
        self.is_mutable_return.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Hint_ {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_hint_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Hint_::Hoption(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hlike(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hfun(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Htuple(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Happly(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Hint_::Hshape(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Haccess(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Hint_::Hsoft(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hany => Ok(()),
            Hint_::Herr => Ok(()),
            Hint_::Hmixed => Ok(()),
            Hint_::Hnonnull => Ok(()),
            Hint_::Habstr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Harray(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Hint_::Hdarray(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Hint_::Hvarray(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::HvarrayOrDarray(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            Hint_::Hprim(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hthis => Ok(()),
            Hint_::Hdynamic => Ok(()),
            Hint_::Hnothing => Ok(()),
            Hint_::HpuAccess(a0, a1, a2) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                a2.accept(c, v)?;
                Ok(())
            }
            Hint_::Hunion(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hintersection(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Id {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_id(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ImportFlavor {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_import_flavor(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ImportFlavor::Include => Ok(()),
            ImportFlavor::Require => Ok(()),
            ImportFlavor::IncludeOnce => Ok(()),
            ImportFlavor::RequireOnce => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for InsteadofAlias {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_insteadof_alias(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for KvcKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_kvc_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            KvcKind::Map => Ok(()),
            KvcKind::ImmMap => Ok(()),
            KvcKind::Dict => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Lid {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_lid(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for MethodRedeclaration<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_method_redeclaration(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.final_.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.static_.accept(c, v)?;
        self.visibility.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.variadic.accept(c, v)?;
        self.params.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.trait_.accept(c, v)?;
        self.method.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Method_<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_method_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &self.annotation)?;
        self.final_.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.static_.accept(c, v)?;
        self.visibility.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.variadic.accept(c, v)?;
        self.params.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.external.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for NastShapeInfo {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_nast_shape_info(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.allows_unknown_fields.accept(c, v)?;
        self.field_map.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for NsKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_ns_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            NsKind::NSNamespace => Ok(()),
            NsKind::NSClass => Ok(()),
            NsKind::NSClassAndNamespace => Ok(()),
            NsKind::NSFun => Ok(()),
            NsKind::NSConst => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for OgNullFlavor {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_og_null_flavor(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            OgNullFlavor::OGNullthrows => Ok(()),
            OgNullFlavor::OGNullsafe => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ParamKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_param_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ParamKind::Pinout => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ParamMutability {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_param_mutability(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ParamMutability::PMutable => Ok(()),
            ParamMutability::POwnedMutable => Ok(()),
            ParamMutability::PMaybeMutable => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for PuEnum<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_pu_enum(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_en(c, &self.annotation)?;
        self.name.accept(c, v)?;
        self.is_final.accept(c, v)?;
        self.case_types.accept(c, v)?;
        self.case_values.accept(c, v)?;
        self.members.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for PuLoc {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_pu_loc(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            PuLoc::Unknown => Ok(()),
            PuLoc::TypeParameter => Ok(()),
            PuLoc::Atom => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for PuMember<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_pu_member(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.atom.accept(c, v)?;
        self.types.accept(c, v)?;
        self.exprs.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for RecordDef<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_record_def(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_en(c, &self.annotation)?;
        self.name.accept(c, v)?;
        self.extends.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.fields.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ReifyKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_reify_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ReifyKind::Erased => Ok(()),
            ReifyKind::SoftReified => Ok(()),
            ReifyKind::Reified => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ShapeFieldInfo {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_shape_field_info(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.optional.accept(c, v)?;
        self.hint.accept(c, v)?;
        self.name.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for ShapeFieldName {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_shape_field_name(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            ShapeFieldName::SFlitInt(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ShapeFieldName::SFlitStr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ShapeFieldName::SFclassConst(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Stmt<Ex, Fb, En, Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_stmt(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Stmt_<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_stmt_(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Stmt_::Fallthrough => Ok(()),
            Stmt_::Expr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Break => Ok(()),
            Stmt_::Continue => Ok(()),
            Stmt_::Throw(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Return(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::GotoLabel(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Goto(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Awaitall(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Stmt_::If(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Stmt_::Do(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Stmt_::While(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Stmt_::Using(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::For(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                a.3.accept(c, v)?;
                Ok(())
            }
            Stmt_::Switch(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Stmt_::Foreach(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Stmt_::Try(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                Ok(())
            }
            Stmt_::DefInline(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Noop => Ok(()),
            Stmt_::Block(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Markup(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Targ<Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_targ(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_hi(c, &self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Tparam<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_tparam(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.variance.accept(c, v)?;
        self.name.accept(c, v)?;
        self.constraints.accept(c, v)?;
        self.reified.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Tprim {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_tprim(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Tprim::Tnull => Ok(()),
            Tprim::Tvoid => Ok(()),
            Tprim::Tint => Ok(()),
            Tprim::Tbool => Ok(()),
            Tprim::Tfloat => Ok(()),
            Tprim::Tstring => Ok(()),
            Tprim::Tresource => Ok(()),
            Tprim::Tnum => Ok(()),
            Tprim::Tarraykey => Ok(()),
            Tprim::Tnoreturn => Ok(()),
            Tprim::Tatom(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for TypeHint<Hi> {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_type_hint(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_hi(c, &self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for TypeconstAbstractKind
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_typeconst_abstract_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            TypeconstAbstractKind::TCAbstract(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            TypeconstAbstractKind::TCPartiallyAbstract => Ok(()),
            TypeconstAbstractKind::TCConcrete => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for Typedef<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_typedef(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_en(c, &self.annotation)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.constraint.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.mode.accept(c, v)?;
        self.vis.accept(c, v)?;
        self.namespace.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for TypedefVisibility {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_typedef_visibility(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            TypedefVisibility::Transparent => Ok(()),
            TypedefVisibility::Opaque => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Uop {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_uop(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Uop::Utild => Ok(()),
            Uop::Unot => Ok(()),
            Uop::Uplus => Ok(()),
            Uop::Uminus => Ok(()),
            Uop::Uincr => Ok(()),
            Uop::Udecr => Ok(()),
            Uop::Upincr => Ok(()),
            Uop::Updecr => Ok(()),
            Uop::Usilence => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for UseAsAlias {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_use_as_alias(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for UseAsVisibility {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_use_as_visibility(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            UseAsVisibility::UseAsPublic => Ok(()),
            UseAsVisibility::UseAsPrivate => Ok(()),
            UseAsVisibility::UseAsProtected => Ok(()),
            UseAsVisibility::UseAsFinal => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for UserAttribute<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_user_attribute(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.name.accept(c, v)?;
        self.params.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for UsingStmt<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_using_stmt(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.is_block_scoped.accept(c, v)?;
        self.has_await.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.block.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Variance {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_variance(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Variance::Covariant => Ok(()),
            Variance::Contravariant => Ok(()),
            Variance::Invariant => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for VcKind {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_vc_kind(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            VcKind::Vector => Ok(()),
            VcKind::ImmVector => Ok(()),
            VcKind::Vec => Ok(()),
            VcKind::Set => Ok(()),
            VcKind::ImmSet => Ok(()),
            VcKind::Pair_ => Ok(()),
            VcKind::Keyset => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for Visibility {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_visibility(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            Visibility::Private => Ok(()),
            Visibility::Public => Ok(()),
            Visibility::Protected => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for WhereConstraint {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_where_constraint(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for XhpAttr<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_attr(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for XhpAttrInfo {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_attr_info(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        self.xai_tag.accept(c, v)?;
        Ok(())
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for XhpAttrTag {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_attr_tag(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            XhpAttrTag::Required => Ok(()),
            XhpAttrTag::LateInit => Ok(()),
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi>
    for XhpAttribute<Ex, Fb, En, Hi>
{
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_attribute(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            XhpAttribute::XhpSimple(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            XhpAttribute::XhpSpread(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for XhpChild {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_child(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            XhpChild::ChildName(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            XhpChild::ChildList(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            XhpChild::ChildUnary(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
            XhpChild::ChildBinary(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<Context, Error, Ex, Fb, En, Hi> Node<Context, Error, Ex, Fb, En, Hi> for XhpChildOp {
    fn accept(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        v.visit_xhp_child_op(c, self)
    }
    fn recurse(
        &self,
        c: &mut Context,
        v: &mut dyn Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    ) -> Result<(), Error> {
        match self {
            XhpChildOp::ChildStar => Ok(()),
            XhpChildOp::ChildPlus => Ok(()),
            XhpChildOp::ChildQuestion => Ok(()),
        }
    }
}
