// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<61dd5abdb00d3834567cad14fd1b5f02>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::node::Node;
use crate::{aast::*, aast_defs::*, ast_defs::*, doc_comment::*, namespace_env::*};
pub trait Visitor<'a> {
    fn object(&mut self) -> &mut dyn Visitor<'a>;
    fn visit_afield(
        &mut self,
        p: &'a Afield<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_as_expr(
        &mut self,
        p: &'a AsExpr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_bop(&mut self, p: &'a Bop<'a>) {
        p.recurse(self.object())
    }
    fn visit_ca_field(
        &mut self,
        p: &'a CaField<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_ca_type(&mut self, p: &'a CaType<'a>) {
        p.recurse(self.object())
    }
    fn visit_case(
        &mut self,
        p: &'a Case<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_catch(
        &mut self,
        p: &'a Catch<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_abstract_typeconst(&mut self, p: &'a ClassAbstractTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_attr(
        &mut self,
        p: &'a ClassAttr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_concrete_typeconst(&mut self, p: &'a ClassConcreteTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_const(
        &mut self,
        p: &'a ClassConst<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_get_expr(
        &mut self,
        p: &'a ClassGetExpr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_id(
        &mut self,
        p: &'a ClassId<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_id_(
        &mut self,
        p: &'a ClassId_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_kind(&mut self, p: &'a ClassKind) {
        p.recurse(self.object())
    }
    fn visit_class_partially_abstract_typeconst(
        &mut self,
        p: &'a ClassPartiallyAbstractTypeconst<'a>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_typeconst(&mut self, p: &'a ClassTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_typeconst_def(
        &mut self,
        p: &'a ClassTypeconstDef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_var(
        &mut self,
        p: &'a ClassVar<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_class_(
        &mut self,
        p: &'a Class_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_collection_targ(&mut self, p: &'a CollectionTarg<'a, ()>) {
        p.recurse(self.object())
    }
    fn visit_constraint_kind(&mut self, p: &'a ConstraintKind) {
        p.recurse(self.object())
    }
    fn visit_contexts(&mut self, p: &'a Contexts<'a>) {
        p.recurse(self.object())
    }
    fn visit_def(
        &mut self,
        p: &'a Def<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_doc_comment(&mut self, p: &'a DocComment<'a>) {
        p.recurse(self.object())
    }
    fn visit_emit_id(&mut self, p: &'a EmitId) {
        p.recurse(self.object())
    }
    fn visit_enum_(&mut self, p: &'a Enum_<'a>) {
        p.recurse(self.object())
    }
    fn visit_env(&mut self, p: &'a Env<'a>) {
        p.recurse(self.object())
    }
    fn visit_env_annot(&mut self, p: &'a EnvAnnot) {
        p.recurse(self.object())
    }
    fn visit_expr(
        &mut self,
        p: &'a Expr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_expr_(
        &mut self,
        p: &'a Expr_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_expression_tree(
        &mut self,
        p: &'a ExpressionTree<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_field(
        &mut self,
        p: &'a Field<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_file_attribute(
        &mut self,
        p: &'a FileAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_fun_kind(&mut self, p: &'a FunKind) {
        p.recurse(self.object())
    }
    fn visit_fun_param(
        &mut self,
        p: &'a FunParam<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_fun_variadicity(
        &mut self,
        p: &'a FunVariadicity<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_fun_(
        &mut self,
        p: &'a Fun_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_func_body(
        &mut self,
        p: &'a FuncBody<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_function_ptr_id(
        &mut self,
        p: &'a FunctionPtrId<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_gconst(
        &mut self,
        p: &'a Gconst<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_hf_param_info(&mut self, p: &'a HfParamInfo) {
        p.recurse(self.object())
    }
    fn visit_hint(&mut self, p: &'a Hint<'a>) {
        p.recurse(self.object())
    }
    fn visit_hint_fun(&mut self, p: &'a HintFun<'a>) {
        p.recurse(self.object())
    }
    fn visit_hint_(&mut self, p: &'a Hint_<'a>) {
        p.recurse(self.object())
    }
    fn visit_hole_source(&mut self, p: &'a HoleSource) {
        p.recurse(self.object())
    }
    fn visit_id(&mut self, p: &'a Id<'a>) {
        p.recurse(self.object())
    }
    fn visit_import_flavor(&mut self, p: &'a ImportFlavor) {
        p.recurse(self.object())
    }
    fn visit_insteadof_alias(&mut self, p: &'a InsteadofAlias<'a>) {
        p.recurse(self.object())
    }
    fn visit_kvc_kind(&mut self, p: &'a KvcKind) {
        p.recurse(self.object())
    }
    fn visit_lid(&mut self, p: &'a Lid<'a>) {
        p.recurse(self.object())
    }
    fn visit_method_(
        &mut self,
        p: &'a Method_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_nast_shape_info(&mut self, p: &'a NastShapeInfo<'a>) {
        p.recurse(self.object())
    }
    fn visit_ns_kind(&mut self, p: &'a NsKind) {
        p.recurse(self.object())
    }
    fn visit_og_null_flavor(&mut self, p: &'a OgNullFlavor) {
        p.recurse(self.object())
    }
    fn visit_param_kind(&mut self, p: &'a ParamKind) {
        p.recurse(self.object())
    }
    fn visit_readonly_kind(&mut self, p: &'a ReadonlyKind) {
        p.recurse(self.object())
    }
    fn visit_record_def(
        &mut self,
        p: &'a RecordDef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_reify_kind(&mut self, p: &'a ReifyKind) {
        p.recurse(self.object())
    }
    fn visit_shape_field_info(&mut self, p: &'a ShapeFieldInfo<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field_name(&mut self, p: &'a ShapeFieldName<'a>) {
        p.recurse(self.object())
    }
    fn visit_stmt(
        &mut self,
        p: &'a Stmt<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_stmt_(
        &mut self,
        p: &'a Stmt_<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_targ(&mut self, p: &'a Targ<'a, ()>) {
        p.recurse(self.object())
    }
    fn visit_tparam(
        &mut self,
        p: &'a Tparam<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_tprim(&mut self, p: &'a Tprim) {
        p.recurse(self.object())
    }
    fn visit_type_hint(&mut self, p: &'a TypeHint<'a, ()>) {
        p.recurse(self.object())
    }
    fn visit_typedef(
        &mut self,
        p: &'a Typedef<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_typedef_visibility(&mut self, p: &'a TypedefVisibility) {
        p.recurse(self.object())
    }
    fn visit_uop(&mut self, p: &'a Uop) {
        p.recurse(self.object())
    }
    fn visit_use_as_alias(&mut self, p: &'a UseAsAlias<'a>) {
        p.recurse(self.object())
    }
    fn visit_use_as_visibility(&mut self, p: &'a UseAsVisibility) {
        p.recurse(self.object())
    }
    fn visit_user_attribute(
        &mut self,
        p: &'a UserAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_using_stmt(
        &mut self,
        p: &'a UsingStmt<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_variance(&mut self, p: &'a Variance) {
        p.recurse(self.object())
    }
    fn visit_vc_kind(&mut self, p: &'a VcKind) {
        p.recurse(self.object())
    }
    fn visit_visibility(&mut self, p: &'a Visibility) {
        p.recurse(self.object())
    }
    fn visit_where_constraint_hint(&mut self, p: &'a WhereConstraintHint<'a>) {
        p.recurse(self.object())
    }
    fn visit_xhp_attr(
        &mut self,
        p: &'a XhpAttr<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_xhp_attr_info(&mut self, p: &'a XhpAttrInfo) {
        p.recurse(self.object())
    }
    fn visit_xhp_attr_tag(&mut self, p: &'a XhpAttrTag) {
        p.recurse(self.object())
    }
    fn visit_xhp_attribute(
        &mut self,
        p: &'a XhpAttribute<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
    fn visit_xhp_child(&mut self, p: &'a XhpChild<'a>) {
        p.recurse(self.object())
    }
    fn visit_xhp_child_op(&mut self, p: &'a XhpChildOp) {
        p.recurse(self.object())
    }
    fn visit_xhp_simple(
        &mut self,
        p: &'a XhpSimple<'a, &'a crate::pos::Pos<'a>, crate::nast::FuncBodyAnn<'a>, (), ()>,
    ) {
        p.recurse(self.object())
    }
}
