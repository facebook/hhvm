// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<76de4eaa565c4b38584fcf107b8b4463>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_imports)]
#![allow(unused_variables)]
use super::node::Node;
use super::type_params::Params;
use super::visitor::Visitor;
use crate::aast_defs::*;
use crate::aast_defs::{self};
use crate::ast_defs::*;
use crate::ast_defs::{self};
use crate::*;
impl<P: Params> Node<P> for Abstraction {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_abstraction(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Abstraction::Concrete => Ok(()),
            Abstraction::Abstract => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Afield<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_afield(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Afield::AFvalue(a0) => a0.accept(c, v),
            Afield::AFkvalue(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for AsExpr<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_as_expr(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            AsExpr::AsV(a0) => a0.accept(c, v),
            AsExpr::AsKv(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            AsExpr::AwaitAsV(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            AsExpr::AwaitAsKv(a0, a1, a2) => {
                a0.accept(c, v)?;
                a1.accept(c, v)?;
                a2.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for Binop<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_binop(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.bop.accept(c, v)?;
        self.lhs.accept(c, v)?;
        self.rhs.accept(c, v)
    }
}
impl<P: Params> Node<P> for Block<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_block(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)
    }
}
impl<P: Params> Node<P> for Bop {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_bop(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
            Bop::Eq(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for CallExpr<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_call_expr(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.func.accept(c, v)?;
        self.targs.accept(c, v)?;
        self.args.accept(c, v)?;
        self.unpacked_arg.accept(c, v)
    }
}
impl<P: Params> Node<P> for CaptureLid<P::Ex> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_capture_lid(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for Case<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_case(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for Catch<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_catch(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassAbstractTypeconst {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_abstract_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.as_constraint.accept(c, v)?;
        self.super_constraint.accept(c, v)?;
        self.default.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassConcreteTypeconst {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_concrete_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.c_tc_type.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassConst<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_const(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.user_attributes.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.id.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassConstKind<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_const_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassConstKind::CCAbstract(a0) => a0.accept(c, v),
            ClassConstKind::CCConcrete(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for ClassGetExpr<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_get_expr(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassGetExpr::CGstring(a0) => a0.accept(c, v),
            ClassGetExpr::CGexpr(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for ClassId<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_id(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassId_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_id_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassId_::CIparent => Ok(()),
            ClassId_::CIself => Ok(()),
            ClassId_::CIstatic => Ok(()),
            ClassId_::CIexpr(a0) => a0.accept(c, v),
            ClassId_::CI(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for ClassReq {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_req(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassTypeconst {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_typeconst(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassTypeconst::TCAbstract(a0) => a0.accept(c, v),
            ClassTypeconst::TCConcrete(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for ClassTypeconstDef<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_typeconst_def(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.user_attributes.accept(c, v)?;
        self.name.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.span.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.is_ctx.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassVar<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_var(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
        self.span.accept(c, v)
    }
}
impl<P: Params> Node<P> for Class_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_class_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
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
        self.xhp_attr_uses.accept(c, v)?;
        self.xhp_category.accept(c, v)?;
        self.reqs.accept(c, v)?;
        self.implements.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.consts.accept(c, v)?;
        self.typeconsts.accept(c, v)?;
        self.vars.accept(c, v)?;
        self.methods.accept(c, v)?;
        self.xhp_children.accept(c, v)?;
        self.xhp_attrs.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.docs_url.accept(c, v)?;
        self.enum_.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        self.internal.accept(c, v)?;
        self.module.accept(c, v)
    }
}
impl<P: Params> Node<P> for ClassishKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_classish_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ClassishKind::Cclass(a0) => a0.accept(c, v),
            ClassishKind::Cinterface => Ok(()),
            ClassishKind::Ctrait => Ok(()),
            ClassishKind::Cenum => Ok(()),
            ClassishKind::CenumClass(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for CollectionTarg<P::Ex> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_collection_targ(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            CollectionTarg::CollectionTV(a0) => a0.accept(c, v),
            CollectionTarg::CollectionTKV(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for ConstraintKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_constraint_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ConstraintKind::ConstraintAs => Ok(()),
            ConstraintKind::ConstraintEq => Ok(()),
            ConstraintKind::ConstraintSuper => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Contexts {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_contexts(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for CtxRefinement {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ctx_refinement(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            CtxRefinement::CRexact(a0) => a0.accept(c, v),
            CtxRefinement::CRloose(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for CtxRefinementBounds {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ctx_refinement_bounds(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.lower.accept(c, v)?;
        self.upper.accept(c, v)
    }
}
impl<P: Params> Node<P> for Def<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_def(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        #[inline]
        fn helper0<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Sid, Vec<Def<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        match self {
            Def::Fun(a0) => a0.accept(c, v),
            Def::Class(a0) => a0.accept(c, v),
            Def::Stmt(a0) => a0.accept(c, v),
            Def::Typedef(a0) => a0.accept(c, v),
            Def::Constant(a0) => a0.accept(c, v),
            Def::Namespace(a) => helper0(a, c, v),
            Def::NamespaceUse(a0) => a0.accept(c, v),
            Def::SetNamespaceEnv(a0) => a0.accept(c, v),
            Def::FileAttributes(a0) => a0.accept(c, v),
            Def::Module(a0) => a0.accept(c, v),
            Def::SetModule(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for DefaultCase<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_default_case(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for Efun<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_efun(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.fun.accept(c, v)?;
        self.use_.accept(c, v)?;
        self.closure_class_name.accept(c, v)
    }
}
impl<P: Params> Node<P> for EmitId {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_emit_id(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            EmitId::EmitId(a0) => a0.accept(c, v),
            EmitId::Anonymous => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Enum_ {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_enum_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.base.accept(c, v)?;
        self.constraint.accept(c, v)?;
        self.includes.accept(c, v)
    }
}
impl<P: Params> Node<P> for EnvAnnot {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_env_annot(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            EnvAnnot::Join => Ok(()),
            EnvAnnot::Refinement => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Expr<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_expr(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}
impl<P: Params> Node<P> for Expr_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_expr_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        #[inline]
        fn helper0<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(
                Option<(Targ<Ex>, Targ<Ex>)>,
                Vec<(Expr<Ex, En>, Expr<Ex, En>)>,
            )>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper1<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Option<Targ<Ex>>, Vec<Expr<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper2<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<((Pos, VcKind), Option<Targ<Ex>>, Vec<Expr<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper3<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(
                (Pos, KvcKind),
                Option<(Targ<Ex>, Targ<Ex>)>,
                Vec<Field<Ex, En>>,
            )>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper4<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Option<Expr<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper5<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Expr<Ex, En>, OgNullFlavor, PropOrMethod)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)?;
            a.3.accept(c, v)
        }
        #[inline]
        fn helper6<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ClassId<Ex, En>, ClassGetExpr<Ex, En>, PropOrMethod)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper7<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ClassId<Ex, En>, Pstring)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper8<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(FunctionPtrId<Ex, En>, Vec<Targ<Ex>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper9<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(String, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper10<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Hint, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper11<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ast_defs::Uop, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper12<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Lid, Expr<Ex, En>, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper13<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Option<Expr<Ex, En>>, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper14<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Hint)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper15<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Hint, bool)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper16<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Hint)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper17<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(
                ClassId<Ex, En>,
                Vec<Targ<Ex>>,
                Vec<Expr<Ex, En>>,
                Option<Expr<Ex, En>>,
                Ex,
            )>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)?;
            a.3.accept(c, v)?;
            v.visit_ex(c, &a.4)
        }
        #[inline]
        fn helper18<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Fun_<Ex, En>, Vec<CaptureLid<Ex>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper19<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ClassName, Vec<XhpAttribute<Ex, En>>, Vec<Expr<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper20<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ImportFlavor, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper21<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ClassName, Option<CollectionTarg<Ex>>, Vec<Afield<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper22<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(ClassName, Pstring)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper23<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Option<(Targ<Ex>, Targ<Ex>)>, Expr<Ex, En>, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper24<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Option<ClassName>, String)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper25<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Ex, Ex, HoleSource)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            v.visit_ex(c, &a.1)?;
            v.visit_ex(c, &a.2)?;
            a.3.accept(c, v)
        }
        match self {
            Expr_::Darray(a) => helper0(a, c, v),
            Expr_::Varray(a) => helper1(a, c, v),
            Expr_::Shape(a0) => a0.accept(c, v),
            Expr_::ValCollection(a) => helper2(a, c, v),
            Expr_::KeyValCollection(a) => helper3(a, c, v),
            Expr_::Null => Ok(()),
            Expr_::This => Ok(()),
            Expr_::True => Ok(()),
            Expr_::False => Ok(()),
            Expr_::Omitted => Ok(()),
            Expr_::Invalid(a0) => a0.accept(c, v),
            Expr_::Id(a0) => a0.accept(c, v),
            Expr_::Lvar(a0) => a0.accept(c, v),
            Expr_::Dollardollar(a0) => a0.accept(c, v),
            Expr_::Clone(a0) => a0.accept(c, v),
            Expr_::ArrayGet(a) => helper4(a, c, v),
            Expr_::ObjGet(a) => helper5(a, c, v),
            Expr_::ClassGet(a) => helper6(a, c, v),
            Expr_::ClassConst(a) => helper7(a, c, v),
            Expr_::Call(a0) => a0.accept(c, v),
            Expr_::FunctionPointer(a) => helper8(a, c, v),
            Expr_::Int(a0) => a0.accept(c, v),
            Expr_::Float(a0) => a0.accept(c, v),
            Expr_::String(a0) => a0.accept(c, v),
            Expr_::String2(a0) => a0.accept(c, v),
            Expr_::PrefixedString(a) => helper9(a, c, v),
            Expr_::Yield(a0) => a0.accept(c, v),
            Expr_::Await(a0) => a0.accept(c, v),
            Expr_::ReadonlyExpr(a0) => a0.accept(c, v),
            Expr_::Tuple(a0) => a0.accept(c, v),
            Expr_::List(a0) => a0.accept(c, v),
            Expr_::Cast(a) => helper10(a, c, v),
            Expr_::Unop(a) => helper11(a, c, v),
            Expr_::Binop(a0) => a0.accept(c, v),
            Expr_::Pipe(a) => helper12(a, c, v),
            Expr_::Eif(a) => helper13(a, c, v),
            Expr_::Is(a) => helper14(a, c, v),
            Expr_::As(a) => helper15(a, c, v),
            Expr_::Upcast(a) => helper16(a, c, v),
            Expr_::New(a) => helper17(a, c, v),
            Expr_::Efun(a0) => a0.accept(c, v),
            Expr_::Lfun(a) => helper18(a, c, v),
            Expr_::Xml(a) => helper19(a, c, v),
            Expr_::Import(a) => helper20(a, c, v),
            Expr_::Collection(a) => helper21(a, c, v),
            Expr_::ExpressionTree(a0) => a0.accept(c, v),
            Expr_::Lplaceholder(a0) => a0.accept(c, v),
            Expr_::MethodCaller(a) => helper22(a, c, v),
            Expr_::Pair(a) => helper23(a, c, v),
            Expr_::ETSplice(a0) => a0.accept(c, v),
            Expr_::EnumClassLabel(a) => helper24(a, c, v),
            Expr_::Hole(a) => helper25(a, c, v),
            Expr_::Package(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for ExpressionTree<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_expression_tree(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.hint.accept(c, v)?;
        self.splices.accept(c, v)?;
        self.function_pointers.accept(c, v)?;
        self.virtualized_expr.accept(c, v)?;
        self.runtime_expr.accept(c, v)?;
        self.dollardollar_pos.accept(c, v)
    }
}
impl<P: Params> Node<P> for Field<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_field(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for FileAttribute<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_file_attribute(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.user_attributes.accept(c, v)?;
        self.namespace.accept(c, v)
    }
}
impl<P: Params> Node<P> for FinallyBlock<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_finally_block(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)
    }
}
impl<P: Params> Node<P> for FunDef<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_def(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.namespace.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.mode.accept(c, v)?;
        self.name.accept(c, v)?;
        self.fun.accept(c, v)?;
        self.internal.accept(c, v)?;
        self.module.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)
    }
}
impl<P: Params> Node<P> for FunKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            FunKind::FSync => Ok(()),
            FunKind::FAsync => Ok(()),
            FunKind::FGenerator => Ok(()),
            FunKind::FAsyncGenerator => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for FunParam<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_param(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.annotation)?;
        self.type_hint.accept(c, v)?;
        self.is_variadic.accept(c, v)?;
        self.pos.accept(c, v)?;
        self.name.accept(c, v)?;
        self.expr.accept(c, v)?;
        self.readonly.accept(c, v)?;
        self.callconv.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.visibility.accept(c, v)
    }
}
impl<P: Params> Node<P> for Fun_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_fun_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.span.accept(c, v)?;
        self.readonly_this.accept(c, v)?;
        v.visit_en(c, &self.annotation)?;
        self.readonly_ret.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.params.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.unsafe_ctxs.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.external.accept(c, v)?;
        self.doc_comment.accept(c, v)
    }
}
impl<P: Params> Node<P> for FuncBody<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_func_body(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.fb_ast.accept(c, v)
    }
}
impl<P: Params> Node<P> for FunctionPtrId<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_function_ptr_id(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            FunctionPtrId::FPId(a0) => a0.accept(c, v),
            FunctionPtrId::FPClassConst(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for Gconst<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_gconst(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &self.annotation)?;
        self.mode.accept(c, v)?;
        self.name.accept(c, v)?;
        self.type_.accept(c, v)?;
        self.value.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.emit_id.accept(c, v)
    }
}
impl<P: Params> Node<P> for HfParamInfo {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_hf_param_info(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.kind.accept(c, v)?;
        self.readonlyness.accept(c, v)
    }
}
impl<P: Params> Node<P> for Hint {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for HintFun {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint_fun(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.is_readonly.accept(c, v)?;
        self.param_tys.accept(c, v)?;
        self.param_info.accept(c, v)?;
        self.variadic_ty.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.return_ty.accept(c, v)?;
        self.is_readonly_return.accept(c, v)
    }
}
impl<P: Params> Node<P> for Hint_ {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_hint_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Hint_::Hoption(a0) => a0.accept(c, v),
            Hint_::Hlike(a0) => a0.accept(c, v),
            Hint_::Hfun(a0) => a0.accept(c, v),
            Hint_::Htuple(a0) => a0.accept(c, v),
            Hint_::Happly(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Hint_::Hshape(a0) => a0.accept(c, v),
            Hint_::Haccess(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Hint_::Hsoft(a0) => a0.accept(c, v),
            Hint_::Hrefinement(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Hint_::Hany => Ok(()),
            Hint_::Herr => Ok(()),
            Hint_::Hmixed => Ok(()),
            Hint_::Hwildcard => Ok(()),
            Hint_::Hnonnull => Ok(()),
            Hint_::Habstr(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Hint_::HvecOrDict(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Hint_::Hprim(a0) => a0.accept(c, v),
            Hint_::Hthis => Ok(()),
            Hint_::Hdynamic => Ok(()),
            Hint_::Hnothing => Ok(()),
            Hint_::Hunion(a0) => a0.accept(c, v),
            Hint_::Hintersection(a0) => a0.accept(c, v),
            Hint_::HfunContext(a0) => a0.accept(c, v),
            Hint_::Hvar(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for HoleSource {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_hole_source(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            HoleSource::Typing => Ok(()),
            HoleSource::UnsafeCast(a0) => a0.accept(c, v),
            HoleSource::UnsafeNonnullCast => Ok(()),
            HoleSource::EnforcedCast(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for Id {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_id(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for ImportFlavor {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_import_flavor(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ImportFlavor::Include => Ok(()),
            ImportFlavor::Require => Ok(()),
            ImportFlavor::IncludeOnce => Ok(()),
            ImportFlavor::RequireOnce => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for KvcKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_kvc_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            KvcKind::Map => Ok(()),
            KvcKind::ImmMap => Ok(()),
            KvcKind::Dict => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Lid {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_lid(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for MdNameKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_md_name_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            MdNameKind::MDNameGlobal(a0) => a0.accept(c, v),
            MdNameKind::MDNamePrefix(a0) => a0.accept(c, v),
            MdNameKind::MDNameExact(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for Method_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_method_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.span.accept(c, v)?;
        v.visit_en(c, &self.annotation)?;
        self.final_.accept(c, v)?;
        self.abstract_.accept(c, v)?;
        self.static_.accept(c, v)?;
        self.readonly_this.accept(c, v)?;
        self.visibility.accept(c, v)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.where_constraints.accept(c, v)?;
        self.params.accept(c, v)?;
        self.ctxs.accept(c, v)?;
        self.unsafe_ctxs.accept(c, v)?;
        self.body.accept(c, v)?;
        self.fun_kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.readonly_ret.accept(c, v)?;
        self.ret.accept(c, v)?;
        self.external.accept(c, v)?;
        self.doc_comment.accept(c, v)
    }
}
impl<P: Params> Node<P> for ModuleDef<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_module_def(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &self.annotation)?;
        self.name.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.span.accept(c, v)?;
        self.mode.accept(c, v)?;
        self.doc_comment.accept(c, v)?;
        self.exports.accept(c, v)?;
        self.imports.accept(c, v)
    }
}
impl<P: Params> Node<P> for NastShapeInfo {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_nast_shape_info(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.allows_unknown_fields.accept(c, v)?;
        self.field_map.accept(c, v)
    }
}
impl<P: Params> Node<P> for NsKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ns_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
impl<P: Params> Node<P> for OgNullFlavor {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_og_null_flavor(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            OgNullFlavor::OGNullthrows => Ok(()),
            OgNullFlavor::OGNullsafe => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for ParamKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_param_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ParamKind::Pinout(a0) => a0.accept(c, v),
            ParamKind::Pnormal => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for PatRefinement {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_pat_refinement(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.pos.accept(c, v)?;
        self.id.accept(c, v)?;
        self.hint.accept(c, v)
    }
}
impl<P: Params> Node<P> for PatVar {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_pat_var(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.pos.accept(c, v)?;
        self.id.accept(c, v)
    }
}
impl<P: Params> Node<P> for Pattern {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_pattern(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Pattern::PVar(a0) => a0.accept(c, v),
            Pattern::PRefinement(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for Program<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_program(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)
    }
}
impl<P: Params> Node<P> for PropOrMethod {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_prop_or_method(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            PropOrMethod::IsProp => Ok(()),
            PropOrMethod::IsMethod => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for ReadonlyKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_readonly_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ReadonlyKind::Readonly => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Refinement {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_refinement(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Refinement::Rctx(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            Refinement::Rtype(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for ReifyKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_reify_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ReifyKind::Erased => Ok(()),
            ReifyKind::SoftReified => Ok(()),
            ReifyKind::Reified => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for RequireKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_require_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            RequireKind::RequireExtends => Ok(()),
            RequireKind::RequireImplements => Ok(()),
            RequireKind::RequireClass => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for ShapeFieldInfo {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_shape_field_info(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.optional.accept(c, v)?;
        self.hint.accept(c, v)?;
        self.name.accept(c, v)
    }
}
impl<P: Params> Node<P> for ShapeFieldName {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_shape_field_name(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            ShapeFieldName::SFlitInt(a0) => a0.accept(c, v),
            ShapeFieldName::SFlitStr(a0) => a0.accept(c, v),
            ShapeFieldName::SFclassConst(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for Stmt<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for StmtMatch<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt_match(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.expr.accept(c, v)?;
        self.arms.accept(c, v)
    }
}
impl<P: Params> Node<P> for StmtMatchArm<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt_match_arm(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.pat.accept(c, v)?;
        self.body.accept(c, v)
    }
}
impl<P: Params> Node<P> for Stmt_<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_stmt_(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        #[inline]
        fn helper0<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Vec<(Lid, Expr<Ex, En>)>, Block<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper1<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Block<Ex, En>, Block<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper2<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Block<Ex, En>, Expr<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper3<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Block<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper4<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(
                Vec<Expr<Ex, En>>,
                Option<Expr<Ex, En>>,
                Vec<Expr<Ex, En>>,
                Block<Ex, En>,
            )>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)?;
            a.3.accept(c, v)
        }
        #[inline]
        fn helper5<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, Vec<Case<Ex, En>>, Option<DefaultCase<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper6<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Expr<Ex, En>, AsExpr<Ex, En>, Block<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper7<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Block<Ex, En>, Vec<Catch<Ex, En>>, FinallyBlock<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper8<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Lid, Hint, Option<Expr<Ex, En>>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)?;
            a.2.accept(c, v)
        }
        #[inline]
        fn helper9<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(Option<Vec<Lid>>, Block<Ex, En>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        #[inline]
        fn helper10<'node, P: Params + Params<Ex = Ex> + Params<En = En>, Ex, En>(
            a: &'node Box<(EnvAnnot, LocalIdMap<(Pos, Ex)>)>,
            c: &mut P::Context,
            v: &mut dyn Visitor<'node, Params = P>,
        ) -> Result<(), P::Error> {
            a.0.accept(c, v)?;
            a.1.accept(c, v)
        }
        match self {
            Stmt_::Fallthrough => Ok(()),
            Stmt_::Expr(a0) => a0.accept(c, v),
            Stmt_::Break => Ok(()),
            Stmt_::Continue => Ok(()),
            Stmt_::Throw(a0) => a0.accept(c, v),
            Stmt_::Return(a0) => a0.accept(c, v),
            Stmt_::YieldBreak => Ok(()),
            Stmt_::Awaitall(a) => helper0(a, c, v),
            Stmt_::If(a) => helper1(a, c, v),
            Stmt_::Do(a) => helper2(a, c, v),
            Stmt_::While(a) => helper3(a, c, v),
            Stmt_::Using(a0) => a0.accept(c, v),
            Stmt_::For(a) => helper4(a, c, v),
            Stmt_::Switch(a) => helper5(a, c, v),
            Stmt_::Match(a0) => a0.accept(c, v),
            Stmt_::Foreach(a) => helper6(a, c, v),
            Stmt_::Try(a) => helper7(a, c, v),
            Stmt_::Noop => Ok(()),
            Stmt_::DeclareLocal(a) => helper8(a, c, v),
            Stmt_::Block(a) => helper9(a, c, v),
            Stmt_::Markup(a0) => a0.accept(c, v),
            Stmt_::AssertEnv(a) => helper10(a, c, v),
        }
    }
}
impl<P: Params> Node<P> for Targ<P::Ex> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_targ(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for Tparam<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_tparam(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.variance.accept(c, v)?;
        self.name.accept(c, v)?;
        self.parameters.accept(c, v)?;
        self.constraints.accept(c, v)?;
        self.reified.accept(c, v)?;
        self.user_attributes.accept(c, v)
    }
}
impl<P: Params> Node<P> for Tprim {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_tprim(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
impl<P: Params> Node<P> for TypeHint<P::Ex> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_type_hint(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_ex(c, &self.0)?;
        self.1.accept(c, v)
    }
}
impl<P: Params> Node<P> for TypeRefinement {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_type_refinement(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            TypeRefinement::TRexact(a0) => a0.accept(c, v),
            TypeRefinement::TRloose(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for TypeRefinementBounds {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_type_refinement_bounds(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.lower.accept(c, v)?;
        self.upper.accept(c, v)
    }
}
impl<P: Params> Node<P> for Typedef<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_typedef(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_en(c, &self.annotation)?;
        self.name.accept(c, v)?;
        self.tparams.accept(c, v)?;
        self.as_constraint.accept(c, v)?;
        self.super_constraint.accept(c, v)?;
        self.kind.accept(c, v)?;
        self.user_attributes.accept(c, v)?;
        self.file_attributes.accept(c, v)?;
        self.mode.accept(c, v)?;
        self.vis.accept(c, v)?;
        self.namespace.accept(c, v)?;
        self.span.accept(c, v)?;
        self.emit_id.accept(c, v)?;
        self.is_ctx.accept(c, v)?;
        self.internal.accept(c, v)?;
        self.module.accept(c, v)?;
        self.docs_url.accept(c, v)?;
        self.doc_comment.accept(c, v)
    }
}
impl<P: Params> Node<P> for TypedefVisibility {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_typedef_visibility(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            TypedefVisibility::Transparent => Ok(()),
            TypedefVisibility::Opaque => Ok(()),
            TypedefVisibility::OpaqueModule => Ok(()),
            TypedefVisibility::CaseType => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for Uop {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_uop(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
impl<P: Params> Node<P> for UserAttribute<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_user_attribute(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.name.accept(c, v)?;
        self.params.accept(c, v)
    }
}
impl<P: Params> Node<P> for UserAttributes<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_user_attributes(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)
    }
}
impl<P: Params> Node<P> for UsingStmt<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_using_stmt(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.is_block_scoped.accept(c, v)?;
        self.has_await.accept(c, v)?;
        self.exprs.accept(c, v)?;
        self.block.accept(c, v)
    }
}
impl<P: Params> Node<P> for Variance {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_variance(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Variance::Covariant => Ok(()),
            Variance::Contravariant => Ok(()),
            Variance::Invariant => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for VcKind {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_vc_kind(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
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
impl<P: Params> Node<P> for Visibility {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_visibility(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            Visibility::Private => Ok(()),
            Visibility::Public => Ok(()),
            Visibility::Protected => Ok(()),
            Visibility::Internal => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for WhereConstraintHint {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_where_constraint_hint(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)
    }
}
impl<P: Params> Node<P> for XhpAttr<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.0.accept(c, v)?;
        self.1.accept(c, v)?;
        self.2.accept(c, v)?;
        self.3.accept(c, v)
    }
}
impl<P: Params> Node<P> for XhpAttrInfo {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr_info(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.like.accept(c, v)?;
        self.tag.accept(c, v)?;
        self.enum_values.accept(c, v)
    }
}
impl<P: Params> Node<P> for XhpAttrTag {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attr_tag(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpAttrTag::Required => Ok(()),
            XhpAttrTag::LateInit => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for XhpAttribute<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_attribute(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpAttribute::XhpSimple(a0) => a0.accept(c, v),
            XhpAttribute::XhpSpread(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for XhpChild {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_child(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpChild::ChildName(a0) => a0.accept(c, v),
            XhpChild::ChildList(a0) => a0.accept(c, v),
            XhpChild::ChildUnary(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
            XhpChild::ChildBinary(a0, a1) => {
                a0.accept(c, v)?;
                a1.accept(c, v)
            }
        }
    }
}
impl<P: Params> Node<P> for XhpChildOp {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_child_op(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpChildOp::ChildStar => Ok(()),
            XhpChildOp::ChildPlus => Ok(()),
            XhpChildOp::ChildQuestion => Ok(()),
        }
    }
}
impl<P: Params> Node<P> for XhpEnumValue {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_enum_value(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        match self {
            XhpEnumValue::XEVInt(a0) => a0.accept(c, v),
            XhpEnumValue::XEVString(a0) => a0.accept(c, v),
        }
    }
}
impl<P: Params> Node<P> for XhpSimple<P::Ex, P::En> {
    fn accept<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        v.visit_xhp_simple(c, self)
    }
    fn recurse<'node>(
        &'node self,
        c: &mut P::Context,
        v: &mut dyn Visitor<'node, Params = P>,
    ) -> Result<(), P::Error> {
        self.name.accept(c, v)?;
        v.visit_ex(c, &self.type_)?;
        self.expr.accept(c, v)
    }
}
