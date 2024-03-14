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
use super::{
    has_arena::HasArena,
    syntax::*, syntax_variant_generated::*,
};
use crate::{
    lexable_token::LexableToken,
    syntax::{SyntaxType, SyntaxValueType},
};

impl<'a, C, T, V> SyntaxType<C> for Syntax<'a, T, V>
where
    T: LexableToken + Copy,
    V: SyntaxValueType<T>,
    C: HasArena<'a>,
{
    fn make_end_of_file(ctx: &C, token: Self) -> Self {
        let syntax = SyntaxVariant::EndOfFile(ctx.get_arena().alloc(EndOfFileChildren {
            token,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_script(ctx: &C, declarations: Self) -> Self {
        let syntax = SyntaxVariant::Script(ctx.get_arena().alloc(ScriptChildren {
            declarations,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_qualified_name(ctx: &C, parts: Self) -> Self {
        let syntax = SyntaxVariant::QualifiedName(ctx.get_arena().alloc(QualifiedNameChildren {
            parts,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_module_name(ctx: &C, parts: Self) -> Self {
        let syntax = SyntaxVariant::ModuleName(ctx.get_arena().alloc(ModuleNameChildren {
            parts,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_simple_type_specifier(ctx: &C, specifier: Self) -> Self {
        let syntax = SyntaxVariant::SimpleTypeSpecifier(ctx.get_arena().alloc(SimpleTypeSpecifierChildren {
            specifier,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_literal_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::LiteralExpression(ctx.get_arena().alloc(LiteralExpressionChildren {
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefixed_string_expression(ctx: &C, name: Self, str: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedStringExpression(ctx.get_arena().alloc(PrefixedStringExpressionChildren {
            name,
            str,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefixed_code_expression(ctx: &C, prefix: Self, left_backtick: Self, body: Self, right_backtick: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedCodeExpression(ctx.get_arena().alloc(PrefixedCodeExpressionChildren {
            prefix,
            left_backtick,
            body,
            right_backtick,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_variable_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::VariableExpression(ctx.get_arena().alloc(VariableExpressionChildren {
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pipe_variable_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::PipeVariableExpression(ctx.get_arena().alloc(PipeVariableExpressionChildren {
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_file_attribute_specification(ctx: &C, left_double_angle: Self, keyword: Self, colon: Self, attributes: Self, right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::FileAttributeSpecification(ctx.get_arena().alloc(FileAttributeSpecificationChildren {
            left_double_angle,
            keyword,
            colon,
            attributes,
            right_double_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, keyword: Self, name: Self, colon: Self, base: Self, type_: Self, left_brace: Self, use_clauses: Self, enumerators: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumDeclaration(ctx.get_arena().alloc(EnumDeclarationChildren {
            attribute_spec,
            modifiers,
            keyword,
            name,
            colon,
            base,
            type_,
            left_brace,
            use_clauses,
            enumerators,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_use(ctx: &C, keyword: Self, names: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EnumUse(ctx.get_arena().alloc(EnumUseChildren {
            keyword,
            names,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enumerator(ctx: &C, name: Self, equal: Self, value: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::Enumerator(ctx.get_arena().alloc(EnumeratorChildren {
            name,
            equal,
            value,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_class_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, enum_keyword: Self, class_keyword: Self, name: Self, colon: Self, base: Self, extends: Self, extends_list: Self, left_brace: Self, elements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumClassDeclaration(ctx.get_arena().alloc(EnumClassDeclarationChildren {
            attribute_spec,
            modifiers,
            enum_keyword,
            class_keyword,
            name,
            colon,
            base,
            extends,
            extends_list,
            left_brace,
            elements,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_class_enumerator(ctx: &C, modifiers: Self, type_: Self, name: Self, initializer: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EnumClassEnumerator(ctx.get_arena().alloc(EnumClassEnumeratorChildren {
            modifiers,
            type_,
            name,
            initializer,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_alias_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, module_kw_opt: Self, keyword: Self, name: Self, generic_parameter: Self, constraint: Self, equal: Self, type_: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AliasDeclaration(ctx.get_arena().alloc(AliasDeclarationChildren {
            attribute_spec,
            modifiers,
            module_kw_opt,
            keyword,
            name,
            generic_parameter,
            constraint,
            equal,
            type_,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_context_alias_declaration(ctx: &C, attribute_spec: Self, keyword: Self, name: Self, generic_parameter: Self, as_constraint: Self, equal: Self, context: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContextAliasDeclaration(ctx.get_arena().alloc(ContextAliasDeclarationChildren {
            attribute_spec,
            keyword,
            name,
            generic_parameter,
            as_constraint,
            equal,
            context,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_case_type_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, case_keyword: Self, type_keyword: Self, name: Self, generic_parameter: Self, as_: Self, bounds: Self, equal: Self, variants: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::CaseTypeDeclaration(ctx.get_arena().alloc(CaseTypeDeclarationChildren {
            attribute_spec,
            modifiers,
            case_keyword,
            type_keyword,
            name,
            generic_parameter,
            as_,
            bounds,
            equal,
            variants,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_case_type_variant(ctx: &C, bar: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::CaseTypeVariant(ctx.get_arena().alloc(CaseTypeVariantChildren {
            bar,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_property_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, type_: Self, declarators: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclaration(ctx.get_arena().alloc(PropertyDeclarationChildren {
            attribute_spec,
            modifiers,
            type_,
            declarators,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_property_declarator(ctx: &C, name: Self, initializer: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclarator(ctx.get_arena().alloc(PropertyDeclaratorChildren {
            name,
            initializer,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_declaration(ctx: &C, header: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclaration(ctx.get_arena().alloc(NamespaceDeclarationChildren {
            header,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_declaration_header(ctx: &C, keyword: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclarationHeader(ctx.get_arena().alloc(NamespaceDeclarationHeaderChildren {
            keyword,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_body(ctx: &C, left_brace: Self, declarations: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceBody(ctx.get_arena().alloc(NamespaceBodyChildren {
            left_brace,
            declarations,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_empty_body(ctx: &C, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceEmptyBody(ctx.get_arena().alloc(NamespaceEmptyBodyChildren {
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_use_declaration(ctx: &C, keyword: Self, kind: Self, clauses: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseDeclaration(ctx.get_arena().alloc(NamespaceUseDeclarationChildren {
            keyword,
            kind,
            clauses,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_group_use_declaration(ctx: &C, keyword: Self, kind: Self, prefix: Self, left_brace: Self, clauses: Self, right_brace: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceGroupUseDeclaration(ctx.get_arena().alloc(NamespaceGroupUseDeclarationChildren {
            keyword,
            kind,
            prefix,
            left_brace,
            clauses,
            right_brace,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_use_clause(ctx: &C, clause_kind: Self, name: Self, as_: Self, alias: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseClause(ctx.get_arena().alloc(NamespaceUseClauseChildren {
            clause_kind,
            name,
            as_,
            alias,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_declaration(ctx: &C, attribute_spec: Self, declaration_header: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclaration(ctx.get_arena().alloc(FunctionDeclarationChildren {
            attribute_spec,
            declaration_header,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_declaration_header(ctx: &C, modifiers: Self, keyword: Self, name: Self, type_parameter_list: Self, left_paren: Self, parameter_list: Self, right_paren: Self, contexts: Self, colon: Self, readonly_return: Self, type_: Self, where_clause: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclarationHeader(ctx.get_arena().alloc(FunctionDeclarationHeaderChildren {
            modifiers,
            keyword,
            name,
            type_parameter_list,
            left_paren,
            parameter_list,
            right_paren,
            contexts,
            colon,
            readonly_return,
            type_,
            where_clause,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_contexts(ctx: &C, left_bracket: Self, types: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::Contexts(ctx.get_arena().alloc(ContextsChildren {
            left_bracket,
            types,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_where_clause(ctx: &C, keyword: Self, constraints: Self) -> Self {
        let syntax = SyntaxVariant::WhereClause(ctx.get_arena().alloc(WhereClauseChildren {
            keyword,
            constraints,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_where_constraint(ctx: &C, left_type: Self, operator: Self, right_type: Self) -> Self {
        let syntax = SyntaxVariant::WhereConstraint(ctx.get_arena().alloc(WhereConstraintChildren {
            left_type,
            operator,
            right_type,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_methodish_declaration(ctx: &C, attribute: Self, function_decl_header: Self, function_body: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishDeclaration(ctx.get_arena().alloc(MethodishDeclarationChildren {
            attribute,
            function_decl_header,
            function_body,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_methodish_trait_resolution(ctx: &C, attribute: Self, function_decl_header: Self, equal: Self, name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishTraitResolution(ctx.get_arena().alloc(MethodishTraitResolutionChildren {
            attribute,
            function_decl_header,
            equal,
            name,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_classish_declaration(ctx: &C, attribute: Self, modifiers: Self, xhp: Self, keyword: Self, name: Self, type_parameters: Self, extends_keyword: Self, extends_list: Self, implements_keyword: Self, implements_list: Self, where_clause: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::ClassishDeclaration(ctx.get_arena().alloc(ClassishDeclarationChildren {
            attribute,
            modifiers,
            xhp,
            keyword,
            name,
            type_parameters,
            extends_keyword,
            extends_list,
            implements_keyword,
            implements_list,
            where_clause,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_classish_body(ctx: &C, left_brace: Self, elements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ClassishBody(ctx.get_arena().alloc(ClassishBodyChildren {
            left_brace,
            elements,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_trait_use(ctx: &C, keyword: Self, names: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TraitUse(ctx.get_arena().alloc(TraitUseChildren {
            keyword,
            names,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_require_clause(ctx: &C, keyword: Self, kind: Self, name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::RequireClause(ctx.get_arena().alloc(RequireClauseChildren {
            keyword,
            kind,
            name,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_const_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, keyword: Self, type_specifier: Self, declarators: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ConstDeclaration(ctx.get_arena().alloc(ConstDeclarationChildren {
            attribute_spec,
            modifiers,
            keyword,
            type_specifier,
            declarators,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_constant_declarator(ctx: &C, name: Self, initializer: Self) -> Self {
        let syntax = SyntaxVariant::ConstantDeclarator(ctx.get_arena().alloc(ConstantDeclaratorChildren {
            name,
            initializer,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_const_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, keyword: Self, type_keyword: Self, name: Self, type_parameters: Self, type_constraints: Self, equal: Self, type_specifier: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstDeclaration(ctx.get_arena().alloc(TypeConstDeclarationChildren {
            attribute_spec,
            modifiers,
            keyword,
            type_keyword,
            name,
            type_parameters,
            type_constraints,
            equal,
            type_specifier,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_context_const_declaration(ctx: &C, modifiers: Self, const_keyword: Self, ctx_keyword: Self, name: Self, type_parameters: Self, constraint: Self, equal: Self, ctx_list: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContextConstDeclaration(ctx.get_arena().alloc(ContextConstDeclarationChildren {
            modifiers,
            const_keyword,
            ctx_keyword,
            name,
            type_parameters,
            constraint,
            equal,
            ctx_list,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_decorated_expression(ctx: &C, decorator: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::DecoratedExpression(ctx.get_arena().alloc(DecoratedExpressionChildren {
            decorator,
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_parameter_declaration(ctx: &C, attribute: Self, visibility: Self, call_convention: Self, readonly: Self, type_: Self, ellipsis: Self, name: Self, default_value: Self, parameter_end: Self) -> Self {
        let syntax = SyntaxVariant::ParameterDeclaration(ctx.get_arena().alloc(ParameterDeclarationChildren {
            attribute,
            visibility,
            call_convention,
            readonly,
            type_,
            ellipsis,
            name,
            default_value,
            parameter_end,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_old_attribute_specification(ctx: &C, left_double_angle: Self, attributes: Self, right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::OldAttributeSpecification(ctx.get_arena().alloc(OldAttributeSpecificationChildren {
            left_double_angle,
            attributes,
            right_double_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attribute_specification(ctx: &C, attributes: Self) -> Self {
        let syntax = SyntaxVariant::AttributeSpecification(ctx.get_arena().alloc(AttributeSpecificationChildren {
            attributes,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attribute(ctx: &C, at: Self, attribute_name: Self) -> Self {
        let syntax = SyntaxVariant::Attribute(ctx.get_arena().alloc(AttributeChildren {
            at,
            attribute_name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_inclusion_expression(ctx: &C, require: Self, filename: Self) -> Self {
        let syntax = SyntaxVariant::InclusionExpression(ctx.get_arena().alloc(InclusionExpressionChildren {
            require,
            filename,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_inclusion_directive(ctx: &C, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::InclusionDirective(ctx.get_arena().alloc(InclusionDirectiveChildren {
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_compound_statement(ctx: &C, left_brace: Self, statements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CompoundStatement(ctx.get_arena().alloc(CompoundStatementChildren {
            left_brace,
            statements,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_expression_statement(ctx: &C, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ExpressionStatement(ctx.get_arena().alloc(ExpressionStatementChildren {
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_markup_section(ctx: &C, hashbang: Self, suffix: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSection(ctx.get_arena().alloc(MarkupSectionChildren {
            hashbang,
            suffix,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_markup_suffix(ctx: &C, less_than_question: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSuffix(ctx.get_arena().alloc(MarkupSuffixChildren {
            less_than_question,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_unset_statement(ctx: &C, keyword: Self, left_paren: Self, variables: Self, right_paren: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UnsetStatement(ctx.get_arena().alloc(UnsetStatementChildren {
            keyword,
            left_paren,
            variables,
            right_paren,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_declare_local_statement(ctx: &C, keyword: Self, variable: Self, colon: Self, type_: Self, initializer: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::DeclareLocalStatement(ctx.get_arena().alloc(DeclareLocalStatementChildren {
            keyword,
            variable,
            colon,
            type_,
            initializer,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_using_statement_block_scoped(ctx: &C, await_keyword: Self, using_keyword: Self, left_paren: Self, expressions: Self, right_paren: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementBlockScoped(ctx.get_arena().alloc(UsingStatementBlockScopedChildren {
            await_keyword,
            using_keyword,
            left_paren,
            expressions,
            right_paren,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_using_statement_function_scoped(ctx: &C, await_keyword: Self, using_keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementFunctionScoped(ctx.get_arena().alloc(UsingStatementFunctionScopedChildren {
            await_keyword,
            using_keyword,
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_while_statement(ctx: &C, keyword: Self, left_paren: Self, condition: Self, right_paren: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::WhileStatement(ctx.get_arena().alloc(WhileStatementChildren {
            keyword,
            left_paren,
            condition,
            right_paren,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_if_statement(ctx: &C, keyword: Self, left_paren: Self, condition: Self, right_paren: Self, statement: Self, else_clause: Self) -> Self {
        let syntax = SyntaxVariant::IfStatement(ctx.get_arena().alloc(IfStatementChildren {
            keyword,
            left_paren,
            condition,
            right_paren,
            statement,
            else_clause,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_else_clause(ctx: &C, keyword: Self, statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseClause(ctx.get_arena().alloc(ElseClauseChildren {
            keyword,
            statement,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_try_statement(ctx: &C, keyword: Self, compound_statement: Self, catch_clauses: Self, finally_clause: Self) -> Self {
        let syntax = SyntaxVariant::TryStatement(ctx.get_arena().alloc(TryStatementChildren {
            keyword,
            compound_statement,
            catch_clauses,
            finally_clause,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_catch_clause(ctx: &C, keyword: Self, left_paren: Self, type_: Self, variable: Self, right_paren: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::CatchClause(ctx.get_arena().alloc(CatchClauseChildren {
            keyword,
            left_paren,
            type_,
            variable,
            right_paren,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_finally_clause(ctx: &C, keyword: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::FinallyClause(ctx.get_arena().alloc(FinallyClauseChildren {
            keyword,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_do_statement(ctx: &C, keyword: Self, body: Self, while_keyword: Self, left_paren: Self, condition: Self, right_paren: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::DoStatement(ctx.get_arena().alloc(DoStatementChildren {
            keyword,
            body,
            while_keyword,
            left_paren,
            condition,
            right_paren,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_for_statement(ctx: &C, keyword: Self, left_paren: Self, initializer: Self, first_semicolon: Self, control: Self, second_semicolon: Self, end_of_loop: Self, right_paren: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::ForStatement(ctx.get_arena().alloc(ForStatementChildren {
            keyword,
            left_paren,
            initializer,
            first_semicolon,
            control,
            second_semicolon,
            end_of_loop,
            right_paren,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_foreach_statement(ctx: &C, keyword: Self, left_paren: Self, collection: Self, await_keyword: Self, as_: Self, key: Self, arrow: Self, value: Self, right_paren: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::ForeachStatement(ctx.get_arena().alloc(ForeachStatementChildren {
            keyword,
            left_paren,
            collection,
            await_keyword,
            as_,
            key,
            arrow,
            value,
            right_paren,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_switch_statement(ctx: &C, keyword: Self, left_paren: Self, expression: Self, right_paren: Self, left_brace: Self, sections: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::SwitchStatement(ctx.get_arena().alloc(SwitchStatementChildren {
            keyword,
            left_paren,
            expression,
            right_paren,
            left_brace,
            sections,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_switch_section(ctx: &C, labels: Self, statements: Self, fallthrough: Self) -> Self {
        let syntax = SyntaxVariant::SwitchSection(ctx.get_arena().alloc(SwitchSectionChildren {
            labels,
            statements,
            fallthrough,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_switch_fallthrough(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::SwitchFallthrough(ctx.get_arena().alloc(SwitchFallthroughChildren {
            keyword,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_case_label(ctx: &C, keyword: Self, expression: Self, colon: Self) -> Self {
        let syntax = SyntaxVariant::CaseLabel(ctx.get_arena().alloc(CaseLabelChildren {
            keyword,
            expression,
            colon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_default_label(ctx: &C, keyword: Self, colon: Self) -> Self {
        let syntax = SyntaxVariant::DefaultLabel(ctx.get_arena().alloc(DefaultLabelChildren {
            keyword,
            colon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_match_statement(ctx: &C, keyword: Self, left_paren: Self, expression: Self, right_paren: Self, left_brace: Self, arms: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::MatchStatement(ctx.get_arena().alloc(MatchStatementChildren {
            keyword,
            left_paren,
            expression,
            right_paren,
            left_brace,
            arms,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_match_statement_arm(ctx: &C, pattern: Self, arrow: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::MatchStatementArm(ctx.get_arena().alloc(MatchStatementArmChildren {
            pattern,
            arrow,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_return_statement(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ReturnStatement(ctx.get_arena().alloc(ReturnStatementChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_yield_break_statement(ctx: &C, keyword: Self, break_: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::YieldBreakStatement(ctx.get_arena().alloc(YieldBreakStatementChildren {
            keyword,
            break_,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_throw_statement(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ThrowStatement(ctx.get_arena().alloc(ThrowStatementChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_break_statement(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::BreakStatement(ctx.get_arena().alloc(BreakStatementChildren {
            keyword,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_continue_statement(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContinueStatement(ctx.get_arena().alloc(ContinueStatementChildren {
            keyword,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_echo_statement(ctx: &C, keyword: Self, expressions: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EchoStatement(ctx.get_arena().alloc(EchoStatementChildren {
            keyword,
            expressions,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_concurrent_statement(ctx: &C, keyword: Self, statement: Self) -> Self {
        let syntax = SyntaxVariant::ConcurrentStatement(ctx.get_arena().alloc(ConcurrentStatementChildren {
            keyword,
            statement,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_simple_initializer(ctx: &C, equal: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::SimpleInitializer(ctx.get_arena().alloc(SimpleInitializerChildren {
            equal,
            value,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_anonymous_class(ctx: &C, class_keyword: Self, left_paren: Self, argument_list: Self, right_paren: Self, extends_keyword: Self, extends_list: Self, implements_keyword: Self, implements_list: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousClass(ctx.get_arena().alloc(AnonymousClassChildren {
            class_keyword,
            left_paren,
            argument_list,
            right_paren,
            extends_keyword,
            extends_list,
            implements_keyword,
            implements_list,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_anonymous_function(ctx: &C, attribute_spec: Self, async_keyword: Self, function_keyword: Self, left_paren: Self, parameters: Self, right_paren: Self, ctx_list: Self, colon: Self, readonly_return: Self, type_: Self, use_: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunction(ctx.get_arena().alloc(AnonymousFunctionChildren {
            attribute_spec,
            async_keyword,
            function_keyword,
            left_paren,
            parameters,
            right_paren,
            ctx_list,
            colon,
            readonly_return,
            type_,
            use_,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_anonymous_function_use_clause(ctx: &C, keyword: Self, left_paren: Self, variables: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunctionUseClause(ctx.get_arena().alloc(AnonymousFunctionUseClauseChildren {
            keyword,
            left_paren,
            variables,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_variable_pattern(ctx: &C, variable: Self) -> Self {
        let syntax = SyntaxVariant::VariablePattern(ctx.get_arena().alloc(VariablePatternChildren {
            variable,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_constructor_pattern(ctx: &C, constructor: Self, left_paren: Self, members: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ConstructorPattern(ctx.get_arena().alloc(ConstructorPatternChildren {
            constructor,
            left_paren,
            members,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_refinement_pattern(ctx: &C, variable: Self, colon: Self, specifier: Self) -> Self {
        let syntax = SyntaxVariant::RefinementPattern(ctx.get_arena().alloc(RefinementPatternChildren {
            variable,
            colon,
            specifier,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_lambda_expression(ctx: &C, attribute_spec: Self, async_: Self, signature: Self, arrow: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::LambdaExpression(ctx.get_arena().alloc(LambdaExpressionChildren {
            attribute_spec,
            async_,
            signature,
            arrow,
            body,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_lambda_signature(ctx: &C, left_paren: Self, parameters: Self, right_paren: Self, contexts: Self, colon: Self, readonly_return: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::LambdaSignature(ctx.get_arena().alloc(LambdaSignatureChildren {
            left_paren,
            parameters,
            right_paren,
            contexts,
            colon,
            readonly_return,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_cast_expression(ctx: &C, left_paren: Self, type_: Self, right_paren: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::CastExpression(ctx.get_arena().alloc(CastExpressionChildren {
            left_paren,
            type_,
            right_paren,
            operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_scope_resolution_expression(ctx: &C, qualifier: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::ScopeResolutionExpression(ctx.get_arena().alloc(ScopeResolutionExpressionChildren {
            qualifier,
            operator,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::MemberSelectionExpression(ctx.get_arena().alloc(MemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_safe_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::SafeMemberSelectionExpression(ctx.get_arena().alloc(SafeMemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedMemberSelectionExpression(ctx.get_arena().alloc(EmbeddedMemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_yield_expression(ctx: &C, keyword: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::YieldExpression(ctx.get_arena().alloc(YieldExpressionChildren {
            keyword,
            operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefix_unary_expression(ctx: &C, operator: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::PrefixUnaryExpression(ctx.get_arena().alloc(PrefixUnaryExpressionChildren {
            operator,
            operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_postfix_unary_expression(ctx: &C, operand: Self, operator: Self) -> Self {
        let syntax = SyntaxVariant::PostfixUnaryExpression(ctx.get_arena().alloc(PostfixUnaryExpressionChildren {
            operand,
            operator,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_binary_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::BinaryExpression(ctx.get_arena().alloc(BinaryExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_is_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::IsExpression(ctx.get_arena().alloc(IsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_as_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::AsExpression(ctx.get_arena().alloc(AsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_nullable_as_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::NullableAsExpression(ctx.get_arena().alloc(NullableAsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_upcast_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::UpcastExpression(ctx.get_arena().alloc(UpcastExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_conditional_expression(ctx: &C, test: Self, question: Self, consequence: Self, colon: Self, alternative: Self) -> Self {
        let syntax = SyntaxVariant::ConditionalExpression(ctx.get_arena().alloc(ConditionalExpressionChildren {
            test,
            question,
            consequence,
            colon,
            alternative,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_eval_expression(ctx: &C, keyword: Self, left_paren: Self, argument: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::EvalExpression(ctx.get_arena().alloc(EvalExpressionChildren {
            keyword,
            left_paren,
            argument,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_isset_expression(ctx: &C, keyword: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IssetExpression(ctx.get_arena().alloc(IssetExpressionChildren {
            keyword,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_nameof_expression(ctx: &C, keyword: Self, target: Self) -> Self {
        let syntax = SyntaxVariant::NameofExpression(ctx.get_arena().alloc(NameofExpressionChildren {
            keyword,
            target,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_call_expression(ctx: &C, receiver: Self, type_args: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::FunctionCallExpression(ctx.get_arena().alloc(FunctionCallExpressionChildren {
            receiver,
            type_args,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_pointer_expression(ctx: &C, receiver: Self, type_args: Self) -> Self {
        let syntax = SyntaxVariant::FunctionPointerExpression(ctx.get_arena().alloc(FunctionPointerExpressionChildren {
            receiver,
            type_args,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_parenthesized_expression(ctx: &C, left_paren: Self, expression: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ParenthesizedExpression(ctx.get_arena().alloc(ParenthesizedExpressionChildren {
            left_paren,
            expression,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_braced_expression(ctx: &C, left_brace: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::BracedExpression(ctx.get_arena().alloc(BracedExpressionChildren {
            left_brace,
            expression,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_et_splice_expression(ctx: &C, dollar: Self, left_brace: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ETSpliceExpression(ctx.get_arena().alloc(ETSpliceExpressionChildren {
            dollar,
            left_brace,
            expression,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_braced_expression(ctx: &C, left_brace: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedBracedExpression(ctx.get_arena().alloc(EmbeddedBracedExpressionChildren {
            left_brace,
            expression,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_list_expression(ctx: &C, keyword: Self, left_paren: Self, members: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ListExpression(ctx.get_arena().alloc(ListExpressionChildren {
            keyword,
            left_paren,
            members,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_collection_literal_expression(ctx: &C, name: Self, left_brace: Self, initializers: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CollectionLiteralExpression(ctx.get_arena().alloc(CollectionLiteralExpressionChildren {
            name,
            left_brace,
            initializers,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_object_creation_expression(ctx: &C, new_keyword: Self, object: Self) -> Self {
        let syntax = SyntaxVariant::ObjectCreationExpression(ctx.get_arena().alloc(ObjectCreationExpressionChildren {
            new_keyword,
            object,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_constructor_call(ctx: &C, type_: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ConstructorCall(ctx.get_arena().alloc(ConstructorCallChildren {
            type_,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_darray_intrinsic_expression(ctx: &C, keyword: Self, explicit_type: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::DarrayIntrinsicExpression(ctx.get_arena().alloc(DarrayIntrinsicExpressionChildren {
            keyword,
            explicit_type,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_dictionary_intrinsic_expression(ctx: &C, keyword: Self, explicit_type: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryIntrinsicExpression(ctx.get_arena().alloc(DictionaryIntrinsicExpressionChildren {
            keyword,
            explicit_type,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_keyset_intrinsic_expression(ctx: &C, keyword: Self, explicit_type: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::KeysetIntrinsicExpression(ctx.get_arena().alloc(KeysetIntrinsicExpressionChildren {
            keyword,
            explicit_type,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_varray_intrinsic_expression(ctx: &C, keyword: Self, explicit_type: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::VarrayIntrinsicExpression(ctx.get_arena().alloc(VarrayIntrinsicExpressionChildren {
            keyword,
            explicit_type,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_vector_intrinsic_expression(ctx: &C, keyword: Self, explicit_type: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::VectorIntrinsicExpression(ctx.get_arena().alloc(VectorIntrinsicExpressionChildren {
            keyword,
            explicit_type,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_element_initializer(ctx: &C, key: Self, arrow: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::ElementInitializer(ctx.get_arena().alloc(ElementInitializerChildren {
            key,
            arrow,
            value,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_subscript_expression(ctx: &C, receiver: Self, left_bracket: Self, index: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::SubscriptExpression(ctx.get_arena().alloc(SubscriptExpressionChildren {
            receiver,
            left_bracket,
            index,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_subscript_expression(ctx: &C, receiver: Self, left_bracket: Self, index: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedSubscriptExpression(ctx.get_arena().alloc(EmbeddedSubscriptExpressionChildren {
            receiver,
            left_bracket,
            index,
            right_bracket,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_awaitable_creation_expression(ctx: &C, attribute_spec: Self, async_: Self, compound_statement: Self) -> Self {
        let syntax = SyntaxVariant::AwaitableCreationExpression(ctx.get_arena().alloc(AwaitableCreationExpressionChildren {
            attribute_spec,
            async_,
            compound_statement,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_children_declaration(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenDeclaration(ctx.get_arena().alloc(XHPChildrenDeclarationChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_children_parenthesized_list(ctx: &C, left_paren: Self, xhp_children: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenParenthesizedList(ctx.get_arena().alloc(XHPChildrenParenthesizedListChildren {
            left_paren,
            xhp_children,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_category_declaration(ctx: &C, keyword: Self, categories: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPCategoryDeclaration(ctx.get_arena().alloc(XHPCategoryDeclarationChildren {
            keyword,
            categories,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_enum_type(ctx: &C, like: Self, keyword: Self, left_brace: Self, values: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPEnumType(ctx.get_arena().alloc(XHPEnumTypeChildren {
            like,
            keyword,
            left_brace,
            values,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_lateinit(ctx: &C, at: Self, keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPLateinit(ctx.get_arena().alloc(XHPLateinitChildren {
            at,
            keyword,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_required(ctx: &C, at: Self, keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPRequired(ctx.get_arena().alloc(XHPRequiredChildren {
            at,
            keyword,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute_declaration(ctx: &C, keyword: Self, attributes: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttributeDeclaration(ctx.get_arena().alloc(XHPClassAttributeDeclarationChildren {
            keyword,
            attributes,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute(ctx: &C, type_: Self, name: Self, initializer: Self, required: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttribute(ctx.get_arena().alloc(XHPClassAttributeChildren {
            type_,
            name,
            initializer,
            required,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_simple_class_attribute(ctx: &C, type_: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleClassAttribute(ctx.get_arena().alloc(XHPSimpleClassAttributeChildren {
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_simple_attribute(ctx: &C, name: Self, equal: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleAttribute(ctx.get_arena().alloc(XHPSimpleAttributeChildren {
            name,
            equal,
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_spread_attribute(ctx: &C, left_brace: Self, spread_operator: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPSpreadAttribute(ctx.get_arena().alloc(XHPSpreadAttributeChildren {
            left_brace,
            spread_operator,
            expression,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_open(ctx: &C, left_angle: Self, name: Self, attributes: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPOpen(ctx.get_arena().alloc(XHPOpenChildren {
            left_angle,
            name,
            attributes,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_expression(ctx: &C, open: Self, body: Self, close: Self) -> Self {
        let syntax = SyntaxVariant::XHPExpression(ctx.get_arena().alloc(XHPExpressionChildren {
            open,
            body,
            close,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_close(ctx: &C, left_angle: Self, name: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPClose(ctx.get_arena().alloc(XHPCloseChildren {
            left_angle,
            name,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_constant(ctx: &C, left_type: Self, separator: Self, right_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstant(ctx.get_arena().alloc(TypeConstantChildren {
            left_type,
            separator,
            right_type,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_vector_type_specifier(ctx: &C, keyword: Self, left_angle: Self, type_: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VectorTypeSpecifier(ctx.get_arena().alloc(VectorTypeSpecifierChildren {
            keyword,
            left_angle,
            type_,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_keyset_type_specifier(ctx: &C, keyword: Self, left_angle: Self, type_: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::KeysetTypeSpecifier(ctx.get_arena().alloc(KeysetTypeSpecifierChildren {
            keyword,
            left_angle,
            type_,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_type_explicit_specifier(ctx: &C, keyword: Self, left_angle: Self, types: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeExplicitSpecifier(ctx.get_arena().alloc(TupleTypeExplicitSpecifierChildren {
            keyword,
            left_angle,
            types,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_varray_type_specifier(ctx: &C, keyword: Self, left_angle: Self, type_: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VarrayTypeSpecifier(ctx.get_arena().alloc(VarrayTypeSpecifierChildren {
            keyword,
            left_angle,
            type_,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_ctx_type_specifier(ctx: &C, keyword: Self, variable: Self) -> Self {
        let syntax = SyntaxVariant::FunctionCtxTypeSpecifier(ctx.get_arena().alloc(FunctionCtxTypeSpecifierChildren {
            keyword,
            variable,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_parameter(ctx: &C, attribute_spec: Self, reified: Self, variance: Self, name: Self, param_params: Self, constraints: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameter(ctx.get_arena().alloc(TypeParameterChildren {
            attribute_spec,
            reified,
            variance,
            name,
            param_params,
            constraints,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_constraint(ctx: &C, keyword: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstraint(ctx.get_arena().alloc(TypeConstraintChildren {
            keyword,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_context_constraint(ctx: &C, keyword: Self, ctx_list: Self) -> Self {
        let syntax = SyntaxVariant::ContextConstraint(ctx.get_arena().alloc(ContextConstraintChildren {
            keyword,
            ctx_list,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_darray_type_specifier(ctx: &C, keyword: Self, left_angle: Self, key: Self, comma: Self, value: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DarrayTypeSpecifier(ctx.get_arena().alloc(DarrayTypeSpecifierChildren {
            keyword,
            left_angle,
            key,
            comma,
            value,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_dictionary_type_specifier(ctx: &C, keyword: Self, left_angle: Self, members: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryTypeSpecifier(ctx.get_arena().alloc(DictionaryTypeSpecifierChildren {
            keyword,
            left_angle,
            members,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_closure_type_specifier(ctx: &C, outer_left_paren: Self, readonly_keyword: Self, function_keyword: Self, inner_left_paren: Self, parameter_list: Self, inner_right_paren: Self, contexts: Self, colon: Self, readonly_return: Self, return_type: Self, outer_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ClosureTypeSpecifier(ctx.get_arena().alloc(ClosureTypeSpecifierChildren {
            outer_left_paren,
            readonly_keyword,
            function_keyword,
            inner_left_paren,
            parameter_list,
            inner_right_paren,
            contexts,
            colon,
            readonly_return,
            return_type,
            outer_right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_closure_parameter_type_specifier(ctx: &C, optional: Self, call_convention: Self, readonly: Self, type_: Self, ellipsis: Self) -> Self {
        let syntax = SyntaxVariant::ClosureParameterTypeSpecifier(ctx.get_arena().alloc(ClosureParameterTypeSpecifierChildren {
            optional,
            call_convention,
            readonly,
            type_,
            ellipsis,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_refinement(ctx: &C, type_: Self, keyword: Self, left_brace: Self, members: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::TypeRefinement(ctx.get_arena().alloc(TypeRefinementChildren {
            type_,
            keyword,
            left_brace,
            members,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_in_refinement(ctx: &C, keyword: Self, name: Self, type_parameters: Self, constraints: Self, equal: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::TypeInRefinement(ctx.get_arena().alloc(TypeInRefinementChildren {
            keyword,
            name,
            type_parameters,
            constraints,
            equal,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_ctx_in_refinement(ctx: &C, keyword: Self, name: Self, type_parameters: Self, constraints: Self, equal: Self, ctx_list: Self) -> Self {
        let syntax = SyntaxVariant::CtxInRefinement(ctx.get_arena().alloc(CtxInRefinementChildren {
            keyword,
            name,
            type_parameters,
            constraints,
            equal,
            ctx_list,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_classname_type_specifier(ctx: &C, keyword: Self, left_angle: Self, type_: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::ClassnameTypeSpecifier(ctx.get_arena().alloc(ClassnameTypeSpecifierChildren {
            keyword,
            left_angle,
            type_,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_class_args_type_specifier(ctx: &C, keyword: Self, left_angle: Self, type_: Self, trailing_comma: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::ClassArgsTypeSpecifier(ctx.get_arena().alloc(ClassArgsTypeSpecifierChildren {
            keyword,
            left_angle,
            type_,
            trailing_comma,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_field_specifier(ctx: &C, question: Self, name: Self, arrow: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::FieldSpecifier(ctx.get_arena().alloc(FieldSpecifierChildren {
            question,
            name,
            arrow,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_field_initializer(ctx: &C, name: Self, arrow: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::FieldInitializer(ctx.get_arena().alloc(FieldInitializerChildren {
            name,
            arrow,
            value,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_shape_type_specifier(ctx: &C, keyword: Self, left_paren: Self, fields: Self, ellipsis: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeTypeSpecifier(ctx.get_arena().alloc(ShapeTypeSpecifierChildren {
            keyword,
            left_paren,
            fields,
            ellipsis,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_shape_expression(ctx: &C, keyword: Self, left_paren: Self, fields: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeExpression(ctx.get_arena().alloc(ShapeExpressionChildren {
            keyword,
            left_paren,
            fields,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_expression(ctx: &C, keyword: Self, left_paren: Self, items: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleExpression(ctx.get_arena().alloc(TupleExpressionChildren {
            keyword,
            left_paren,
            items,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_generic_type_specifier(ctx: &C, class_type: Self, argument_list: Self) -> Self {
        let syntax = SyntaxVariant::GenericTypeSpecifier(ctx.get_arena().alloc(GenericTypeSpecifierChildren {
            class_type,
            argument_list,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_nullable_type_specifier(ctx: &C, question: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::NullableTypeSpecifier(ctx.get_arena().alloc(NullableTypeSpecifierChildren {
            question,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_like_type_specifier(ctx: &C, tilde: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::LikeTypeSpecifier(ctx.get_arena().alloc(LikeTypeSpecifierChildren {
            tilde,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_soft_type_specifier(ctx: &C, at: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::SoftTypeSpecifier(ctx.get_arena().alloc(SoftTypeSpecifierChildren {
            at,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attributized_specifier(ctx: &C, attribute_spec: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::AttributizedSpecifier(ctx.get_arena().alloc(AttributizedSpecifierChildren {
            attribute_spec,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_reified_type_argument(ctx: &C, reified: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::ReifiedTypeArgument(ctx.get_arena().alloc(ReifiedTypeArgumentChildren {
            reified,
            type_,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_arguments(ctx: &C, left_angle: Self, types: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeArguments(ctx.get_arena().alloc(TypeArgumentsChildren {
            left_angle,
            types,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_parameters(ctx: &C, left_angle: Self, parameters: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameters(ctx.get_arena().alloc(TypeParametersChildren {
            left_angle,
            parameters,
            right_angle,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeSpecifier(ctx.get_arena().alloc(TupleTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_union_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::UnionTypeSpecifier(ctx.get_arena().alloc(UnionTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_intersection_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IntersectionTypeSpecifier(ctx.get_arena().alloc(IntersectionTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_error(ctx: &C, error: Self) -> Self {
        let syntax = SyntaxVariant::ErrorSyntax(ctx.get_arena().alloc(ErrorSyntaxChildren {
            error,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_list_item(ctx: &C, item: Self, separator: Self) -> Self {
        let syntax = SyntaxVariant::ListItem(ctx.get_arena().alloc(ListItemChildren {
            item,
            separator,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_class_label_expression(ctx: &C, qualifier: Self, hash: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::EnumClassLabelExpression(ctx.get_arena().alloc(EnumClassLabelExpressionChildren {
            qualifier,
            hash,
            expression,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_module_declaration(ctx: &C, attribute_spec: Self, new_keyword: Self, module_keyword: Self, name: Self, left_brace: Self, exports: Self, imports: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ModuleDeclaration(ctx.get_arena().alloc(ModuleDeclarationChildren {
            attribute_spec,
            new_keyword,
            module_keyword,
            name,
            left_brace,
            exports,
            imports,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_module_exports(ctx: &C, exports_keyword: Self, left_brace: Self, exports: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ModuleExports(ctx.get_arena().alloc(ModuleExportsChildren {
            exports_keyword,
            left_brace,
            exports,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_module_imports(ctx: &C, imports_keyword: Self, left_brace: Self, imports: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ModuleImports(ctx.get_arena().alloc(ModuleImportsChildren {
            imports_keyword,
            left_brace,
            imports,
            right_brace,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_module_membership_declaration(ctx: &C, module_keyword: Self, name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ModuleMembershipDeclaration(ctx.get_arena().alloc(ModuleMembershipDeclarationChildren {
            module_keyword,
            name,
            semicolon,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_package_expression(ctx: &C, keyword: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::PackageExpression(ctx.get_arena().alloc(PackageExpressionChildren {
            keyword,
            name,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

 }
