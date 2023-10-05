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
use flatten_smart_constructors::*;
use parser_core_types::compact_token::CompactToken;
use parser_core_types::token_factory::SimpleTokenFactoryImpl;
use smart_constructors::SmartConstructors;

use crate::{DirectDeclSmartConstructors, Node, SourceTextAllocator};

impl<'a, 'o, 't, S: SourceTextAllocator<'t, 'a>> SmartConstructors for DirectDeclSmartConstructors<'a, 'o, 't, S> {
    type State = Self;
    type Factory = SimpleTokenFactoryImpl<CompactToken>;
    type Output = Node<'a>;

    fn state_mut(&mut self) -> &mut Self {
        self
    }

    fn into_state(self) -> Self {
        self
    }

    fn token_factory_mut(&mut self) -> &mut Self::Factory {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_missing(self, offset)
    }

    fn make_token(&mut self, token: CompactToken) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_token(self, token)
    }

    fn make_list(&mut self, items: Vec<Self::Output>, offset: usize) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_list(self, items, offset)
    }

    fn begin_enumerator(&mut self) {
        <Self as FlattenSmartConstructors>::begin_enumerator(self)
    }

    fn begin_enum_class_enumerator(&mut self) {
        <Self as FlattenSmartConstructors>::begin_enum_class_enumerator(self)
    }

    fn begin_constant_declarator(&mut self) {
        <Self as FlattenSmartConstructors>::begin_constant_declarator(self)
    }



    fn make_end_of_file(&mut self, token: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_end_of_file(self, token)
    }

    fn make_script(&mut self, declarations: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_script(self, declarations)
    }

    fn make_qualified_name(&mut self, parts: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_qualified_name(self, parts)
    }

    fn make_module_name(&mut self, parts: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_module_name(self, parts)
    }

    fn make_simple_type_specifier(&mut self, specifier: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_simple_type_specifier(self, specifier)
    }

    fn make_literal_expression(&mut self, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_literal_expression(self, expression)
    }

    fn make_prefixed_string_expression(&mut self, name: Self::Output, str: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_prefixed_string_expression(self, name, str)
    }

    fn make_prefixed_code_expression(&mut self, prefix: Self::Output, left_backtick: Self::Output, body: Self::Output, right_backtick: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_prefixed_code_expression(self, prefix, left_backtick, body, right_backtick)
    }

    fn make_variable_expression(&mut self, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_variable_expression(self, expression)
    }

    fn make_pipe_variable_expression(&mut self, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_pipe_variable_expression(self, expression)
    }

    fn make_file_attribute_specification(&mut self, left_double_angle: Self::Output, keyword: Self::Output, colon: Self::Output, attributes: Self::Output, right_double_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_file_attribute_specification(self, left_double_angle, keyword, colon, attributes, right_double_angle)
    }

    fn make_enum_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, name: Self::Output, colon: Self::Output, base: Self::Output, type_: Self::Output, left_brace: Self::Output, use_clauses: Self::Output, enumerators: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enum_declaration(self, attribute_spec, modifiers, keyword, name, colon, base, type_, left_brace, use_clauses, enumerators, right_brace)
    }

    fn make_enum_use(&mut self, keyword: Self::Output, names: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enum_use(self, keyword, names, semicolon)
    }

    fn make_enumerator(&mut self, name: Self::Output, equal: Self::Output, value: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enumerator(self, name, equal, value, semicolon)
    }

    fn make_enum_class_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, enum_keyword: Self::Output, class_keyword: Self::Output, name: Self::Output, colon: Self::Output, base: Self::Output, extends: Self::Output, extends_list: Self::Output, left_brace: Self::Output, elements: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enum_class_declaration(self, attribute_spec, modifiers, enum_keyword, class_keyword, name, colon, base, extends, extends_list, left_brace, elements, right_brace)
    }

    fn make_enum_class_enumerator(&mut self, modifiers: Self::Output, type_: Self::Output, name: Self::Output, initializer: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enum_class_enumerator(self, modifiers, type_, name, initializer, semicolon)
    }

    fn make_alias_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, module_kw_opt: Self::Output, keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, constraint: Self::Output, equal: Self::Output, type_: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_alias_declaration(self, attribute_spec, modifiers, module_kw_opt, keyword, name, generic_parameter, constraint, equal, type_, semicolon)
    }

    fn make_context_alias_declaration(&mut self, attribute_spec: Self::Output, keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, as_constraint: Self::Output, equal: Self::Output, context: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_context_alias_declaration(self, attribute_spec, keyword, name, generic_parameter, as_constraint, equal, context, semicolon)
    }

    fn make_case_type_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, case_keyword: Self::Output, type_keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, as_: Self::Output, bounds: Self::Output, equal: Self::Output, variants: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_case_type_declaration(self, attribute_spec, modifiers, case_keyword, type_keyword, name, generic_parameter, as_, bounds, equal, variants, semicolon)
    }

    fn make_case_type_variant(&mut self, bar: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_case_type_variant(self, bar, type_)
    }

    fn make_property_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, type_: Self::Output, declarators: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_property_declaration(self, attribute_spec, modifiers, type_, declarators, semicolon)
    }

    fn make_property_declarator(&mut self, name: Self::Output, initializer: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_property_declarator(self, name, initializer)
    }

    fn make_namespace_declaration(&mut self, header: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_declaration(self, header, body)
    }

    fn make_namespace_declaration_header(&mut self, keyword: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_declaration_header(self, keyword, name)
    }

    fn make_namespace_body(&mut self, left_brace: Self::Output, declarations: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_body(self, left_brace, declarations, right_brace)
    }

    fn make_namespace_empty_body(&mut self, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_empty_body(self, semicolon)
    }

    fn make_namespace_use_declaration(&mut self, keyword: Self::Output, kind: Self::Output, clauses: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_use_declaration(self, keyword, kind, clauses, semicolon)
    }

    fn make_namespace_group_use_declaration(&mut self, keyword: Self::Output, kind: Self::Output, prefix: Self::Output, left_brace: Self::Output, clauses: Self::Output, right_brace: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_group_use_declaration(self, keyword, kind, prefix, left_brace, clauses, right_brace, semicolon)
    }

    fn make_namespace_use_clause(&mut self, clause_kind: Self::Output, name: Self::Output, as_: Self::Output, alias: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_namespace_use_clause(self, clause_kind, name, as_, alias)
    }

    fn make_function_declaration(&mut self, attribute_spec: Self::Output, declaration_header: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_function_declaration(self, attribute_spec, declaration_header, body)
    }

    fn make_function_declaration_header(&mut self, modifiers: Self::Output, keyword: Self::Output, name: Self::Output, type_parameter_list: Self::Output, left_paren: Self::Output, parameter_list: Self::Output, right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output, where_clause: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_function_declaration_header(self, modifiers, keyword, name, type_parameter_list, left_paren, parameter_list, right_paren, contexts, colon, readonly_return, type_, where_clause)
    }

    fn make_contexts(&mut self, left_bracket: Self::Output, types: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_contexts(self, left_bracket, types, right_bracket)
    }

    fn make_where_clause(&mut self, keyword: Self::Output, constraints: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_where_clause(self, keyword, constraints)
    }

    fn make_where_constraint(&mut self, left_type: Self::Output, operator: Self::Output, right_type: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_where_constraint(self, left_type, operator, right_type)
    }

    fn make_methodish_declaration(&mut self, attribute: Self::Output, function_decl_header: Self::Output, function_body: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_methodish_declaration(self, attribute, function_decl_header, function_body, semicolon)
    }

    fn make_methodish_trait_resolution(&mut self, attribute: Self::Output, function_decl_header: Self::Output, equal: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_methodish_trait_resolution(self, attribute, function_decl_header, equal, name, semicolon)
    }

    fn make_classish_declaration(&mut self, attribute: Self::Output, modifiers: Self::Output, xhp: Self::Output, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, extends_keyword: Self::Output, extends_list: Self::Output, implements_keyword: Self::Output, implements_list: Self::Output, where_clause: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_classish_declaration(self, attribute, modifiers, xhp, keyword, name, type_parameters, extends_keyword, extends_list, implements_keyword, implements_list, where_clause, body)
    }

    fn make_classish_body(&mut self, left_brace: Self::Output, elements: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_classish_body(self, left_brace, elements, right_brace)
    }

    fn make_trait_use(&mut self, keyword: Self::Output, names: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_trait_use(self, keyword, names, semicolon)
    }

    fn make_require_clause(&mut self, keyword: Self::Output, kind: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_require_clause(self, keyword, kind, name, semicolon)
    }

    fn make_const_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, type_specifier: Self::Output, declarators: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_const_declaration(self, attribute_spec, modifiers, keyword, type_specifier, declarators, semicolon)
    }

    fn make_constant_declarator(&mut self, name: Self::Output, initializer: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_constant_declarator(self, name, initializer)
    }

    fn make_type_const_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, type_keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, type_constraints: Self::Output, equal: Self::Output, type_specifier: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_const_declaration(self, attribute_spec, modifiers, keyword, type_keyword, name, type_parameters, type_constraints, equal, type_specifier, semicolon)
    }

    fn make_context_const_declaration(&mut self, modifiers: Self::Output, const_keyword: Self::Output, ctx_keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraint: Self::Output, equal: Self::Output, ctx_list: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_context_const_declaration(self, modifiers, const_keyword, ctx_keyword, name, type_parameters, constraint, equal, ctx_list, semicolon)
    }

    fn make_decorated_expression(&mut self, decorator: Self::Output, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_decorated_expression(self, decorator, expression)
    }

    fn make_parameter_declaration(&mut self, attribute: Self::Output, visibility: Self::Output, call_convention: Self::Output, readonly: Self::Output, type_: Self::Output, name: Self::Output, default_value: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_parameter_declaration(self, attribute, visibility, call_convention, readonly, type_, name, default_value)
    }

    fn make_variadic_parameter(&mut self, call_convention: Self::Output, type_: Self::Output, ellipsis: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_variadic_parameter(self, call_convention, type_, ellipsis)
    }

    fn make_old_attribute_specification(&mut self, left_double_angle: Self::Output, attributes: Self::Output, right_double_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_old_attribute_specification(self, left_double_angle, attributes, right_double_angle)
    }

    fn make_attribute_specification(&mut self, attributes: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_attribute_specification(self, attributes)
    }

    fn make_attribute(&mut self, at: Self::Output, attribute_name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_attribute(self, at, attribute_name)
    }

    fn make_inclusion_expression(&mut self, require: Self::Output, filename: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_inclusion_expression(self, require, filename)
    }

    fn make_inclusion_directive(&mut self, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_inclusion_directive(self, expression, semicolon)
    }

    fn make_compound_statement(&mut self, left_brace: Self::Output, statements: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_compound_statement(self, left_brace, statements, right_brace)
    }

    fn make_expression_statement(&mut self, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_expression_statement(self, expression, semicolon)
    }

    fn make_markup_section(&mut self, hashbang: Self::Output, suffix: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_markup_section(self, hashbang, suffix)
    }

    fn make_markup_suffix(&mut self, less_than_question: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_markup_suffix(self, less_than_question, name)
    }

    fn make_unset_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, variables: Self::Output, right_paren: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_unset_statement(self, keyword, left_paren, variables, right_paren, semicolon)
    }

    fn make_declare_local_statement(&mut self, keyword: Self::Output, variable: Self::Output, colon: Self::Output, type_: Self::Output, initializer: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_declare_local_statement(self, keyword, variable, colon, type_, initializer, semicolon)
    }

    fn make_using_statement_block_scoped(&mut self, await_keyword: Self::Output, using_keyword: Self::Output, left_paren: Self::Output, expressions: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_using_statement_block_scoped(self, await_keyword, using_keyword, left_paren, expressions, right_paren, body)
    }

    fn make_using_statement_function_scoped(&mut self, await_keyword: Self::Output, using_keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_using_statement_function_scoped(self, await_keyword, using_keyword, expression, semicolon)
    }

    fn make_while_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_while_statement(self, keyword, left_paren, condition, right_paren, body)
    }

    fn make_if_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, statement: Self::Output, else_clause: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_if_statement(self, keyword, left_paren, condition, right_paren, statement, else_clause)
    }

    fn make_else_clause(&mut self, keyword: Self::Output, statement: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_else_clause(self, keyword, statement)
    }

    fn make_try_statement(&mut self, keyword: Self::Output, compound_statement: Self::Output, catch_clauses: Self::Output, finally_clause: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_try_statement(self, keyword, compound_statement, catch_clauses, finally_clause)
    }

    fn make_catch_clause(&mut self, keyword: Self::Output, left_paren: Self::Output, type_: Self::Output, variable: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_catch_clause(self, keyword, left_paren, type_, variable, right_paren, body)
    }

    fn make_finally_clause(&mut self, keyword: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_finally_clause(self, keyword, body)
    }

    fn make_do_statement(&mut self, keyword: Self::Output, body: Self::Output, while_keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_do_statement(self, keyword, body, while_keyword, left_paren, condition, right_paren, semicolon)
    }

    fn make_for_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, initializer: Self::Output, first_semicolon: Self::Output, control: Self::Output, second_semicolon: Self::Output, end_of_loop: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_for_statement(self, keyword, left_paren, initializer, first_semicolon, control, second_semicolon, end_of_loop, right_paren, body)
    }

    fn make_foreach_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, collection: Self::Output, await_keyword: Self::Output, as_: Self::Output, key: Self::Output, arrow: Self::Output, value: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_foreach_statement(self, keyword, left_paren, collection, await_keyword, as_, key, arrow, value, right_paren, body)
    }

    fn make_switch_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output, left_brace: Self::Output, sections: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_switch_statement(self, keyword, left_paren, expression, right_paren, left_brace, sections, right_brace)
    }

    fn make_switch_section(&mut self, labels: Self::Output, statements: Self::Output, fallthrough: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_switch_section(self, labels, statements, fallthrough)
    }

    fn make_switch_fallthrough(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_switch_fallthrough(self, keyword, semicolon)
    }

    fn make_case_label(&mut self, keyword: Self::Output, expression: Self::Output, colon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_case_label(self, keyword, expression, colon)
    }

    fn make_default_label(&mut self, keyword: Self::Output, colon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_default_label(self, keyword, colon)
    }

    fn make_match_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output, left_brace: Self::Output, arms: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_match_statement(self, keyword, left_paren, expression, right_paren, left_brace, arms, right_brace)
    }

    fn make_match_statement_arm(&mut self, pattern: Self::Output, arrow: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_match_statement_arm(self, pattern, arrow, body)
    }

    fn make_return_statement(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_return_statement(self, keyword, expression, semicolon)
    }

    fn make_yield_break_statement(&mut self, keyword: Self::Output, break_: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_yield_break_statement(self, keyword, break_, semicolon)
    }

    fn make_throw_statement(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_throw_statement(self, keyword, expression, semicolon)
    }

    fn make_break_statement(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_break_statement(self, keyword, semicolon)
    }

    fn make_continue_statement(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_continue_statement(self, keyword, semicolon)
    }

    fn make_echo_statement(&mut self, keyword: Self::Output, expressions: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_echo_statement(self, keyword, expressions, semicolon)
    }

    fn make_concurrent_statement(&mut self, keyword: Self::Output, statement: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_concurrent_statement(self, keyword, statement)
    }

    fn make_simple_initializer(&mut self, equal: Self::Output, value: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_simple_initializer(self, equal, value)
    }

    fn make_anonymous_class(&mut self, class_keyword: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output, extends_keyword: Self::Output, extends_list: Self::Output, implements_keyword: Self::Output, implements_list: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_anonymous_class(self, class_keyword, left_paren, argument_list, right_paren, extends_keyword, extends_list, implements_keyword, implements_list, body)
    }

    fn make_anonymous_function(&mut self, attribute_spec: Self::Output, async_keyword: Self::Output, function_keyword: Self::Output, left_paren: Self::Output, parameters: Self::Output, right_paren: Self::Output, ctx_list: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output, use_: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_anonymous_function(self, attribute_spec, async_keyword, function_keyword, left_paren, parameters, right_paren, ctx_list, colon, readonly_return, type_, use_, body)
    }

    fn make_anonymous_function_use_clause(&mut self, keyword: Self::Output, left_paren: Self::Output, variables: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_anonymous_function_use_clause(self, keyword, left_paren, variables, right_paren)
    }

    fn make_variable_pattern(&mut self, variable: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_variable_pattern(self, variable)
    }

    fn make_constructor_pattern(&mut self, constructor: Self::Output, left_paren: Self::Output, members: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_constructor_pattern(self, constructor, left_paren, members, right_paren)
    }

    fn make_refinement_pattern(&mut self, variable: Self::Output, colon: Self::Output, specifier: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_refinement_pattern(self, variable, colon, specifier)
    }

    fn make_lambda_expression(&mut self, attribute_spec: Self::Output, async_: Self::Output, signature: Self::Output, arrow: Self::Output, body: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_lambda_expression(self, attribute_spec, async_, signature, arrow, body)
    }

    fn make_lambda_signature(&mut self, left_paren: Self::Output, parameters: Self::Output, right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_lambda_signature(self, left_paren, parameters, right_paren, contexts, colon, readonly_return, type_)
    }

    fn make_cast_expression(&mut self, left_paren: Self::Output, type_: Self::Output, right_paren: Self::Output, operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_cast_expression(self, left_paren, type_, right_paren, operand)
    }

    fn make_scope_resolution_expression(&mut self, qualifier: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_scope_resolution_expression(self, qualifier, operator, name)
    }

    fn make_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_member_selection_expression(self, object, operator, name)
    }

    fn make_safe_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_safe_member_selection_expression(self, object, operator, name)
    }

    fn make_embedded_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_embedded_member_selection_expression(self, object, operator, name)
    }

    fn make_yield_expression(&mut self, keyword: Self::Output, operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_yield_expression(self, keyword, operand)
    }

    fn make_prefix_unary_expression(&mut self, operator: Self::Output, operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_prefix_unary_expression(self, operator, operand)
    }

    fn make_postfix_unary_expression(&mut self, operand: Self::Output, operator: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_postfix_unary_expression(self, operand, operator)
    }

    fn make_binary_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_binary_expression(self, left_operand, operator, right_operand)
    }

    fn make_is_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_is_expression(self, left_operand, operator, right_operand)
    }

    fn make_as_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_as_expression(self, left_operand, operator, right_operand)
    }

    fn make_nullable_as_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_nullable_as_expression(self, left_operand, operator, right_operand)
    }

    fn make_upcast_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_upcast_expression(self, left_operand, operator, right_operand)
    }

    fn make_conditional_expression(&mut self, test: Self::Output, question: Self::Output, consequence: Self::Output, colon: Self::Output, alternative: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_conditional_expression(self, test, question, consequence, colon, alternative)
    }

    fn make_eval_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, argument: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_eval_expression(self, keyword, left_paren, argument, right_paren)
    }

    fn make_isset_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_isset_expression(self, keyword, left_paren, argument_list, right_paren)
    }

    fn make_nameof_expression(&mut self, keyword: Self::Output, target: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_nameof_expression(self, keyword, target)
    }

    fn make_function_call_expression(&mut self, receiver: Self::Output, type_args: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_function_call_expression(self, receiver, type_args, left_paren, argument_list, right_paren)
    }

    fn make_function_pointer_expression(&mut self, receiver: Self::Output, type_args: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_function_pointer_expression(self, receiver, type_args)
    }

    fn make_parenthesized_expression(&mut self, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_parenthesized_expression(self, left_paren, expression, right_paren)
    }

    fn make_braced_expression(&mut self, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_braced_expression(self, left_brace, expression, right_brace)
    }

    fn make_et_splice_expression(&mut self, dollar: Self::Output, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_et_splice_expression(self, dollar, left_brace, expression, right_brace)
    }

    fn make_embedded_braced_expression(&mut self, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_embedded_braced_expression(self, left_brace, expression, right_brace)
    }

    fn make_list_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, members: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_list_expression(self, keyword, left_paren, members, right_paren)
    }

    fn make_collection_literal_expression(&mut self, name: Self::Output, left_brace: Self::Output, initializers: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_collection_literal_expression(self, name, left_brace, initializers, right_brace)
    }

    fn make_object_creation_expression(&mut self, new_keyword: Self::Output, object: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_object_creation_expression(self, new_keyword, object)
    }

    fn make_constructor_call(&mut self, type_: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_constructor_call(self, type_, left_paren, argument_list, right_paren)
    }

    fn make_darray_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_darray_intrinsic_expression(self, keyword, explicit_type, left_bracket, members, right_bracket)
    }

    fn make_dictionary_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_dictionary_intrinsic_expression(self, keyword, explicit_type, left_bracket, members, right_bracket)
    }

    fn make_keyset_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_keyset_intrinsic_expression(self, keyword, explicit_type, left_bracket, members, right_bracket)
    }

    fn make_varray_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_varray_intrinsic_expression(self, keyword, explicit_type, left_bracket, members, right_bracket)
    }

    fn make_vector_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_vector_intrinsic_expression(self, keyword, explicit_type, left_bracket, members, right_bracket)
    }

    fn make_element_initializer(&mut self, key: Self::Output, arrow: Self::Output, value: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_element_initializer(self, key, arrow, value)
    }

    fn make_subscript_expression(&mut self, receiver: Self::Output, left_bracket: Self::Output, index: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_subscript_expression(self, receiver, left_bracket, index, right_bracket)
    }

    fn make_embedded_subscript_expression(&mut self, receiver: Self::Output, left_bracket: Self::Output, index: Self::Output, right_bracket: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_embedded_subscript_expression(self, receiver, left_bracket, index, right_bracket)
    }

    fn make_awaitable_creation_expression(&mut self, attribute_spec: Self::Output, async_: Self::Output, compound_statement: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_awaitable_creation_expression(self, attribute_spec, async_, compound_statement)
    }

    fn make_xhp_children_declaration(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_children_declaration(self, keyword, expression, semicolon)
    }

    fn make_xhp_children_parenthesized_list(&mut self, left_paren: Self::Output, xhp_children: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_children_parenthesized_list(self, left_paren, xhp_children, right_paren)
    }

    fn make_xhp_category_declaration(&mut self, keyword: Self::Output, categories: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_category_declaration(self, keyword, categories, semicolon)
    }

    fn make_xhp_enum_type(&mut self, like: Self::Output, keyword: Self::Output, left_brace: Self::Output, values: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_enum_type(self, like, keyword, left_brace, values, right_brace)
    }

    fn make_xhp_lateinit(&mut self, at: Self::Output, keyword: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_lateinit(self, at, keyword)
    }

    fn make_xhp_required(&mut self, at: Self::Output, keyword: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_required(self, at, keyword)
    }

    fn make_xhp_class_attribute_declaration(&mut self, keyword: Self::Output, attributes: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_class_attribute_declaration(self, keyword, attributes, semicolon)
    }

    fn make_xhp_class_attribute(&mut self, type_: Self::Output, name: Self::Output, initializer: Self::Output, required: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_class_attribute(self, type_, name, initializer, required)
    }

    fn make_xhp_simple_class_attribute(&mut self, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_simple_class_attribute(self, type_)
    }

    fn make_xhp_simple_attribute(&mut self, name: Self::Output, equal: Self::Output, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_simple_attribute(self, name, equal, expression)
    }

    fn make_xhp_spread_attribute(&mut self, left_brace: Self::Output, spread_operator: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_spread_attribute(self, left_brace, spread_operator, expression, right_brace)
    }

    fn make_xhp_open(&mut self, left_angle: Self::Output, name: Self::Output, attributes: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_open(self, left_angle, name, attributes, right_angle)
    }

    fn make_xhp_expression(&mut self, open: Self::Output, body: Self::Output, close: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_expression(self, open, body, close)
    }

    fn make_xhp_close(&mut self, left_angle: Self::Output, name: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_xhp_close(self, left_angle, name, right_angle)
    }

    fn make_type_constant(&mut self, left_type: Self::Output, separator: Self::Output, right_type: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_constant(self, left_type, separator, right_type)
    }

    fn make_vector_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_vector_type_specifier(self, keyword, left_angle, type_, trailing_comma, right_angle)
    }

    fn make_keyset_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_keyset_type_specifier(self, keyword, left_angle, type_, trailing_comma, right_angle)
    }

    fn make_tuple_type_explicit_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, types: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_tuple_type_explicit_specifier(self, keyword, left_angle, types, right_angle)
    }

    fn make_varray_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_varray_type_specifier(self, keyword, left_angle, type_, trailing_comma, right_angle)
    }

    fn make_function_ctx_type_specifier(&mut self, keyword: Self::Output, variable: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_function_ctx_type_specifier(self, keyword, variable)
    }

    fn make_type_parameter(&mut self, attribute_spec: Self::Output, reified: Self::Output, variance: Self::Output, name: Self::Output, param_params: Self::Output, constraints: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_parameter(self, attribute_spec, reified, variance, name, param_params, constraints)
    }

    fn make_type_constraint(&mut self, keyword: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_constraint(self, keyword, type_)
    }

    fn make_context_constraint(&mut self, keyword: Self::Output, ctx_list: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_context_constraint(self, keyword, ctx_list)
    }

    fn make_darray_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, key: Self::Output, comma: Self::Output, value: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_darray_type_specifier(self, keyword, left_angle, key, comma, value, trailing_comma, right_angle)
    }

    fn make_dictionary_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, members: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_dictionary_type_specifier(self, keyword, left_angle, members, right_angle)
    }

    fn make_closure_type_specifier(&mut self, outer_left_paren: Self::Output, readonly_keyword: Self::Output, function_keyword: Self::Output, inner_left_paren: Self::Output, parameter_list: Self::Output, inner_right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, return_type: Self::Output, outer_right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_closure_type_specifier(self, outer_left_paren, readonly_keyword, function_keyword, inner_left_paren, parameter_list, inner_right_paren, contexts, colon, readonly_return, return_type, outer_right_paren)
    }

    fn make_closure_parameter_type_specifier(&mut self, call_convention: Self::Output, readonly: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_closure_parameter_type_specifier(self, call_convention, readonly, type_)
    }

    fn make_type_refinement(&mut self, type_: Self::Output, keyword: Self::Output, left_brace: Self::Output, members: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_refinement(self, type_, keyword, left_brace, members, right_brace)
    }

    fn make_type_in_refinement(&mut self, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraints: Self::Output, equal: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_in_refinement(self, keyword, name, type_parameters, constraints, equal, type_)
    }

    fn make_ctx_in_refinement(&mut self, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraints: Self::Output, equal: Self::Output, ctx_list: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_ctx_in_refinement(self, keyword, name, type_parameters, constraints, equal, ctx_list)
    }

    fn make_classname_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_classname_type_specifier(self, keyword, left_angle, type_, trailing_comma, right_angle)
    }

    fn make_class_args_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_class_args_type_specifier(self, keyword, left_angle, type_, trailing_comma, right_angle)
    }

    fn make_field_specifier(&mut self, question: Self::Output, name: Self::Output, arrow: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_field_specifier(self, question, name, arrow, type_)
    }

    fn make_field_initializer(&mut self, name: Self::Output, arrow: Self::Output, value: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_field_initializer(self, name, arrow, value)
    }

    fn make_shape_type_specifier(&mut self, keyword: Self::Output, left_paren: Self::Output, fields: Self::Output, ellipsis: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_shape_type_specifier(self, keyword, left_paren, fields, ellipsis, right_paren)
    }

    fn make_shape_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, fields: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_shape_expression(self, keyword, left_paren, fields, right_paren)
    }

    fn make_tuple_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, items: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_tuple_expression(self, keyword, left_paren, items, right_paren)
    }

    fn make_generic_type_specifier(&mut self, class_type: Self::Output, argument_list: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_generic_type_specifier(self, class_type, argument_list)
    }

    fn make_nullable_type_specifier(&mut self, question: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_nullable_type_specifier(self, question, type_)
    }

    fn make_like_type_specifier(&mut self, tilde: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_like_type_specifier(self, tilde, type_)
    }

    fn make_soft_type_specifier(&mut self, at: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_soft_type_specifier(self, at, type_)
    }

    fn make_attributized_specifier(&mut self, attribute_spec: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_attributized_specifier(self, attribute_spec, type_)
    }

    fn make_reified_type_argument(&mut self, reified: Self::Output, type_: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_reified_type_argument(self, reified, type_)
    }

    fn make_type_arguments(&mut self, left_angle: Self::Output, types: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_arguments(self, left_angle, types, right_angle)
    }

    fn make_type_parameters(&mut self, left_angle: Self::Output, parameters: Self::Output, right_angle: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_type_parameters(self, left_angle, parameters, right_angle)
    }

    fn make_tuple_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_tuple_type_specifier(self, left_paren, types, right_paren)
    }

    fn make_union_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_union_type_specifier(self, left_paren, types, right_paren)
    }

    fn make_intersection_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_intersection_type_specifier(self, left_paren, types, right_paren)
    }

    fn make_error(&mut self, error: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_error(self, error)
    }

    fn make_list_item(&mut self, item: Self::Output, separator: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_list_item(self, item, separator)
    }

    fn make_enum_class_label_expression(&mut self, qualifier: Self::Output, hash: Self::Output, expression: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_enum_class_label_expression(self, qualifier, hash, expression)
    }

    fn make_module_declaration(&mut self, attribute_spec: Self::Output, new_keyword: Self::Output, module_keyword: Self::Output, name: Self::Output, left_brace: Self::Output, exports: Self::Output, imports: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_module_declaration(self, attribute_spec, new_keyword, module_keyword, name, left_brace, exports, imports, right_brace)
    }

    fn make_module_exports(&mut self, exports_keyword: Self::Output, left_brace: Self::Output, exports: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_module_exports(self, exports_keyword, left_brace, exports, right_brace)
    }

    fn make_module_imports(&mut self, imports_keyword: Self::Output, left_brace: Self::Output, imports: Self::Output, right_brace: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_module_imports(self, imports_keyword, left_brace, imports, right_brace)
    }

    fn make_module_membership_declaration(&mut self, module_keyword: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_module_membership_declaration(self, module_keyword, name, semicolon)
    }

    fn make_package_expression(&mut self, keyword: Self::Output, name: Self::Output) -> Self::Output {
        <Self as FlattenSmartConstructors>::make_package_expression(self, keyword, name)
    }

}
