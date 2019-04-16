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
use crate::lexable_token::LexableToken;
use crate::smart_constructors::NodeType;

pub trait SmartConstructors {
    type Token: LexableToken;
    type R: NodeType;
    fn make_missing(offset : usize) -> Self::R;
    fn make_token(arg0: Self::Token) -> Self::R;
    fn make_list(arg0: Box<Vec<Self::R>>, offset: usize) -> Self::R;
    fn make_end_of_file(arg0 : Self::R) -> Self::R;
    fn make_script(arg0 : Self::R) -> Self::R;
    fn make_qualified_name(arg0 : Self::R) -> Self::R;
    fn make_simple_type_specifier(arg0 : Self::R) -> Self::R;
    fn make_literal_expression(arg0 : Self::R) -> Self::R;
    fn make_prefixed_string_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_variable_expression(arg0 : Self::R) -> Self::R;
    fn make_pipe_variable_expression(arg0 : Self::R) -> Self::R;
    fn make_file_attribute_specification(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_enum_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_enumerator(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_record_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_record_field(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_alias_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R;
    fn make_property_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_property_declarator(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_namespace_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_namespace_body(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_namespace_empty_body(arg0 : Self::R) -> Self::R;
    fn make_namespace_use_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_namespace_group_use_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_namespace_use_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_function_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_function_declaration_header(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_where_clause(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_where_constraint(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_methodish_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_methodish_trait_resolution(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_classish_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_classish_body(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_trait_use_precedence_item(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_trait_use_alias_item(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_trait_use_conflict_resolution(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_trait_use(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_require_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_const_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_constant_declarator(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_type_const_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_decorated_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_parameter_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_variadic_parameter(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_attribute_specification(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_inclusion_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_inclusion_directive(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_compound_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_alternate_loop_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_expression_statement(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_markup_section(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_markup_suffix(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_unset_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_let_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_using_statement_block_scoped(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_using_statement_function_scoped(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_declare_directive_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_declare_block_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_while_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_if_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_elseif_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_else_clause(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_alternate_if_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_alternate_elseif_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_alternate_else_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_try_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_catch_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_finally_clause(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_do_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_for_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_foreach_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R;
    fn make_switch_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_alternate_switch_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R;
    fn make_switch_section(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_switch_fallthrough(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_case_label(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_default_label(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_return_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_goto_label(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_goto_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_throw_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_break_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_continue_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_echo_statement(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_concurrent_statement(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_simple_initializer(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_anonymous_class(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_anonymous_function(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R;
    fn make_php7_anonymous_function(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R;
    fn make_anonymous_function_use_clause(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_lambda_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_lambda_signature(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_cast_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_scope_resolution_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_member_selection_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_safe_member_selection_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_embedded_member_selection_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_yield_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_yield_from_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_prefix_unary_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_postfix_unary_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_binary_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_instanceof_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_is_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_as_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_nullable_as_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_conditional_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_eval_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_empty_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_define_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_halt_compiler_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_isset_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_function_call_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_parenthesized_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_braced_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_embedded_braced_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_list_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_collection_literal_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_object_creation_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_constructor_call(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_record_creation_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_array_creation_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_array_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_darray_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_dictionary_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_keyset_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_varray_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_vector_intrinsic_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_element_initializer(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_subscript_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_embedded_subscript_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_awaitable_creation_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_children_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_children_parenthesized_list(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_category_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_enum_type(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_xhp_required(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_xhp_class_attribute_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_class_attribute(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_simple_class_attribute(arg0 : Self::R) -> Self::R;
    fn make_xhp_simple_attribute(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_spread_attribute(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_open(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_xhp_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_xhp_close(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_type_constant(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_vector_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_keyset_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_tuple_type_explicit_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_varray_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_vector_array_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_type_parameter(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_type_constraint(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_darray_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R;
    fn make_map_array_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_dictionary_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_closure_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R;
    fn make_closure_parameter_type_specifier(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_classname_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_field_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_field_initializer(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_shape_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_shape_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_tuple_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_generic_type_specifier(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_nullable_type_specifier(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_like_type_specifier(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_soft_type_specifier(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_reified_type_argument(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_type_arguments(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_type_parameters(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_tuple_type_specifier(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R;
    fn make_error(arg0 : Self::R) -> Self::R;
    fn make_list_item(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_pocket_atom_expression(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_pocket_identifier_expression(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R;
    fn make_pocket_atom_mapping_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_pocket_enum_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R;
    fn make_pocket_field_type_expr_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_pocket_field_type_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;
    fn make_pocket_mapping_id_declaration(arg0 : Self::R, arg1 : Self::R) -> Self::R;
    fn make_pocket_mapping_type_declaration(arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R;

}