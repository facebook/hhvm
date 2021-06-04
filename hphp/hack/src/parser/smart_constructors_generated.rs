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
use parser_core_types::token_factory::TokenFactory;
use parser_core_types::lexable_token::LexableToken;

pub type Token<S> = <<S as SmartConstructors>::TF as TokenFactory>::Token;
pub type Trivia<S> = <Token<S> as LexableToken>::Trivia;

pub trait SmartConstructors: Clone {
    type TF: TokenFactory;
    type State;
    type R;

    fn state_mut(&mut self) -> &mut Self::State;
    fn into_state(self) -> Self::State;
    fn token_factory_mut(&mut self) -> &mut Self::TF;

    fn make_missing(&mut self, offset : usize) -> Self::R;
    fn make_token(&mut self, arg0: Token<Self>) -> Self::R;
    fn make_list(&mut self, arg0: Vec<Self::R>, offset: usize) -> Self::R;

    fn begin_enumerator(&mut self) {}
    fn begin_enum_class_enumerator(&mut self) {}
    fn begin_constant_declarator(&mut self) {}

    fn make_end_of_file(&mut self, arg0 : Self::R) -> Self::R;
    fn make_script(&mut self, arg0 : Self::R) -> Self::R;
    fn make_qualified_name(&mut self, arg0 : Self::R) -> Self::R;
    fn make_simple_type_specifier(&mut self, arg0 : Self::R) -> Self::R;
    fn make_literal_expression(&mut self, arg0 : Self::R) -> Self::R;
    fn make_prefixed_string_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_prefixed_code_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_variable_expression(&mut self, arg0 : Self::R) -> Self::R;
    fn make_pipe_variable_expression(&mut self, arg0 : Self::R) -> Self::R;
    fn make_file_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_enum_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_enum_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_enum_class_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R;
    fn make_enum_class_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_record_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_record_field(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_alias_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R;
    fn make_property_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_property_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_namespace_declaration(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_namespace_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_namespace_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_namespace_empty_body(&mut self, arg0 : Self::R) -> Self::R;
    fn make_namespace_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_namespace_group_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_namespace_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_function_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_function_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R;
    fn make_contexts(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_where_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_where_constraint(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_methodish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_methodish_trait_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_classish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R;
    fn make_classish_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_trait_use_precedence_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_trait_use_alias_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_trait_use_conflict_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_trait_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_require_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_constant_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_type_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_context_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_decorated_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_parameter_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_variadic_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_old_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_attribute_specification(&mut self, arg0 : Self::R) -> Self::R;
    fn make_attribute(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_inclusion_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_inclusion_directive(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_compound_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_expression_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_markup_section(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_markup_suffix(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_unset_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_using_statement_block_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_using_statement_function_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_while_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_if_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_elseif_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_else_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_try_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_catch_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_finally_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_do_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_for_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_foreach_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_switch_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_switch_section(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_switch_fallthrough(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_case_label(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_default_label(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_return_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_yield_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_throw_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_continue_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_echo_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_concurrent_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_simple_initializer(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_anonymous_class(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_anonymous_function(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R;
    fn make_anonymous_function_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_lambda_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_lambda_signature(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_cast_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_scope_resolution_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_safe_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_embedded_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_yield_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_prefix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_postfix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_binary_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_is_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_nullable_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_conditional_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_eval_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_isset_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_function_call_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_function_pointer_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_parenthesized_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_et_splice_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_embedded_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_list_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_collection_literal_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_object_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_constructor_call(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_record_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_darray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_dictionary_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_keyset_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_varray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_vector_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_element_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_embedded_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_awaitable_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_children_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_children_parenthesized_list(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_category_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_enum_type(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_lateinit(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_xhp_required(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_xhp_class_attribute_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_class_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_simple_class_attribute(&mut self, arg0 : Self::R) -> Self::R;
    fn make_xhp_simple_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_spread_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_open(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_close(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_type_constant(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_vector_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_keyset_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_tuple_type_explicit_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_varray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_function_ctx_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_type_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_type_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_context_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_darray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_dictionary_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_closure_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R;
    fn make_closure_parameter_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_classname_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_field_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_field_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_shape_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_shape_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_tuple_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_generic_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_nullable_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_like_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_soft_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_attributized_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_reified_type_argument(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_type_arguments(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_type_parameters(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_tuple_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_union_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_intersection_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_error(&mut self, arg0 : Self::R) -> Self::R;
    fn make_list_item(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_enum_class_label_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;

}
