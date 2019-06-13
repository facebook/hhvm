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
use crate::minimal_syntax::MinimalSyntax;
use crate::minimal_token::MinimalToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::{NoState, SmartConstructors};
use crate::source_text::SourceText;
use crate::syntax_smart_constructors::SyntaxSmartConstructors;

pub struct MinimalSmartConstructors;
impl<'a> SyntaxSmartConstructors<'a, MinimalSyntax, NoState>
    for MinimalSmartConstructors {}
impl<'a> SmartConstructors<'a, NoState> for MinimalSmartConstructors {
    type Token = MinimalToken;
    type R = MinimalSyntax;

    fn initial_state<'b: 'a>(env: &ParserEnv, src: &'b SourceText<'b>) -> NoState {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::initial_state(env, src)
    }

    fn make_missing(s: NoState, offset: usize) -> (NoState, Self::R) {
       <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_missing(s, offset)
    }

    fn make_token(s: NoState, offset: Self::Token) -> (NoState, Self::R) {
       <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_token(s, offset)
    }

    fn make_list(
        s: NoState,
        lst: Box<Vec<Self::R>>,
        offset: usize,
    ) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_list(s, lst, offset)
    }

    fn make_end_of_file(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_end_of_file(s, arg0)
    }

    fn make_script(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_script(s, arg0)
    }

    fn make_qualified_name(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_qualified_name(s, arg0)
    }

    fn make_simple_type_specifier(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_simple_type_specifier(s, arg0)
    }

    fn make_literal_expression(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_literal_expression(s, arg0)
    }

    fn make_prefixed_string_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_prefixed_string_expression(s, arg0, arg1)
    }

    fn make_variable_expression(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_variable_expression(s, arg0)
    }

    fn make_pipe_variable_expression(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pipe_variable_expression(s, arg0)
    }

    fn make_file_attribute_specification(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_file_attribute_specification(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_enumerator(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_enumerator(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_record_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_record_field(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_record_field(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_alias_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_property_declaration(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_property_declarator(s, arg0, arg1)
    }

    fn make_namespace_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_declaration(s, arg0, arg1, arg2)
    }

    fn make_namespace_body(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_body(s, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_empty_body(s, arg0)
    }

    fn make_namespace_use_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_use_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_group_use_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_namespace_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_function_declaration(s, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_function_declaration_header(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_where_clause(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_where_clause(s, arg0, arg1)
    }

    fn make_where_constraint(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_where_constraint(s, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_methodish_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_methodish_trait_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_classish_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_classish_body(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_classish_body(s, arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_trait_use_precedence_item(s, arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_trait_use_alias_item(s, arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_trait_use_conflict_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_trait_use(s, arg0, arg1, arg2)
    }

    fn make_require_clause(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_require_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_constant_declarator(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_constant_declarator(s, arg0, arg1)
    }

    fn make_type_const_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_decorated_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_decorated_expression(s, arg0, arg1)
    }

    fn make_parameter_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_parameter_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_variadic_parameter(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_variadic_parameter(s, arg0, arg1, arg2)
    }

    fn make_attribute_specification(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_attribute_specification(s, arg0, arg1, arg2)
    }

    fn make_inclusion_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_inclusion_expression(s, arg0, arg1)
    }

    fn make_inclusion_directive(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_inclusion_directive(s, arg0, arg1)
    }

    fn make_compound_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_compound_statement(s, arg0, arg1, arg2)
    }

    fn make_expression_statement(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_expression_statement(s, arg0, arg1)
    }

    fn make_markup_section(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_markup_section(s, arg0, arg1, arg2, arg3)
    }

    fn make_markup_suffix(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_markup_suffix(s, arg0, arg1)
    }

    fn make_unset_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_unset_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_let_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_let_statement(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_using_statement_block_scoped(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_using_statement_function_scoped(s, arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_while_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_if_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_elseif_clause(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_else_clause(s, arg0, arg1)
    }

    fn make_try_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_try_statement(s, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_catch_clause(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_finally_clause(s, arg0, arg1)
    }

    fn make_do_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_do_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_for_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_foreach_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_switch_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_switch_section(s, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_switch_fallthrough(s, arg0, arg1)
    }

    fn make_case_label(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_case_label(s, arg0, arg1, arg2)
    }

    fn make_default_label(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_default_label(s, arg0, arg1)
    }

    fn make_return_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_return_statement(s, arg0, arg1, arg2)
    }

    fn make_goto_label(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_goto_label(s, arg0, arg1)
    }

    fn make_goto_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_goto_statement(s, arg0, arg1, arg2)
    }

    fn make_throw_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_throw_statement(s, arg0, arg1, arg2)
    }

    fn make_break_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_break_statement(s, arg0, arg1, arg2)
    }

    fn make_continue_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_continue_statement(s, arg0, arg1, arg2)
    }

    fn make_echo_statement(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_echo_statement(s, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_concurrent_statement(s, arg0, arg1)
    }

    fn make_simple_initializer(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_simple_initializer(s, arg0, arg1)
    }

    fn make_anonymous_class(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_anonymous_class(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_php7_anonymous_function(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_php7_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_anonymous_function_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_lambda_expression(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_lambda_signature(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_lambda_signature(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_cast_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_cast_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_scope_resolution_expression(s, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_safe_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_embedded_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_yield_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_yield_expression(s, arg0, arg1)
    }

    fn make_yield_from_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_yield_from_expression(s, arg0, arg1, arg2)
    }

    fn make_prefix_unary_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_prefix_unary_expression(s, arg0, arg1)
    }

    fn make_postfix_unary_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_postfix_unary_expression(s, arg0, arg1)
    }

    fn make_binary_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_binary_expression(s, arg0, arg1, arg2)
    }

    fn make_instanceof_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_instanceof_expression(s, arg0, arg1, arg2)
    }

    fn make_is_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_is_expression(s, arg0, arg1, arg2)
    }

    fn make_as_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_as_expression(s, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_nullable_as_expression(s, arg0, arg1, arg2)
    }

    fn make_conditional_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_conditional_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_eval_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_define_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_define_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_halt_compiler_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_halt_compiler_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_isset_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_function_call_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_parenthesized_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_parenthesized_expression(s, arg0, arg1, arg2)
    }

    fn make_braced_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_braced_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_embedded_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_list_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_list_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_collection_literal_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_object_creation_expression(s, arg0, arg1)
    }

    fn make_constructor_call(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_constructor_call(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_record_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_array_creation_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_array_creation_expression(s, arg0, arg1, arg2)
    }

    fn make_array_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_array_intrinsic_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_darray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_dictionary_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_keyset_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_varray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_vector_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_element_initializer(s, arg0, arg1, arg2)
    }

    fn make_subscript_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_embedded_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_awaitable_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_children_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_children_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_children_parenthesized_list(s, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_category_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_enum_type(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_lateinit(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_lateinit(s, arg0, arg1)
    }

    fn make_xhp_required(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_required(s, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_class_attribute_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_class_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_simple_class_attribute(s, arg0)
    }

    fn make_xhp_simple_attribute(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_simple_attribute(s, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_spread_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_open(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_expression(s, arg0, arg1, arg2)
    }

    fn make_xhp_close(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_xhp_close(s, arg0, arg1, arg2)
    }

    fn make_type_constant(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_constant(s, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_vector_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_keyset_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_tuple_type_explicit_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_varray_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_array_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_vector_array_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_type_parameter(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_parameter(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_constraint(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_constraint(s, arg0, arg1)
    }

    fn make_darray_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_darray_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_map_array_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_map_array_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_dictionary_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_dictionary_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_closure_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_closure_parameter_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_closure_parameter_type_specifier(s, arg0, arg1)
    }

    fn make_classname_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_classname_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_field_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_field_initializer(s, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_shape_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_shape_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_tuple_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_generic_type_specifier(s, arg0, arg1)
    }

    fn make_nullable_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_nullable_type_specifier(s, arg0, arg1)
    }

    fn make_like_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_like_type_specifier(s, arg0, arg1)
    }

    fn make_soft_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_soft_type_specifier(s, arg0, arg1)
    }

    fn make_reified_type_argument(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_reified_type_argument(s, arg0, arg1)
    }

    fn make_type_arguments(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_arguments(s, arg0, arg1, arg2)
    }

    fn make_type_parameters(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_type_parameters(s, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_tuple_type_specifier(s, arg0, arg1, arg2)
    }

    fn make_error(s: NoState, arg0: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_error(s, arg0)
    }

    fn make_list_item(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_list_item(s, arg0, arg1)
    }

    fn make_pocket_atom_expression(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_atom_expression(s, arg0, arg1)
    }

    fn make_pocket_identifier_expression(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_identifier_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_pocket_atom_mapping_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_atom_mapping_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_enum_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_field_type_expr_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_field_type_expr_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_field_type_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_field_type_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_mapping_id_declaration(s: NoState, arg0: Self::R, arg1: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_mapping_id_declaration(s, arg0, arg1)
    }

    fn make_pocket_mapping_type_declaration(s: NoState, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (NoState, Self::R) {
        <Self as SyntaxSmartConstructors<'a, MinimalSyntax, NoState>>::make_pocket_mapping_type_declaration(s, arg0, arg1, arg2, arg3)
    }

}
