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
 *   buck run //hphp/hack/src:generate_full_fidelity -- --rust
 *
 **
 *
 */
use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::source_text::SourceText;

pub trait SmartConstructors<'src, State> {
    type Token: LexableToken;
    type R;

    fn initial_state(env: &ParserEnv, src: &SourceText<'src>) -> State;

    fn make_missing(st: State, offset : usize) -> (State, Self::R);
    fn make_token(st: State, arg0: Self::Token) -> (State, Self::R);
    fn make_list(st: State, arg0: Vec<Self::R>, offset: usize) -> (State, Self::R);
    fn make_end_of_file(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_script(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_qualified_name(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_simple_type_specifier(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_literal_expression(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_prefixed_string_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_variable_expression(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_pipe_variable_expression(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_file_attribute_specification(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_enum_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R);
    fn make_enumerator(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_record_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R);
    fn make_record_field(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_alias_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> (State, Self::R);
    fn make_property_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_property_declarator(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_namespace_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_namespace_body(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_namespace_empty_body(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_namespace_use_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_namespace_group_use_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R);
    fn make_namespace_use_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_function_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_function_declaration_header(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R);
    fn make_where_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_where_constraint(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_methodish_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_methodish_trait_resolution(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_classish_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> (State, Self::R);
    fn make_classish_body(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_trait_use_precedence_item(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_trait_use_alias_item(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_trait_use_conflict_resolution(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_trait_use(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_require_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_const_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_constant_declarator(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_type_const_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R);
    fn make_decorated_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_parameter_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_variadic_parameter(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_attribute_specification(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_inclusion_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_inclusion_directive(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_compound_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_expression_statement(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_markup_section(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_markup_suffix(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_unset_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_let_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_using_statement_block_scoped(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_using_statement_function_scoped(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_while_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_if_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R);
    fn make_elseif_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_else_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_try_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_catch_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_finally_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_do_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R);
    fn make_for_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R);
    fn make_foreach_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R);
    fn make_switch_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R);
    fn make_switch_section(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_switch_fallthrough(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_case_label(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_default_label(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_return_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_goto_label(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_goto_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_throw_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_break_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_continue_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_echo_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_concurrent_statement(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_simple_initializer(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_anonymous_class(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R);
    fn make_anonymous_function(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> (State, Self::R);
    fn make_anonymous_function_use_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_lambda_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_lambda_signature(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_cast_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_scope_resolution_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_safe_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_embedded_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_yield_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_yield_from_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_prefix_unary_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_postfix_unary_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_binary_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_instanceof_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_is_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_as_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_nullable_as_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_conditional_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_eval_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_define_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_halt_compiler_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_isset_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_function_call_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_parenthesized_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_braced_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_embedded_braced_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_list_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_collection_literal_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_object_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_constructor_call(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_record_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_array_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_array_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_darray_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_dictionary_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_keyset_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_varray_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_vector_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_element_initializer(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_subscript_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_embedded_subscript_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_awaitable_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_xhp_children_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_children_parenthesized_list(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_category_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_enum_type(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_xhp_lateinit(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_xhp_required(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_xhp_class_attribute_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_class_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_xhp_simple_class_attribute(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_xhp_simple_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_spread_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_xhp_open(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_xhp_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_xhp_close(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_type_constant(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_vector_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_keyset_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_tuple_type_explicit_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_varray_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_vector_array_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_type_parameter(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_type_constraint(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_darray_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R);
    fn make_map_array_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_dictionary_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_closure_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R);
    fn make_closure_parameter_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_classname_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_field_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_field_initializer(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_shape_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_shape_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_tuple_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_generic_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_nullable_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_like_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_soft_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_attributized_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_reified_type_argument(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_type_arguments(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_type_parameters(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_tuple_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R);
    fn make_error(st: State, arg0 : Self::R) -> (State, Self::R);
    fn make_list_item(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_pocket_atom_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_pocket_identifier_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R);
    fn make_pocket_atom_mapping_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_pocket_enum_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R);
    fn make_pocket_field_type_expr_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_pocket_field_type_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);
    fn make_pocket_mapping_id_declaration(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R);
    fn make_pocket_mapping_type_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R);

}
