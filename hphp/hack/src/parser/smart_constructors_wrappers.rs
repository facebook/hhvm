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
 // This module contains smart constructors implementation that can be used to
 // build AST.


use crate::lexable_token::LexableToken;
use crate::parser_env::ParserEnv;
use crate::smart_constructors::SmartConstructors;
use crate::source_text::SourceText;
use crate::syntax_kind::SyntaxKind;

pub struct WithKind<S> {
    phantom_s: std::marker::PhantomData<S>,
}
impl<'a, S, State> SmartConstructors<'a, State> for WithKind<S>
where S: SmartConstructors<'a, State> {
    type Token = S::Token;
    type R = (SyntaxKind, S::R);

    fn initial_state<'b: 'a>(env: &ParserEnv, src: &'b SourceText<'b>) -> State {
        S::initial_state(env, src)
    }

    fn make_token(st: State, token: Self::Token) -> (State, Self::R) {
        compose(SyntaxKind::Token(token.kind()), S::make_token(st, token))
    }

    fn make_missing(st: State, p: usize) -> (State, Self::R) {
        compose(SyntaxKind::Missing, S::make_missing(st, p))
    }

    fn make_list(st: State, items: Box<Vec<Self::R>>, p: usize) -> (State, Self::R) {
        let kind = if items.is_empty() {
            SyntaxKind::Missing
        } else {
            SyntaxKind::SyntaxList
        };
        compose(kind, S::make_list(st, Box::new(items.into_iter().map(|x| x.1).collect()), p))
    }

    fn make_end_of_file(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EndOfFile, S::make_end_of_file(st, arg0.1))
    }
    fn make_script(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::Script, S::make_script(st, arg0.1))
    }
    fn make_qualified_name(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::QualifiedName, S::make_qualified_name(st, arg0.1))
    }
    fn make_simple_type_specifier(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SimpleTypeSpecifier, S::make_simple_type_specifier(st, arg0.1))
    }
    fn make_literal_expression(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::LiteralExpression, S::make_literal_expression(st, arg0.1))
    }
    fn make_prefixed_string_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PrefixedStringExpression, S::make_prefixed_string_expression(st, arg0.1, arg1.1))
    }
    fn make_variable_expression(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VariableExpression, S::make_variable_expression(st, arg0.1))
    }
    fn make_pipe_variable_expression(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PipeVariableExpression, S::make_pipe_variable_expression(st, arg0.1))
    }
    fn make_file_attribute_specification(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FileAttributeSpecification, S::make_file_attribute_specification(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_enum_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EnumDeclaration, S::make_enum_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_enumerator(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::Enumerator, S::make_enumerator(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_record_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::RecordDeclaration, S::make_record_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_record_field(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::RecordField, S::make_record_field(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_alias_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AliasDeclaration, S::make_alias_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1))
    }
    fn make_property_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PropertyDeclaration, S::make_property_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_property_declarator(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PropertyDeclarator, S::make_property_declarator(st, arg0.1, arg1.1))
    }
    fn make_namespace_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceDeclaration, S::make_namespace_declaration(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_namespace_body(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceBody, S::make_namespace_body(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_namespace_empty_body(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceEmptyBody, S::make_namespace_empty_body(st, arg0.1))
    }
    fn make_namespace_use_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceUseDeclaration, S::make_namespace_use_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_namespace_group_use_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceGroupUseDeclaration, S::make_namespace_group_use_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_namespace_use_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NamespaceUseClause, S::make_namespace_use_clause(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_function_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FunctionDeclaration, S::make_function_declaration(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_function_declaration_header(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FunctionDeclarationHeader, S::make_function_declaration_header(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_where_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::WhereClause, S::make_where_clause(st, arg0.1, arg1.1))
    }
    fn make_where_constraint(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::WhereConstraint, S::make_where_constraint(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_methodish_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MethodishDeclaration, S::make_methodish_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_methodish_trait_resolution(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MethodishTraitResolution, S::make_methodish_trait_resolution(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_classish_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ClassishDeclaration, S::make_classish_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_classish_body(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ClassishBody, S::make_classish_body(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_trait_use_precedence_item(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TraitUsePrecedenceItem, S::make_trait_use_precedence_item(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_trait_use_alias_item(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TraitUseAliasItem, S::make_trait_use_alias_item(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_trait_use_conflict_resolution(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TraitUseConflictResolution, S::make_trait_use_conflict_resolution(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_trait_use(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TraitUse, S::make_trait_use(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_require_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::RequireClause, S::make_require_clause(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_const_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ConstDeclaration, S::make_const_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_constant_declarator(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ConstantDeclarator, S::make_constant_declarator(st, arg0.1, arg1.1))
    }
    fn make_type_const_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeConstDeclaration, S::make_type_const_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_decorated_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DecoratedExpression, S::make_decorated_expression(st, arg0.1, arg1.1))
    }
    fn make_parameter_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ParameterDeclaration, S::make_parameter_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_variadic_parameter(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VariadicParameter, S::make_variadic_parameter(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_attribute_specification(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AttributeSpecification, S::make_attribute_specification(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_inclusion_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::InclusionExpression, S::make_inclusion_expression(st, arg0.1, arg1.1))
    }
    fn make_inclusion_directive(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::InclusionDirective, S::make_inclusion_directive(st, arg0.1, arg1.1))
    }
    fn make_compound_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::CompoundStatement, S::make_compound_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_expression_statement(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ExpressionStatement, S::make_expression_statement(st, arg0.1, arg1.1))
    }
    fn make_markup_section(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MarkupSection, S::make_markup_section(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_markup_suffix(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MarkupSuffix, S::make_markup_suffix(st, arg0.1, arg1.1))
    }
    fn make_unset_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::UnsetStatement, S::make_unset_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_let_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::LetStatement, S::make_let_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_using_statement_block_scoped(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::UsingStatementBlockScoped, S::make_using_statement_block_scoped(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_using_statement_function_scoped(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::UsingStatementFunctionScoped, S::make_using_statement_function_scoped(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_while_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::WhileStatement, S::make_while_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_if_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::IfStatement, S::make_if_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_elseif_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ElseifClause, S::make_elseif_clause(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_else_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ElseClause, S::make_else_clause(st, arg0.1, arg1.1))
    }
    fn make_try_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TryStatement, S::make_try_statement(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_catch_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::CatchClause, S::make_catch_clause(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_finally_clause(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FinallyClause, S::make_finally_clause(st, arg0.1, arg1.1))
    }
    fn make_do_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DoStatement, S::make_do_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_for_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ForStatement, S::make_for_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_foreach_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ForeachStatement, S::make_foreach_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_switch_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SwitchStatement, S::make_switch_statement(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_switch_section(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SwitchSection, S::make_switch_section(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_switch_fallthrough(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SwitchFallthrough, S::make_switch_fallthrough(st, arg0.1, arg1.1))
    }
    fn make_case_label(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::CaseLabel, S::make_case_label(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_default_label(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DefaultLabel, S::make_default_label(st, arg0.1, arg1.1))
    }
    fn make_return_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ReturnStatement, S::make_return_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_goto_label(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::GotoLabel, S::make_goto_label(st, arg0.1, arg1.1))
    }
    fn make_goto_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::GotoStatement, S::make_goto_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_throw_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ThrowStatement, S::make_throw_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_break_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::BreakStatement, S::make_break_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_continue_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ContinueStatement, S::make_continue_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_echo_statement(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EchoStatement, S::make_echo_statement(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_concurrent_statement(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ConcurrentStatement, S::make_concurrent_statement(st, arg0.1, arg1.1))
    }
    fn make_simple_initializer(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SimpleInitializer, S::make_simple_initializer(st, arg0.1, arg1.1))
    }
    fn make_anonymous_class(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AnonymousClass, S::make_anonymous_class(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_anonymous_function(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AnonymousFunction, S::make_anonymous_function(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1, arg11.1))
    }
    fn make_anonymous_function_use_clause(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AnonymousFunctionUseClause, S::make_anonymous_function_use_clause(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_lambda_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::LambdaExpression, S::make_lambda_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_lambda_signature(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::LambdaSignature, S::make_lambda_signature(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_cast_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::CastExpression, S::make_cast_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_scope_resolution_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ScopeResolutionExpression, S::make_scope_resolution_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MemberSelectionExpression, S::make_member_selection_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_safe_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SafeMemberSelectionExpression, S::make_safe_member_selection_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_embedded_member_selection_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EmbeddedMemberSelectionExpression, S::make_embedded_member_selection_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_yield_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::YieldExpression, S::make_yield_expression(st, arg0.1, arg1.1))
    }
    fn make_yield_from_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::YieldFromExpression, S::make_yield_from_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_prefix_unary_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PrefixUnaryExpression, S::make_prefix_unary_expression(st, arg0.1, arg1.1))
    }
    fn make_postfix_unary_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PostfixUnaryExpression, S::make_postfix_unary_expression(st, arg0.1, arg1.1))
    }
    fn make_binary_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::BinaryExpression, S::make_binary_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_instanceof_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::InstanceofExpression, S::make_instanceof_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_is_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::IsExpression, S::make_is_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_as_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AsExpression, S::make_as_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_nullable_as_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NullableAsExpression, S::make_nullable_as_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_conditional_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ConditionalExpression, S::make_conditional_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_eval_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EvalExpression, S::make_eval_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_define_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DefineExpression, S::make_define_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_halt_compiler_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::HaltCompilerExpression, S::make_halt_compiler_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_isset_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::IssetExpression, S::make_isset_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_function_call_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FunctionCallExpression, S::make_function_call_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_parenthesized_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ParenthesizedExpression, S::make_parenthesized_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_braced_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::BracedExpression, S::make_braced_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_embedded_braced_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EmbeddedBracedExpression, S::make_embedded_braced_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_list_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ListExpression, S::make_list_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_collection_literal_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::CollectionLiteralExpression, S::make_collection_literal_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_object_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ObjectCreationExpression, S::make_object_creation_expression(st, arg0.1, arg1.1))
    }
    fn make_constructor_call(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ConstructorCall, S::make_constructor_call(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_record_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::RecordCreationExpression, S::make_record_creation_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_array_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ArrayCreationExpression, S::make_array_creation_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_array_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ArrayIntrinsicExpression, S::make_array_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_darray_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DarrayIntrinsicExpression, S::make_darray_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_dictionary_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DictionaryIntrinsicExpression, S::make_dictionary_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_keyset_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::KeysetIntrinsicExpression, S::make_keyset_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_varray_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VarrayIntrinsicExpression, S::make_varray_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_vector_intrinsic_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VectorIntrinsicExpression, S::make_vector_intrinsic_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_element_initializer(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ElementInitializer, S::make_element_initializer(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_subscript_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SubscriptExpression, S::make_subscript_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_embedded_subscript_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::EmbeddedSubscriptExpression, S::make_embedded_subscript_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_awaitable_creation_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::AwaitableCreationExpression, S::make_awaitable_creation_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_children_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPChildrenDeclaration, S::make_xhp_children_declaration(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_children_parenthesized_list(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPChildrenParenthesizedList, S::make_xhp_children_parenthesized_list(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_category_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPCategoryDeclaration, S::make_xhp_category_declaration(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_enum_type(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPEnumType, S::make_xhp_enum_type(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_xhp_lateinit(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPLateinit, S::make_xhp_lateinit(st, arg0.1, arg1.1))
    }
    fn make_xhp_required(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPRequired, S::make_xhp_required(st, arg0.1, arg1.1))
    }
    fn make_xhp_class_attribute_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPClassAttributeDeclaration, S::make_xhp_class_attribute_declaration(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_class_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPClassAttribute, S::make_xhp_class_attribute(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_simple_class_attribute(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPSimpleClassAttribute, S::make_xhp_simple_class_attribute(st, arg0.1))
    }
    fn make_xhp_simple_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPSimpleAttribute, S::make_xhp_simple_attribute(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_spread_attribute(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPSpreadAttribute, S::make_xhp_spread_attribute(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_open(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPOpen, S::make_xhp_open(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPExpression, S::make_xhp_expression(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_close(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::XHPClose, S::make_xhp_close(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_type_constant(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeConstant, S::make_type_constant(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_vector_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VectorTypeSpecifier, S::make_vector_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_keyset_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::KeysetTypeSpecifier, S::make_keyset_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_tuple_type_explicit_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TupleTypeExplicitSpecifier, S::make_tuple_type_explicit_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_varray_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VarrayTypeSpecifier, S::make_varray_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_vector_array_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::VectorArrayTypeSpecifier, S::make_vector_array_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_type_parameter(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeParameter, S::make_type_parameter(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_type_constraint(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeConstraint, S::make_type_constraint(st, arg0.1, arg1.1))
    }
    fn make_darray_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DarrayTypeSpecifier, S::make_darray_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_map_array_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::MapArrayTypeSpecifier, S::make_map_array_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_dictionary_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::DictionaryTypeSpecifier, S::make_dictionary_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_closure_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ClosureTypeSpecifier, S::make_closure_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_closure_parameter_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ClosureParameterTypeSpecifier, S::make_closure_parameter_type_specifier(st, arg0.1, arg1.1))
    }
    fn make_classname_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ClassnameTypeSpecifier, S::make_classname_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_field_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FieldSpecifier, S::make_field_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_field_initializer(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::FieldInitializer, S::make_field_initializer(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_shape_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ShapeTypeSpecifier, S::make_shape_type_specifier(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_shape_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ShapeExpression, S::make_shape_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_tuple_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TupleExpression, S::make_tuple_expression(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_generic_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::GenericTypeSpecifier, S::make_generic_type_specifier(st, arg0.1, arg1.1))
    }
    fn make_nullable_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::NullableTypeSpecifier, S::make_nullable_type_specifier(st, arg0.1, arg1.1))
    }
    fn make_like_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::LikeTypeSpecifier, S::make_like_type_specifier(st, arg0.1, arg1.1))
    }
    fn make_soft_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::SoftTypeSpecifier, S::make_soft_type_specifier(st, arg0.1, arg1.1))
    }
    fn make_reified_type_argument(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ReifiedTypeArgument, S::make_reified_type_argument(st, arg0.1, arg1.1))
    }
    fn make_type_arguments(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeArguments, S::make_type_arguments(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_type_parameters(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TypeParameters, S::make_type_parameters(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_tuple_type_specifier(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::TupleTypeSpecifier, S::make_tuple_type_specifier(st, arg0.1, arg1.1, arg2.1))
    }
    fn make_error(st: State, arg0 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ErrorSyntax, S::make_error(st, arg0.1))
    }
    fn make_list_item(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::ListItem, S::make_list_item(st, arg0.1, arg1.1))
    }
    fn make_pocket_atom_expression(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketAtomExpression, S::make_pocket_atom_expression(st, arg0.1, arg1.1))
    }
    fn make_pocket_identifier_expression(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketIdentifierExpression, S::make_pocket_identifier_expression(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_pocket_atom_mapping_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketAtomMappingDeclaration, S::make_pocket_atom_mapping_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_pocket_enum_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketEnumDeclaration, S::make_pocket_enum_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_pocket_field_type_expr_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketFieldTypeExprDeclaration, S::make_pocket_field_type_expr_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_pocket_field_type_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketFieldTypeDeclaration, S::make_pocket_field_type_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_pocket_mapping_id_declaration(st: State, arg0 : Self::R, arg1 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketMappingIdDeclaration, S::make_pocket_mapping_id_declaration(st, arg0.1, arg1.1))
    }
    fn make_pocket_mapping_type_declaration(st: State, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> (State, Self::R) {
        compose(SyntaxKind::PocketMappingTypeDeclaration, S::make_pocket_mapping_type_declaration(st, arg0.1, arg1.1, arg2.1, arg3.1))
    }

}

// TODO: will this always be inlined? If not, rewrite as macro
fn compose<St, R>(kind: SyntaxKind, st_r: (St, R)) -> (St, (SyntaxKind, R)) {
    let (st, r) = st_r;
    (st, (kind, r))
}
