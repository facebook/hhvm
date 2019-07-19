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
use crate::coroutine_smart_constructors::*;
use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::SmartConstructors;
use crate::source_text::SourceText;
use crate::syntax::*;
use crate::syntax_smart_constructors::{SyntaxSmartConstructors, StateType};

impl<'src, S, T, Token, Value> SmartConstructors<'src, T>
    for CoroutineSmartConstructors<S>
where
    Token: LexableToken,
    Value: SyntaxValueType<Token> + SyntaxValueWithKind,
    S: SyntaxType<T, Token=Token, Value=Value>,
    T: StateType<'src, S> + CoroutineStateType,
{
    type Token = Token;
    type R = S;

    fn initial_state(env: &ParserEnv, src: &SourceText<'src>) -> T {
        <Self as SyntaxSmartConstructors<'src, Self::R, T>>::initial_state(env, src)
    }

    fn make_missing(st: T, offset: usize) -> (T, Self::R) {
       <Self as SyntaxSmartConstructors<'src, Self::R, T>>::make_missing(st, offset)
    }

    fn make_token(st: T, offset: Self::Token) -> (T, Self::R) {
       <Self as SyntaxSmartConstructors<'src, Self::R, T>>::make_token(st, offset)
    }

    fn make_list(
        st: T,
        lst: Vec<Self::R>,
        offset: usize,
    ) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<'src, Self::R, T>>::make_list(st, lst, offset)
    }

    fn make_end_of_file(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_end_of_file(st, arg0)
    }

    fn make_script(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_script(st, arg0)
    }

    fn make_qualified_name(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_qualified_name(st, arg0)
    }

    fn make_simple_type_specifier(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_simple_type_specifier(st, arg0)
    }

    fn make_literal_expression(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_literal_expression(st, arg0)
    }

    fn make_prefixed_string_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_prefixed_string_expression(st, arg0, arg1)
    }

    fn make_variable_expression(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_variable_expression(st, arg0)
    }

    fn make_pipe_variable_expression(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pipe_variable_expression(st, arg0)
    }

    fn make_file_attribute_specification(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_file_attribute_specification(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_enum_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_enumerator(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_enumerator(st, arg0, arg1, arg2, arg3)
    }

    fn make_record_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_record_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_record_field(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_record_field(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_alias_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_property_declaration(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_property_declarator(st, arg0, arg1)
    }

    fn make_namespace_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_declaration(st, arg0, arg1, arg2)
    }

    fn make_namespace_body(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_body(st, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_empty_body(st, arg0)
    }

    fn make_namespace_use_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_use_declaration(st, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_group_use_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_namespace_use_clause(st, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_function_declaration(st, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_function_declaration_header(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_where_clause(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_where_clause(st, arg0, arg1)
    }

    fn make_where_constraint(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_where_constraint(st, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_methodish_declaration(st, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_methodish_trait_resolution(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_classish_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_classish_body(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_classish_body(st, arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_trait_use_precedence_item(st, arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_trait_use_alias_item(st, arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_trait_use_conflict_resolution(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_trait_use(st, arg0, arg1, arg2)
    }

    fn make_require_clause(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_require_clause(st, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_const_declaration(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_constant_declarator(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_constant_declarator(st, arg0, arg1)
    }

    fn make_type_const_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_const_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_decorated_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_decorated_expression(st, arg0, arg1)
    }

    fn make_parameter_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_parameter_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_variadic_parameter(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_variadic_parameter(st, arg0, arg1, arg2)
    }

    fn make_attribute_specification(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_attribute_specification(st, arg0, arg1, arg2)
    }

    fn make_inclusion_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_inclusion_expression(st, arg0, arg1)
    }

    fn make_inclusion_directive(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_inclusion_directive(st, arg0, arg1)
    }

    fn make_compound_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_compound_statement(st, arg0, arg1, arg2)
    }

    fn make_expression_statement(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_expression_statement(st, arg0, arg1)
    }

    fn make_markup_section(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_markup_section(st, arg0, arg1, arg2, arg3)
    }

    fn make_markup_suffix(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_markup_suffix(st, arg0, arg1)
    }

    fn make_unset_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_unset_statement(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_let_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_let_statement(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_using_statement_block_scoped(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_using_statement_function_scoped(st, arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_while_statement(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_if_statement(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_elseif_clause(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_else_clause(st, arg0, arg1)
    }

    fn make_try_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_try_statement(st, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_catch_clause(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_finally_clause(st, arg0, arg1)
    }

    fn make_do_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_do_statement(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_for_statement(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_foreach_statement(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_switch_statement(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_switch_section(st, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_switch_fallthrough(st, arg0, arg1)
    }

    fn make_case_label(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_case_label(st, arg0, arg1, arg2)
    }

    fn make_default_label(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_default_label(st, arg0, arg1)
    }

    fn make_return_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_return_statement(st, arg0, arg1, arg2)
    }

    fn make_goto_label(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_goto_label(st, arg0, arg1)
    }

    fn make_goto_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_goto_statement(st, arg0, arg1, arg2)
    }

    fn make_throw_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_throw_statement(st, arg0, arg1, arg2)
    }

    fn make_break_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_break_statement(st, arg0, arg1, arg2)
    }

    fn make_continue_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_continue_statement(st, arg0, arg1, arg2)
    }

    fn make_echo_statement(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_echo_statement(st, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_concurrent_statement(st, arg0, arg1)
    }

    fn make_simple_initializer(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_simple_initializer(st, arg0, arg1)
    }

    fn make_anonymous_class(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_anonymous_class(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_anonymous_function(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_anonymous_function_use_clause(st, arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_lambda_expression(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_lambda_signature(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_lambda_signature(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_cast_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_cast_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_scope_resolution_expression(st, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_member_selection_expression(st, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_safe_member_selection_expression(st, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_embedded_member_selection_expression(st, arg0, arg1, arg2)
    }

    fn make_yield_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_yield_expression(st, arg0, arg1)
    }

    fn make_yield_from_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_yield_from_expression(st, arg0, arg1, arg2)
    }

    fn make_prefix_unary_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_prefix_unary_expression(st, arg0, arg1)
    }

    fn make_postfix_unary_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_postfix_unary_expression(st, arg0, arg1)
    }

    fn make_binary_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_binary_expression(st, arg0, arg1, arg2)
    }

    fn make_instanceof_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_instanceof_expression(st, arg0, arg1, arg2)
    }

    fn make_is_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_is_expression(st, arg0, arg1, arg2)
    }

    fn make_as_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_as_expression(st, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_nullable_as_expression(st, arg0, arg1, arg2)
    }

    fn make_conditional_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_conditional_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_eval_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_define_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_define_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_halt_compiler_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_halt_compiler_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_isset_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_function_call_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_parenthesized_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_parenthesized_expression(st, arg0, arg1, arg2)
    }

    fn make_braced_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_braced_expression(st, arg0, arg1, arg2)
    }

    fn make_embedded_braced_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_embedded_braced_expression(st, arg0, arg1, arg2)
    }

    fn make_list_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_list_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_collection_literal_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_object_creation_expression(st, arg0, arg1)
    }

    fn make_constructor_call(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_constructor_call(st, arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_record_creation_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_array_creation_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_array_creation_expression(st, arg0, arg1, arg2)
    }

    fn make_array_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_array_intrinsic_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_darray_intrinsic_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_dictionary_intrinsic_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_keyset_intrinsic_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_varray_intrinsic_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_vector_intrinsic_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_element_initializer(st, arg0, arg1, arg2)
    }

    fn make_subscript_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_subscript_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_embedded_subscript_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_awaitable_creation_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_children_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_children_declaration(st, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_children_parenthesized_list(st, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_category_declaration(st, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_enum_type(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_lateinit(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_lateinit(st, arg0, arg1)
    }

    fn make_xhp_required(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_required(st, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_class_attribute_declaration(st, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_class_attribute(st, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_simple_class_attribute(st, arg0)
    }

    fn make_xhp_simple_attribute(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_simple_attribute(st, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_spread_attribute(st, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_open(st, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_expression(st, arg0, arg1, arg2)
    }

    fn make_xhp_close(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_xhp_close(st, arg0, arg1, arg2)
    }

    fn make_type_constant(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_constant(st, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_vector_type_specifier(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_keyset_type_specifier(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_tuple_type_explicit_specifier(st, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_varray_type_specifier(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_array_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_vector_array_type_specifier(st, arg0, arg1, arg2, arg3)
    }

    fn make_type_parameter(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_parameter(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_constraint(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_constraint(st, arg0, arg1)
    }

    fn make_darray_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_darray_type_specifier(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_map_array_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_map_array_type_specifier(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_dictionary_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_dictionary_type_specifier(st, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_closure_type_specifier(st, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_closure_parameter_type_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_closure_parameter_type_specifier(st, arg0, arg1)
    }

    fn make_classname_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_classname_type_specifier(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_field_specifier(st, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_field_initializer(st, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_shape_type_specifier(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_shape_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_tuple_expression(st, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_generic_type_specifier(st, arg0, arg1)
    }

    fn make_nullable_type_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_nullable_type_specifier(st, arg0, arg1)
    }

    fn make_like_type_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_like_type_specifier(st, arg0, arg1)
    }

    fn make_soft_type_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_soft_type_specifier(st, arg0, arg1)
    }

    fn make_attributized_specifier(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_attributized_specifier(st, arg0, arg1)
    }

    fn make_reified_type_argument(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_reified_type_argument(st, arg0, arg1)
    }

    fn make_type_arguments(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_arguments(st, arg0, arg1, arg2)
    }

    fn make_type_parameters(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_type_parameters(st, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_tuple_type_specifier(st, arg0, arg1, arg2)
    }

    fn make_error(st: T, arg0: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_error(st, arg0)
    }

    fn make_list_item(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_list_item(st, arg0, arg1)
    }

    fn make_pocket_atom_expression(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_atom_expression(st, arg0, arg1)
    }

    fn make_pocket_identifier_expression(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_identifier_expression(st, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_pocket_atom_mapping_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_atom_mapping_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_enum_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_enum_declaration(st, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_field_type_expr_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_field_type_expr_declaration(st, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_field_type_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_field_type_declaration(st, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_mapping_id_declaration(st: T, arg0: Self::R, arg1: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_mapping_id_declaration(st, arg0, arg1)
    }

    fn make_pocket_mapping_type_declaration(st: T, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (T, Self::R) {
        <Self as SyntaxSmartConstructors<Self::R, T>>::make_pocket_mapping_type_declaration(st, arg0, arg1, arg2, arg3)
    }

}
