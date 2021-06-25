// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<08fc8a0ec548786903d89f8be904989b>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_variables)]
use super::node::Node;
use crate::{
    aast_defs::*, ast_defs::*, direct_decl_parser::*, shallow_decl_defs::*, t_shape_map::*,
    typing_defs::*, typing_defs_core::*, typing_reason::*,
};
pub trait Visitor<'a> {
    fn object(&mut self) -> &mut dyn Visitor<'a>;
    fn visit_abstract_typeconst(&mut self, p: &'a AbstractTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_arg_position(&mut self, p: &'a ArgPosition) {
        p.recurse(self.object())
    }
    fn visit_blame(&mut self, p: &'a Blame<'a>) {
        p.recurse(self.object())
    }
    fn visit_blame_source(&mut self, p: &'a BlameSource) {
        p.recurse(self.object())
    }
    fn visit_capability(&mut self, p: &'a Capability<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_const_from(&mut self, p: &'a ClassConstFrom<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_const_kind(&mut self, p: &'a ClassConstKind) {
        p.recurse(self.object())
    }
    fn visit_class_const_ref(&mut self, p: &'a ClassConstRef<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_kind(&mut self, p: &'a ClassKind) {
        p.recurse(self.object())
    }
    fn visit_collection_style(&mut self, p: &'a CollectionStyle) {
        p.recurse(self.object())
    }
    fn visit_concrete_typeconst(&mut self, p: &'a ConcreteTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_const_decl(&mut self, p: &'a ConstDecl<'a>) {
        p.recurse(self.object())
    }
    fn visit_constraint_kind(&mut self, p: &'a ConstraintKind) {
        p.recurse(self.object())
    }
    fn visit_decl(&mut self, p: &'a Decl<'a>) {
        p.recurse(self.object())
    }
    fn visit_decls(&mut self, p: &'a Decls<'a>) {
        p.recurse(self.object())
    }
    fn visit_dependent_type(&mut self, p: &'a DependentType) {
        p.recurse(self.object())
    }
    fn visit_enforcement(&mut self, p: &'a Enforcement<'a>) {
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
    fn visit_ifc_fun_decl(&mut self, p: &'a IfcFunDecl<'a>) {
        p.recurse(self.object())
    }
    fn visit_partially_abstract_typeconst(&mut self, p: &'a PartiallyAbstractTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_pos_byte_string(&mut self, p: &'a PosByteString<'a>) {
        p.recurse(self.object())
    }
    fn visit_pos_string(&mut self, p: &'a PosString<'a>) {
        p.recurse(self.object())
    }
    fn visit_possibly_enforced_ty(&mut self, p: &'a PossiblyEnforcedTy<'a>) {
        p.recurse(self.object())
    }
    fn visit_record_def_type(&mut self, p: &'a RecordDefType<'a>) {
        p.recurse(self.object())
    }
    fn visit_record_field_req(&mut self, p: &'a RecordFieldReq) {
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
    fn visit_shallow_typeconst(&mut self, p: &'a ShallowTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field_type(&mut self, p: &'a ShapeFieldType<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_kind(&mut self, p: &'a ShapeKind) {
        p.recurse(self.object())
    }
    fn visit_tshape_field(&mut self, p: &'a TShapeField<'a>) {
        p.recurse(self.object())
    }
    fn visit_t_(&mut self, p: &'a T_<'a>) {
        p.recurse(self.object())
    }
    fn visit_taccess_type(&mut self, p: &'a TaccessType<'a>) {
        p.recurse(self.object())
    }
    fn visit_tparam(&mut self, p: &'a Tparam<'a>) {
        p.recurse(self.object())
    }
    fn visit_tprim(&mut self, p: &'a Tprim) {
        p.recurse(self.object())
    }
    fn visit_tshape_field_name(&mut self, p: &'a TshapeFieldName<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty(&mut self, p: &'a Ty<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty_(&mut self, p: &'a Ty_<'a>) {
        p.recurse(self.object())
    }
    fn visit_typeconst(&mut self, p: &'a Typeconst<'a>) {
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
    fn visit_xhp_enum_value(&mut self, p: &'a XhpEnumValue<'a>) {
        p.recurse(self.object())
    }
}
