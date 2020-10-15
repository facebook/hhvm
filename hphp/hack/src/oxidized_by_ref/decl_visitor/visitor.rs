// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<88de7a867192327c3deb9be2afce5823>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_by_ref/regen.sh

#![allow(unused_variables)]
use super::node::Node;
use crate::{
    aast_defs::*, ast_defs::*, decl_defs::*, direct_decl_parser::*, shallow_decl_defs::*,
    shape_map::*, typing_defs::*, typing_defs_core::*, typing_reason::*,
};
pub trait Visitor<'a> {
    fn object(&mut self) -> &mut dyn Visitor<'a>;
    fn visit_arg_position(&mut self, p: &'a ArgPosition) {
        p.recurse(self.object())
    }
    fn visit_blame(&mut self, p: &'a Blame<'a>) {
        p.recurse(self.object())
    }
    fn visit_blame_source(&mut self, p: &'a BlameSource) {
        p.recurse(self.object())
    }
    fn visit_class_kind(&mut self, p: &'a ClassKind) {
        p.recurse(self.object())
    }
    fn visit_const_decl(&mut self, p: &'a ConstDecl<'a>) {
        p.recurse(self.object())
    }
    fn visit_constraint_kind(&mut self, p: &'a ConstraintKind) {
        p.recurse(self.object())
    }
    fn visit_decls(&mut self, p: &'a Decls<'a>) {
        p.recurse(self.object())
    }
    fn visit_dependent_type(&mut self, p: &'a DependentType) {
        p.recurse(self.object())
    }
    fn visit_enum_type(&mut self, p: &'a EnumType<'a>) {
        p.recurse(self.object())
    }
    fn visit_exact(&mut self, p: &'a Exact) {
        p.recurse(self.object())
    }
    fn visit_expr_dep_type_reason(&mut self, p: &'a ExprDepTypeReason<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_arity(&mut self, p: &'a FunArity<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_elt(&mut self, p: &'a FunElt<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_implicit_params(&mut self, p: &'a FunImplicitParams<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_kind(&mut self, p: &'a FunKind) {
        p.recurse(self.object())
    }
    fn visit_fun_param(&mut self, p: &'a FunParam<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_type(&mut self, p: &'a FunType<'a>) {
        p.recurse(self.object())
    }
    fn visit_id(&mut self, p: &'a Id<'a>) {
        p.recurse(self.object())
    }
    fn visit_method_reactivity(&mut self, p: &'a MethodReactivity<'a>) {
        p.recurse(self.object())
    }
    fn visit_param_rx_annotation(&mut self, p: &'a ParamRxAnnotation<'a>) {
        p.recurse(self.object())
    }
    fn visit_possibly_enforced_ty(&mut self, p: &'a PossiblyEnforcedTy<'a>) {
        p.recurse(self.object())
    }
    fn visit_reactivity(&mut self, p: &'a Reactivity<'a>) {
        p.recurse(self.object())
    }
    fn visit_reason(&mut self, p: &'a Reason<'a>) {
        p.recurse(self.object())
    }
    fn visit_reify_kind(&mut self, p: &'a ReifyKind) {
        p.recurse(self.object())
    }
    fn visit_shallow_class(&mut self, p: &'a ShallowClass<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_class_const(&mut self, p: &'a ShallowClassConst<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_method(&mut self, p: &'a ShallowMethod<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_prop(&mut self, p: &'a ShallowProp<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_pu_enum(&mut self, p: &'a ShallowPuEnum<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_pu_member(&mut self, p: &'a ShallowPuMember<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_typeconst(&mut self, p: &'a ShallowTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field(&mut self, p: &'a ShapeField<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field_name(&mut self, p: &'a ShapeFieldName<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field_type(&mut self, p: &'a ShapeFieldType<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_kind(&mut self, p: &'a ShapeKind) {
        p.recurse(self.object())
    }
    fn visit_taccess_type(&mut self, p: &'a TaccessType<'a>) {
        p.recurse(self.object())
    }
    fn visit_tparam(&mut self, p: &'a Tparam<'a>) {
        p.recurse(self.object())
    }
    fn visit_tprim(&mut self, p: &'a Tprim<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty(&mut self, p: &'a Ty<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty_(&mut self, p: &'a Ty_<'a>) {
        p.recurse(self.object())
    }
    fn visit_typeconst_abstract_kind(&mut self, p: &'a TypeconstAbstractKind<'a>) {
        p.recurse(self.object())
    }
    fn visit_typedef_type(&mut self, p: &'a TypedefType<'a>) {
        p.recurse(self.object())
    }
    fn visit_typedef_visibility(&mut self, p: &'a TypedefVisibility) {
        p.recurse(self.object())
    }
    fn visit_user_attribute(&mut self, p: &'a UserAttribute<'a>) {
        p.recurse(self.object())
    }
    fn visit_variance(&mut self, p: &'a Variance) {
        p.recurse(self.object())
    }
    fn visit_visibility(&mut self, p: &'a Visibility) {
        p.recurse(self.object())
    }
    fn visit_where_constraint(&mut self, p: &'a WhereConstraint<'a>) {
        p.recurse(self.object())
    }
    fn visit_xhp_attr(&mut self, p: &'a XhpAttr) {
        p.recurse(self.object())
    }
    fn visit_xhp_attr_tag(&mut self, p: &'a XhpAttrTag) {
        p.recurse(self.object())
    }
}
