// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<5a7f612090a632c6b565dff75735e000>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node::Node;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*};
pub fn visit<Context, Error, Ex, Fb, En, Hi>(
    v: &mut impl Visitor<Context = Context, Error = Error, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    c: &mut Context,
    p: &impl Node<Context, Error, Ex, Fb, En, Hi>,
) -> Result<(), Error> {
    p.accept(c, v)
}
pub trait Visitor {
    type Context;
    type Error;
    type Ex;
    type Fb;
    type En;
    type Hi;
    fn object(
        &mut self,
    ) -> &mut dyn Visitor<
        Context = Self::Context,
        Error = Self::Error,
        Ex = Self::Ex,
        Fb = Self::Fb,
        En = Self::En,
        Hi = Self::Hi,
    >;
    fn visit_ex(&mut self, c: &mut Self::Context, p: &Self::Ex) -> Result<(), Self::Error> {
        Ok(())
    }
    fn visit_fb(&mut self, c: &mut Self::Context, p: &Self::Fb) -> Result<(), Self::Error> {
        Ok(())
    }
    fn visit_en(&mut self, c: &mut Self::Context, p: &Self::En) -> Result<(), Self::Error> {
        Ok(())
    }
    fn visit_hi(&mut self, c: &mut Self::Context, p: &Self::Hi) -> Result<(), Self::Error> {
        Ok(())
    }
    fn visit_afield(
        &mut self,
        c: &mut Self::Context,
        p: &Afield<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_as_expr(
        &mut self,
        c: &mut Self::Context,
        p: &AsExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_assert_expr(
        &mut self,
        c: &mut Self::Context,
        p: &AssertExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_bop(&mut self, c: &mut Self::Context, p: &Bop) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut Self::Context,
        p: &CaField<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ca_type(&mut self, c: &mut Self::Context, p: &CaType) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_call_type(&mut self, c: &mut Self::Context, p: &CallType) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut Self::Context,
        p: &Case<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_catch(
        &mut self,
        c: &mut Self::Context,
        p: &Catch<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_attr(
        &mut self,
        c: &mut Self::Context,
        p: &ClassAttr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut Self::Context,
        p: &ClassConst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        c: &mut Self::Context,
        p: &ClassGetExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id(
        &mut self,
        c: &mut Self::Context,
        p: &ClassId<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_id_(
        &mut self,
        c: &mut Self::Context,
        p: &ClassId_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_kind(
        &mut self,
        c: &mut Self::Context,
        p: &ClassKind,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_tparams(
        &mut self,
        c: &mut Self::Context,
        p: &ClassTparams<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut Self::Context,
        p: &ClassTypeconst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_var(
        &mut self,
        c: &mut Self::Context,
        p: &ClassVar<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_class_(
        &mut self,
        c: &mut Self::Context,
        p: &Class_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_collection_targ(
        &mut self,
        c: &mut Self::Context,
        p: &CollectionTarg<Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(
        &mut self,
        c: &mut Self::Context,
        p: &ConstraintKind,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut Self::Context,
        p: &Def<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_doc_comment(
        &mut self,
        c: &mut Self::Context,
        p: &DocComment,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_enum_(&mut self, c: &mut Self::Context, p: &Enum_) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut Self::Context,
        p: &Expr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_expr_(
        &mut self,
        c: &mut Self::Context,
        p: &Expr_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_field(
        &mut self,
        c: &mut Self::Context,
        p: &Field<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_file_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &FileAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_kind(&mut self, c: &mut Self::Context, p: &FunKind) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut Self::Context,
        p: &FunParam<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_variadicity(
        &mut self,
        c: &mut Self::Context,
        p: &FunVariadicity<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_fun_(
        &mut self,
        c: &mut Self::Context,
        p: &Fun_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_body(
        &mut self,
        c: &mut Self::Context,
        p: &FuncBody<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_func_reactive(
        &mut self,
        c: &mut Self::Context,
        p: &FuncReactive,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut Self::Context,
        p: &Gconst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint(&mut self, c: &mut Self::Context, p: &Hint) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(&mut self, c: &mut Self::Context, p: &HintFun) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_hint_(&mut self, c: &mut Self::Context, p: &Hint_) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_id(&mut self, c: &mut Self::Context, p: &Id) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(
        &mut self,
        c: &mut Self::Context,
        p: &ImportFlavor,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(
        &mut self,
        c: &mut Self::Context,
        p: &InsteadofAlias,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(&mut self, c: &mut Self::Context, p: &KvcKind) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_lid(&mut self, c: &mut Self::Context, p: &Lid) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_redeclaration(
        &mut self,
        c: &mut Self::Context,
        p: &MethodRedeclaration<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut Self::Context,
        p: &Method_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_nast_shape_info(
        &mut self,
        c: &mut Self::Context,
        p: &NastShapeInfo,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(&mut self, c: &mut Self::Context, p: &NsKind) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(
        &mut self,
        c: &mut Self::Context,
        p: &OgNullFlavor,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(
        &mut self,
        c: &mut Self::Context,
        p: &ParamKind,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_param_mutability(
        &mut self,
        c: &mut Self::Context,
        p: &ParamMutability,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_enum(
        &mut self,
        c: &mut Self::Context,
        p: &PuEnum<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_loc(&mut self, c: &mut Self::Context, p: &PuLoc) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_pu_member(
        &mut self,
        c: &mut Self::Context,
        p: &PuMember<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_record_def(
        &mut self,
        c: &mut Self::Context,
        p: &RecordDef<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_reify_kind(
        &mut self,
        c: &mut Self::Context,
        p: &ReifyKind,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(
        &mut self,
        c: &mut Self::Context,
        p: &ShapeFieldInfo,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(
        &mut self,
        c: &mut Self::Context,
        p: &ShapeFieldName,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut Self::Context,
        p: &Stmt<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_stmt_(
        &mut self,
        c: &mut Self::Context,
        p: &Stmt_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_targ(&mut self, c: &mut Self::Context, p: &Targ<Self::Hi>) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut Self::Context,
        p: &Tparam<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_tprim(&mut self, c: &mut Self::Context, p: &Tprim) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(
        &mut self,
        c: &mut Self::Context,
        p: &TypeHint<Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typeconst_abstract_kind(
        &mut self,
        c: &mut Self::Context,
        p: &TypeconstAbstractKind,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut Self::Context,
        p: &Typedef<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_typedef_visibility(
        &mut self,
        c: &mut Self::Context,
        p: &TypedefVisibility,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_uop(&mut self, c: &mut Self::Context, p: &Uop) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(
        &mut self,
        c: &mut Self::Context,
        p: &UseAsAlias,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(
        &mut self,
        c: &mut Self::Context,
        p: &UseAsVisibility,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &UserAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_using_stmt(
        &mut self,
        c: &mut Self::Context,
        p: &UsingStmt<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_variance(&mut self, c: &mut Self::Context, p: &Variance) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(&mut self, c: &mut Self::Context, p: &VcKind) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_visibility(
        &mut self,
        c: &mut Self::Context,
        p: &Visibility,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint(
        &mut self,
        c: &mut Self::Context,
        p: &WhereConstraint,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut Self::Context,
        p: &XhpAttr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_info(
        &mut self,
        c: &mut Self::Context,
        p: &XhpAttrInfo,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(
        &mut self,
        c: &mut Self::Context,
        p: &XhpAttrTag,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &XhpAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child(&mut self, c: &mut Self::Context, p: &XhpChild) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(
        &mut self,
        c: &mut Self::Context,
        p: &XhpChildOp,
    ) -> Result<(), Self::Error> {
        p.recurse(c, self.object())
    }
}
