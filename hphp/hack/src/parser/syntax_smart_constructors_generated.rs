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
use crate::parser_env::ParserEnv;
use crate::smart_constructors::{NoState, SmartConstructors};
use crate::syntax_smart_constructors::StateType;
use crate::source_text::SourceText;
use crate::syntax::*;

pub trait SyntaxSmartConstructors<'src, S: SyntaxType, State = NoState>:
    SmartConstructors<'src, State, R=S, Token=S::Token>
where
    State: StateType<'src, S>,
{
    fn initial_state(env: &ParserEnv, src: &SourceText<'src>) -> State {
        State::initial(env, src)
    }

    fn make_missing(s: State, offset: usize) -> (State, Self::R) {
        (State::next(s, vec![]), Self::R::make_missing(offset))
    }

    fn make_token(s: State, arg: Self::Token) -> (State, Self::R) {
        (State::next(s, vec![]), Self::R::make_token(arg))
    }

    fn make_list(s: State, items: Vec<Self::R>, offset: usize) -> (State, Self::R) {
        if items.is_empty() {
            <Self as SyntaxSmartConstructors<'src, S, State>>::make_missing(s, offset)
        } else {
            let item_refs: Vec<_> = items.iter().collect();
            let s = State::next(s, item_refs);
            (s, Self::R::make_list(items, offset))
        }
    }

    fn make_end_of_file(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_end_of_file(arg0))
    }

    fn make_script(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_script(arg0))
    }

    fn make_qualified_name(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_qualified_name(arg0))
    }

    fn make_simple_type_specifier(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_simple_type_specifier(arg0))
    }

    fn make_literal_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_literal_expression(arg0))
    }

    fn make_prefixed_string_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_prefixed_string_expression(arg0, arg1))
    }

    fn make_variable_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_variable_expression(arg0))
    }

    fn make_pipe_variable_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_pipe_variable_expression(arg0))
    }

    fn make_file_attribute_specification(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_file_attribute_specification(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_enum_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        (s, Self::R::make_enum_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
    }

    fn make_enumerator(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_enumerator(arg0, arg1, arg2, arg3))
    }

    fn make_record_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        (s, Self::R::make_record_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
    }

    fn make_record_field(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_record_field(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_alias_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7]);
        (s, Self::R::make_alias_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7))
    }

    fn make_property_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_property_declaration(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_property_declarator(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_property_declarator(arg0, arg1))
    }

    fn make_namespace_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_namespace_declaration(arg0, arg1, arg2))
    }

    fn make_namespace_body(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_namespace_body(arg0, arg1, arg2))
    }

    fn make_namespace_empty_body(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_namespace_empty_body(arg0))
    }

    fn make_namespace_use_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_namespace_use_declaration(arg0, arg1, arg2, arg3))
    }

    fn make_namespace_group_use_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        (s, Self::R::make_namespace_group_use_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
    }

    fn make_namespace_use_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_namespace_use_clause(arg0, arg1, arg2, arg3))
    }

    fn make_function_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_function_declaration(arg0, arg1, arg2))
    }

    fn make_function_declaration_header(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        (s, Self::R::make_function_declaration_header(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
    }

    fn make_where_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_where_clause(arg0, arg1))
    }

    fn make_where_constraint(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_where_constraint(arg0, arg1, arg2))
    }

    fn make_methodish_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_methodish_declaration(arg0, arg1, arg2, arg3))
    }

    fn make_methodish_trait_resolution(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_methodish_trait_resolution(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_classish_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10]);
        (s, Self::R::make_classish_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10))
    }

    fn make_classish_body(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_classish_body(arg0, arg1, arg2))
    }

    fn make_trait_use_precedence_item(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_trait_use_precedence_item(arg0, arg1, arg2))
    }

    fn make_trait_use_alias_item(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_trait_use_alias_item(arg0, arg1, arg2, arg3))
    }

    fn make_trait_use_conflict_resolution(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_trait_use_conflict_resolution(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_trait_use(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_trait_use(arg0, arg1, arg2))
    }

    fn make_require_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_require_clause(arg0, arg1, arg2, arg3))
    }

    fn make_const_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_const_declaration(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_constant_declarator(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_constant_declarator(arg0, arg1))
    }

    fn make_type_const_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        (s, Self::R::make_type_const_declaration(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
    }

    fn make_decorated_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_decorated_expression(arg0, arg1))
    }

    fn make_parameter_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_parameter_declaration(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_variadic_parameter(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_variadic_parameter(arg0, arg1, arg2))
    }

    fn make_attribute_specification(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_attribute_specification(arg0, arg1, arg2))
    }

    fn make_inclusion_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_inclusion_expression(arg0, arg1))
    }

    fn make_inclusion_directive(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_inclusion_directive(arg0, arg1))
    }

    fn make_compound_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_compound_statement(arg0, arg1, arg2))
    }

    fn make_expression_statement(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_expression_statement(arg0, arg1))
    }

    fn make_markup_section(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_markup_section(arg0, arg1, arg2, arg3))
    }

    fn make_markup_suffix(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_markup_suffix(arg0, arg1))
    }

    fn make_unset_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_unset_statement(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_let_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_let_statement(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_using_statement_block_scoped(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_using_statement_block_scoped(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_using_statement_function_scoped(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_using_statement_function_scoped(arg0, arg1, arg2, arg3))
    }

    fn make_while_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_while_statement(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_if_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        (s, Self::R::make_if_statement(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
    }

    fn make_elseif_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_elseif_clause(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_else_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_else_clause(arg0, arg1))
    }

    fn make_try_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_try_statement(arg0, arg1, arg2, arg3))
    }

    fn make_catch_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_catch_clause(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_finally_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_finally_clause(arg0, arg1))
    }

    fn make_do_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        (s, Self::R::make_do_statement(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
    }

    fn make_for_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        (s, Self::R::make_for_statement(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
    }

    fn make_foreach_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        (s, Self::R::make_foreach_statement(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
    }

    fn make_switch_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        (s, Self::R::make_switch_statement(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
    }

    fn make_switch_section(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_switch_section(arg0, arg1, arg2))
    }

    fn make_switch_fallthrough(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_switch_fallthrough(arg0, arg1))
    }

    fn make_case_label(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_case_label(arg0, arg1, arg2))
    }

    fn make_default_label(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_default_label(arg0, arg1))
    }

    fn make_return_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_return_statement(arg0, arg1, arg2))
    }

    fn make_goto_label(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_goto_label(arg0, arg1))
    }

    fn make_goto_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_goto_statement(arg0, arg1, arg2))
    }

    fn make_throw_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_throw_statement(arg0, arg1, arg2))
    }

    fn make_break_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_break_statement(arg0, arg1, arg2))
    }

    fn make_continue_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_continue_statement(arg0, arg1, arg2))
    }

    fn make_echo_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_echo_statement(arg0, arg1, arg2))
    }

    fn make_concurrent_statement(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_concurrent_statement(arg0, arg1))
    }

    fn make_simple_initializer(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_simple_initializer(arg0, arg1))
    }

    fn make_anonymous_class(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        (s, Self::R::make_anonymous_class(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
    }

    fn make_anonymous_function(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10, &arg11]);
        (s, Self::R::make_anonymous_function(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
    }

    fn make_anonymous_function_use_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_anonymous_function_use_clause(arg0, arg1, arg2, arg3))
    }

    fn make_lambda_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_lambda_expression(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_lambda_signature(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_lambda_signature(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_cast_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_cast_expression(arg0, arg1, arg2, arg3))
    }

    fn make_scope_resolution_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_scope_resolution_expression(arg0, arg1, arg2))
    }

    fn make_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_member_selection_expression(arg0, arg1, arg2))
    }

    fn make_safe_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_safe_member_selection_expression(arg0, arg1, arg2))
    }

    fn make_embedded_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_embedded_member_selection_expression(arg0, arg1, arg2))
    }

    fn make_yield_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_yield_expression(arg0, arg1))
    }

    fn make_yield_from_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_yield_from_expression(arg0, arg1, arg2))
    }

    fn make_prefix_unary_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_prefix_unary_expression(arg0, arg1))
    }

    fn make_postfix_unary_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_postfix_unary_expression(arg0, arg1))
    }

    fn make_binary_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_binary_expression(arg0, arg1, arg2))
    }

    fn make_instanceof_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_instanceof_expression(arg0, arg1, arg2))
    }

    fn make_is_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_is_expression(arg0, arg1, arg2))
    }

    fn make_as_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_as_expression(arg0, arg1, arg2))
    }

    fn make_nullable_as_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_nullable_as_expression(arg0, arg1, arg2))
    }

    fn make_conditional_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_conditional_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_eval_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_eval_expression(arg0, arg1, arg2, arg3))
    }

    fn make_define_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_define_expression(arg0, arg1, arg2, arg3))
    }

    fn make_halt_compiler_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_halt_compiler_expression(arg0, arg1, arg2, arg3))
    }

    fn make_isset_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_isset_expression(arg0, arg1, arg2, arg3))
    }

    fn make_function_call_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_function_call_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_parenthesized_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_parenthesized_expression(arg0, arg1, arg2))
    }

    fn make_braced_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_braced_expression(arg0, arg1, arg2))
    }

    fn make_embedded_braced_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_embedded_braced_expression(arg0, arg1, arg2))
    }

    fn make_list_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_list_expression(arg0, arg1, arg2, arg3))
    }

    fn make_collection_literal_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_collection_literal_expression(arg0, arg1, arg2, arg3))
    }

    fn make_object_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_object_creation_expression(arg0, arg1))
    }

    fn make_constructor_call(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_constructor_call(arg0, arg1, arg2, arg3))
    }

    fn make_record_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_record_creation_expression(arg0, arg1, arg2, arg3))
    }

    fn make_array_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_array_creation_expression(arg0, arg1, arg2))
    }

    fn make_array_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_array_intrinsic_expression(arg0, arg1, arg2, arg3))
    }

    fn make_darray_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_darray_intrinsic_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_dictionary_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_dictionary_intrinsic_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_keyset_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_keyset_intrinsic_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_varray_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_varray_intrinsic_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_vector_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_vector_intrinsic_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_element_initializer(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_element_initializer(arg0, arg1, arg2))
    }

    fn make_subscript_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_subscript_expression(arg0, arg1, arg2, arg3))
    }

    fn make_embedded_subscript_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_embedded_subscript_expression(arg0, arg1, arg2, arg3))
    }

    fn make_awaitable_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_awaitable_creation_expression(arg0, arg1, arg2, arg3))
    }

    fn make_xhp_children_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_children_declaration(arg0, arg1, arg2))
    }

    fn make_xhp_children_parenthesized_list(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_children_parenthesized_list(arg0, arg1, arg2))
    }

    fn make_xhp_category_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_category_declaration(arg0, arg1, arg2))
    }

    fn make_xhp_enum_type(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_xhp_enum_type(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_xhp_lateinit(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_xhp_lateinit(arg0, arg1))
    }

    fn make_xhp_required(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_xhp_required(arg0, arg1))
    }

    fn make_xhp_class_attribute_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_class_attribute_declaration(arg0, arg1, arg2))
    }

    fn make_xhp_class_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_xhp_class_attribute(arg0, arg1, arg2, arg3))
    }

    fn make_xhp_simple_class_attribute(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_xhp_simple_class_attribute(arg0))
    }

    fn make_xhp_simple_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_simple_attribute(arg0, arg1, arg2))
    }

    fn make_xhp_spread_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_xhp_spread_attribute(arg0, arg1, arg2, arg3))
    }

    fn make_xhp_open(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_xhp_open(arg0, arg1, arg2, arg3))
    }

    fn make_xhp_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_expression(arg0, arg1, arg2))
    }

    fn make_xhp_close(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_xhp_close(arg0, arg1, arg2))
    }

    fn make_type_constant(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_type_constant(arg0, arg1, arg2))
    }

    fn make_vector_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_vector_type_specifier(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_keyset_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_keyset_type_specifier(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_tuple_type_explicit_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_tuple_type_explicit_specifier(arg0, arg1, arg2, arg3))
    }

    fn make_varray_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_varray_type_specifier(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_vector_array_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_vector_array_type_specifier(arg0, arg1, arg2, arg3))
    }

    fn make_type_parameter(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_type_parameter(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_type_constraint(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_type_constraint(arg0, arg1))
    }

    fn make_darray_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        (s, Self::R::make_darray_type_specifier(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
    }

    fn make_map_array_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_map_array_type_specifier(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_dictionary_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_dictionary_type_specifier(arg0, arg1, arg2, arg3))
    }

    fn make_closure_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        (s, Self::R::make_closure_type_specifier(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
    }

    fn make_closure_parameter_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_closure_parameter_type_specifier(arg0, arg1))
    }

    fn make_classname_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_classname_type_specifier(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_field_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_field_specifier(arg0, arg1, arg2, arg3))
    }

    fn make_field_initializer(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_field_initializer(arg0, arg1, arg2))
    }

    fn make_shape_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_shape_type_specifier(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_shape_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_shape_expression(arg0, arg1, arg2, arg3))
    }

    fn make_tuple_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_tuple_expression(arg0, arg1, arg2, arg3))
    }

    fn make_generic_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_generic_type_specifier(arg0, arg1))
    }

    fn make_nullable_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_nullable_type_specifier(arg0, arg1))
    }

    fn make_like_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_like_type_specifier(arg0, arg1))
    }

    fn make_soft_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_soft_type_specifier(arg0, arg1))
    }

    fn make_attributized_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_attributized_specifier(arg0, arg1))
    }

    fn make_reified_type_argument(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_reified_type_argument(arg0, arg1))
    }

    fn make_type_arguments(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_type_arguments(arg0, arg1, arg2))
    }

    fn make_type_parameters(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_type_parameters(arg0, arg1, arg2))
    }

    fn make_tuple_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2]);
        (s, Self::R::make_tuple_type_specifier(arg0, arg1, arg2))
    }

    fn make_error(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0]);
        (s, Self::R::make_error(arg0))
    }

    fn make_list_item(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_list_item(arg0, arg1))
    }

    fn make_pocket_atom_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_pocket_atom_expression(arg0, arg1))
    }

    fn make_pocket_identifier_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4]);
        (s, Self::R::make_pocket_identifier_expression(arg0, arg1, arg2, arg3, arg4))
    }

    fn make_pocket_atom_mapping_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_pocket_atom_mapping_declaration(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_pocket_enum_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        (s, Self::R::make_pocket_enum_declaration(arg0, arg1, arg2, arg3, arg4, arg5))
    }

    fn make_pocket_field_type_expr_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_pocket_field_type_expr_declaration(arg0, arg1, arg2, arg3))
    }

    fn make_pocket_field_type_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_pocket_field_type_declaration(arg0, arg1, arg2, arg3))
    }

    fn make_pocket_mapping_id_declaration(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1]);
        (s, Self::R::make_pocket_mapping_id_declaration(arg0, arg1))
    }

    fn make_pocket_mapping_type_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, vec![&arg0, &arg1, &arg2, &arg3]);
        (s, Self::R::make_pocket_mapping_type_declaration(arg0, arg1, arg2, arg3))
    }

}
