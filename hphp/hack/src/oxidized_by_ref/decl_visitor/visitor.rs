// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
//
// @generated SignedSource<<881fa623a5c3df235da52fe4d3cca6ff>>
//
// To regenerate this file, run:
//   hphp/hack/src/oxidized_regen.sh

#![allow(unused_imports)]
#![allow(unused_variables)]
use super::node::Node;
use crate::ast_defs::*;
use crate::ast_defs::{self};
use crate::direct_decl_parser::*;
use crate::direct_decl_parser::{self};
use crate::shallow_decl_defs::*;
use crate::shallow_decl_defs::{self};
use crate::t_shape_map::*;
use crate::t_shape_map::{self};
use crate::typing_defs::*;
use crate::typing_defs::{self};
use crate::typing_defs_core::*;
use crate::typing_defs_core::{self};
use crate::typing_reason::*;
use crate::typing_reason::{self};
use crate::xhp_attribute::*;
use crate::xhp_attribute::{self};
use crate::*;
pub trait Visitor<'a> {
    fn object(&mut self) -> &mut dyn Visitor<'a>;
    fn visit_id(&mut self, p: &'a Id<'a>) {
        p.recurse(self.object())
    }
    fn visit_xhp_enum_value(&mut self, p: &'a XhpEnumValue<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_class_const(&mut self, p: &'a ShallowClassConst<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_typeconst(&mut self, p: &'a ShallowTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_prop(&mut self, p: &'a ShallowProp<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_method(&mut self, p: &'a ShallowMethod<'a>) {
        p.recurse(self.object())
    }
    fn visit_shallow_class(&mut self, p: &'a ShallowClass<'a>) {
        p.recurse(self.object())
    }
    fn visit_decl(&mut self, p: &'a Decl<'a>) {
        p.recurse(self.object())
    }
    fn visit_type_origin(&mut self, p: &'a TypeOrigin<'a>) {
        p.recurse(self.object())
    }
    fn visit_pos_string(&mut self, p: &'a PosString<'a>) {
        p.recurse(self.object())
    }
    fn visit_pos_byte_string(&mut self, p: &'a PosByteString<'a>) {
        p.recurse(self.object())
    }
    fn visit_tshape_field_name(&mut self, p: &'a TshapeFieldName<'a>) {
        p.recurse(self.object())
    }
    fn visit_dependent_type(&mut self, p: &'a DependentType) {
        p.recurse(self.object())
    }
    fn visit_user_attribute_param(&mut self, p: &'a UserAttributeParam<'a>) {
        p.recurse(self.object())
    }
    fn visit_user_attribute(&mut self, p: &'a UserAttribute<'a>) {
        p.recurse(self.object())
    }
    fn visit_tparam(&mut self, p: &'a Tparam<'a>) {
        p.recurse(self.object())
    }
    fn visit_where_constraint(&mut self, p: &'a WhereConstraint<'a>) {
        p.recurse(self.object())
    }
    fn visit_capability(&mut self, p: &'a Capability<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_implicit_params(&mut self, p: &'a FunImplicitParams<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_param(&mut self, p: &'a FunParam<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_type(&mut self, p: &'a FunType<'a>) {
        p.recurse(self.object())
    }
    fn visit_neg_type(&mut self, p: &'a NegType<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty(&mut self, p: &'a Ty<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_field_type(&mut self, p: &'a ShapeFieldType<'a>) {
        p.recurse(self.object())
    }
    fn visit_ty_(&mut self, p: &'a Ty_<'a>) {
        p.recurse(self.object())
    }
    fn visit_taccess_type(&mut self, p: &'a TaccessType<'a>) {
        p.recurse(self.object())
    }
    fn visit_exact(&mut self, p: &'a Exact<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_refinement(&mut self, p: &'a ClassRefinement<'a>) {
        p.recurse(self.object())
    }
    fn visit_refined_const(&mut self, p: &'a RefinedConst<'a>) {
        p.recurse(self.object())
    }
    fn visit_refined_const_bound(&mut self, p: &'a RefinedConstBound<'a>) {
        p.recurse(self.object())
    }
    fn visit_refined_const_bounds(&mut self, p: &'a RefinedConstBounds<'a>) {
        p.recurse(self.object())
    }
    fn visit_shape_type(&mut self, p: &'a ShapeType<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_const_from(&mut self, p: &'a ClassConstFrom<'a>) {
        p.recurse(self.object())
    }
    fn visit_class_const_ref(&mut self, p: &'a ClassConstRef<'a>) {
        p.recurse(self.object())
    }
    fn visit_const_decl(&mut self, p: &'a ConstDecl<'a>) {
        p.recurse(self.object())
    }
    fn visit_fun_elt(&mut self, p: &'a FunElt<'a>) {
        p.recurse(self.object())
    }
    fn visit_module_reference(&mut self, p: &'a ModuleReference<'a>) {
        p.recurse(self.object())
    }
    fn visit_module_def_type(&mut self, p: &'a ModuleDefType<'a>) {
        p.recurse(self.object())
    }
    fn visit_abstract_typeconst(&mut self, p: &'a AbstractTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_concrete_typeconst(&mut self, p: &'a ConcreteTypeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_typeconst(&mut self, p: &'a Typeconst<'a>) {
        p.recurse(self.object())
    }
    fn visit_enum_type(&mut self, p: &'a EnumType<'a>) {
        p.recurse(self.object())
    }
    fn visit_typedef_type(&mut self, p: &'a TypedefType<'a>) {
        p.recurse(self.object())
    }
    fn visit_expr_dep_type_reason(&mut self, p: &'a ExprDepTypeReason<'a>) {
        p.recurse(self.object())
    }
    fn visit_blame(&mut self, p: &'a Blame<'a>) {
        p.recurse(self.object())
    }
    fn visit_t_(&mut self, p: &'a T_<'a>) {
        p.recurse(self.object())
    }
    fn visit_decls(&mut self, p: &'a Decls<'a>) {
        p.recurse(self.object())
    }
    fn visit_tshape_field(&mut self, p: &'a TShapeField<'a>) {
        p.recurse(self.object())
    }
    fn visit_variance(&mut self, p: &'a Variance) {
        p.recurse(self.object())
    }
    fn visit_constraint_kind(&mut self, p: &'a ConstraintKind) {
        p.recurse(self.object())
    }
    fn visit_abstraction(&mut self, p: &'a Abstraction) {
        p.recurse(self.object())
    }
    fn visit_classish_kind(&mut self, p: &'a ClassishKind) {
        p.recurse(self.object())
    }
    fn visit_fun_kind(&mut self, p: &'a FunKind) {
        p.recurse(self.object())
    }
    fn visit_visibility(&mut self, p: &'a Visibility) {
        p.recurse(self.object())
    }
    fn visit_tprim(&mut self, p: &'a Tprim) {
        p.recurse(self.object())
    }
    fn visit_typedef_visibility(&mut self, p: &'a TypedefVisibility) {
        p.recurse(self.object())
    }
    fn visit_reify_kind(&mut self, p: &'a ReifyKind) {
        p.recurse(self.object())
    }
    fn visit_class_const_kind(&mut self, p: &'a ClassConstKind) {
        p.recurse(self.object())
    }
    fn visit_arg_position(&mut self, p: &'a ArgPosition) {
        p.recurse(self.object())
    }
    fn visit_blame_source(&mut self, p: &'a BlameSource) {
        p.recurse(self.object())
    }
    fn visit_tag(&mut self, p: &'a Tag) {
        p.recurse(self.object())
    }
    fn visit_xhp_attribute(&mut self, p: &'a XhpAttribute) {
        p.recurse(self.object())
    }
}
