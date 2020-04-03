/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */
use smart_constructors::SmartConstructors;

pub trait FlattenOp {
    type S;
    fn is_zero(s: &Self::S) -> bool;
    fn zero() -> Self::S;
    fn flatten(lst: Vec<Self::S>) -> Self::S;
}

pub trait FlattenSmartConstructors<'src, State>
: SmartConstructors<'src, State> + FlattenOp<S=<Self as SmartConstructors<'src, State>>::R>
{
    fn make_missing(&mut self, _: usize) -> Self::R {
       Self::zero()
    }

    fn make_token(&mut self, _: Self::Token) -> Self::R {
        Self::zero()
    }

    fn make_list(&mut self, _: Vec<Self::R>, _: usize) -> Self::R {
        Self::zero()
    }

    fn make_end_of_file(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_script(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_qualified_name(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_literal_expression(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_prefixed_string_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_variable_expression(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_pipe_variable_expression(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_file_attribute_specification(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_enum_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_enumerator(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_record_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_record_field(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_alias_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7))
        }
    }

    fn make_property_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_property_declarator(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_namespace_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_namespace_body(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_namespace_empty_body(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_namespace_use_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_namespace_group_use_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_namespace_use_clause(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_function_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_function_declaration_header(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_where_clause(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_where_constraint(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_methodish_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_methodish_trait_resolution(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_classish_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_classish_body(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_trait_use_precedence_item(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_trait_use_alias_item(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_trait_use_conflict_resolution(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_trait_use(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_require_clause(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_const_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_constant_declarator(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_type_const_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_decorated_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_parameter_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_variadic_parameter(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_old_attribute_specification(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_attribute_specification(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_attribute(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_inclusion_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_inclusion_directive(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_compound_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_expression_statement(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_markup_section(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_markup_suffix(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_unset_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_using_statement_block_scoped(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_using_statement_function_scoped(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_while_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_if_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_elseif_clause(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_else_clause(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_try_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_catch_clause(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_finally_clause(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_do_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_for_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_foreach_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_switch_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_switch_section(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_switch_fallthrough(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_case_label(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_default_label(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_return_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_goto_label(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_goto_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_throw_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_break_statement(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_continue_statement(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_echo_statement(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_concurrent_statement(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_simple_initializer(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_anonymous_class(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_anonymous_function(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_anonymous_function_use_clause(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_lambda_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_lambda_signature(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_cast_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_scope_resolution_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_member_selection_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_safe_member_selection_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_embedded_member_selection_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_yield_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_yield_from_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_prefix_unary_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_postfix_unary_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_binary_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_is_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_as_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_nullable_as_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_conditional_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_eval_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_define_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_isset_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_function_call_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_function_pointer_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_parenthesized_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_braced_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_embedded_braced_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_list_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_collection_literal_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_object_creation_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_constructor_call(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_record_creation_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_array_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_darray_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_dictionary_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_keyset_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_varray_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_vector_intrinsic_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_element_initializer(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_subscript_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_embedded_subscript_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_awaitable_creation_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_children_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_children_parenthesized_list(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_category_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_enum_type(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_xhp_lateinit(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_xhp_required(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_xhp_class_attribute_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_class_attribute(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_simple_class_attribute(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_xhp_simple_attribute(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_spread_attribute(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_open(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_close(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_type_constant(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_pu_access(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_vector_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_keyset_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_tuple_type_explicit_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_varray_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_vector_array_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_type_parameter(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_type_constraint(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_darray_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_map_array_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_dictionary_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_closure_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_closure_parameter_type_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_classname_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_field_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_field_initializer(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_shape_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_shape_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_tuple_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_generic_type_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_nullable_type_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_like_type_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_soft_type_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_attributized_specifier(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_reified_type_argument(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_type_arguments(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_type_parameters(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_tuple_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_union_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_intersection_type_specifier(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2))
        }
    }

    fn make_error(&mut self, arg0: Self::R) -> Self::R {
        if Self::is_zero(&arg0) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0))
        }
    }

    fn make_list_item(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_pocket_atom_expression(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_pocket_identifier_expression(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_pocket_atom_mapping_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_pocket_enum_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_pocket_field_type_expr_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_pocket_field_type_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_pocket_mapping_id_declaration(&mut self, arg0: Self::R, arg1: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1))
        }
    }

    fn make_pocket_mapping_type_declaration(&mut self, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> Self::R {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero()
        } else {
          Self::flatten(vec!(arg0, arg1, arg2, arg3))
        }
    }

}
