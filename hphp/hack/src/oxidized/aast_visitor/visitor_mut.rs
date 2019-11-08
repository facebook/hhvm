// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<d9fda8993f146e3e789a6c276526beb1>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized/regen.sh

#![allow(unused_variables)]
use super::node_mut::NodeMut;
use crate::{aast::*, aast_defs::*, ast_defs::*};
pub fn visit<Context, Ex, Fb, En, Hi>(
    v: &mut impl VisitorMut<Context = Context, Ex = Ex, Fb = Fb, En = En, Hi = Hi>,
    c: &mut Context,
    p: &mut impl NodeMut<Context, Ex, Fb, En, Hi>,
) {
    p.accept(c, v);
}
pub trait VisitorMut {
    type Context;
    type Ex;
    type Fb;
    type En;
    type Hi;
    fn object(
        &mut self,
    ) -> &mut dyn VisitorMut<
        Context = Self::Context,
        Ex = Self::Ex,
        Fb = Self::Fb,
        En = Self::En,
        Hi = Self::Hi,
    >;
    fn visit_ex(&mut self, c: &mut Self::Context, p: &mut Self::Ex) {}
    fn visit_fb(&mut self, c: &mut Self::Context, p: &mut Self::Fb) {}
    fn visit_en(&mut self, c: &mut Self::Context, p: &mut Self::En) {}
    fn visit_hi(&mut self, c: &mut Self::Context, p: &mut Self::Hi) {}
    fn visit_afield(
        &mut self,
        c: &mut Self::Context,
        p: &mut Afield<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_as_expr(
        &mut self,
        c: &mut Self::Context,
        p: &mut AsExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_assert_expr(
        &mut self,
        c: &mut Self::Context,
        p: &mut AssertExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_block(
        &mut self,
        c: &mut Self::Context,
        p: &mut Block<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_bop(&mut self, c: &mut Self::Context, p: &mut Bop) {
        p.recurse(c, self.object())
    }
    fn visit_ca_field(
        &mut self,
        c: &mut Self::Context,
        p: &mut CaField<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_ca_type(&mut self, c: &mut Self::Context, p: &mut CaType) {
        p.recurse(c, self.object())
    }
    fn visit_call_type(&mut self, c: &mut Self::Context, p: &mut CallType) {
        p.recurse(c, self.object())
    }
    fn visit_case(
        &mut self,
        c: &mut Self::Context,
        p: &mut Case<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_catch(
        &mut self,
        c: &mut Self::Context,
        p: &mut Catch<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_attr(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassAttr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_const(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassConst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassGetExpr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_id(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassId<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_id_(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassId_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_kind(&mut self, c: &mut Self::Context, p: &mut ClassKind) {
        p.recurse(c, self.object())
    }
    fn visit_class_tparams(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassTparams<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_typeconst(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassTypeconst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_var(
        &mut self,
        c: &mut Self::Context,
        p: &mut ClassVar<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_class_(
        &mut self,
        c: &mut Self::Context,
        p: &mut Class_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_collection_targ(&mut self, c: &mut Self::Context, p: &mut CollectionTarg<Self::Hi>) {
        p.recurse(c, self.object())
    }
    fn visit_constraint_kind(&mut self, c: &mut Self::Context, p: &mut ConstraintKind) {
        p.recurse(c, self.object())
    }
    fn visit_def(
        &mut self,
        c: &mut Self::Context,
        p: &mut Def<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_enum_(&mut self, c: &mut Self::Context, p: &mut Enum_) {
        p.recurse(c, self.object())
    }
    fn visit_expr(
        &mut self,
        c: &mut Self::Context,
        p: &mut Expr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_expr_(
        &mut self,
        c: &mut Self::Context,
        p: &mut Expr_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_field(
        &mut self,
        c: &mut Self::Context,
        p: &mut Field<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_file_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &mut FileAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_fun_def(
        &mut self,
        c: &mut Self::Context,
        p: &mut FunDef<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_fun_kind(&mut self, c: &mut Self::Context, p: &mut FunKind) {
        p.recurse(c, self.object())
    }
    fn visit_fun_param(
        &mut self,
        c: &mut Self::Context,
        p: &mut FunParam<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_fun_variadicity(
        &mut self,
        c: &mut Self::Context,
        p: &mut FunVariadicity<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_fun_(
        &mut self,
        c: &mut Self::Context,
        p: &mut Fun_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_func_body(
        &mut self,
        c: &mut Self::Context,
        p: &mut FuncBody<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_func_reactive(&mut self, c: &mut Self::Context, p: &mut FuncReactive) {
        p.recurse(c, self.object())
    }
    fn visit_gconst(
        &mut self,
        c: &mut Self::Context,
        p: &mut Gconst<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_hint(&mut self, c: &mut Self::Context, p: &mut Hint) {
        p.recurse(c, self.object())
    }
    fn visit_hint_fun(&mut self, c: &mut Self::Context, p: &mut HintFun) {
        p.recurse(c, self.object())
    }
    fn visit_hint_(&mut self, c: &mut Self::Context, p: &mut Hint_) {
        p.recurse(c, self.object())
    }
    fn visit_id(&mut self, c: &mut Self::Context, p: &mut Id) {
        p.recurse(c, self.object())
    }
    fn visit_import_flavor(&mut self, c: &mut Self::Context, p: &mut ImportFlavor) {
        p.recurse(c, self.object())
    }
    fn visit_insteadof_alias(&mut self, c: &mut Self::Context, p: &mut InsteadofAlias) {
        p.recurse(c, self.object())
    }
    fn visit_is_coroutine(&mut self, c: &mut Self::Context, p: &mut IsCoroutine) {
        p.recurse(c, self.object())
    }
    fn visit_is_extends(&mut self, c: &mut Self::Context, p: &mut IsExtends) {
        p.recurse(c, self.object())
    }
    fn visit_is_reference(&mut self, c: &mut Self::Context, p: &mut IsReference) {
        p.recurse(c, self.object())
    }
    fn visit_is_variadic(&mut self, c: &mut Self::Context, p: &mut IsVariadic) {
        p.recurse(c, self.object())
    }
    fn visit_kvc_kind(&mut self, c: &mut Self::Context, p: &mut KvcKind) {
        p.recurse(c, self.object())
    }
    fn visit_lid(&mut self, c: &mut Self::Context, p: &mut Lid) {
        p.recurse(c, self.object())
    }
    fn visit_method_redeclaration(
        &mut self,
        c: &mut Self::Context,
        p: &mut MethodRedeclaration<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_method_(
        &mut self,
        c: &mut Self::Context,
        p: &mut Method_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_mutable_return(&mut self, c: &mut Self::Context, p: &mut MutableReturn) {
        p.recurse(c, self.object())
    }
    fn visit_nast_shape_info(&mut self, c: &mut Self::Context, p: &mut NastShapeInfo) {
        p.recurse(c, self.object())
    }
    fn visit_ns_kind(&mut self, c: &mut Self::Context, p: &mut NsKind) {
        p.recurse(c, self.object())
    }
    fn visit_nsenv(&mut self, c: &mut Self::Context, p: &mut Nsenv) {
        p.recurse(c, self.object())
    }
    fn visit_og_null_flavor(&mut self, c: &mut Self::Context, p: &mut OgNullFlavor) {
        p.recurse(c, self.object())
    }
    fn visit_param_kind(&mut self, c: &mut Self::Context, p: &mut ParamKind) {
        p.recurse(c, self.object())
    }
    fn visit_param_mutability(&mut self, c: &mut Self::Context, p: &mut ParamMutability) {
        p.recurse(c, self.object())
    }
    fn visit_program(
        &mut self,
        c: &mut Self::Context,
        p: &mut Program<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_pstring(&mut self, c: &mut Self::Context, p: &mut Pstring) {
        p.recurse(c, self.object())
    }
    fn visit_pu_enum(
        &mut self,
        c: &mut Self::Context,
        p: &mut PuEnum<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_pu_member(
        &mut self,
        c: &mut Self::Context,
        p: &mut PuMember<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_record_def(
        &mut self,
        c: &mut Self::Context,
        p: &mut RecordDef<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_reify_kind(&mut self, c: &mut Self::Context, p: &mut ReifyKind) {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_info(&mut self, c: &mut Self::Context, p: &mut ShapeFieldInfo) {
        p.recurse(c, self.object())
    }
    fn visit_shape_field_name(&mut self, c: &mut Self::Context, p: &mut ShapeFieldName) {
        p.recurse(c, self.object())
    }
    fn visit_sid(&mut self, c: &mut Self::Context, p: &mut Sid) {
        p.recurse(c, self.object())
    }
    fn visit_stmt(
        &mut self,
        c: &mut Self::Context,
        p: &mut Stmt<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_stmt_(
        &mut self,
        c: &mut Self::Context,
        p: &mut Stmt_<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_targ(&mut self, c: &mut Self::Context, p: &mut Targ<Self::Hi>) {
        p.recurse(c, self.object())
    }
    fn visit_tparam(
        &mut self,
        c: &mut Self::Context,
        p: &mut Tparam<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_tprim(&mut self, c: &mut Self::Context, p: &mut Tprim) {
        p.recurse(c, self.object())
    }
    fn visit_type_hint(&mut self, c: &mut Self::Context, p: &mut TypeHint<Self::Hi>) {
        p.recurse(c, self.object())
    }
    fn visit_type_hint_(&mut self, c: &mut Self::Context, p: &mut TypeHint_) {
        p.recurse(c, self.object())
    }
    fn visit_typeconst_abstract_kind(
        &mut self,
        c: &mut Self::Context,
        p: &mut TypeconstAbstractKind,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_typedef(
        &mut self,
        c: &mut Self::Context,
        p: &mut Typedef<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_typedef_visibility(&mut self, c: &mut Self::Context, p: &mut TypedefVisibility) {
        p.recurse(c, self.object())
    }
    fn visit_uop(&mut self, c: &mut Self::Context, p: &mut Uop) {
        p.recurse(c, self.object())
    }
    fn visit_use_as_alias(&mut self, c: &mut Self::Context, p: &mut UseAsAlias) {
        p.recurse(c, self.object())
    }
    fn visit_use_as_visibility(&mut self, c: &mut Self::Context, p: &mut UseAsVisibility) {
        p.recurse(c, self.object())
    }
    fn visit_user_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &mut UserAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_using_stmt(
        &mut self,
        c: &mut Self::Context,
        p: &mut UsingStmt<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_variadic_hint(&mut self, c: &mut Self::Context, p: &mut VariadicHint) {
        p.recurse(c, self.object())
    }
    fn visit_variance(&mut self, c: &mut Self::Context, p: &mut Variance) {
        p.recurse(c, self.object())
    }
    fn visit_vc_kind(&mut self, c: &mut Self::Context, p: &mut VcKind) {
        p.recurse(c, self.object())
    }
    fn visit_visibility(&mut self, c: &mut Self::Context, p: &mut Visibility) {
        p.recurse(c, self.object())
    }
    fn visit_where_constraint(&mut self, c: &mut Self::Context, p: &mut WhereConstraint) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        c: &mut Self::Context,
        p: &mut XhpAttr<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_info(&mut self, c: &mut Self::Context, p: &mut XhpAttrInfo) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attr_tag(&mut self, c: &mut Self::Context, p: &mut XhpAttrTag) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        c: &mut Self::Context,
        p: &mut XhpAttribute<Self::Ex, Self::Fb, Self::En, Self::Hi>,
    ) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child(&mut self, c: &mut Self::Context, p: &mut XhpChild) {
        p.recurse(c, self.object())
    }
    fn visit_xhp_child_op(&mut self, c: &mut Self::Context, p: &mut XhpChildOp) {
        p.recurse(c, self.object())
    }
}
