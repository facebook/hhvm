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
use smart_constructors::{NodeType, SmartConstructors};

use crate::{PairTokenFactory, Node};

#[derive(Clone)]
pub struct PairSmartConstructors<SC0, SC1>(pub SC0, pub SC1, PairTokenFactory<SC0::Factory, SC1::Factory>)
where
    SC0: SmartConstructors,
    SC0::Output: NodeType,
    SC1: SmartConstructors,
    SC1::Output: NodeType;

impl<SC0, SC1> PairSmartConstructors<SC0, SC1>
where
    SC0: SmartConstructors,
    SC0::Output: NodeType,
    SC1: SmartConstructors,
    SC1::Output: NodeType,
{
    pub fn new(mut sc0: SC0, mut sc1: SC1) -> Self {
        let tf0 = sc0.token_factory_mut().clone();
        let tf1 = sc1.token_factory_mut().clone();
        let tf = PairTokenFactory::new(tf0, tf1);
        Self(sc0, sc1, tf)
    }
}

impl<SC0, SC1> SmartConstructors for PairSmartConstructors<SC0, SC1>
where
    SC0: SmartConstructors,
    SC0::Output: NodeType,
    SC1: SmartConstructors,
    SC1::Output: NodeType,
{
    type State = Self;
    type Factory = PairTokenFactory<SC0::Factory, SC1::Factory>;
    type Output = Node<SC0::Output, SC1::Output>;

    fn state_mut(&mut self) -> &mut Self {
        self
    }

    fn into_state(self) -> Self {
        self
    }

    fn token_factory_mut(&mut self) -> &mut Self::Factory {
        &mut self.2
    }

    fn make_missing(&mut self, offset: usize) -> Self::Output {
        Node(self.0.make_missing(offset), self.1.make_missing(offset))
    }

    fn make_token(&mut self, token: <Self::Factory as TokenFactory>::Token) -> Self::Output {
        Node(self.0.make_token(token.0), self.1.make_token(token.1))
    }

    fn make_list(&mut self, items: Vec<Self::Output>, offset: usize) -> Self::Output {
        let (items0, items1) = items.into_iter().map(|n| (n.0, n.1)).unzip();
        Node(self.0.make_list(items0, offset), self.1.make_list(items1, offset))
    }

    fn make_end_of_file(&mut self, token: Self::Output) -> Self::Output {
        Node(self.0.make_end_of_file(token.0), self.1.make_end_of_file(token.1))
    }

    fn make_script(&mut self, declarations: Self::Output) -> Self::Output {
        Node(self.0.make_script(declarations.0), self.1.make_script(declarations.1))
    }

    fn make_qualified_name(&mut self, parts: Self::Output) -> Self::Output {
        Node(self.0.make_qualified_name(parts.0), self.1.make_qualified_name(parts.1))
    }

    fn make_module_name(&mut self, parts: Self::Output) -> Self::Output {
        Node(self.0.make_module_name(parts.0), self.1.make_module_name(parts.1))
    }

    fn make_simple_type_specifier(&mut self, specifier: Self::Output) -> Self::Output {
        Node(self.0.make_simple_type_specifier(specifier.0), self.1.make_simple_type_specifier(specifier.1))
    }

    fn make_literal_expression(&mut self, expression: Self::Output) -> Self::Output {
        Node(self.0.make_literal_expression(expression.0), self.1.make_literal_expression(expression.1))
    }

    fn make_prefixed_string_expression(&mut self, name: Self::Output, str: Self::Output) -> Self::Output {
        Node(self.0.make_prefixed_string_expression(name.0, str.0), self.1.make_prefixed_string_expression(name.1, str.1))
    }

    fn make_prefixed_code_expression(&mut self, prefix: Self::Output, left_backtick: Self::Output, body: Self::Output, right_backtick: Self::Output) -> Self::Output {
        Node(self.0.make_prefixed_code_expression(prefix.0, left_backtick.0, body.0, right_backtick.0), self.1.make_prefixed_code_expression(prefix.1, left_backtick.1, body.1, right_backtick.1))
    }

    fn make_variable_expression(&mut self, expression: Self::Output) -> Self::Output {
        Node(self.0.make_variable_expression(expression.0), self.1.make_variable_expression(expression.1))
    }

    fn make_pipe_variable_expression(&mut self, expression: Self::Output) -> Self::Output {
        Node(self.0.make_pipe_variable_expression(expression.0), self.1.make_pipe_variable_expression(expression.1))
    }

    fn make_file_attribute_specification(&mut self, left_double_angle: Self::Output, keyword: Self::Output, colon: Self::Output, attributes: Self::Output, right_double_angle: Self::Output) -> Self::Output {
        Node(self.0.make_file_attribute_specification(left_double_angle.0, keyword.0, colon.0, attributes.0, right_double_angle.0), self.1.make_file_attribute_specification(left_double_angle.1, keyword.1, colon.1, attributes.1, right_double_angle.1))
    }

    fn make_enum_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, name: Self::Output, colon: Self::Output, base: Self::Output, type_: Self::Output, left_brace: Self::Output, use_clauses: Self::Output, enumerators: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_enum_declaration(attribute_spec.0, modifiers.0, keyword.0, name.0, colon.0, base.0, type_.0, left_brace.0, use_clauses.0, enumerators.0, right_brace.0), self.1.make_enum_declaration(attribute_spec.1, modifiers.1, keyword.1, name.1, colon.1, base.1, type_.1, left_brace.1, use_clauses.1, enumerators.1, right_brace.1))
    }

    fn make_enum_use(&mut self, keyword: Self::Output, names: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_enum_use(keyword.0, names.0, semicolon.0), self.1.make_enum_use(keyword.1, names.1, semicolon.1))
    }

    fn make_enumerator(&mut self, name: Self::Output, equal: Self::Output, value: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_enumerator(name.0, equal.0, value.0, semicolon.0), self.1.make_enumerator(name.1, equal.1, value.1, semicolon.1))
    }

    fn make_enum_class_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, enum_keyword: Self::Output, class_keyword: Self::Output, name: Self::Output, colon: Self::Output, base: Self::Output, extends: Self::Output, extends_list: Self::Output, left_brace: Self::Output, elements: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_enum_class_declaration(attribute_spec.0, modifiers.0, enum_keyword.0, class_keyword.0, name.0, colon.0, base.0, extends.0, extends_list.0, left_brace.0, elements.0, right_brace.0), self.1.make_enum_class_declaration(attribute_spec.1, modifiers.1, enum_keyword.1, class_keyword.1, name.1, colon.1, base.1, extends.1, extends_list.1, left_brace.1, elements.1, right_brace.1))
    }

    fn make_enum_class_enumerator(&mut self, modifiers: Self::Output, type_: Self::Output, name: Self::Output, initializer: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_enum_class_enumerator(modifiers.0, type_.0, name.0, initializer.0, semicolon.0), self.1.make_enum_class_enumerator(modifiers.1, type_.1, name.1, initializer.1, semicolon.1))
    }

    fn make_alias_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, module_kw_opt: Self::Output, keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, constraint: Self::Output, equal: Self::Output, type_: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_alias_declaration(attribute_spec.0, modifiers.0, module_kw_opt.0, keyword.0, name.0, generic_parameter.0, constraint.0, equal.0, type_.0, semicolon.0), self.1.make_alias_declaration(attribute_spec.1, modifiers.1, module_kw_opt.1, keyword.1, name.1, generic_parameter.1, constraint.1, equal.1, type_.1, semicolon.1))
    }

    fn make_context_alias_declaration(&mut self, attribute_spec: Self::Output, keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, as_constraint: Self::Output, equal: Self::Output, context: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_context_alias_declaration(attribute_spec.0, keyword.0, name.0, generic_parameter.0, as_constraint.0, equal.0, context.0, semicolon.0), self.1.make_context_alias_declaration(attribute_spec.1, keyword.1, name.1, generic_parameter.1, as_constraint.1, equal.1, context.1, semicolon.1))
    }

    fn make_case_type_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, case_keyword: Self::Output, type_keyword: Self::Output, name: Self::Output, generic_parameter: Self::Output, as_: Self::Output, bounds: Self::Output, equal: Self::Output, variants: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_case_type_declaration(attribute_spec.0, modifiers.0, case_keyword.0, type_keyword.0, name.0, generic_parameter.0, as_.0, bounds.0, equal.0, variants.0, semicolon.0), self.1.make_case_type_declaration(attribute_spec.1, modifiers.1, case_keyword.1, type_keyword.1, name.1, generic_parameter.1, as_.1, bounds.1, equal.1, variants.1, semicolon.1))
    }

    fn make_case_type_variant(&mut self, bar: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_case_type_variant(bar.0, type_.0), self.1.make_case_type_variant(bar.1, type_.1))
    }

    fn make_property_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, type_: Self::Output, declarators: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_property_declaration(attribute_spec.0, modifiers.0, type_.0, declarators.0, semicolon.0), self.1.make_property_declaration(attribute_spec.1, modifiers.1, type_.1, declarators.1, semicolon.1))
    }

    fn make_property_declarator(&mut self, name: Self::Output, initializer: Self::Output) -> Self::Output {
        Node(self.0.make_property_declarator(name.0, initializer.0), self.1.make_property_declarator(name.1, initializer.1))
    }

    fn make_namespace_declaration(&mut self, header: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_declaration(header.0, body.0), self.1.make_namespace_declaration(header.1, body.1))
    }

    fn make_namespace_declaration_header(&mut self, keyword: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_declaration_header(keyword.0, name.0), self.1.make_namespace_declaration_header(keyword.1, name.1))
    }

    fn make_namespace_body(&mut self, left_brace: Self::Output, declarations: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_body(left_brace.0, declarations.0, right_brace.0), self.1.make_namespace_body(left_brace.1, declarations.1, right_brace.1))
    }

    fn make_namespace_empty_body(&mut self, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_empty_body(semicolon.0), self.1.make_namespace_empty_body(semicolon.1))
    }

    fn make_namespace_use_declaration(&mut self, keyword: Self::Output, kind: Self::Output, clauses: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_use_declaration(keyword.0, kind.0, clauses.0, semicolon.0), self.1.make_namespace_use_declaration(keyword.1, kind.1, clauses.1, semicolon.1))
    }

    fn make_namespace_group_use_declaration(&mut self, keyword: Self::Output, kind: Self::Output, prefix: Self::Output, left_brace: Self::Output, clauses: Self::Output, right_brace: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_group_use_declaration(keyword.0, kind.0, prefix.0, left_brace.0, clauses.0, right_brace.0, semicolon.0), self.1.make_namespace_group_use_declaration(keyword.1, kind.1, prefix.1, left_brace.1, clauses.1, right_brace.1, semicolon.1))
    }

    fn make_namespace_use_clause(&mut self, clause_kind: Self::Output, name: Self::Output, as_: Self::Output, alias: Self::Output) -> Self::Output {
        Node(self.0.make_namespace_use_clause(clause_kind.0, name.0, as_.0, alias.0), self.1.make_namespace_use_clause(clause_kind.1, name.1, as_.1, alias.1))
    }

    fn make_function_declaration(&mut self, attribute_spec: Self::Output, declaration_header: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_function_declaration(attribute_spec.0, declaration_header.0, body.0), self.1.make_function_declaration(attribute_spec.1, declaration_header.1, body.1))
    }

    fn make_function_declaration_header(&mut self, modifiers: Self::Output, keyword: Self::Output, name: Self::Output, type_parameter_list: Self::Output, left_paren: Self::Output, parameter_list: Self::Output, right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output, where_clause: Self::Output) -> Self::Output {
        Node(self.0.make_function_declaration_header(modifiers.0, keyword.0, name.0, type_parameter_list.0, left_paren.0, parameter_list.0, right_paren.0, contexts.0, colon.0, readonly_return.0, type_.0, where_clause.0), self.1.make_function_declaration_header(modifiers.1, keyword.1, name.1, type_parameter_list.1, left_paren.1, parameter_list.1, right_paren.1, contexts.1, colon.1, readonly_return.1, type_.1, where_clause.1))
    }

    fn make_contexts(&mut self, left_bracket: Self::Output, types: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_contexts(left_bracket.0, types.0, right_bracket.0), self.1.make_contexts(left_bracket.1, types.1, right_bracket.1))
    }

    fn make_where_clause(&mut self, keyword: Self::Output, constraints: Self::Output) -> Self::Output {
        Node(self.0.make_where_clause(keyword.0, constraints.0), self.1.make_where_clause(keyword.1, constraints.1))
    }

    fn make_where_constraint(&mut self, left_type: Self::Output, operator: Self::Output, right_type: Self::Output) -> Self::Output {
        Node(self.0.make_where_constraint(left_type.0, operator.0, right_type.0), self.1.make_where_constraint(left_type.1, operator.1, right_type.1))
    }

    fn make_methodish_declaration(&mut self, attribute: Self::Output, function_decl_header: Self::Output, function_body: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_methodish_declaration(attribute.0, function_decl_header.0, function_body.0, semicolon.0), self.1.make_methodish_declaration(attribute.1, function_decl_header.1, function_body.1, semicolon.1))
    }

    fn make_methodish_trait_resolution(&mut self, attribute: Self::Output, function_decl_header: Self::Output, equal: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_methodish_trait_resolution(attribute.0, function_decl_header.0, equal.0, name.0, semicolon.0), self.1.make_methodish_trait_resolution(attribute.1, function_decl_header.1, equal.1, name.1, semicolon.1))
    }

    fn make_classish_declaration(&mut self, attribute: Self::Output, modifiers: Self::Output, xhp: Self::Output, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, extends_keyword: Self::Output, extends_list: Self::Output, implements_keyword: Self::Output, implements_list: Self::Output, where_clause: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_classish_declaration(attribute.0, modifiers.0, xhp.0, keyword.0, name.0, type_parameters.0, extends_keyword.0, extends_list.0, implements_keyword.0, implements_list.0, where_clause.0, body.0), self.1.make_classish_declaration(attribute.1, modifiers.1, xhp.1, keyword.1, name.1, type_parameters.1, extends_keyword.1, extends_list.1, implements_keyword.1, implements_list.1, where_clause.1, body.1))
    }

    fn make_classish_body(&mut self, left_brace: Self::Output, elements: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_classish_body(left_brace.0, elements.0, right_brace.0), self.1.make_classish_body(left_brace.1, elements.1, right_brace.1))
    }

    fn make_trait_use(&mut self, keyword: Self::Output, names: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_trait_use(keyword.0, names.0, semicolon.0), self.1.make_trait_use(keyword.1, names.1, semicolon.1))
    }

    fn make_require_clause(&mut self, keyword: Self::Output, kind: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_require_clause(keyword.0, kind.0, name.0, semicolon.0), self.1.make_require_clause(keyword.1, kind.1, name.1, semicolon.1))
    }

    fn make_const_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, type_specifier: Self::Output, declarators: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_const_declaration(attribute_spec.0, modifiers.0, keyword.0, type_specifier.0, declarators.0, semicolon.0), self.1.make_const_declaration(attribute_spec.1, modifiers.1, keyword.1, type_specifier.1, declarators.1, semicolon.1))
    }

    fn make_constant_declarator(&mut self, name: Self::Output, initializer: Self::Output) -> Self::Output {
        Node(self.0.make_constant_declarator(name.0, initializer.0), self.1.make_constant_declarator(name.1, initializer.1))
    }

    fn make_type_const_declaration(&mut self, attribute_spec: Self::Output, modifiers: Self::Output, keyword: Self::Output, type_keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, type_constraints: Self::Output, equal: Self::Output, type_specifier: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_type_const_declaration(attribute_spec.0, modifiers.0, keyword.0, type_keyword.0, name.0, type_parameters.0, type_constraints.0, equal.0, type_specifier.0, semicolon.0), self.1.make_type_const_declaration(attribute_spec.1, modifiers.1, keyword.1, type_keyword.1, name.1, type_parameters.1, type_constraints.1, equal.1, type_specifier.1, semicolon.1))
    }

    fn make_context_const_declaration(&mut self, modifiers: Self::Output, const_keyword: Self::Output, ctx_keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraint: Self::Output, equal: Self::Output, ctx_list: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_context_const_declaration(modifiers.0, const_keyword.0, ctx_keyword.0, name.0, type_parameters.0, constraint.0, equal.0, ctx_list.0, semicolon.0), self.1.make_context_const_declaration(modifiers.1, const_keyword.1, ctx_keyword.1, name.1, type_parameters.1, constraint.1, equal.1, ctx_list.1, semicolon.1))
    }

    fn make_decorated_expression(&mut self, decorator: Self::Output, expression: Self::Output) -> Self::Output {
        Node(self.0.make_decorated_expression(decorator.0, expression.0), self.1.make_decorated_expression(decorator.1, expression.1))
    }

    fn make_parameter_declaration(&mut self, attribute: Self::Output, visibility: Self::Output, call_convention: Self::Output, readonly: Self::Output, type_: Self::Output, name: Self::Output, default_value: Self::Output, parameter_end: Self::Output) -> Self::Output {
        Node(self.0.make_parameter_declaration(attribute.0, visibility.0, call_convention.0, readonly.0, type_.0, name.0, default_value.0, parameter_end.0), self.1.make_parameter_declaration(attribute.1, visibility.1, call_convention.1, readonly.1, type_.1, name.1, default_value.1, parameter_end.1))
    }

    fn make_variadic_parameter(&mut self, call_convention: Self::Output, type_: Self::Output, ellipsis: Self::Output) -> Self::Output {
        Node(self.0.make_variadic_parameter(call_convention.0, type_.0, ellipsis.0), self.1.make_variadic_parameter(call_convention.1, type_.1, ellipsis.1))
    }

    fn make_old_attribute_specification(&mut self, left_double_angle: Self::Output, attributes: Self::Output, right_double_angle: Self::Output) -> Self::Output {
        Node(self.0.make_old_attribute_specification(left_double_angle.0, attributes.0, right_double_angle.0), self.1.make_old_attribute_specification(left_double_angle.1, attributes.1, right_double_angle.1))
    }

    fn make_attribute_specification(&mut self, attributes: Self::Output) -> Self::Output {
        Node(self.0.make_attribute_specification(attributes.0), self.1.make_attribute_specification(attributes.1))
    }

    fn make_attribute(&mut self, at: Self::Output, attribute_name: Self::Output) -> Self::Output {
        Node(self.0.make_attribute(at.0, attribute_name.0), self.1.make_attribute(at.1, attribute_name.1))
    }

    fn make_inclusion_expression(&mut self, require: Self::Output, filename: Self::Output) -> Self::Output {
        Node(self.0.make_inclusion_expression(require.0, filename.0), self.1.make_inclusion_expression(require.1, filename.1))
    }

    fn make_inclusion_directive(&mut self, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_inclusion_directive(expression.0, semicolon.0), self.1.make_inclusion_directive(expression.1, semicolon.1))
    }

    fn make_compound_statement(&mut self, left_brace: Self::Output, statements: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_compound_statement(left_brace.0, statements.0, right_brace.0), self.1.make_compound_statement(left_brace.1, statements.1, right_brace.1))
    }

    fn make_expression_statement(&mut self, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_expression_statement(expression.0, semicolon.0), self.1.make_expression_statement(expression.1, semicolon.1))
    }

    fn make_markup_section(&mut self, hashbang: Self::Output, suffix: Self::Output) -> Self::Output {
        Node(self.0.make_markup_section(hashbang.0, suffix.0), self.1.make_markup_section(hashbang.1, suffix.1))
    }

    fn make_markup_suffix(&mut self, less_than_question: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_markup_suffix(less_than_question.0, name.0), self.1.make_markup_suffix(less_than_question.1, name.1))
    }

    fn make_unset_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, variables: Self::Output, right_paren: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_unset_statement(keyword.0, left_paren.0, variables.0, right_paren.0, semicolon.0), self.1.make_unset_statement(keyword.1, left_paren.1, variables.1, right_paren.1, semicolon.1))
    }

    fn make_declare_local_statement(&mut self, keyword: Self::Output, variable: Self::Output, colon: Self::Output, type_: Self::Output, initializer: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_declare_local_statement(keyword.0, variable.0, colon.0, type_.0, initializer.0, semicolon.0), self.1.make_declare_local_statement(keyword.1, variable.1, colon.1, type_.1, initializer.1, semicolon.1))
    }

    fn make_using_statement_block_scoped(&mut self, await_keyword: Self::Output, using_keyword: Self::Output, left_paren: Self::Output, expressions: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_using_statement_block_scoped(await_keyword.0, using_keyword.0, left_paren.0, expressions.0, right_paren.0, body.0), self.1.make_using_statement_block_scoped(await_keyword.1, using_keyword.1, left_paren.1, expressions.1, right_paren.1, body.1))
    }

    fn make_using_statement_function_scoped(&mut self, await_keyword: Self::Output, using_keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_using_statement_function_scoped(await_keyword.0, using_keyword.0, expression.0, semicolon.0), self.1.make_using_statement_function_scoped(await_keyword.1, using_keyword.1, expression.1, semicolon.1))
    }

    fn make_while_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_while_statement(keyword.0, left_paren.0, condition.0, right_paren.0, body.0), self.1.make_while_statement(keyword.1, left_paren.1, condition.1, right_paren.1, body.1))
    }

    fn make_if_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, statement: Self::Output, else_clause: Self::Output) -> Self::Output {
        Node(self.0.make_if_statement(keyword.0, left_paren.0, condition.0, right_paren.0, statement.0, else_clause.0), self.1.make_if_statement(keyword.1, left_paren.1, condition.1, right_paren.1, statement.1, else_clause.1))
    }

    fn make_else_clause(&mut self, keyword: Self::Output, statement: Self::Output) -> Self::Output {
        Node(self.0.make_else_clause(keyword.0, statement.0), self.1.make_else_clause(keyword.1, statement.1))
    }

    fn make_try_statement(&mut self, keyword: Self::Output, compound_statement: Self::Output, catch_clauses: Self::Output, finally_clause: Self::Output) -> Self::Output {
        Node(self.0.make_try_statement(keyword.0, compound_statement.0, catch_clauses.0, finally_clause.0), self.1.make_try_statement(keyword.1, compound_statement.1, catch_clauses.1, finally_clause.1))
    }

    fn make_catch_clause(&mut self, keyword: Self::Output, left_paren: Self::Output, type_: Self::Output, variable: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_catch_clause(keyword.0, left_paren.0, type_.0, variable.0, right_paren.0, body.0), self.1.make_catch_clause(keyword.1, left_paren.1, type_.1, variable.1, right_paren.1, body.1))
    }

    fn make_finally_clause(&mut self, keyword: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_finally_clause(keyword.0, body.0), self.1.make_finally_clause(keyword.1, body.1))
    }

    fn make_do_statement(&mut self, keyword: Self::Output, body: Self::Output, while_keyword: Self::Output, left_paren: Self::Output, condition: Self::Output, right_paren: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_do_statement(keyword.0, body.0, while_keyword.0, left_paren.0, condition.0, right_paren.0, semicolon.0), self.1.make_do_statement(keyword.1, body.1, while_keyword.1, left_paren.1, condition.1, right_paren.1, semicolon.1))
    }

    fn make_for_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, initializer: Self::Output, first_semicolon: Self::Output, control: Self::Output, second_semicolon: Self::Output, end_of_loop: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_for_statement(keyword.0, left_paren.0, initializer.0, first_semicolon.0, control.0, second_semicolon.0, end_of_loop.0, right_paren.0, body.0), self.1.make_for_statement(keyword.1, left_paren.1, initializer.1, first_semicolon.1, control.1, second_semicolon.1, end_of_loop.1, right_paren.1, body.1))
    }

    fn make_foreach_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, collection: Self::Output, await_keyword: Self::Output, as_: Self::Output, key: Self::Output, arrow: Self::Output, value: Self::Output, right_paren: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_foreach_statement(keyword.0, left_paren.0, collection.0, await_keyword.0, as_.0, key.0, arrow.0, value.0, right_paren.0, body.0), self.1.make_foreach_statement(keyword.1, left_paren.1, collection.1, await_keyword.1, as_.1, key.1, arrow.1, value.1, right_paren.1, body.1))
    }

    fn make_switch_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output, left_brace: Self::Output, sections: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_switch_statement(keyword.0, left_paren.0, expression.0, right_paren.0, left_brace.0, sections.0, right_brace.0), self.1.make_switch_statement(keyword.1, left_paren.1, expression.1, right_paren.1, left_brace.1, sections.1, right_brace.1))
    }

    fn make_switch_section(&mut self, labels: Self::Output, statements: Self::Output, fallthrough: Self::Output) -> Self::Output {
        Node(self.0.make_switch_section(labels.0, statements.0, fallthrough.0), self.1.make_switch_section(labels.1, statements.1, fallthrough.1))
    }

    fn make_switch_fallthrough(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_switch_fallthrough(keyword.0, semicolon.0), self.1.make_switch_fallthrough(keyword.1, semicolon.1))
    }

    fn make_case_label(&mut self, keyword: Self::Output, expression: Self::Output, colon: Self::Output) -> Self::Output {
        Node(self.0.make_case_label(keyword.0, expression.0, colon.0), self.1.make_case_label(keyword.1, expression.1, colon.1))
    }

    fn make_default_label(&mut self, keyword: Self::Output, colon: Self::Output) -> Self::Output {
        Node(self.0.make_default_label(keyword.0, colon.0), self.1.make_default_label(keyword.1, colon.1))
    }

    fn make_match_statement(&mut self, keyword: Self::Output, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output, left_brace: Self::Output, arms: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_match_statement(keyword.0, left_paren.0, expression.0, right_paren.0, left_brace.0, arms.0, right_brace.0), self.1.make_match_statement(keyword.1, left_paren.1, expression.1, right_paren.1, left_brace.1, arms.1, right_brace.1))
    }

    fn make_match_statement_arm(&mut self, pattern: Self::Output, arrow: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_match_statement_arm(pattern.0, arrow.0, body.0), self.1.make_match_statement_arm(pattern.1, arrow.1, body.1))
    }

    fn make_return_statement(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_return_statement(keyword.0, expression.0, semicolon.0), self.1.make_return_statement(keyword.1, expression.1, semicolon.1))
    }

    fn make_yield_break_statement(&mut self, keyword: Self::Output, break_: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_yield_break_statement(keyword.0, break_.0, semicolon.0), self.1.make_yield_break_statement(keyword.1, break_.1, semicolon.1))
    }

    fn make_throw_statement(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_throw_statement(keyword.0, expression.0, semicolon.0), self.1.make_throw_statement(keyword.1, expression.1, semicolon.1))
    }

    fn make_break_statement(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_break_statement(keyword.0, semicolon.0), self.1.make_break_statement(keyword.1, semicolon.1))
    }

    fn make_continue_statement(&mut self, keyword: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_continue_statement(keyword.0, semicolon.0), self.1.make_continue_statement(keyword.1, semicolon.1))
    }

    fn make_echo_statement(&mut self, keyword: Self::Output, expressions: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_echo_statement(keyword.0, expressions.0, semicolon.0), self.1.make_echo_statement(keyword.1, expressions.1, semicolon.1))
    }

    fn make_concurrent_statement(&mut self, keyword: Self::Output, statement: Self::Output) -> Self::Output {
        Node(self.0.make_concurrent_statement(keyword.0, statement.0), self.1.make_concurrent_statement(keyword.1, statement.1))
    }

    fn make_simple_initializer(&mut self, equal: Self::Output, value: Self::Output) -> Self::Output {
        Node(self.0.make_simple_initializer(equal.0, value.0), self.1.make_simple_initializer(equal.1, value.1))
    }

    fn make_anonymous_class(&mut self, class_keyword: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output, extends_keyword: Self::Output, extends_list: Self::Output, implements_keyword: Self::Output, implements_list: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_anonymous_class(class_keyword.0, left_paren.0, argument_list.0, right_paren.0, extends_keyword.0, extends_list.0, implements_keyword.0, implements_list.0, body.0), self.1.make_anonymous_class(class_keyword.1, left_paren.1, argument_list.1, right_paren.1, extends_keyword.1, extends_list.1, implements_keyword.1, implements_list.1, body.1))
    }

    fn make_anonymous_function(&mut self, attribute_spec: Self::Output, async_keyword: Self::Output, function_keyword: Self::Output, left_paren: Self::Output, parameters: Self::Output, right_paren: Self::Output, ctx_list: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output, use_: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_anonymous_function(attribute_spec.0, async_keyword.0, function_keyword.0, left_paren.0, parameters.0, right_paren.0, ctx_list.0, colon.0, readonly_return.0, type_.0, use_.0, body.0), self.1.make_anonymous_function(attribute_spec.1, async_keyword.1, function_keyword.1, left_paren.1, parameters.1, right_paren.1, ctx_list.1, colon.1, readonly_return.1, type_.1, use_.1, body.1))
    }

    fn make_anonymous_function_use_clause(&mut self, keyword: Self::Output, left_paren: Self::Output, variables: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_anonymous_function_use_clause(keyword.0, left_paren.0, variables.0, right_paren.0), self.1.make_anonymous_function_use_clause(keyword.1, left_paren.1, variables.1, right_paren.1))
    }

    fn make_variable_pattern(&mut self, variable: Self::Output) -> Self::Output {
        Node(self.0.make_variable_pattern(variable.0), self.1.make_variable_pattern(variable.1))
    }

    fn make_constructor_pattern(&mut self, constructor: Self::Output, left_paren: Self::Output, members: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_constructor_pattern(constructor.0, left_paren.0, members.0, right_paren.0), self.1.make_constructor_pattern(constructor.1, left_paren.1, members.1, right_paren.1))
    }

    fn make_refinement_pattern(&mut self, variable: Self::Output, colon: Self::Output, specifier: Self::Output) -> Self::Output {
        Node(self.0.make_refinement_pattern(variable.0, colon.0, specifier.0), self.1.make_refinement_pattern(variable.1, colon.1, specifier.1))
    }

    fn make_lambda_expression(&mut self, attribute_spec: Self::Output, async_: Self::Output, signature: Self::Output, arrow: Self::Output, body: Self::Output) -> Self::Output {
        Node(self.0.make_lambda_expression(attribute_spec.0, async_.0, signature.0, arrow.0, body.0), self.1.make_lambda_expression(attribute_spec.1, async_.1, signature.1, arrow.1, body.1))
    }

    fn make_lambda_signature(&mut self, left_paren: Self::Output, parameters: Self::Output, right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_lambda_signature(left_paren.0, parameters.0, right_paren.0, contexts.0, colon.0, readonly_return.0, type_.0), self.1.make_lambda_signature(left_paren.1, parameters.1, right_paren.1, contexts.1, colon.1, readonly_return.1, type_.1))
    }

    fn make_cast_expression(&mut self, left_paren: Self::Output, type_: Self::Output, right_paren: Self::Output, operand: Self::Output) -> Self::Output {
        Node(self.0.make_cast_expression(left_paren.0, type_.0, right_paren.0, operand.0), self.1.make_cast_expression(left_paren.1, type_.1, right_paren.1, operand.1))
    }

    fn make_scope_resolution_expression(&mut self, qualifier: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_scope_resolution_expression(qualifier.0, operator.0, name.0), self.1.make_scope_resolution_expression(qualifier.1, operator.1, name.1))
    }

    fn make_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_member_selection_expression(object.0, operator.0, name.0), self.1.make_member_selection_expression(object.1, operator.1, name.1))
    }

    fn make_safe_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_safe_member_selection_expression(object.0, operator.0, name.0), self.1.make_safe_member_selection_expression(object.1, operator.1, name.1))
    }

    fn make_embedded_member_selection_expression(&mut self, object: Self::Output, operator: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_embedded_member_selection_expression(object.0, operator.0, name.0), self.1.make_embedded_member_selection_expression(object.1, operator.1, name.1))
    }

    fn make_yield_expression(&mut self, keyword: Self::Output, operand: Self::Output) -> Self::Output {
        Node(self.0.make_yield_expression(keyword.0, operand.0), self.1.make_yield_expression(keyword.1, operand.1))
    }

    fn make_prefix_unary_expression(&mut self, operator: Self::Output, operand: Self::Output) -> Self::Output {
        Node(self.0.make_prefix_unary_expression(operator.0, operand.0), self.1.make_prefix_unary_expression(operator.1, operand.1))
    }

    fn make_postfix_unary_expression(&mut self, operand: Self::Output, operator: Self::Output) -> Self::Output {
        Node(self.0.make_postfix_unary_expression(operand.0, operator.0), self.1.make_postfix_unary_expression(operand.1, operator.1))
    }

    fn make_binary_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        Node(self.0.make_binary_expression(left_operand.0, operator.0, right_operand.0), self.1.make_binary_expression(left_operand.1, operator.1, right_operand.1))
    }

    fn make_is_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        Node(self.0.make_is_expression(left_operand.0, operator.0, right_operand.0), self.1.make_is_expression(left_operand.1, operator.1, right_operand.1))
    }

    fn make_as_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        Node(self.0.make_as_expression(left_operand.0, operator.0, right_operand.0), self.1.make_as_expression(left_operand.1, operator.1, right_operand.1))
    }

    fn make_nullable_as_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        Node(self.0.make_nullable_as_expression(left_operand.0, operator.0, right_operand.0), self.1.make_nullable_as_expression(left_operand.1, operator.1, right_operand.1))
    }

    fn make_upcast_expression(&mut self, left_operand: Self::Output, operator: Self::Output, right_operand: Self::Output) -> Self::Output {
        Node(self.0.make_upcast_expression(left_operand.0, operator.0, right_operand.0), self.1.make_upcast_expression(left_operand.1, operator.1, right_operand.1))
    }

    fn make_conditional_expression(&mut self, test: Self::Output, question: Self::Output, consequence: Self::Output, colon: Self::Output, alternative: Self::Output) -> Self::Output {
        Node(self.0.make_conditional_expression(test.0, question.0, consequence.0, colon.0, alternative.0), self.1.make_conditional_expression(test.1, question.1, consequence.1, colon.1, alternative.1))
    }

    fn make_eval_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, argument: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_eval_expression(keyword.0, left_paren.0, argument.0, right_paren.0), self.1.make_eval_expression(keyword.1, left_paren.1, argument.1, right_paren.1))
    }

    fn make_isset_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_isset_expression(keyword.0, left_paren.0, argument_list.0, right_paren.0), self.1.make_isset_expression(keyword.1, left_paren.1, argument_list.1, right_paren.1))
    }

    fn make_nameof_expression(&mut self, keyword: Self::Output, target: Self::Output) -> Self::Output {
        Node(self.0.make_nameof_expression(keyword.0, target.0), self.1.make_nameof_expression(keyword.1, target.1))
    }

    fn make_function_call_expression(&mut self, receiver: Self::Output, type_args: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_function_call_expression(receiver.0, type_args.0, left_paren.0, argument_list.0, right_paren.0), self.1.make_function_call_expression(receiver.1, type_args.1, left_paren.1, argument_list.1, right_paren.1))
    }

    fn make_function_pointer_expression(&mut self, receiver: Self::Output, type_args: Self::Output) -> Self::Output {
        Node(self.0.make_function_pointer_expression(receiver.0, type_args.0), self.1.make_function_pointer_expression(receiver.1, type_args.1))
    }

    fn make_parenthesized_expression(&mut self, left_paren: Self::Output, expression: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_parenthesized_expression(left_paren.0, expression.0, right_paren.0), self.1.make_parenthesized_expression(left_paren.1, expression.1, right_paren.1))
    }

    fn make_braced_expression(&mut self, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_braced_expression(left_brace.0, expression.0, right_brace.0), self.1.make_braced_expression(left_brace.1, expression.1, right_brace.1))
    }

    fn make_et_splice_expression(&mut self, dollar: Self::Output, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_et_splice_expression(dollar.0, left_brace.0, expression.0, right_brace.0), self.1.make_et_splice_expression(dollar.1, left_brace.1, expression.1, right_brace.1))
    }

    fn make_embedded_braced_expression(&mut self, left_brace: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_embedded_braced_expression(left_brace.0, expression.0, right_brace.0), self.1.make_embedded_braced_expression(left_brace.1, expression.1, right_brace.1))
    }

    fn make_list_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, members: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_list_expression(keyword.0, left_paren.0, members.0, right_paren.0), self.1.make_list_expression(keyword.1, left_paren.1, members.1, right_paren.1))
    }

    fn make_collection_literal_expression(&mut self, name: Self::Output, left_brace: Self::Output, initializers: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_collection_literal_expression(name.0, left_brace.0, initializers.0, right_brace.0), self.1.make_collection_literal_expression(name.1, left_brace.1, initializers.1, right_brace.1))
    }

    fn make_object_creation_expression(&mut self, new_keyword: Self::Output, object: Self::Output) -> Self::Output {
        Node(self.0.make_object_creation_expression(new_keyword.0, object.0), self.1.make_object_creation_expression(new_keyword.1, object.1))
    }

    fn make_constructor_call(&mut self, type_: Self::Output, left_paren: Self::Output, argument_list: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_constructor_call(type_.0, left_paren.0, argument_list.0, right_paren.0), self.1.make_constructor_call(type_.1, left_paren.1, argument_list.1, right_paren.1))
    }

    fn make_darray_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_darray_intrinsic_expression(keyword.0, explicit_type.0, left_bracket.0, members.0, right_bracket.0), self.1.make_darray_intrinsic_expression(keyword.1, explicit_type.1, left_bracket.1, members.1, right_bracket.1))
    }

    fn make_dictionary_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_dictionary_intrinsic_expression(keyword.0, explicit_type.0, left_bracket.0, members.0, right_bracket.0), self.1.make_dictionary_intrinsic_expression(keyword.1, explicit_type.1, left_bracket.1, members.1, right_bracket.1))
    }

    fn make_keyset_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_keyset_intrinsic_expression(keyword.0, explicit_type.0, left_bracket.0, members.0, right_bracket.0), self.1.make_keyset_intrinsic_expression(keyword.1, explicit_type.1, left_bracket.1, members.1, right_bracket.1))
    }

    fn make_varray_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_varray_intrinsic_expression(keyword.0, explicit_type.0, left_bracket.0, members.0, right_bracket.0), self.1.make_varray_intrinsic_expression(keyword.1, explicit_type.1, left_bracket.1, members.1, right_bracket.1))
    }

    fn make_vector_intrinsic_expression(&mut self, keyword: Self::Output, explicit_type: Self::Output, left_bracket: Self::Output, members: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_vector_intrinsic_expression(keyword.0, explicit_type.0, left_bracket.0, members.0, right_bracket.0), self.1.make_vector_intrinsic_expression(keyword.1, explicit_type.1, left_bracket.1, members.1, right_bracket.1))
    }

    fn make_element_initializer(&mut self, key: Self::Output, arrow: Self::Output, value: Self::Output) -> Self::Output {
        Node(self.0.make_element_initializer(key.0, arrow.0, value.0), self.1.make_element_initializer(key.1, arrow.1, value.1))
    }

    fn make_subscript_expression(&mut self, receiver: Self::Output, left_bracket: Self::Output, index: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_subscript_expression(receiver.0, left_bracket.0, index.0, right_bracket.0), self.1.make_subscript_expression(receiver.1, left_bracket.1, index.1, right_bracket.1))
    }

    fn make_embedded_subscript_expression(&mut self, receiver: Self::Output, left_bracket: Self::Output, index: Self::Output, right_bracket: Self::Output) -> Self::Output {
        Node(self.0.make_embedded_subscript_expression(receiver.0, left_bracket.0, index.0, right_bracket.0), self.1.make_embedded_subscript_expression(receiver.1, left_bracket.1, index.1, right_bracket.1))
    }

    fn make_awaitable_creation_expression(&mut self, attribute_spec: Self::Output, async_: Self::Output, compound_statement: Self::Output) -> Self::Output {
        Node(self.0.make_awaitable_creation_expression(attribute_spec.0, async_.0, compound_statement.0), self.1.make_awaitable_creation_expression(attribute_spec.1, async_.1, compound_statement.1))
    }

    fn make_xhp_children_declaration(&mut self, keyword: Self::Output, expression: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_children_declaration(keyword.0, expression.0, semicolon.0), self.1.make_xhp_children_declaration(keyword.1, expression.1, semicolon.1))
    }

    fn make_xhp_children_parenthesized_list(&mut self, left_paren: Self::Output, xhp_children: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_children_parenthesized_list(left_paren.0, xhp_children.0, right_paren.0), self.1.make_xhp_children_parenthesized_list(left_paren.1, xhp_children.1, right_paren.1))
    }

    fn make_xhp_category_declaration(&mut self, keyword: Self::Output, categories: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_category_declaration(keyword.0, categories.0, semicolon.0), self.1.make_xhp_category_declaration(keyword.1, categories.1, semicolon.1))
    }

    fn make_xhp_enum_type(&mut self, like: Self::Output, keyword: Self::Output, left_brace: Self::Output, values: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_enum_type(like.0, keyword.0, left_brace.0, values.0, right_brace.0), self.1.make_xhp_enum_type(like.1, keyword.1, left_brace.1, values.1, right_brace.1))
    }

    fn make_xhp_lateinit(&mut self, at: Self::Output, keyword: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_lateinit(at.0, keyword.0), self.1.make_xhp_lateinit(at.1, keyword.1))
    }

    fn make_xhp_required(&mut self, at: Self::Output, keyword: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_required(at.0, keyword.0), self.1.make_xhp_required(at.1, keyword.1))
    }

    fn make_xhp_class_attribute_declaration(&mut self, keyword: Self::Output, attributes: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_class_attribute_declaration(keyword.0, attributes.0, semicolon.0), self.1.make_xhp_class_attribute_declaration(keyword.1, attributes.1, semicolon.1))
    }

    fn make_xhp_class_attribute(&mut self, type_: Self::Output, name: Self::Output, initializer: Self::Output, required: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_class_attribute(type_.0, name.0, initializer.0, required.0), self.1.make_xhp_class_attribute(type_.1, name.1, initializer.1, required.1))
    }

    fn make_xhp_simple_class_attribute(&mut self, type_: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_simple_class_attribute(type_.0), self.1.make_xhp_simple_class_attribute(type_.1))
    }

    fn make_xhp_simple_attribute(&mut self, name: Self::Output, equal: Self::Output, expression: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_simple_attribute(name.0, equal.0, expression.0), self.1.make_xhp_simple_attribute(name.1, equal.1, expression.1))
    }

    fn make_xhp_spread_attribute(&mut self, left_brace: Self::Output, spread_operator: Self::Output, expression: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_spread_attribute(left_brace.0, spread_operator.0, expression.0, right_brace.0), self.1.make_xhp_spread_attribute(left_brace.1, spread_operator.1, expression.1, right_brace.1))
    }

    fn make_xhp_open(&mut self, left_angle: Self::Output, name: Self::Output, attributes: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_open(left_angle.0, name.0, attributes.0, right_angle.0), self.1.make_xhp_open(left_angle.1, name.1, attributes.1, right_angle.1))
    }

    fn make_xhp_expression(&mut self, open: Self::Output, body: Self::Output, close: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_expression(open.0, body.0, close.0), self.1.make_xhp_expression(open.1, body.1, close.1))
    }

    fn make_xhp_close(&mut self, left_angle: Self::Output, name: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_xhp_close(left_angle.0, name.0, right_angle.0), self.1.make_xhp_close(left_angle.1, name.1, right_angle.1))
    }

    fn make_type_constant(&mut self, left_type: Self::Output, separator: Self::Output, right_type: Self::Output) -> Self::Output {
        Node(self.0.make_type_constant(left_type.0, separator.0, right_type.0), self.1.make_type_constant(left_type.1, separator.1, right_type.1))
    }

    fn make_vector_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_vector_type_specifier(keyword.0, left_angle.0, type_.0, trailing_comma.0, right_angle.0), self.1.make_vector_type_specifier(keyword.1, left_angle.1, type_.1, trailing_comma.1, right_angle.1))
    }

    fn make_keyset_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_keyset_type_specifier(keyword.0, left_angle.0, type_.0, trailing_comma.0, right_angle.0), self.1.make_keyset_type_specifier(keyword.1, left_angle.1, type_.1, trailing_comma.1, right_angle.1))
    }

    fn make_tuple_type_explicit_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, types: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_tuple_type_explicit_specifier(keyword.0, left_angle.0, types.0, right_angle.0), self.1.make_tuple_type_explicit_specifier(keyword.1, left_angle.1, types.1, right_angle.1))
    }

    fn make_varray_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_varray_type_specifier(keyword.0, left_angle.0, type_.0, trailing_comma.0, right_angle.0), self.1.make_varray_type_specifier(keyword.1, left_angle.1, type_.1, trailing_comma.1, right_angle.1))
    }

    fn make_function_ctx_type_specifier(&mut self, keyword: Self::Output, variable: Self::Output) -> Self::Output {
        Node(self.0.make_function_ctx_type_specifier(keyword.0, variable.0), self.1.make_function_ctx_type_specifier(keyword.1, variable.1))
    }

    fn make_type_parameter(&mut self, attribute_spec: Self::Output, reified: Self::Output, variance: Self::Output, name: Self::Output, param_params: Self::Output, constraints: Self::Output) -> Self::Output {
        Node(self.0.make_type_parameter(attribute_spec.0, reified.0, variance.0, name.0, param_params.0, constraints.0), self.1.make_type_parameter(attribute_spec.1, reified.1, variance.1, name.1, param_params.1, constraints.1))
    }

    fn make_type_constraint(&mut self, keyword: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_type_constraint(keyword.0, type_.0), self.1.make_type_constraint(keyword.1, type_.1))
    }

    fn make_context_constraint(&mut self, keyword: Self::Output, ctx_list: Self::Output) -> Self::Output {
        Node(self.0.make_context_constraint(keyword.0, ctx_list.0), self.1.make_context_constraint(keyword.1, ctx_list.1))
    }

    fn make_darray_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, key: Self::Output, comma: Self::Output, value: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_darray_type_specifier(keyword.0, left_angle.0, key.0, comma.0, value.0, trailing_comma.0, right_angle.0), self.1.make_darray_type_specifier(keyword.1, left_angle.1, key.1, comma.1, value.1, trailing_comma.1, right_angle.1))
    }

    fn make_dictionary_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, members: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_dictionary_type_specifier(keyword.0, left_angle.0, members.0, right_angle.0), self.1.make_dictionary_type_specifier(keyword.1, left_angle.1, members.1, right_angle.1))
    }

    fn make_closure_type_specifier(&mut self, outer_left_paren: Self::Output, readonly_keyword: Self::Output, function_keyword: Self::Output, inner_left_paren: Self::Output, parameter_list: Self::Output, inner_right_paren: Self::Output, contexts: Self::Output, colon: Self::Output, readonly_return: Self::Output, return_type: Self::Output, outer_right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_closure_type_specifier(outer_left_paren.0, readonly_keyword.0, function_keyword.0, inner_left_paren.0, parameter_list.0, inner_right_paren.0, contexts.0, colon.0, readonly_return.0, return_type.0, outer_right_paren.0), self.1.make_closure_type_specifier(outer_left_paren.1, readonly_keyword.1, function_keyword.1, inner_left_paren.1, parameter_list.1, inner_right_paren.1, contexts.1, colon.1, readonly_return.1, return_type.1, outer_right_paren.1))
    }

    fn make_closure_parameter_type_specifier(&mut self, call_convention: Self::Output, readonly: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_closure_parameter_type_specifier(call_convention.0, readonly.0, type_.0), self.1.make_closure_parameter_type_specifier(call_convention.1, readonly.1, type_.1))
    }

    fn make_type_refinement(&mut self, type_: Self::Output, keyword: Self::Output, left_brace: Self::Output, members: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_type_refinement(type_.0, keyword.0, left_brace.0, members.0, right_brace.0), self.1.make_type_refinement(type_.1, keyword.1, left_brace.1, members.1, right_brace.1))
    }

    fn make_type_in_refinement(&mut self, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraints: Self::Output, equal: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_type_in_refinement(keyword.0, name.0, type_parameters.0, constraints.0, equal.0, type_.0), self.1.make_type_in_refinement(keyword.1, name.1, type_parameters.1, constraints.1, equal.1, type_.1))
    }

    fn make_ctx_in_refinement(&mut self, keyword: Self::Output, name: Self::Output, type_parameters: Self::Output, constraints: Self::Output, equal: Self::Output, ctx_list: Self::Output) -> Self::Output {
        Node(self.0.make_ctx_in_refinement(keyword.0, name.0, type_parameters.0, constraints.0, equal.0, ctx_list.0), self.1.make_ctx_in_refinement(keyword.1, name.1, type_parameters.1, constraints.1, equal.1, ctx_list.1))
    }

    fn make_classname_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_classname_type_specifier(keyword.0, left_angle.0, type_.0, trailing_comma.0, right_angle.0), self.1.make_classname_type_specifier(keyword.1, left_angle.1, type_.1, trailing_comma.1, right_angle.1))
    }

    fn make_class_args_type_specifier(&mut self, keyword: Self::Output, left_angle: Self::Output, type_: Self::Output, trailing_comma: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_class_args_type_specifier(keyword.0, left_angle.0, type_.0, trailing_comma.0, right_angle.0), self.1.make_class_args_type_specifier(keyword.1, left_angle.1, type_.1, trailing_comma.1, right_angle.1))
    }

    fn make_field_specifier(&mut self, question: Self::Output, name: Self::Output, arrow: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_field_specifier(question.0, name.0, arrow.0, type_.0), self.1.make_field_specifier(question.1, name.1, arrow.1, type_.1))
    }

    fn make_field_initializer(&mut self, name: Self::Output, arrow: Self::Output, value: Self::Output) -> Self::Output {
        Node(self.0.make_field_initializer(name.0, arrow.0, value.0), self.1.make_field_initializer(name.1, arrow.1, value.1))
    }

    fn make_shape_type_specifier(&mut self, keyword: Self::Output, left_paren: Self::Output, fields: Self::Output, ellipsis: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_shape_type_specifier(keyword.0, left_paren.0, fields.0, ellipsis.0, right_paren.0), self.1.make_shape_type_specifier(keyword.1, left_paren.1, fields.1, ellipsis.1, right_paren.1))
    }

    fn make_shape_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, fields: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_shape_expression(keyword.0, left_paren.0, fields.0, right_paren.0), self.1.make_shape_expression(keyword.1, left_paren.1, fields.1, right_paren.1))
    }

    fn make_tuple_expression(&mut self, keyword: Self::Output, left_paren: Self::Output, items: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_tuple_expression(keyword.0, left_paren.0, items.0, right_paren.0), self.1.make_tuple_expression(keyword.1, left_paren.1, items.1, right_paren.1))
    }

    fn make_generic_type_specifier(&mut self, class_type: Self::Output, argument_list: Self::Output) -> Self::Output {
        Node(self.0.make_generic_type_specifier(class_type.0, argument_list.0), self.1.make_generic_type_specifier(class_type.1, argument_list.1))
    }

    fn make_nullable_type_specifier(&mut self, question: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_nullable_type_specifier(question.0, type_.0), self.1.make_nullable_type_specifier(question.1, type_.1))
    }

    fn make_like_type_specifier(&mut self, tilde: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_like_type_specifier(tilde.0, type_.0), self.1.make_like_type_specifier(tilde.1, type_.1))
    }

    fn make_soft_type_specifier(&mut self, at: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_soft_type_specifier(at.0, type_.0), self.1.make_soft_type_specifier(at.1, type_.1))
    }

    fn make_attributized_specifier(&mut self, attribute_spec: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_attributized_specifier(attribute_spec.0, type_.0), self.1.make_attributized_specifier(attribute_spec.1, type_.1))
    }

    fn make_reified_type_argument(&mut self, reified: Self::Output, type_: Self::Output) -> Self::Output {
        Node(self.0.make_reified_type_argument(reified.0, type_.0), self.1.make_reified_type_argument(reified.1, type_.1))
    }

    fn make_type_arguments(&mut self, left_angle: Self::Output, types: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_type_arguments(left_angle.0, types.0, right_angle.0), self.1.make_type_arguments(left_angle.1, types.1, right_angle.1))
    }

    fn make_type_parameters(&mut self, left_angle: Self::Output, parameters: Self::Output, right_angle: Self::Output) -> Self::Output {
        Node(self.0.make_type_parameters(left_angle.0, parameters.0, right_angle.0), self.1.make_type_parameters(left_angle.1, parameters.1, right_angle.1))
    }

    fn make_tuple_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_tuple_type_specifier(left_paren.0, types.0, right_paren.0), self.1.make_tuple_type_specifier(left_paren.1, types.1, right_paren.1))
    }

    fn make_union_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_union_type_specifier(left_paren.0, types.0, right_paren.0), self.1.make_union_type_specifier(left_paren.1, types.1, right_paren.1))
    }

    fn make_intersection_type_specifier(&mut self, left_paren: Self::Output, types: Self::Output, right_paren: Self::Output) -> Self::Output {
        Node(self.0.make_intersection_type_specifier(left_paren.0, types.0, right_paren.0), self.1.make_intersection_type_specifier(left_paren.1, types.1, right_paren.1))
    }

    fn make_error(&mut self, error: Self::Output) -> Self::Output {
        Node(self.0.make_error(error.0), self.1.make_error(error.1))
    }

    fn make_list_item(&mut self, item: Self::Output, separator: Self::Output) -> Self::Output {
        Node(self.0.make_list_item(item.0, separator.0), self.1.make_list_item(item.1, separator.1))
    }

    fn make_enum_class_label_expression(&mut self, qualifier: Self::Output, hash: Self::Output, expression: Self::Output) -> Self::Output {
        Node(self.0.make_enum_class_label_expression(qualifier.0, hash.0, expression.0), self.1.make_enum_class_label_expression(qualifier.1, hash.1, expression.1))
    }

    fn make_module_declaration(&mut self, attribute_spec: Self::Output, new_keyword: Self::Output, module_keyword: Self::Output, name: Self::Output, left_brace: Self::Output, exports: Self::Output, imports: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_module_declaration(attribute_spec.0, new_keyword.0, module_keyword.0, name.0, left_brace.0, exports.0, imports.0, right_brace.0), self.1.make_module_declaration(attribute_spec.1, new_keyword.1, module_keyword.1, name.1, left_brace.1, exports.1, imports.1, right_brace.1))
    }

    fn make_module_exports(&mut self, exports_keyword: Self::Output, left_brace: Self::Output, exports: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_module_exports(exports_keyword.0, left_brace.0, exports.0, right_brace.0), self.1.make_module_exports(exports_keyword.1, left_brace.1, exports.1, right_brace.1))
    }

    fn make_module_imports(&mut self, imports_keyword: Self::Output, left_brace: Self::Output, imports: Self::Output, right_brace: Self::Output) -> Self::Output {
        Node(self.0.make_module_imports(imports_keyword.0, left_brace.0, imports.0, right_brace.0), self.1.make_module_imports(imports_keyword.1, left_brace.1, imports.1, right_brace.1))
    }

    fn make_module_membership_declaration(&mut self, module_keyword: Self::Output, name: Self::Output, semicolon: Self::Output) -> Self::Output {
        Node(self.0.make_module_membership_declaration(module_keyword.0, name.0, semicolon.0), self.1.make_module_membership_declaration(module_keyword.1, name.1, semicolon.1))
    }

    fn make_package_expression(&mut self, keyword: Self::Output, name: Self::Output) -> Self::Output {
        Node(self.0.make_package_expression(keyword.0, name.0), self.1.make_package_expression(keyword.1, name.1))
    }

}
