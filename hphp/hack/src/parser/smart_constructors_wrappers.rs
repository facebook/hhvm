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
 // This module contains smart constructors implementation that can be used to
 // build AST.

use parser_core_types::{
  lexable_token::LexableToken,
  syntax_kind::SyntaxKind,
  token_factory::TokenFactory,
};
use crate::SmartConstructors;

#[derive(Clone)]
pub struct WithKind<S> {
    s: S,
}

impl<S> WithKind<S> {
    pub fn new(s: S) -> Self {
        Self { s }
    }
}

impl<S, State> SmartConstructors for WithKind<S>
where S: SmartConstructors<State = State>,
{
    type TF = S::TF;
    type State = State;
    type R = (SyntaxKind, S::R);

    fn state_mut(&mut self) -> &mut State {
        self.s.state_mut()
    }

    fn into_state(self) -> State {
      self.s.into_state()
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        self.s.token_factory_mut()
    }


    fn make_token(&mut self, token: <Self::TF as TokenFactory>::Token) -> Self::R {
        compose(SyntaxKind::Token(token.kind()), self.s.make_token(token))
    }

    fn make_missing(&mut self, p: usize) -> Self::R {
        compose(SyntaxKind::Missing, self.s.make_missing(p))
    }

    fn make_list(&mut self, items: Vec<Self::R>, p: usize) -> Self::R {
        let kind = if items.is_empty() {
            SyntaxKind::Missing
        } else {
            SyntaxKind::SyntaxList
        };
        compose(kind, self.s.make_list(items.into_iter().map(|x| x.1).collect(), p))
    }

    fn make_end_of_file(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::EndOfFile, self.s.make_end_of_file(arg0.1))
    }
    fn make_script(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::Script, self.s.make_script(arg0.1))
    }
    fn make_qualified_name(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::QualifiedName, self.s.make_qualified_name(arg0.1))
    }
    fn make_simple_type_specifier(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::SimpleTypeSpecifier, self.s.make_simple_type_specifier(arg0.1))
    }
    fn make_literal_expression(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::LiteralExpression, self.s.make_literal_expression(arg0.1))
    }
    fn make_prefixed_string_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::PrefixedStringExpression, self.s.make_prefixed_string_expression(arg0.1, arg1.1))
    }
    fn make_prefixed_code_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::PrefixedCodeExpression, self.s.make_prefixed_code_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_variable_expression(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::VariableExpression, self.s.make_variable_expression(arg0.1))
    }
    fn make_pipe_variable_expression(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::PipeVariableExpression, self.s.make_pipe_variable_expression(arg0.1))
    }
    fn make_file_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::FileAttributeSpecification, self.s.make_file_attribute_specification(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_enum_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        compose(SyntaxKind::EnumDeclaration, self.s.make_enum_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_enum_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::EnumUse, self.s.make_enum_use(arg0.1, arg1.1, arg2.1))
    }
    fn make_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::Enumerator, self.s.make_enumerator(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_enum_class_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R {
        compose(SyntaxKind::EnumClassDeclaration, self.s.make_enum_class_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1))
    }
    fn make_enum_class_enumerator(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::EnumClassEnumerator, self.s.make_enum_class_enumerator(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_record_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        compose(SyntaxKind::RecordDeclaration, self.s.make_record_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_record_field(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::RecordField, self.s.make_record_field(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_alias_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R {
        compose(SyntaxKind::AliasDeclaration, self.s.make_alias_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1))
    }
    fn make_context_alias_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R) -> Self::R {
        compose(SyntaxKind::ContextAliasDeclaration, self.s.make_context_alias_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1))
    }
    fn make_property_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::PropertyDeclaration, self.s.make_property_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_property_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::PropertyDeclarator, self.s.make_property_declarator(arg0.1, arg1.1))
    }
    fn make_namespace_declaration(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceDeclaration, self.s.make_namespace_declaration(arg0.1, arg1.1))
    }
    fn make_namespace_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceDeclarationHeader, self.s.make_namespace_declaration_header(arg0.1, arg1.1))
    }
    fn make_namespace_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceBody, self.s.make_namespace_body(arg0.1, arg1.1, arg2.1))
    }
    fn make_namespace_empty_body(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceEmptyBody, self.s.make_namespace_empty_body(arg0.1))
    }
    fn make_namespace_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceUseDeclaration, self.s.make_namespace_use_declaration(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_namespace_group_use_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceGroupUseDeclaration, self.s.make_namespace_group_use_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_namespace_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::NamespaceUseClause, self.s.make_namespace_use_clause(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_function_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::FunctionDeclaration, self.s.make_function_declaration(arg0.1, arg1.1, arg2.1))
    }
    fn make_function_declaration_header(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        compose(SyntaxKind::FunctionDeclarationHeader, self.s.make_function_declaration_header(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1, arg11.1))
    }
    fn make_contexts(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::Contexts, self.s.make_contexts(arg0.1, arg1.1, arg2.1))
    }
    fn make_where_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::WhereClause, self.s.make_where_clause(arg0.1, arg1.1))
    }
    fn make_where_constraint(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::WhereConstraint, self.s.make_where_constraint(arg0.1, arg1.1, arg2.1))
    }
    fn make_methodish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::MethodishDeclaration, self.s.make_methodish_declaration(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_methodish_trait_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::MethodishTraitResolution, self.s.make_methodish_trait_resolution(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_classish_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        compose(SyntaxKind::ClassishDeclaration, self.s.make_classish_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1, arg11.1))
    }
    fn make_classish_body(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ClassishBody, self.s.make_classish_body(arg0.1, arg1.1, arg2.1))
    }
    fn make_trait_use_precedence_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TraitUsePrecedenceItem, self.s.make_trait_use_precedence_item(arg0.1, arg1.1, arg2.1))
    }
    fn make_trait_use_alias_item(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::TraitUseAliasItem, self.s.make_trait_use_alias_item(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_trait_use_conflict_resolution(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::TraitUseConflictResolution, self.s.make_trait_use_conflict_resolution(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_trait_use(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TraitUse, self.s.make_trait_use(arg0.1, arg1.1, arg2.1))
    }
    fn make_require_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::RequireClause, self.s.make_require_clause(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::ConstDeclaration, self.s.make_const_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_constant_declarator(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ConstantDeclarator, self.s.make_constant_declarator(arg0.1, arg1.1))
    }
    fn make_type_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeConstDeclaration, self.s.make_type_const_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_context_const_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        compose(SyntaxKind::ContextConstDeclaration, self.s.make_context_const_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_decorated_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::DecoratedExpression, self.s.make_decorated_expression(arg0.1, arg1.1))
    }
    fn make_parameter_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::ParameterDeclaration, self.s.make_parameter_declaration(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_variadic_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::VariadicParameter, self.s.make_variadic_parameter(arg0.1, arg1.1, arg2.1))
    }
    fn make_old_attribute_specification(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::OldAttributeSpecification, self.s.make_old_attribute_specification(arg0.1, arg1.1, arg2.1))
    }
    fn make_attribute_specification(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::AttributeSpecification, self.s.make_attribute_specification(arg0.1))
    }
    fn make_attribute(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::Attribute, self.s.make_attribute(arg0.1, arg1.1))
    }
    fn make_inclusion_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::InclusionExpression, self.s.make_inclusion_expression(arg0.1, arg1.1))
    }
    fn make_inclusion_directive(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::InclusionDirective, self.s.make_inclusion_directive(arg0.1, arg1.1))
    }
    fn make_compound_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::CompoundStatement, self.s.make_compound_statement(arg0.1, arg1.1, arg2.1))
    }
    fn make_expression_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ExpressionStatement, self.s.make_expression_statement(arg0.1, arg1.1))
    }
    fn make_markup_section(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::MarkupSection, self.s.make_markup_section(arg0.1, arg1.1))
    }
    fn make_markup_suffix(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::MarkupSuffix, self.s.make_markup_suffix(arg0.1, arg1.1))
    }
    fn make_unset_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::UnsetStatement, self.s.make_unset_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_using_statement_block_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        compose(SyntaxKind::UsingStatementBlockScoped, self.s.make_using_statement_block_scoped(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_using_statement_function_scoped(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::UsingStatementFunctionScoped, self.s.make_using_statement_function_scoped(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_while_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::WhileStatement, self.s.make_while_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_if_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::IfStatement, self.s.make_if_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_elseif_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::ElseifClause, self.s.make_elseif_clause(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_else_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ElseClause, self.s.make_else_clause(arg0.1, arg1.1))
    }
    fn make_try_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::TryStatement, self.s.make_try_statement(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_catch_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        compose(SyntaxKind::CatchClause, self.s.make_catch_clause(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_finally_clause(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::FinallyClause, self.s.make_finally_clause(arg0.1, arg1.1))
    }
    fn make_do_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::DoStatement, self.s.make_do_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_for_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        compose(SyntaxKind::ForStatement, self.s.make_for_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_foreach_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R) -> Self::R {
        compose(SyntaxKind::ForeachStatement, self.s.make_foreach_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1))
    }
    fn make_switch_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::SwitchStatement, self.s.make_switch_statement(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_switch_section(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::SwitchSection, self.s.make_switch_section(arg0.1, arg1.1, arg2.1))
    }
    fn make_switch_fallthrough(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::SwitchFallthrough, self.s.make_switch_fallthrough(arg0.1, arg1.1))
    }
    fn make_case_label(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::CaseLabel, self.s.make_case_label(arg0.1, arg1.1, arg2.1))
    }
    fn make_default_label(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::DefaultLabel, self.s.make_default_label(arg0.1, arg1.1))
    }
    fn make_return_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ReturnStatement, self.s.make_return_statement(arg0.1, arg1.1, arg2.1))
    }
    fn make_yield_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::YieldBreakStatement, self.s.make_yield_break_statement(arg0.1, arg1.1, arg2.1))
    }
    fn make_throw_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ThrowStatement, self.s.make_throw_statement(arg0.1, arg1.1, arg2.1))
    }
    fn make_break_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::BreakStatement, self.s.make_break_statement(arg0.1, arg1.1))
    }
    fn make_continue_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ContinueStatement, self.s.make_continue_statement(arg0.1, arg1.1))
    }
    fn make_echo_statement(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::EchoStatement, self.s.make_echo_statement(arg0.1, arg1.1, arg2.1))
    }
    fn make_concurrent_statement(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ConcurrentStatement, self.s.make_concurrent_statement(arg0.1, arg1.1))
    }
    fn make_simple_initializer(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::SimpleInitializer, self.s.make_simple_initializer(arg0.1, arg1.1))
    }
    fn make_anonymous_class(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R) -> Self::R {
        compose(SyntaxKind::AnonymousClass, self.s.make_anonymous_class(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1))
    }
    fn make_anonymous_function(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R, arg11 : Self::R) -> Self::R {
        compose(SyntaxKind::AnonymousFunction, self.s.make_anonymous_function(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1, arg11.1))
    }
    fn make_anonymous_function_use_clause(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::AnonymousFunctionUseClause, self.s.make_anonymous_function_use_clause(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_lambda_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::LambdaExpression, self.s.make_lambda_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_lambda_signature(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::LambdaSignature, self.s.make_lambda_signature(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_cast_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::CastExpression, self.s.make_cast_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_scope_resolution_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ScopeResolutionExpression, self.s.make_scope_resolution_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::MemberSelectionExpression, self.s.make_member_selection_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_safe_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::SafeMemberSelectionExpression, self.s.make_safe_member_selection_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_embedded_member_selection_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::EmbeddedMemberSelectionExpression, self.s.make_embedded_member_selection_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_yield_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::YieldExpression, self.s.make_yield_expression(arg0.1, arg1.1))
    }
    fn make_prefix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::PrefixUnaryExpression, self.s.make_prefix_unary_expression(arg0.1, arg1.1))
    }
    fn make_postfix_unary_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::PostfixUnaryExpression, self.s.make_postfix_unary_expression(arg0.1, arg1.1))
    }
    fn make_binary_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::BinaryExpression, self.s.make_binary_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_is_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::IsExpression, self.s.make_is_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::AsExpression, self.s.make_as_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_nullable_as_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::NullableAsExpression, self.s.make_nullable_as_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_conditional_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::ConditionalExpression, self.s.make_conditional_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_eval_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::EvalExpression, self.s.make_eval_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_isset_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::IssetExpression, self.s.make_isset_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_function_call_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        compose(SyntaxKind::FunctionCallExpression, self.s.make_function_call_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_function_pointer_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::FunctionPointerExpression, self.s.make_function_pointer_expression(arg0.1, arg1.1))
    }
    fn make_parenthesized_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ParenthesizedExpression, self.s.make_parenthesized_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::BracedExpression, self.s.make_braced_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_et_splice_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::ETSpliceExpression, self.s.make_et_splice_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_embedded_braced_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::EmbeddedBracedExpression, self.s.make_embedded_braced_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_list_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::ListExpression, self.s.make_list_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_collection_literal_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::CollectionLiteralExpression, self.s.make_collection_literal_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_object_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ObjectCreationExpression, self.s.make_object_creation_expression(arg0.1, arg1.1))
    }
    fn make_constructor_call(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::ConstructorCall, self.s.make_constructor_call(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_record_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::RecordCreationExpression, self.s.make_record_creation_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_darray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::DarrayIntrinsicExpression, self.s.make_darray_intrinsic_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_dictionary_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::DictionaryIntrinsicExpression, self.s.make_dictionary_intrinsic_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_keyset_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::KeysetIntrinsicExpression, self.s.make_keyset_intrinsic_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_varray_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::VarrayIntrinsicExpression, self.s.make_varray_intrinsic_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_vector_intrinsic_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::VectorIntrinsicExpression, self.s.make_vector_intrinsic_expression(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_element_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ElementInitializer, self.s.make_element_initializer(arg0.1, arg1.1, arg2.1))
    }
    fn make_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::SubscriptExpression, self.s.make_subscript_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_embedded_subscript_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::EmbeddedSubscriptExpression, self.s.make_embedded_subscript_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_awaitable_creation_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::AwaitableCreationExpression, self.s.make_awaitable_creation_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_children_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPChildrenDeclaration, self.s.make_xhp_children_declaration(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_children_parenthesized_list(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPChildrenParenthesizedList, self.s.make_xhp_children_parenthesized_list(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_category_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPCategoryDeclaration, self.s.make_xhp_category_declaration(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_enum_type(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPEnumType, self.s.make_xhp_enum_type(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_lateinit(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPLateinit, self.s.make_xhp_lateinit(arg0.1, arg1.1))
    }
    fn make_xhp_required(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPRequired, self.s.make_xhp_required(arg0.1, arg1.1))
    }
    fn make_xhp_class_attribute_declaration(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPClassAttributeDeclaration, self.s.make_xhp_class_attribute_declaration(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_class_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPClassAttribute, self.s.make_xhp_class_attribute(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_simple_class_attribute(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPSimpleClassAttribute, self.s.make_xhp_simple_class_attribute(arg0.1))
    }
    fn make_xhp_simple_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPSimpleAttribute, self.s.make_xhp_simple_attribute(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_spread_attribute(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPSpreadAttribute, self.s.make_xhp_spread_attribute(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_open(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPOpen, self.s.make_xhp_open(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_xhp_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPExpression, self.s.make_xhp_expression(arg0.1, arg1.1, arg2.1))
    }
    fn make_xhp_close(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::XHPClose, self.s.make_xhp_close(arg0.1, arg1.1, arg2.1))
    }
    fn make_type_constant(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeConstant, self.s.make_type_constant(arg0.1, arg1.1, arg2.1))
    }
    fn make_vector_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::VectorTypeSpecifier, self.s.make_vector_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_keyset_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::KeysetTypeSpecifier, self.s.make_keyset_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_tuple_type_explicit_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::TupleTypeExplicitSpecifier, self.s.make_tuple_type_explicit_specifier(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_varray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::VarrayTypeSpecifier, self.s.make_varray_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_function_ctx_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::FunctionCtxTypeSpecifier, self.s.make_function_ctx_type_specifier(arg0.1, arg1.1))
    }
    fn make_type_parameter(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeParameter, self.s.make_type_parameter(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1))
    }
    fn make_type_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeConstraint, self.s.make_type_constraint(arg0.1, arg1.1))
    }
    fn make_context_constraint(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ContextConstraint, self.s.make_context_constraint(arg0.1, arg1.1))
    }
    fn make_darray_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R) -> Self::R {
        compose(SyntaxKind::DarrayTypeSpecifier, self.s.make_darray_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1))
    }
    fn make_dictionary_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::DictionaryTypeSpecifier, self.s.make_dictionary_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_closure_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R, arg5 : Self::R, arg6 : Self::R, arg7 : Self::R, arg8 : Self::R, arg9 : Self::R, arg10 : Self::R) -> Self::R {
        compose(SyntaxKind::ClosureTypeSpecifier, self.s.make_closure_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1, arg5.1, arg6.1, arg7.1, arg8.1, arg9.1, arg10.1))
    }
    fn make_closure_parameter_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::ClosureParameterTypeSpecifier, self.s.make_closure_parameter_type_specifier(arg0.1, arg1.1, arg2.1))
    }
    fn make_classname_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::ClassnameTypeSpecifier, self.s.make_classname_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_field_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::FieldSpecifier, self.s.make_field_specifier(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_field_initializer(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::FieldInitializer, self.s.make_field_initializer(arg0.1, arg1.1, arg2.1))
    }
    fn make_shape_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R, arg4 : Self::R) -> Self::R {
        compose(SyntaxKind::ShapeTypeSpecifier, self.s.make_shape_type_specifier(arg0.1, arg1.1, arg2.1, arg3.1, arg4.1))
    }
    fn make_shape_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::ShapeExpression, self.s.make_shape_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_tuple_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R, arg3 : Self::R) -> Self::R {
        compose(SyntaxKind::TupleExpression, self.s.make_tuple_expression(arg0.1, arg1.1, arg2.1, arg3.1))
    }
    fn make_generic_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::GenericTypeSpecifier, self.s.make_generic_type_specifier(arg0.1, arg1.1))
    }
    fn make_nullable_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::NullableTypeSpecifier, self.s.make_nullable_type_specifier(arg0.1, arg1.1))
    }
    fn make_like_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::LikeTypeSpecifier, self.s.make_like_type_specifier(arg0.1, arg1.1))
    }
    fn make_soft_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::SoftTypeSpecifier, self.s.make_soft_type_specifier(arg0.1, arg1.1))
    }
    fn make_attributized_specifier(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::AttributizedSpecifier, self.s.make_attributized_specifier(arg0.1, arg1.1))
    }
    fn make_reified_type_argument(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ReifiedTypeArgument, self.s.make_reified_type_argument(arg0.1, arg1.1))
    }
    fn make_type_arguments(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeArguments, self.s.make_type_arguments(arg0.1, arg1.1, arg2.1))
    }
    fn make_type_parameters(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TypeParameters, self.s.make_type_parameters(arg0.1, arg1.1, arg2.1))
    }
    fn make_tuple_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::TupleTypeSpecifier, self.s.make_tuple_type_specifier(arg0.1, arg1.1, arg2.1))
    }
    fn make_union_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::UnionTypeSpecifier, self.s.make_union_type_specifier(arg0.1, arg1.1, arg2.1))
    }
    fn make_intersection_type_specifier(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::IntersectionTypeSpecifier, self.s.make_intersection_type_specifier(arg0.1, arg1.1, arg2.1))
    }
    fn make_error(&mut self, arg0 : Self::R) -> Self::R {
        compose(SyntaxKind::ErrorSyntax, self.s.make_error(arg0.1))
    }
    fn make_list_item(&mut self, arg0 : Self::R, arg1 : Self::R) -> Self::R {
        compose(SyntaxKind::ListItem, self.s.make_list_item(arg0.1, arg1.1))
    }
    fn make_enum_class_label_expression(&mut self, arg0 : Self::R, arg1 : Self::R, arg2 : Self::R) -> Self::R {
        compose(SyntaxKind::EnumClassLabelExpression, self.s.make_enum_class_label_expression(arg0.1, arg1.1, arg2.1))
    }

}

#[inline(always)]
fn compose<R>(kind: SyntaxKind, r: R) -> (SyntaxKind, R) {
    (kind, r)
}
