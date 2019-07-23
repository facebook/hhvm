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

pub trait SyntaxSmartConstructors<'src, S: SyntaxType<State>, State = NoState>:
    SmartConstructors<'src, State, R=S, Token=S::Token>
where
    State: StateType<'src, S>,
{
    fn initial_state(env: &ParserEnv, src: &SourceText<'src>) -> State {
        State::initial(env, src)
    }

    fn make_missing(s: State, offset: usize) -> (State, Self::R) {
        let r = Self::R::make_missing(&s, offset);
        (State::next(s, &[]), r)
    }

    fn make_token(s: State, arg: Self::Token) -> (State, Self::R) {
        let r = Self::R::make_token(&s, arg);
        (State::next(s, &[]), r)
    }

    fn make_list(s: State, items: Vec<Self::R>, offset: usize) -> (State, Self::R) {
        if items.is_empty() {
            <Self as SyntaxSmartConstructors<'src, S, State>>::make_missing(s, offset)
        } else {
            let item_refs: Vec<_> = items.iter().collect();
            let s = State::next(s, &item_refs);
            let r = Self::R::make_list(&s, items, offset);
            (s, r)
        }
    }

    fn make_end_of_file(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_end_of_file(&s, arg0);
        (s, r)
    }

    fn make_script(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_script(&s, arg0);
        (s, r)
    }

    fn make_qualified_name(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_qualified_name(&s, arg0);
        (s, r)
    }

    fn make_simple_type_specifier(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_simple_type_specifier(&s, arg0);
        (s, r)
    }

    fn make_literal_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_literal_expression(&s, arg0);
        (s, r)
    }

    fn make_prefixed_string_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_prefixed_string_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_variable_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_variable_expression(&s, arg0);
        (s, r)
    }

    fn make_pipe_variable_expression(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_pipe_variable_expression(&s, arg0);
        (s, r)
    }

    fn make_file_attribute_specification(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_file_attribute_specification(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_enum_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        let r = Self::R::make_enum_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        (s, r)
    }

    fn make_enumerator(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_enumerator(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_record_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        let r = Self::R::make_record_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        (s, r)
    }

    fn make_record_field(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_record_field(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_alias_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7]);
        let r = Self::R::make_alias_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        (s, r)
    }

    fn make_property_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_property_declaration(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_property_declarator(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_property_declarator(&s, arg0, arg1);
        (s, r)
    }

    fn make_namespace_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_namespace_declaration(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_namespace_body(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_namespace_body(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_namespace_empty_body(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_namespace_empty_body(&s, arg0);
        (s, r)
    }

    fn make_namespace_use_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_namespace_use_declaration(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_namespace_group_use_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        let r = Self::R::make_namespace_group_use_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
        (s, r)
    }

    fn make_namespace_use_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_namespace_use_clause(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_function_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_function_declaration(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_function_declaration_header(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        let r = Self::R::make_function_declaration_header(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        (s, r)
    }

    fn make_where_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_where_clause(&s, arg0, arg1);
        (s, r)
    }

    fn make_where_constraint(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_where_constraint(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_methodish_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_methodish_declaration(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_methodish_trait_resolution(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_methodish_trait_resolution(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_classish_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10]);
        let r = Self::R::make_classish_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
        (s, r)
    }

    fn make_classish_body(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_classish_body(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_trait_use_precedence_item(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_trait_use_precedence_item(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_trait_use_alias_item(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_trait_use_alias_item(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_trait_use_conflict_resolution(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_trait_use_conflict_resolution(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_trait_use(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_trait_use(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_require_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_require_clause(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_const_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_const_declaration(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_constant_declarator(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_constant_declarator(&s, arg0, arg1);
        (s, r)
    }

    fn make_type_const_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        let r = Self::R::make_type_const_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        (s, r)
    }

    fn make_decorated_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_decorated_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_parameter_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_parameter_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_variadic_parameter(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_variadic_parameter(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_old_attribute_specification(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_old_attribute_specification(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_attribute_specification(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_attribute_specification(&s, arg0);
        (s, r)
    }

    fn make_attribute(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_attribute(&s, arg0, arg1);
        (s, r)
    }

    fn make_inclusion_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_inclusion_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_inclusion_directive(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_inclusion_directive(&s, arg0, arg1);
        (s, r)
    }

    fn make_compound_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_compound_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_expression_statement(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_expression_statement(&s, arg0, arg1);
        (s, r)
    }

    fn make_markup_section(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_markup_section(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_markup_suffix(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_markup_suffix(&s, arg0, arg1);
        (s, r)
    }

    fn make_unset_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_unset_statement(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_let_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_let_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_using_statement_block_scoped(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_using_statement_block_scoped(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_using_statement_function_scoped(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_using_statement_function_scoped(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_while_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_while_statement(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_if_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        let r = Self::R::make_if_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
        (s, r)
    }

    fn make_elseif_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_elseif_clause(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_else_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_else_clause(&s, arg0, arg1);
        (s, r)
    }

    fn make_try_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_try_statement(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_catch_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_catch_clause(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_finally_clause(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_finally_clause(&s, arg0, arg1);
        (s, r)
    }

    fn make_do_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        let r = Self::R::make_do_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
        (s, r)
    }

    fn make_for_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        let r = Self::R::make_for_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        (s, r)
    }

    fn make_foreach_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9]);
        let r = Self::R::make_foreach_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        (s, r)
    }

    fn make_switch_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        let r = Self::R::make_switch_statement(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
        (s, r)
    }

    fn make_switch_section(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_switch_section(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_switch_fallthrough(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_switch_fallthrough(&s, arg0, arg1);
        (s, r)
    }

    fn make_case_label(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_case_label(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_default_label(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_default_label(&s, arg0, arg1);
        (s, r)
    }

    fn make_return_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_return_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_goto_label(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_goto_label(&s, arg0, arg1);
        (s, r)
    }

    fn make_goto_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_goto_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_throw_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_throw_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_break_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_break_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_continue_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_continue_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_echo_statement(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_echo_statement(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_concurrent_statement(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_concurrent_statement(&s, arg0, arg1);
        (s, r)
    }

    fn make_simple_initializer(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_simple_initializer(&s, arg0, arg1);
        (s, r)
    }

    fn make_anonymous_class(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        let r = Self::R::make_anonymous_class(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        (s, r)
    }

    fn make_anonymous_function(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8, &arg9, &arg10, &arg11]);
        let r = Self::R::make_anonymous_function(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
        (s, r)
    }

    fn make_anonymous_function_use_clause(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_anonymous_function_use_clause(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_lambda_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_lambda_expression(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_lambda_signature(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_lambda_signature(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_cast_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_cast_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_scope_resolution_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_scope_resolution_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_member_selection_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_safe_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_safe_member_selection_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_embedded_member_selection_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_embedded_member_selection_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_yield_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_yield_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_yield_from_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_yield_from_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_prefix_unary_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_prefix_unary_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_postfix_unary_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_postfix_unary_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_binary_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_binary_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_instanceof_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_instanceof_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_is_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_is_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_as_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_as_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_nullable_as_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_nullable_as_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_conditional_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_conditional_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_eval_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_eval_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_define_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_define_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_halt_compiler_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_halt_compiler_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_isset_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_isset_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_function_call_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_function_call_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_parenthesized_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_parenthesized_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_braced_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_braced_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_embedded_braced_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_embedded_braced_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_list_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_list_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_collection_literal_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_collection_literal_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_object_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_object_creation_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_constructor_call(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_constructor_call(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_record_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_record_creation_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_array_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_array_creation_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_array_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_array_intrinsic_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_darray_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_darray_intrinsic_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_dictionary_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_dictionary_intrinsic_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_keyset_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_keyset_intrinsic_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_varray_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_varray_intrinsic_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_vector_intrinsic_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_vector_intrinsic_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_element_initializer(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_element_initializer(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_subscript_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_subscript_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_embedded_subscript_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_embedded_subscript_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_awaitable_creation_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_awaitable_creation_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_xhp_children_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_children_declaration(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_children_parenthesized_list(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_children_parenthesized_list(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_category_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_category_declaration(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_enum_type(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_xhp_enum_type(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_xhp_lateinit(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_xhp_lateinit(&s, arg0, arg1);
        (s, r)
    }

    fn make_xhp_required(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_xhp_required(&s, arg0, arg1);
        (s, r)
    }

    fn make_xhp_class_attribute_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_class_attribute_declaration(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_class_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_xhp_class_attribute(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_xhp_simple_class_attribute(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_xhp_simple_class_attribute(&s, arg0);
        (s, r)
    }

    fn make_xhp_simple_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_simple_attribute(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_spread_attribute(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_xhp_spread_attribute(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_xhp_open(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_xhp_open(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_xhp_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_expression(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_xhp_close(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_xhp_close(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_type_constant(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_type_constant(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_vector_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_vector_type_specifier(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_keyset_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_keyset_type_specifier(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_tuple_type_explicit_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_tuple_type_explicit_specifier(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_varray_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_varray_type_specifier(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_vector_array_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_vector_array_type_specifier(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_type_parameter(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_type_parameter(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_type_constraint(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_type_constraint(&s, arg0, arg1);
        (s, r)
    }

    fn make_darray_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6]);
        let r = Self::R::make_darray_type_specifier(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
        (s, r)
    }

    fn make_map_array_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_map_array_type_specifier(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_dictionary_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_dictionary_type_specifier(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_closure_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6, &arg7, &arg8]);
        let r = Self::R::make_closure_type_specifier(&s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        (s, r)
    }

    fn make_closure_parameter_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_closure_parameter_type_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_classname_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_classname_type_specifier(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_field_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_field_specifier(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_field_initializer(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_field_initializer(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_shape_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_shape_type_specifier(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_shape_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_shape_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_tuple_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_tuple_expression(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_generic_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_generic_type_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_nullable_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_nullable_type_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_like_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_like_type_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_soft_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_soft_type_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_attributized_specifier(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_attributized_specifier(&s, arg0, arg1);
        (s, r)
    }

    fn make_reified_type_argument(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_reified_type_argument(&s, arg0, arg1);
        (s, r)
    }

    fn make_type_arguments(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_type_arguments(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_type_parameters(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_type_parameters(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_tuple_type_specifier(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2]);
        let r = Self::R::make_tuple_type_specifier(&s, arg0, arg1, arg2);
        (s, r)
    }

    fn make_error(s: State, arg0 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0]);
        let r = Self::R::make_error(&s, arg0);
        (s, r)
    }

    fn make_list_item(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_list_item(&s, arg0, arg1);
        (s, r)
    }

    fn make_pocket_atom_expression(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_pocket_atom_expression(&s, arg0, arg1);
        (s, r)
    }

    fn make_pocket_identifier_expression(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4]);
        let r = Self::R::make_pocket_identifier_expression(&s, arg0, arg1, arg2, arg3, arg4);
        (s, r)
    }

    fn make_pocket_atom_mapping_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_pocket_atom_mapping_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_pocket_enum_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3, &arg4, &arg5]);
        let r = Self::R::make_pocket_enum_declaration(&s, arg0, arg1, arg2, arg3, arg4, arg5);
        (s, r)
    }

    fn make_pocket_field_type_expr_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_pocket_field_type_expr_declaration(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_pocket_field_type_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_pocket_field_type_declaration(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

    fn make_pocket_mapping_id_declaration(s: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1]);
        let r = Self::R::make_pocket_mapping_id_declaration(&s, arg0, arg1);
        (s, r)
    }

    fn make_pocket_mapping_type_declaration(s: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        let s = State::next(s, &[&arg0, &arg1, &arg2, &arg3]);
        let r = Self::R::make_pocket_mapping_type_declaration(&s, arg0, arg1, arg2, arg3);
        (s, r)
    }

}
