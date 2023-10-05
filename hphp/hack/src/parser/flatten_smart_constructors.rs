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
use smart_constructors::SmartConstructors;
use parser_core_types::{
  lexable_token::LexableToken,
  syntax_kind::SyntaxKind,
  token_factory::TokenFactory,
};

pub trait FlattenSmartConstructors: SmartConstructors
{
    fn is_zero(s: &Self::Output) -> bool;
    fn zero(kind: SyntaxKind) -> Self::Output;
    fn flatten(&self, kind: SyntaxKind, lst: Vec<Self::Output>) -> Self::Output;

    fn make_missing(&mut self, _: usize) -> Self::Output {
       Self::zero(SyntaxKind::Missing)
    }

    fn make_token(&mut self, token: <Self::Factory as TokenFactory>::Token) -> Self::Output {
        Self::zero(SyntaxKind::Token(token.kind()))
    }

    fn make_list(&mut self, _: Vec<Self::Output>, _: usize) -> Self::Output {
        Self::zero(SyntaxKind::SyntaxList)
    }

    fn begin_enumerator(&mut self) {}

    fn begin_enum_class_enumerator(&mut self) {}

    fn begin_constant_declarator(&mut self) {}

    fn make_end_of_file(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::EndOfFile)
        } else {
          self.flatten(SyntaxKind::EndOfFile, vec!(arg0))
        }
    }

    fn make_script(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::Script)
        } else {
          self.flatten(SyntaxKind::Script, vec!(arg0))
        }
    }

    fn make_qualified_name(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::QualifiedName)
        } else {
          self.flatten(SyntaxKind::QualifiedName, vec!(arg0))
        }
    }

    fn make_module_name(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::ModuleName)
        } else {
          self.flatten(SyntaxKind::ModuleName, vec!(arg0))
        }
    }

    fn make_simple_type_specifier(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::SimpleTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::SimpleTypeSpecifier, vec!(arg0))
        }
    }

    fn make_literal_expression(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::LiteralExpression)
        } else {
          self.flatten(SyntaxKind::LiteralExpression, vec!(arg0))
        }
    }

    fn make_prefixed_string_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::PrefixedStringExpression)
        } else {
          self.flatten(SyntaxKind::PrefixedStringExpression, vec!(arg0, arg1))
        }
    }

    fn make_prefixed_code_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::PrefixedCodeExpression)
        } else {
          self.flatten(SyntaxKind::PrefixedCodeExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_variable_expression(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::VariableExpression)
        } else {
          self.flatten(SyntaxKind::VariableExpression, vec!(arg0))
        }
    }

    fn make_pipe_variable_expression(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::PipeVariableExpression)
        } else {
          self.flatten(SyntaxKind::PipeVariableExpression, vec!(arg0))
        }
    }

    fn make_file_attribute_specification(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::FileAttributeSpecification)
        } else {
          self.flatten(SyntaxKind::FileAttributeSpecification, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_enum_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) {
          Self::zero(SyntaxKind::EnumDeclaration)
        } else {
          self.flatten(SyntaxKind::EnumDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10))
        }
    }

    fn make_enum_use(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::EnumUse)
        } else {
          self.flatten(SyntaxKind::EnumUse, vec!(arg0, arg1, arg2))
        }
    }

    fn make_enumerator(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::Enumerator)
        } else {
          self.flatten(SyntaxKind::Enumerator, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_enum_class_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero(SyntaxKind::EnumClassDeclaration)
        } else {
          self.flatten(SyntaxKind::EnumClassDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_enum_class_enumerator(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::EnumClassEnumerator)
        } else {
          self.flatten(SyntaxKind::EnumClassEnumerator, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_alias_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero(SyntaxKind::AliasDeclaration)
        } else {
          self.flatten(SyntaxKind::AliasDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_context_alias_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) {
          Self::zero(SyntaxKind::ContextAliasDeclaration)
        } else {
          self.flatten(SyntaxKind::ContextAliasDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7))
        }
    }

    fn make_case_type_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) {
          Self::zero(SyntaxKind::CaseTypeDeclaration)
        } else {
          self.flatten(SyntaxKind::CaseTypeDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10))
        }
    }

    fn make_case_type_variant(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::CaseTypeVariant)
        } else {
          self.flatten(SyntaxKind::CaseTypeVariant, vec!(arg0, arg1))
        }
    }

    fn make_property_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::PropertyDeclaration)
        } else {
          self.flatten(SyntaxKind::PropertyDeclaration, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_property_declarator(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::PropertyDeclarator)
        } else {
          self.flatten(SyntaxKind::PropertyDeclarator, vec!(arg0, arg1))
        }
    }

    fn make_namespace_declaration(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::NamespaceDeclaration)
        } else {
          self.flatten(SyntaxKind::NamespaceDeclaration, vec!(arg0, arg1))
        }
    }

    fn make_namespace_declaration_header(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::NamespaceDeclarationHeader)
        } else {
          self.flatten(SyntaxKind::NamespaceDeclarationHeader, vec!(arg0, arg1))
        }
    }

    fn make_namespace_body(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::NamespaceBody)
        } else {
          self.flatten(SyntaxKind::NamespaceBody, vec!(arg0, arg1, arg2))
        }
    }

    fn make_namespace_empty_body(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::NamespaceEmptyBody)
        } else {
          self.flatten(SyntaxKind::NamespaceEmptyBody, vec!(arg0))
        }
    }

    fn make_namespace_use_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::NamespaceUseDeclaration)
        } else {
          self.flatten(SyntaxKind::NamespaceUseDeclaration, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_namespace_group_use_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::NamespaceGroupUseDeclaration)
        } else {
          self.flatten(SyntaxKind::NamespaceGroupUseDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_namespace_use_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::NamespaceUseClause)
        } else {
          self.flatten(SyntaxKind::NamespaceUseClause, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_function_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::FunctionDeclaration)
        } else {
          self.flatten(SyntaxKind::FunctionDeclaration, vec!(arg0, arg1, arg2))
        }
    }

    fn make_function_declaration_header(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero(SyntaxKind::FunctionDeclarationHeader)
        } else {
          self.flatten(SyntaxKind::FunctionDeclarationHeader, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_contexts(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::Contexts)
        } else {
          self.flatten(SyntaxKind::Contexts, vec!(arg0, arg1, arg2))
        }
    }

    fn make_where_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::WhereClause)
        } else {
          self.flatten(SyntaxKind::WhereClause, vec!(arg0, arg1))
        }
    }

    fn make_where_constraint(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::WhereConstraint)
        } else {
          self.flatten(SyntaxKind::WhereConstraint, vec!(arg0, arg1, arg2))
        }
    }

    fn make_methodish_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::MethodishDeclaration)
        } else {
          self.flatten(SyntaxKind::MethodishDeclaration, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_methodish_trait_resolution(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::MethodishTraitResolution)
        } else {
          self.flatten(SyntaxKind::MethodishTraitResolution, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_classish_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero(SyntaxKind::ClassishDeclaration)
        } else {
          self.flatten(SyntaxKind::ClassishDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_classish_body(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ClassishBody)
        } else {
          self.flatten(SyntaxKind::ClassishBody, vec!(arg0, arg1, arg2))
        }
    }

    fn make_trait_use(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::TraitUse)
        } else {
          self.flatten(SyntaxKind::TraitUse, vec!(arg0, arg1, arg2))
        }
    }

    fn make_require_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::RequireClause)
        } else {
          self.flatten(SyntaxKind::RequireClause, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::ConstDeclaration)
        } else {
          self.flatten(SyntaxKind::ConstDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_constant_declarator(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ConstantDeclarator)
        } else {
          self.flatten(SyntaxKind::ConstantDeclarator, vec!(arg0, arg1))
        }
    }

    fn make_type_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero(SyntaxKind::TypeConstDeclaration)
        } else {
          self.flatten(SyntaxKind::TypeConstDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_context_const_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero(SyntaxKind::ContextConstDeclaration)
        } else {
          self.flatten(SyntaxKind::ContextConstDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_decorated_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::DecoratedExpression)
        } else {
          self.flatten(SyntaxKind::DecoratedExpression, vec!(arg0, arg1))
        }
    }

    fn make_parameter_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::ParameterDeclaration)
        } else {
          self.flatten(SyntaxKind::ParameterDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_variadic_parameter(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::VariadicParameter)
        } else {
          self.flatten(SyntaxKind::VariadicParameter, vec!(arg0, arg1, arg2))
        }
    }

    fn make_old_attribute_specification(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::OldAttributeSpecification)
        } else {
          self.flatten(SyntaxKind::OldAttributeSpecification, vec!(arg0, arg1, arg2))
        }
    }

    fn make_attribute_specification(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::AttributeSpecification)
        } else {
          self.flatten(SyntaxKind::AttributeSpecification, vec!(arg0))
        }
    }

    fn make_attribute(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::Attribute)
        } else {
          self.flatten(SyntaxKind::Attribute, vec!(arg0, arg1))
        }
    }

    fn make_inclusion_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::InclusionExpression)
        } else {
          self.flatten(SyntaxKind::InclusionExpression, vec!(arg0, arg1))
        }
    }

    fn make_inclusion_directive(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::InclusionDirective)
        } else {
          self.flatten(SyntaxKind::InclusionDirective, vec!(arg0, arg1))
        }
    }

    fn make_compound_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::CompoundStatement)
        } else {
          self.flatten(SyntaxKind::CompoundStatement, vec!(arg0, arg1, arg2))
        }
    }

    fn make_expression_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ExpressionStatement)
        } else {
          self.flatten(SyntaxKind::ExpressionStatement, vec!(arg0, arg1))
        }
    }

    fn make_markup_section(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::MarkupSection)
        } else {
          self.flatten(SyntaxKind::MarkupSection, vec!(arg0, arg1))
        }
    }

    fn make_markup_suffix(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::MarkupSuffix)
        } else {
          self.flatten(SyntaxKind::MarkupSuffix, vec!(arg0, arg1))
        }
    }

    fn make_unset_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::UnsetStatement)
        } else {
          self.flatten(SyntaxKind::UnsetStatement, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_declare_local_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::DeclareLocalStatement)
        } else {
          self.flatten(SyntaxKind::DeclareLocalStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_using_statement_block_scoped(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::UsingStatementBlockScoped)
        } else {
          self.flatten(SyntaxKind::UsingStatementBlockScoped, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_using_statement_function_scoped(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::UsingStatementFunctionScoped)
        } else {
          self.flatten(SyntaxKind::UsingStatementFunctionScoped, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_while_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::WhileStatement)
        } else {
          self.flatten(SyntaxKind::WhileStatement, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_if_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::IfStatement)
        } else {
          self.flatten(SyntaxKind::IfStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_else_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ElseClause)
        } else {
          self.flatten(SyntaxKind::ElseClause, vec!(arg0, arg1))
        }
    }

    fn make_try_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::TryStatement)
        } else {
          self.flatten(SyntaxKind::TryStatement, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_catch_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::CatchClause)
        } else {
          self.flatten(SyntaxKind::CatchClause, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_finally_clause(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::FinallyClause)
        } else {
          self.flatten(SyntaxKind::FinallyClause, vec!(arg0, arg1))
        }
    }

    fn make_do_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::DoStatement)
        } else {
          self.flatten(SyntaxKind::DoStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_for_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero(SyntaxKind::ForStatement)
        } else {
          self.flatten(SyntaxKind::ForStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_foreach_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) {
          Self::zero(SyntaxKind::ForeachStatement)
        } else {
          self.flatten(SyntaxKind::ForeachStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9))
        }
    }

    fn make_switch_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::SwitchStatement)
        } else {
          self.flatten(SyntaxKind::SwitchStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_switch_section(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::SwitchSection)
        } else {
          self.flatten(SyntaxKind::SwitchSection, vec!(arg0, arg1, arg2))
        }
    }

    fn make_switch_fallthrough(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::SwitchFallthrough)
        } else {
          self.flatten(SyntaxKind::SwitchFallthrough, vec!(arg0, arg1))
        }
    }

    fn make_case_label(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::CaseLabel)
        } else {
          self.flatten(SyntaxKind::CaseLabel, vec!(arg0, arg1, arg2))
        }
    }

    fn make_default_label(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::DefaultLabel)
        } else {
          self.flatten(SyntaxKind::DefaultLabel, vec!(arg0, arg1))
        }
    }

    fn make_match_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::MatchStatement)
        } else {
          self.flatten(SyntaxKind::MatchStatement, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_match_statement_arm(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::MatchStatementArm)
        } else {
          self.flatten(SyntaxKind::MatchStatementArm, vec!(arg0, arg1, arg2))
        }
    }

    fn make_return_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ReturnStatement)
        } else {
          self.flatten(SyntaxKind::ReturnStatement, vec!(arg0, arg1, arg2))
        }
    }

    fn make_yield_break_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::YieldBreakStatement)
        } else {
          self.flatten(SyntaxKind::YieldBreakStatement, vec!(arg0, arg1, arg2))
        }
    }

    fn make_throw_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ThrowStatement)
        } else {
          self.flatten(SyntaxKind::ThrowStatement, vec!(arg0, arg1, arg2))
        }
    }

    fn make_break_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::BreakStatement)
        } else {
          self.flatten(SyntaxKind::BreakStatement, vec!(arg0, arg1))
        }
    }

    fn make_continue_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ContinueStatement)
        } else {
          self.flatten(SyntaxKind::ContinueStatement, vec!(arg0, arg1))
        }
    }

    fn make_echo_statement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::EchoStatement)
        } else {
          self.flatten(SyntaxKind::EchoStatement, vec!(arg0, arg1, arg2))
        }
    }

    fn make_concurrent_statement(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ConcurrentStatement)
        } else {
          self.flatten(SyntaxKind::ConcurrentStatement, vec!(arg0, arg1))
        }
    }

    fn make_simple_initializer(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::SimpleInitializer)
        } else {
          self.flatten(SyntaxKind::SimpleInitializer, vec!(arg0, arg1))
        }
    }

    fn make_anonymous_class(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) {
          Self::zero(SyntaxKind::AnonymousClass)
        } else {
          self.flatten(SyntaxKind::AnonymousClass, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8))
        }
    }

    fn make_anonymous_function(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output, arg11: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) && Self::is_zero(&arg11) {
          Self::zero(SyntaxKind::AnonymousFunction)
        } else {
          self.flatten(SyntaxKind::AnonymousFunction, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11))
        }
    }

    fn make_anonymous_function_use_clause(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::AnonymousFunctionUseClause)
        } else {
          self.flatten(SyntaxKind::AnonymousFunctionUseClause, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_variable_pattern(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::VariablePattern)
        } else {
          self.flatten(SyntaxKind::VariablePattern, vec!(arg0))
        }
    }

    fn make_constructor_pattern(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ConstructorPattern)
        } else {
          self.flatten(SyntaxKind::ConstructorPattern, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_refinement_pattern(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::RefinementPattern)
        } else {
          self.flatten(SyntaxKind::RefinementPattern, vec!(arg0, arg1, arg2))
        }
    }

    fn make_lambda_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::LambdaExpression)
        } else {
          self.flatten(SyntaxKind::LambdaExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_lambda_signature(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::LambdaSignature)
        } else {
          self.flatten(SyntaxKind::LambdaSignature, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_cast_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::CastExpression)
        } else {
          self.flatten(SyntaxKind::CastExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_scope_resolution_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ScopeResolutionExpression)
        } else {
          self.flatten(SyntaxKind::ScopeResolutionExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::MemberSelectionExpression)
        } else {
          self.flatten(SyntaxKind::MemberSelectionExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_safe_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::SafeMemberSelectionExpression)
        } else {
          self.flatten(SyntaxKind::SafeMemberSelectionExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_embedded_member_selection_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::EmbeddedMemberSelectionExpression)
        } else {
          self.flatten(SyntaxKind::EmbeddedMemberSelectionExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_yield_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::YieldExpression)
        } else {
          self.flatten(SyntaxKind::YieldExpression, vec!(arg0, arg1))
        }
    }

    fn make_prefix_unary_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::PrefixUnaryExpression)
        } else {
          self.flatten(SyntaxKind::PrefixUnaryExpression, vec!(arg0, arg1))
        }
    }

    fn make_postfix_unary_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::PostfixUnaryExpression)
        } else {
          self.flatten(SyntaxKind::PostfixUnaryExpression, vec!(arg0, arg1))
        }
    }

    fn make_binary_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::BinaryExpression)
        } else {
          self.flatten(SyntaxKind::BinaryExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_is_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::IsExpression)
        } else {
          self.flatten(SyntaxKind::IsExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_as_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::AsExpression)
        } else {
          self.flatten(SyntaxKind::AsExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_nullable_as_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::NullableAsExpression)
        } else {
          self.flatten(SyntaxKind::NullableAsExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_upcast_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::UpcastExpression)
        } else {
          self.flatten(SyntaxKind::UpcastExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_conditional_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::ConditionalExpression)
        } else {
          self.flatten(SyntaxKind::ConditionalExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_eval_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::EvalExpression)
        } else {
          self.flatten(SyntaxKind::EvalExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_isset_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::IssetExpression)
        } else {
          self.flatten(SyntaxKind::IssetExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_nameof_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::NameofExpression)
        } else {
          self.flatten(SyntaxKind::NameofExpression, vec!(arg0, arg1))
        }
    }

    fn make_function_call_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::FunctionCallExpression)
        } else {
          self.flatten(SyntaxKind::FunctionCallExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_function_pointer_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::FunctionPointerExpression)
        } else {
          self.flatten(SyntaxKind::FunctionPointerExpression, vec!(arg0, arg1))
        }
    }

    fn make_parenthesized_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ParenthesizedExpression)
        } else {
          self.flatten(SyntaxKind::ParenthesizedExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_braced_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::BracedExpression)
        } else {
          self.flatten(SyntaxKind::BracedExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_et_splice_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ETSpliceExpression)
        } else {
          self.flatten(SyntaxKind::ETSpliceExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_embedded_braced_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::EmbeddedBracedExpression)
        } else {
          self.flatten(SyntaxKind::EmbeddedBracedExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_list_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ListExpression)
        } else {
          self.flatten(SyntaxKind::ListExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_collection_literal_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::CollectionLiteralExpression)
        } else {
          self.flatten(SyntaxKind::CollectionLiteralExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_object_creation_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ObjectCreationExpression)
        } else {
          self.flatten(SyntaxKind::ObjectCreationExpression, vec!(arg0, arg1))
        }
    }

    fn make_constructor_call(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ConstructorCall)
        } else {
          self.flatten(SyntaxKind::ConstructorCall, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_darray_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::DarrayIntrinsicExpression)
        } else {
          self.flatten(SyntaxKind::DarrayIntrinsicExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_dictionary_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::DictionaryIntrinsicExpression)
        } else {
          self.flatten(SyntaxKind::DictionaryIntrinsicExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_keyset_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::KeysetIntrinsicExpression)
        } else {
          self.flatten(SyntaxKind::KeysetIntrinsicExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_varray_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::VarrayIntrinsicExpression)
        } else {
          self.flatten(SyntaxKind::VarrayIntrinsicExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_vector_intrinsic_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::VectorIntrinsicExpression)
        } else {
          self.flatten(SyntaxKind::VectorIntrinsicExpression, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_element_initializer(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ElementInitializer)
        } else {
          self.flatten(SyntaxKind::ElementInitializer, vec!(arg0, arg1, arg2))
        }
    }

    fn make_subscript_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::SubscriptExpression)
        } else {
          self.flatten(SyntaxKind::SubscriptExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_embedded_subscript_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::EmbeddedSubscriptExpression)
        } else {
          self.flatten(SyntaxKind::EmbeddedSubscriptExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_awaitable_creation_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::AwaitableCreationExpression)
        } else {
          self.flatten(SyntaxKind::AwaitableCreationExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_children_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPChildrenDeclaration)
        } else {
          self.flatten(SyntaxKind::XHPChildrenDeclaration, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_children_parenthesized_list(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPChildrenParenthesizedList)
        } else {
          self.flatten(SyntaxKind::XHPChildrenParenthesizedList, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_category_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPCategoryDeclaration)
        } else {
          self.flatten(SyntaxKind::XHPCategoryDeclaration, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_enum_type(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::XHPEnumType)
        } else {
          self.flatten(SyntaxKind::XHPEnumType, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_xhp_lateinit(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::XHPLateinit)
        } else {
          self.flatten(SyntaxKind::XHPLateinit, vec!(arg0, arg1))
        }
    }

    fn make_xhp_required(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::XHPRequired)
        } else {
          self.flatten(SyntaxKind::XHPRequired, vec!(arg0, arg1))
        }
    }

    fn make_xhp_class_attribute_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPClassAttributeDeclaration)
        } else {
          self.flatten(SyntaxKind::XHPClassAttributeDeclaration, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_class_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::XHPClassAttribute)
        } else {
          self.flatten(SyntaxKind::XHPClassAttribute, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_simple_class_attribute(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::XHPSimpleClassAttribute)
        } else {
          self.flatten(SyntaxKind::XHPSimpleClassAttribute, vec!(arg0))
        }
    }

    fn make_xhp_simple_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPSimpleAttribute)
        } else {
          self.flatten(SyntaxKind::XHPSimpleAttribute, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_spread_attribute(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::XHPSpreadAttribute)
        } else {
          self.flatten(SyntaxKind::XHPSpreadAttribute, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_open(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::XHPOpen)
        } else {
          self.flatten(SyntaxKind::XHPOpen, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_xhp_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPExpression)
        } else {
          self.flatten(SyntaxKind::XHPExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_xhp_close(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::XHPClose)
        } else {
          self.flatten(SyntaxKind::XHPClose, vec!(arg0, arg1, arg2))
        }
    }

    fn make_type_constant(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::TypeConstant)
        } else {
          self.flatten(SyntaxKind::TypeConstant, vec!(arg0, arg1, arg2))
        }
    }

    fn make_vector_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::VectorTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::VectorTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_keyset_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::KeysetTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::KeysetTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_tuple_type_explicit_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::TupleTypeExplicitSpecifier)
        } else {
          self.flatten(SyntaxKind::TupleTypeExplicitSpecifier, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_varray_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::VarrayTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::VarrayTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_function_ctx_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::FunctionCtxTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::FunctionCtxTypeSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_type_parameter(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::TypeParameter)
        } else {
          self.flatten(SyntaxKind::TypeParameter, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_type_constraint(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::TypeConstraint)
        } else {
          self.flatten(SyntaxKind::TypeConstraint, vec!(arg0, arg1))
        }
    }

    fn make_context_constraint(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ContextConstraint)
        } else {
          self.flatten(SyntaxKind::ContextConstraint, vec!(arg0, arg1))
        }
    }

    fn make_darray_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) {
          Self::zero(SyntaxKind::DarrayTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::DarrayTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6))
        }
    }

    fn make_dictionary_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::DictionaryTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::DictionaryTypeSpecifier, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_closure_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output, arg8: Self::Output, arg9: Self::Output, arg10: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) && Self::is_zero(&arg8) && Self::is_zero(&arg9) && Self::is_zero(&arg10) {
          Self::zero(SyntaxKind::ClosureTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::ClosureTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10))
        }
    }

    fn make_closure_parameter_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ClosureParameterTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::ClosureParameterTypeSpecifier, vec!(arg0, arg1, arg2))
        }
    }

    fn make_type_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::TypeRefinement)
        } else {
          self.flatten(SyntaxKind::TypeRefinement, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_type_in_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::TypeInRefinement)
        } else {
          self.flatten(SyntaxKind::TypeInRefinement, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_ctx_in_refinement(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) {
          Self::zero(SyntaxKind::CtxInRefinement)
        } else {
          self.flatten(SyntaxKind::CtxInRefinement, vec!(arg0, arg1, arg2, arg3, arg4, arg5))
        }
    }

    fn make_classname_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::ClassnameTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::ClassnameTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_class_args_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::ClassArgsTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::ClassArgsTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_field_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::FieldSpecifier)
        } else {
          self.flatten(SyntaxKind::FieldSpecifier, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_field_initializer(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::FieldInitializer)
        } else {
          self.flatten(SyntaxKind::FieldInitializer, vec!(arg0, arg1, arg2))
        }
    }

    fn make_shape_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) {
          Self::zero(SyntaxKind::ShapeTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::ShapeTypeSpecifier, vec!(arg0, arg1, arg2, arg3, arg4))
        }
    }

    fn make_shape_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ShapeExpression)
        } else {
          self.flatten(SyntaxKind::ShapeExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_tuple_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::TupleExpression)
        } else {
          self.flatten(SyntaxKind::TupleExpression, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_generic_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::GenericTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::GenericTypeSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_nullable_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::NullableTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::NullableTypeSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_like_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::LikeTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::LikeTypeSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_soft_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::SoftTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::SoftTypeSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_attributized_specifier(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::AttributizedSpecifier)
        } else {
          self.flatten(SyntaxKind::AttributizedSpecifier, vec!(arg0, arg1))
        }
    }

    fn make_reified_type_argument(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ReifiedTypeArgument)
        } else {
          self.flatten(SyntaxKind::ReifiedTypeArgument, vec!(arg0, arg1))
        }
    }

    fn make_type_arguments(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::TypeArguments)
        } else {
          self.flatten(SyntaxKind::TypeArguments, vec!(arg0, arg1, arg2))
        }
    }

    fn make_type_parameters(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::TypeParameters)
        } else {
          self.flatten(SyntaxKind::TypeParameters, vec!(arg0, arg1, arg2))
        }
    }

    fn make_tuple_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::TupleTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::TupleTypeSpecifier, vec!(arg0, arg1, arg2))
        }
    }

    fn make_union_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::UnionTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::UnionTypeSpecifier, vec!(arg0, arg1, arg2))
        }
    }

    fn make_intersection_type_specifier(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::IntersectionTypeSpecifier)
        } else {
          self.flatten(SyntaxKind::IntersectionTypeSpecifier, vec!(arg0, arg1, arg2))
        }
    }

    fn make_error(&mut self, arg0: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) {
          Self::zero(SyntaxKind::ErrorSyntax)
        } else {
          self.flatten(SyntaxKind::ErrorSyntax, vec!(arg0))
        }
    }

    fn make_list_item(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::ListItem)
        } else {
          self.flatten(SyntaxKind::ListItem, vec!(arg0, arg1))
        }
    }

    fn make_enum_class_label_expression(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::EnumClassLabelExpression)
        } else {
          self.flatten(SyntaxKind::EnumClassLabelExpression, vec!(arg0, arg1, arg2))
        }
    }

    fn make_module_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output, arg4: Self::Output, arg5: Self::Output, arg6: Self::Output, arg7: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) && Self::is_zero(&arg4) && Self::is_zero(&arg5) && Self::is_zero(&arg6) && Self::is_zero(&arg7) {
          Self::zero(SyntaxKind::ModuleDeclaration)
        } else {
          self.flatten(SyntaxKind::ModuleDeclaration, vec!(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7))
        }
    }

    fn make_module_exports(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ModuleExports)
        } else {
          self.flatten(SyntaxKind::ModuleExports, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_module_imports(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output, arg3: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) && Self::is_zero(&arg3) {
          Self::zero(SyntaxKind::ModuleImports)
        } else {
          self.flatten(SyntaxKind::ModuleImports, vec!(arg0, arg1, arg2, arg3))
        }
    }

    fn make_module_membership_declaration(&mut self, arg0: Self::Output, arg1: Self::Output, arg2: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) && Self::is_zero(&arg2) {
          Self::zero(SyntaxKind::ModuleMembershipDeclaration)
        } else {
          self.flatten(SyntaxKind::ModuleMembershipDeclaration, vec!(arg0, arg1, arg2))
        }
    }

    fn make_package_expression(&mut self, arg0: Self::Output, arg1: Self::Output) -> Self::Output {
        if Self::is_zero(&arg0) && Self::is_zero(&arg1) {
          Self::zero(SyntaxKind::PackageExpression)
        } else {
          self.flatten(SyntaxKind::PackageExpression, vec!(arg0, arg1))
        }
    }

}
