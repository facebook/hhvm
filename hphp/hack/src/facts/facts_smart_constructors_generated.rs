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
use parser_rust as parser;
use parser::flatten_smart_constructors::*;
use parser::smart_constructors::SmartConstructors;
use parser::source_text::SourceText;
use parser::parser_env::ParserEnv;
use parser::positioned_token::PositionedToken;

use crate::facts_smart_constructors::*;

pub struct FactsSmartConstructors;
impl<'src> SmartConstructors<'src, HasScriptContent<'src>> for FactsSmartConstructors {
    type Token = PositionedToken;
    type R = Node;

    fn initial_state(_: &ParserEnv, src: &SourceText<'src>) -> HasScriptContent<'src> {
        (false, *src)
    }

    fn make_missing(s: HasScriptContent<'src>, offset: usize) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_missing(s, offset)
    }

    fn make_token(s: HasScriptContent<'src>, token: Self::Token) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_token(s, token)
    }

    fn make_list(
        s: HasScriptContent<'src>,
        items: Vec<Self::R>,
        offset: usize,
    ) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_list(s, items, offset)
    }

    fn make_end_of_file(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_end_of_file(s, arg0)
    }

    fn make_script(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_script(s, arg0)
    }

    fn make_qualified_name(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_qualified_name(s, arg0)
    }

    fn make_simple_type_specifier(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_simple_type_specifier(s, arg0)
    }

    fn make_literal_expression(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_literal_expression(s, arg0)
    }

    fn make_prefixed_string_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_prefixed_string_expression(s, arg0, arg1)
    }

    fn make_variable_expression(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_variable_expression(s, arg0)
    }

    fn make_pipe_variable_expression(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pipe_variable_expression(s, arg0)
    }

    fn make_file_attribute_specification(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_file_attribute_specification(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_enum_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_enumerator(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_enumerator(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_record_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_record_field(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_record_field(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_alias_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_alias_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    }

    fn make_property_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_property_declaration(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_property_declarator(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_property_declarator(s, arg0, arg1)
    }

    fn make_namespace_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_declaration(s, arg0, arg1, arg2)
    }

    fn make_namespace_body(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_body(s, arg0, arg1, arg2)
    }

    fn make_namespace_empty_body(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_empty_body(s, arg0)
    }

    fn make_namespace_use_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_use_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_namespace_group_use_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_group_use_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_namespace_use_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_namespace_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_function_declaration(s, arg0, arg1, arg2)
    }

    fn make_function_declaration_header(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_function_declaration_header(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_where_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_where_clause(s, arg0, arg1)
    }

    fn make_where_constraint(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_where_constraint(s, arg0, arg1, arg2)
    }

    fn make_methodish_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_methodish_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_methodish_trait_resolution(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_methodish_trait_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_classish_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_classish_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
    }

    fn make_classish_body(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_classish_body(s, arg0, arg1, arg2)
    }

    fn make_trait_use_precedence_item(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_trait_use_precedence_item(s, arg0, arg1, arg2)
    }

    fn make_trait_use_alias_item(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_trait_use_alias_item(s, arg0, arg1, arg2, arg3)
    }

    fn make_trait_use_conflict_resolution(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_trait_use_conflict_resolution(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_trait_use(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_trait_use(s, arg0, arg1, arg2)
    }

    fn make_require_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_require_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_const_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_const_declaration(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_constant_declarator(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_constant_declarator(s, arg0, arg1)
    }

    fn make_type_const_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_const_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_decorated_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_decorated_expression(s, arg0, arg1)
    }

    fn make_parameter_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_parameter_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_variadic_parameter(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_variadic_parameter(s, arg0, arg1, arg2)
    }

    fn make_old_attribute_specification(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_old_attribute_specification(s, arg0, arg1, arg2)
    }

    fn make_attribute_specification(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_attribute_specification(s, arg0)
    }

    fn make_attribute(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_attribute(s, arg0, arg1)
    }

    fn make_inclusion_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_inclusion_expression(s, arg0, arg1)
    }

    fn make_inclusion_directive(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_inclusion_directive(s, arg0, arg1)
    }

    fn make_compound_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_compound_statement(s, arg0, arg1, arg2)
    }

    fn make_expression_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_expression_statement(s, arg0, arg1)
    }

    fn make_markup_section(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_markup_section(s, arg0, arg1, arg2, arg3)
    }

    fn make_markup_suffix(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_markup_suffix(s, arg0, arg1)
    }

    fn make_unset_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_unset_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_let_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_let_statement(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_block_scoped(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_using_statement_block_scoped(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_using_statement_function_scoped(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_using_statement_function_scoped(s, arg0, arg1, arg2, arg3)
    }

    fn make_while_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_while_statement(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_if_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_if_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_elseif_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_elseif_clause(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_else_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_else_clause(s, arg0, arg1)
    }

    fn make_try_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_try_statement(s, arg0, arg1, arg2, arg3)
    }

    fn make_catch_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_catch_clause(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_finally_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_finally_clause(s, arg0, arg1)
    }

    fn make_do_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_do_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_for_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_for_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_foreach_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_foreach_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
    }

    fn make_switch_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_switch_statement(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_switch_section(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_switch_section(s, arg0, arg1, arg2)
    }

    fn make_switch_fallthrough(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_switch_fallthrough(s, arg0, arg1)
    }

    fn make_case_label(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_case_label(s, arg0, arg1, arg2)
    }

    fn make_default_label(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_default_label(s, arg0, arg1)
    }

    fn make_return_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_return_statement(s, arg0, arg1, arg2)
    }

    fn make_goto_label(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_goto_label(s, arg0, arg1)
    }

    fn make_goto_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_goto_statement(s, arg0, arg1, arg2)
    }

    fn make_throw_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_throw_statement(s, arg0, arg1, arg2)
    }

    fn make_break_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_break_statement(s, arg0, arg1, arg2)
    }

    fn make_continue_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_continue_statement(s, arg0, arg1, arg2)
    }

    fn make_echo_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_echo_statement(s, arg0, arg1, arg2)
    }

    fn make_concurrent_statement(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_concurrent_statement(s, arg0, arg1)
    }

    fn make_simple_initializer(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_simple_initializer(s, arg0, arg1)
    }

    fn make_anonymous_class(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_anonymous_class(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_anonymous_function(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R, arg9: Self::R, arg10: Self::R, arg11: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_anonymous_function(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11)
    }

    fn make_anonymous_function_use_clause(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_anonymous_function_use_clause(s, arg0, arg1, arg2, arg3)
    }

    fn make_lambda_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_lambda_expression(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_lambda_signature(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_lambda_signature(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_cast_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_cast_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_scope_resolution_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_scope_resolution_expression(s, arg0, arg1, arg2)
    }

    fn make_member_selection_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_safe_member_selection_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_safe_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_member_selection_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_embedded_member_selection_expression(s, arg0, arg1, arg2)
    }

    fn make_yield_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_yield_expression(s, arg0, arg1)
    }

    fn make_yield_from_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_yield_from_expression(s, arg0, arg1, arg2)
    }

    fn make_prefix_unary_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_prefix_unary_expression(s, arg0, arg1)
    }

    fn make_postfix_unary_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_postfix_unary_expression(s, arg0, arg1)
    }

    fn make_binary_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_binary_expression(s, arg0, arg1, arg2)
    }

    fn make_instanceof_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_instanceof_expression(s, arg0, arg1, arg2)
    }

    fn make_is_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_is_expression(s, arg0, arg1, arg2)
    }

    fn make_as_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_as_expression(s, arg0, arg1, arg2)
    }

    fn make_nullable_as_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_nullable_as_expression(s, arg0, arg1, arg2)
    }

    fn make_conditional_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_conditional_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_eval_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_eval_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_define_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_define_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_halt_compiler_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_halt_compiler_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_isset_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_isset_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_function_call_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_function_call_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_parenthesized_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_parenthesized_expression(s, arg0, arg1, arg2)
    }

    fn make_braced_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_embedded_braced_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_embedded_braced_expression(s, arg0, arg1, arg2)
    }

    fn make_list_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_list_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_collection_literal_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_collection_literal_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_object_creation_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_object_creation_expression(s, arg0, arg1)
    }

    fn make_constructor_call(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_constructor_call(s, arg0, arg1, arg2, arg3)
    }

    fn make_record_creation_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_record_creation_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_array_creation_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_array_creation_expression(s, arg0, arg1, arg2)
    }

    fn make_array_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_array_intrinsic_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_darray_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_darray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_dictionary_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_dictionary_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_keyset_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_varray_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_varray_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_intrinsic_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_vector_intrinsic_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_element_initializer(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_element_initializer(s, arg0, arg1, arg2)
    }

    fn make_subscript_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_embedded_subscript_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_embedded_subscript_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_awaitable_creation_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_awaitable_creation_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_children_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_children_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_children_parenthesized_list(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_children_parenthesized_list(s, arg0, arg1, arg2)
    }

    fn make_xhp_category_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_category_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_enum_type(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_enum_type(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_xhp_lateinit(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_lateinit(s, arg0, arg1)
    }

    fn make_xhp_required(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_required(s, arg0, arg1)
    }

    fn make_xhp_class_attribute_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_class_attribute_declaration(s, arg0, arg1, arg2)
    }

    fn make_xhp_class_attribute(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_class_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_simple_class_attribute(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_simple_class_attribute(s, arg0)
    }

    fn make_xhp_simple_attribute(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_simple_attribute(s, arg0, arg1, arg2)
    }

    fn make_xhp_spread_attribute(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_spread_attribute(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_open(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_open(s, arg0, arg1, arg2, arg3)
    }

    fn make_xhp_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_expression(s, arg0, arg1, arg2)
    }

    fn make_xhp_close(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_xhp_close(s, arg0, arg1, arg2)
    }

    fn make_type_constant(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_constant(s, arg0, arg1, arg2)
    }

    fn make_vector_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_vector_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_keyset_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_keyset_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_tuple_type_explicit_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_tuple_type_explicit_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_varray_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_varray_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_vector_array_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_vector_array_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_type_parameter(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_parameter(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_type_constraint(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_constraint(s, arg0, arg1)
    }

    fn make_darray_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_darray_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    }

    fn make_map_array_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_map_array_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_dictionary_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_dictionary_type_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_closure_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R, arg6: Self::R, arg7: Self::R, arg8: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_closure_type_specifier(s, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
    }

    fn make_closure_parameter_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_closure_parameter_type_specifier(s, arg0, arg1)
    }

    fn make_classname_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_classname_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_field_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_field_specifier(s, arg0, arg1, arg2, arg3)
    }

    fn make_field_initializer(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_field_initializer(s, arg0, arg1, arg2)
    }

    fn make_shape_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_shape_type_specifier(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_shape_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_shape_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_tuple_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_tuple_expression(s, arg0, arg1, arg2, arg3)
    }

    fn make_generic_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_generic_type_specifier(s, arg0, arg1)
    }

    fn make_nullable_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_nullable_type_specifier(s, arg0, arg1)
    }

    fn make_like_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_like_type_specifier(s, arg0, arg1)
    }

    fn make_soft_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_soft_type_specifier(s, arg0, arg1)
    }

    fn make_attributized_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_attributized_specifier(s, arg0, arg1)
    }

    fn make_reified_type_argument(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_reified_type_argument(s, arg0, arg1)
    }

    fn make_type_arguments(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_arguments(s, arg0, arg1, arg2)
    }

    fn make_type_parameters(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_type_parameters(s, arg0, arg1, arg2)
    }

    fn make_tuple_type_specifier(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_tuple_type_specifier(s, arg0, arg1, arg2)
    }

    fn make_error(s: HasScriptContent<'src>, arg0: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_error(s, arg0)
    }

    fn make_list_item(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_list_item(s, arg0, arg1)
    }

    fn make_pocket_atom_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_atom_expression(s, arg0, arg1)
    }

    fn make_pocket_identifier_expression(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_identifier_expression(s, arg0, arg1, arg2, arg3, arg4)
    }

    fn make_pocket_atom_mapping_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_atom_mapping_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_enum_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R, arg4: Self::R, arg5: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_enum_declaration(s, arg0, arg1, arg2, arg3, arg4, arg5)
    }

    fn make_pocket_field_type_expr_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_field_type_expr_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_field_type_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_field_type_declaration(s, arg0, arg1, arg2, arg3)
    }

    fn make_pocket_mapping_id_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_mapping_id_declaration(s, arg0, arg1)
    }

    fn make_pocket_mapping_type_declaration(s: HasScriptContent<'src>, arg0: Self::R, arg1: Self::R, arg2: Self::R, arg3: Self::R) -> (HasScriptContent<'src>, Self::R) {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_pocket_mapping_type_declaration(s, arg0, arg1, arg2, arg3)
    }

}
