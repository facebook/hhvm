// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<0c14e203dd7bee35a57ef6956b41b112>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::node_mut::NodeMut;
use super::type_params::Params;
use super::visitor_mut::VisitorMut;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
impl<P: Params> NodeMut<P> for Afield<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_afield(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for AsExpr<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_as_expr(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for Bop {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_bop(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for CaField<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ca_field(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.value.accept(c, v)?;
        self.required.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for CaType {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ca_type(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for Case<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_case(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for Catch<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_catch(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassAbstractTypeconst {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_abstract_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.as_constraint.accept(c, v)?;
        self.super_constraint.accept(c, v)?;
        self.default.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassAttr<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_attr(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for ClassConcreteTypeconst {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_concrete_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.c_tc_type.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassConst<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_const(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassGetExpr<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_get_expr(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for ClassId<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_id(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &mut self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassId_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_id_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for ClassKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassKind::Cabstract => Ok(()),
            ClassKind::Cnormal => Ok(()),
            ClassKind::Cinterface => Ok(()),
            ClassKind::Ctrait => Ok(()),
            ClassKind::Cenum => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for ClassPartiallyAbstractTypeconst {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_partially_abstract_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.constraint.accept(c, v)?;
        self.type_.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassTypeconst {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassTypeconst::TCAbstract(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ClassTypeconst::TCConcrete(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            ClassTypeconst::TCPartiallyAbstract(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for ClassTypeconstDef<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_typeconst_def(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.user_attributes.accept(c, v)?;
        self.name.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.is_ctx.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ClassVar<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_var(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.final_.accept(c, v)?;
        self.xhp_attr.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.readonly.accept(c, v)?;
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
impl<P: Params> NodeMut<P> for Class_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &mut self.annotation)?;
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
        self.xhp_attr_uses.accept(c, v)?;
        self.xhp_category.accept(c, v)?;
        self.reqs.accept(c, v)?;
        self.implements.accept(c, v)?;
        self.implements_dynamic.accept(c, v)?;
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
        self.doc_comment.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for CollectionTarg<P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_collection_targ(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for ConstraintKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_constraint_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ConstraintKind::ConstraintAs => Ok(()),
            ConstraintKind::ConstraintEq => Ok(()),
            ConstraintKind::ConstraintSuper => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Contexts {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_contexts(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Def<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_def(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for DocComment {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_doc_comment(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for EmitId {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_emit_id(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            EmitId::EmitId(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            EmitId::Anonymous => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Enum_ {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_enum_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.base.accept(c, v)?;
        self.constraint.accept(c, v)?;
        self.includes.accept(c, v)?;
        self.enum_class.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for EnvAnnot {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_env_annot(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            EnvAnnot::Join => Ok(()),
            EnvAnnot::Refinement => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Expr<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_expr(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &mut self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Expr_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_expr_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
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
            Expr_::ArrayGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
            Expr_::ObjGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
                a.3.accept(c, v)?;
                Ok(())
            }
            Expr_::ClassGet(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                a.2.accept(c, v)?;
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
            Expr_::Await(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::ReadonlyExpr(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Tuple(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::List(a0) => {
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
                v.visit_ex(c, &mut a.4)?;
                Ok(())
            }
            Expr_::Record(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
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
            Expr_::ExpressionTree(a0) => {
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
                a.2.accept(c, v)?;
                Ok(())
            }
            Expr_::ETSplice(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::EnumAtom(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Expr_::Any => Ok(()),
            Expr_::Hole(a) => {
                a.0.accept(c, v)?;
                v.visit_hi(c, &mut a.1)?;
                v.visit_hi(c, &mut a.2)?;
                a.3.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for ExpressionTree<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_expression_tree(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.hint.accept(c, v)?;
        self.splices.accept(c, v)?;
        self.virtualized_expr.accept(c, v)?;
        self.runtime_expr.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Field<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_field(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for FileAttribute<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_file_attribute(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.user_attributes.accept(c, v)?;
        self.namespace.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for FunKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            FunKind::FSync => Ok(()),
            FunKind::FAsync => Ok(()),
            FunKind::FGenerator => Ok(()),
            FunKind::FAsyncGenerator => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for FunParam<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_param(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &mut self.annotation)?;
        self.type_hint.accept(c, v)?;
        self.is_variadic.accept(c, v)?;
        self.pos.accept(c, v)?;
        self.name.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.readonly.accept(c, v)?;
        self.callconv.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.visibility.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for FunVariadicity<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_variadicity(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for Fun_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.span.accept(c, v)?;
        self.readonly_this.accept(c, v)?;
        v.visit_en(c, &mut self.annotation)?;
        self.mode.accept(c, v)?;
        self.readonly_ret.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.variadic.accept(c, v)?;
        self.params.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.unsafe_ctxs.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.external.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for FuncBody<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_func_body(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.ast.accept(c, v)?;
        v.visit_fb(c, &mut self.annotation)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for FunctionPtrId<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_function_ptr_id(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            FunctionPtrId::FPId(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            FunctionPtrId::FPClassConst(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for Gconst<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_gconst(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &mut self.annotation)?;
        self.mode.accept(c, v)?;
        self.name.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.value.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for HfParamInfo {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hf_param_info(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.kind.accept(c, v)?;
        self.readonlyness.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Hint {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for HintFun {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint_fun(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.is_readonly.accept(c, v)?;
        self.param_tys.accept(c, v)?;
        self.param_info.accept(c, v)?;
        self.variadic_ty.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.return_ty.accept(c, v)?;
        self.is_readonly_return.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Hint_ {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
            Hint_::Habstr(a0, a1) => {
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
            Hint_::HvecOrDict(a0, a1) => {
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
            Hint_::Hunion(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hintersection(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::HfunContext(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Hint_::Hvar(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for HoleSource {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hole_source(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            HoleSource::Typing => Ok(()),
            HoleSource::UnsafeCast => Ok(()),
            HoleSource::EnforcedCast => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Id {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_id(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ImportFlavor {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_import_flavor(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ImportFlavor::Include => Ok(()),
            ImportFlavor::Require => Ok(()),
            ImportFlavor::IncludeOnce => Ok(()),
            ImportFlavor::RequireOnce => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for InsteadofAlias {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_insteadof_alias(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for KvcKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_kvc_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            KvcKind::Map => Ok(()),
            KvcKind::ImmMap => Ok(()),
            KvcKind::Dict => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Lid {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_lid(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Method_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_method_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &mut self.annotation)?;
        self.final_.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.static_.accept(c, v)?;
        self.readonly_this.accept(c, v)?;
        self.visibility.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.variadic.accept(c, v)?;
        self.params.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.unsafe_ctxs.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.readonly_ret.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.external.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for NastShapeInfo {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_nast_shape_info(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.allows_unknown_fields.accept(c, v)?;
        self.field_map.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for NsKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_ns_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            NsKind::NSNamespace => Ok(()),
            NsKind::NSClass => Ok(()),
            NsKind::NSClassAndNamespace => Ok(()),
            NsKind::NSFun => Ok(()),
            NsKind::NSConst => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for OgNullFlavor {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_og_null_flavor(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            OgNullFlavor::OGNullthrows => Ok(()),
            OgNullFlavor::OGNullsafe => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for ParamKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_param_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ParamKind::Pinout => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for ReadonlyKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_readonly_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ReadonlyKind::Readonly => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for RecordDef<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_record_def(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &mut self.annotation)?;
        self.name.accept(c, v)?;
        self.extends.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.fields.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ReifyKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_reify_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            ReifyKind::Erased => Ok(()),
            ReifyKind::SoftReified => Ok(()),
            ReifyKind::Reified => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for ShapeFieldInfo {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_shape_field_info(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.optional.accept(c, v)?;
        self.hint.accept(c, v)?;
        self.name.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for ShapeFieldName {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_shape_field_name(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for Stmt<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Stmt_<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt_(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
            Stmt_::YieldBreak => Ok(()),
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
            Stmt_::Noop => Ok(()),
            Stmt_::Block(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::Markup(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            Stmt_::AssertEnv(a) => {
                a.0.accept(c, v)?;
                a.1.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for Targ<P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_targ(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hi(c, &mut self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Tparam<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_tparam(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.variance.accept(c, v)?;
        self.name.accept(c, v)?;
        self.parameters.accept(c, v)?;
        self.constraints.accept(c, v)?;
        self.reified.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Tprim {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_tprim(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
        }
    }
}
impl<P: Params> NodeMut<P> for TypeHint<P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_type_hint(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_hi(c, &mut self.0)?;
        self.1.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Typedef<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_typedef(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &mut self.annotation)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.constraint.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.mode.accept(c, v)?;
        self.vis.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for TypedefVisibility {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_typedef_visibility(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            TypedefVisibility::Transparent => Ok(()),
            TypedefVisibility::Opaque => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Uop {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_uop(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for UseAsAlias {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_use_as_alias(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for UseAsVisibility {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_use_as_visibility(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            UseAsVisibility::UseAsPublic => Ok(()),
            UseAsVisibility::UseAsPrivate => Ok(()),
            UseAsVisibility::UseAsProtected => Ok(()),
            UseAsVisibility::UseAsFinal => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for UserAttribute<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_user_attribute(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.name.accept(c, v)?;
        self.params.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for UsingStmt<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_using_stmt(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.is_block_scoped.accept(c, v)?;
        self.has_await.accept(c, v)?;
        self.exprs.accept(c, v)?;
        self.block.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for Variance {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_variance(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            Variance::Covariant => Ok(()),
            Variance::Contravariant => Ok(()),
            Variance::Invariant => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for VcKind {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_vc_kind(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            VcKind::Vector => Ok(()),
            VcKind::ImmVector => Ok(()),
            VcKind::Vec => Ok(()),
            VcKind::Set => Ok(()),
            VcKind::ImmSet => Ok(()),
            VcKind::Keyset => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for Visibility {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_visibility(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            Visibility::Private => Ok(()),
            Visibility::Public => Ok(()),
            Visibility::Protected => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for WhereConstraintHint {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_where_constraint_hint(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for XhpAttr<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for XhpAttrInfo {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr_info(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.xai_tag.accept(c, v)?;
        Ok(())
    }
}
impl<P: Params> NodeMut<P> for XhpAttrTag {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr_tag(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpAttrTag::Required => Ok(()),
            XhpAttrTag::LateInit => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for XhpAttribute<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attribute(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpAttribute::XhpSimple(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
            XhpAttribute::XhpSpread(a0) => {
                a0.accept(c, v)?;
                Ok(())
            }
        }
    }
}
impl<P: Params> NodeMut<P> for XhpChild {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_child(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
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
impl<P: Params> NodeMut<P> for XhpChildOp {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_child_op(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpChildOp::ChildStar => Ok(()),
            XhpChildOp::ChildPlus => Ok(()),
            XhpChildOp::ChildQuestion => Ok(()),
        }
    }
}
impl<P: Params> NodeMut<P> for XhpSimple<P::Ex, P::Fb, P::En, P::Hi> {
    fn accept<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_simple(c, self)
    }
    fn recurse<'node>(
        &'node mut self,
        c: &mut P::Context,
        v: &mut dyn VisitorMut<'node, P = P>,
    ) -> Result<(), P::Error> {
        self.name.accept(c, v)?;
        v.visit_hi(c, &mut self.type_)?;
        self.expr.accept(c, v)?;
        Ok(())
    }
}
