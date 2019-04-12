/**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
pub use crate::smart_constructors_generated::*;

pub trait NodeType {
    fn is_missing(&self) -> bool;
    fn is_abstract(&self) -> bool;
    fn is_variable_expression(&self) -> bool;
    fn is_subscript_expression(&self) -> bool;
    fn is_member_selection_expression(&self) -> bool;
    fn is_scope_resolution_expression(&self) -> bool;
    fn is_object_creation_expression(&self) -> bool;
    fn is_qualified_name(&self) -> bool;
    fn is_safe_member_selection_expression(&self) -> bool;
    fn is_function_call_expression(&self) -> bool;
    fn is_list_expression(&self) -> bool;
    fn is_name(&self) -> bool;
    fn is_halt_compiler_expression(&self) -> bool;
    fn is_prefix_unary_expression(&self) -> bool;
}
