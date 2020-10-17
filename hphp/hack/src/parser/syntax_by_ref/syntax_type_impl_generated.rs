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
    syntax::*,
    syntax_variant_generated::*,
    has_arena::HasArena,
    positioned_token::PositionedToken,
    positioned_value::PositionedValue
};
use crate::syntax::{SyntaxType, SyntaxValueType};

impl<'a, C> SyntaxType<C> for Syntax<'a, PositionedToken<'a>, PositionedValue<'a>>
where
    C: HasArena<'a>,
{
    fn make_end_of_file(ctx: &C, token: Self) -> Self {
        let syntax = SyntaxVariant::EndOfFile(ctx.get_arena().alloc(EndOfFileChildren {
            token,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_script(ctx: &C, declarations: Self) -> Self {
        let syntax = SyntaxVariant::Script(ctx.get_arena().alloc(ScriptChildren {
            declarations,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_qualified_name(ctx: &C, parts: Self) -> Self {
        let syntax = SyntaxVariant::QualifiedName(ctx.get_arena().alloc(QualifiedNameChildren {
            parts,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_simple_type_specifier(ctx: &C, specifier: Self) -> Self {
        let syntax = SyntaxVariant::SimpleTypeSpecifier(ctx.get_arena().alloc(SimpleTypeSpecifierChildren {
            specifier,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_literal_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::LiteralExpression(ctx.get_arena().alloc(LiteralExpressionChildren {
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefixed_string_expression(ctx: &C, name: Self, str: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedStringExpression(ctx.get_arena().alloc(PrefixedStringExpressionChildren {
            name,
            str,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefixed_code_expression(ctx: &C, prefix: Self, left_backtick: Self, expression: Self, right_backtick: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedCodeExpression(ctx.get_arena().alloc(PrefixedCodeExpressionChildren {
            prefix,
            left_backtick,
            expression,
            right_backtick,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_variable_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::VariableExpression(ctx.get_arena().alloc(VariableExpressionChildren {
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pipe_variable_expression(ctx: &C, expression: Self) -> Self {
        let syntax = SyntaxVariant::PipeVariableExpression(ctx.get_arena().alloc(PipeVariableExpressionChildren {
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_declaration(ctx: &C, attribute_spec: Self, keyword: Self, name: Self, colon: Self, base: Self, type_: Self, includes_keyword: Self, includes_list: Self, left_brace: Self, enumerators: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumDeclaration(ctx.get_arena().alloc(EnumDeclarationChildren {
            attribute_spec,
            keyword,
            name,
            colon,
            base,
            type_,
            includes_keyword,
            includes_list,
            left_brace,
            enumerators,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enumerator(ctx: &C, name: Self, equal: Self, value: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::Enumerator(ctx.get_arena().alloc(EnumeratorChildren {
            name,
            equal,
            value,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_class_declaration(ctx: &C, attribute_spec: Self, enum_keyword: Self, class_keyword: Self, name: Self, colon: Self, base: Self, extends: Self, extends_list: Self, left_brace: Self, elements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumClassDeclaration(ctx.get_arena().alloc(EnumClassDeclarationChildren {
            attribute_spec,
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_enum_class_enumerator(ctx: &C, name: Self, left_angle: Self, type_: Self, right_angle: Self, left_paren: Self, initial_value: Self, right_paren: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EnumClassEnumerator(ctx.get_arena().alloc(EnumClassEnumeratorChildren {
            name,
            left_angle,
            type_,
            right_angle,
            left_paren,
            initial_value,
            right_paren,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_record_declaration(ctx: &C, attribute_spec: Self, modifier: Self, keyword: Self, name: Self, extends_keyword: Self, extends_opt: Self, left_brace: Self, fields: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::RecordDeclaration(ctx.get_arena().alloc(RecordDeclarationChildren {
            attribute_spec,
            modifier,
            keyword,
            name,
            extends_keyword,
            extends_opt,
            left_brace,
            fields,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_record_field(ctx: &C, type_: Self, name: Self, init: Self, semi: Self) -> Self {
        let syntax = SyntaxVariant::RecordField(ctx.get_arena().alloc(RecordFieldChildren {
            type_,
            name,
            init,
            semi,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_alias_declaration(ctx: &C, attribute_spec: Self, keyword: Self, name: Self, generic_parameter: Self, constraint: Self, equal: Self, type_: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AliasDeclaration(ctx.get_arena().alloc(AliasDeclarationChildren {
            attribute_spec,
            keyword,
            name,
            generic_parameter,
            constraint,
            equal,
            type_,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_property_declarator(ctx: &C, name: Self, initializer: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclarator(ctx.get_arena().alloc(PropertyDeclaratorChildren {
            name,
            initializer,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_declaration(ctx: &C, header: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclaration(ctx.get_arena().alloc(NamespaceDeclarationChildren {
            header,
            body,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_declaration_header(ctx: &C, keyword: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclarationHeader(ctx.get_arena().alloc(NamespaceDeclarationHeaderChildren {
            keyword,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_body(ctx: &C, left_brace: Self, declarations: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceBody(ctx.get_arena().alloc(NamespaceBodyChildren {
            left_brace,
            declarations,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_empty_body(ctx: &C, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceEmptyBody(ctx.get_arena().alloc(NamespaceEmptyBodyChildren {
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_use_declaration(ctx: &C, keyword: Self, kind: Self, clauses: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseDeclaration(ctx.get_arena().alloc(NamespaceUseDeclarationChildren {
            keyword,
            kind,
            clauses,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_namespace_use_clause(ctx: &C, clause_kind: Self, name: Self, as_: Self, alias: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseClause(ctx.get_arena().alloc(NamespaceUseClauseChildren {
            clause_kind,
            name,
            as_,
            alias,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_declaration(ctx: &C, attribute_spec: Self, declaration_header: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclaration(ctx.get_arena().alloc(FunctionDeclarationChildren {
            attribute_spec,
            declaration_header,
            body,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_declaration_header(ctx: &C, modifiers: Self, keyword: Self, name: Self, type_parameter_list: Self, left_paren: Self, parameter_list: Self, right_paren: Self, capability: Self, capability_provisional: Self, colon: Self, type_: Self, where_clause: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclarationHeader(ctx.get_arena().alloc(FunctionDeclarationHeaderChildren {
            modifiers,
            keyword,
            name,
            type_parameter_list,
            left_paren,
            parameter_list,
            right_paren,
            capability,
            capability_provisional,
            colon,
            type_,
            where_clause,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_capability(ctx: &C, left_bracket: Self, types: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::Capability(ctx.get_arena().alloc(CapabilityChildren {
            left_bracket,
            types,
            right_bracket,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_capability_provisional(ctx: &C, at: Self, left_brace: Self, type_: Self, unsafe_plus: Self, unsafe_type: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CapabilityProvisional(ctx.get_arena().alloc(CapabilityProvisionalChildren {
            at,
            left_brace,
            type_,
            unsafe_plus,
            unsafe_type,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_where_clause(ctx: &C, keyword: Self, constraints: Self) -> Self {
        let syntax = SyntaxVariant::WhereClause(ctx.get_arena().alloc(WhereClauseChildren {
            keyword,
            constraints,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_where_constraint(ctx: &C, left_type: Self, operator: Self, right_type: Self) -> Self {
        let syntax = SyntaxVariant::WhereConstraint(ctx.get_arena().alloc(WhereConstraintChildren {
            left_type,
            operator,
            right_type,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_methodish_declaration(ctx: &C, attribute: Self, function_decl_header: Self, function_body: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishDeclaration(ctx.get_arena().alloc(MethodishDeclarationChildren {
            attribute,
            function_decl_header,
            function_body,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_classish_body(ctx: &C, left_brace: Self, elements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ClassishBody(ctx.get_arena().alloc(ClassishBodyChildren {
            left_brace,
            elements,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_trait_use_precedence_item(ctx: &C, name: Self, keyword: Self, removed_names: Self) -> Self {
        let syntax = SyntaxVariant::TraitUsePrecedenceItem(ctx.get_arena().alloc(TraitUsePrecedenceItemChildren {
            name,
            keyword,
            removed_names,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_trait_use_alias_item(ctx: &C, aliasing_name: Self, keyword: Self, modifiers: Self, aliased_name: Self) -> Self {
        let syntax = SyntaxVariant::TraitUseAliasItem(ctx.get_arena().alloc(TraitUseAliasItemChildren {
            aliasing_name,
            keyword,
            modifiers,
            aliased_name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_trait_use_conflict_resolution(ctx: &C, keyword: Self, names: Self, left_brace: Self, clauses: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::TraitUseConflictResolution(ctx.get_arena().alloc(TraitUseConflictResolutionChildren {
            keyword,
            names,
            left_brace,
            clauses,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_trait_use(ctx: &C, keyword: Self, names: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TraitUse(ctx.get_arena().alloc(TraitUseChildren {
            keyword,
            names,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_require_clause(ctx: &C, keyword: Self, kind: Self, name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::RequireClause(ctx.get_arena().alloc(RequireClauseChildren {
            keyword,
            kind,
            name,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_const_declaration(ctx: &C, modifiers: Self, keyword: Self, type_specifier: Self, declarators: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ConstDeclaration(ctx.get_arena().alloc(ConstDeclarationChildren {
            modifiers,
            keyword,
            type_specifier,
            declarators,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_constant_declarator(ctx: &C, name: Self, initializer: Self) -> Self {
        let syntax = SyntaxVariant::ConstantDeclarator(ctx.get_arena().alloc(ConstantDeclaratorChildren {
            name,
            initializer,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_const_declaration(ctx: &C, attribute_spec: Self, modifiers: Self, keyword: Self, type_keyword: Self, name: Self, type_parameters: Self, type_constraint: Self, equal: Self, type_specifier: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstDeclaration(ctx.get_arena().alloc(TypeConstDeclarationChildren {
            attribute_spec,
            modifiers,
            keyword,
            type_keyword,
            name,
            type_parameters,
            type_constraint,
            equal,
            type_specifier,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_decorated_expression(ctx: &C, decorator: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::DecoratedExpression(ctx.get_arena().alloc(DecoratedExpressionChildren {
            decorator,
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_parameter_declaration(ctx: &C, attribute: Self, visibility: Self, call_convention: Self, type_: Self, name: Self, default_value: Self) -> Self {
        let syntax = SyntaxVariant::ParameterDeclaration(ctx.get_arena().alloc(ParameterDeclarationChildren {
            attribute,
            visibility,
            call_convention,
            type_,
            name,
            default_value,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_variadic_parameter(ctx: &C, call_convention: Self, type_: Self, ellipsis: Self) -> Self {
        let syntax = SyntaxVariant::VariadicParameter(ctx.get_arena().alloc(VariadicParameterChildren {
            call_convention,
            type_,
            ellipsis,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_old_attribute_specification(ctx: &C, left_double_angle: Self, attributes: Self, right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::OldAttributeSpecification(ctx.get_arena().alloc(OldAttributeSpecificationChildren {
            left_double_angle,
            attributes,
            right_double_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attribute_specification(ctx: &C, attributes: Self) -> Self {
        let syntax = SyntaxVariant::AttributeSpecification(ctx.get_arena().alloc(AttributeSpecificationChildren {
            attributes,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attribute(ctx: &C, at: Self, attribute_name: Self) -> Self {
        let syntax = SyntaxVariant::Attribute(ctx.get_arena().alloc(AttributeChildren {
            at,
            attribute_name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_inclusion_expression(ctx: &C, require: Self, filename: Self) -> Self {
        let syntax = SyntaxVariant::InclusionExpression(ctx.get_arena().alloc(InclusionExpressionChildren {
            require,
            filename,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_inclusion_directive(ctx: &C, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::InclusionDirective(ctx.get_arena().alloc(InclusionDirectiveChildren {
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_compound_statement(ctx: &C, left_brace: Self, statements: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CompoundStatement(ctx.get_arena().alloc(CompoundStatementChildren {
            left_brace,
            statements,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_expression_statement(ctx: &C, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ExpressionStatement(ctx.get_arena().alloc(ExpressionStatementChildren {
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_markup_section(ctx: &C, hashbang: Self, suffix: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSection(ctx.get_arena().alloc(MarkupSectionChildren {
            hashbang,
            suffix,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_markup_suffix(ctx: &C, less_than_question: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSuffix(ctx.get_arena().alloc(MarkupSuffixChildren {
            less_than_question,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_using_statement_function_scoped(ctx: &C, await_keyword: Self, using_keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementFunctionScoped(ctx.get_arena().alloc(UsingStatementFunctionScopedChildren {
            await_keyword,
            using_keyword,
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_if_statement(ctx: &C, keyword: Self, left_paren: Self, condition: Self, right_paren: Self, statement: Self, elseif_clauses: Self, else_clause: Self) -> Self {
        let syntax = SyntaxVariant::IfStatement(ctx.get_arena().alloc(IfStatementChildren {
            keyword,
            left_paren,
            condition,
            right_paren,
            statement,
            elseif_clauses,
            else_clause,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_elseif_clause(ctx: &C, keyword: Self, left_paren: Self, condition: Self, right_paren: Self, statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseifClause(ctx.get_arena().alloc(ElseifClauseChildren {
            keyword,
            left_paren,
            condition,
            right_paren,
            statement,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_else_clause(ctx: &C, keyword: Self, statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseClause(ctx.get_arena().alloc(ElseClauseChildren {
            keyword,
            statement,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_try_statement(ctx: &C, keyword: Self, compound_statement: Self, catch_clauses: Self, finally_clause: Self) -> Self {
        let syntax = SyntaxVariant::TryStatement(ctx.get_arena().alloc(TryStatementChildren {
            keyword,
            compound_statement,
            catch_clauses,
            finally_clause,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_finally_clause(ctx: &C, keyword: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::FinallyClause(ctx.get_arena().alloc(FinallyClauseChildren {
            keyword,
            body,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_switch_section(ctx: &C, labels: Self, statements: Self, fallthrough: Self) -> Self {
        let syntax = SyntaxVariant::SwitchSection(ctx.get_arena().alloc(SwitchSectionChildren {
            labels,
            statements,
            fallthrough,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_switch_fallthrough(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::SwitchFallthrough(ctx.get_arena().alloc(SwitchFallthroughChildren {
            keyword,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_case_label(ctx: &C, keyword: Self, expression: Self, colon: Self) -> Self {
        let syntax = SyntaxVariant::CaseLabel(ctx.get_arena().alloc(CaseLabelChildren {
            keyword,
            expression,
            colon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_default_label(ctx: &C, keyword: Self, colon: Self) -> Self {
        let syntax = SyntaxVariant::DefaultLabel(ctx.get_arena().alloc(DefaultLabelChildren {
            keyword,
            colon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_return_statement(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ReturnStatement(ctx.get_arena().alloc(ReturnStatementChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_goto_label(ctx: &C, name: Self, colon: Self) -> Self {
        let syntax = SyntaxVariant::GotoLabel(ctx.get_arena().alloc(GotoLabelChildren {
            name,
            colon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_goto_statement(ctx: &C, keyword: Self, label_name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::GotoStatement(ctx.get_arena().alloc(GotoStatementChildren {
            keyword,
            label_name,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_throw_statement(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ThrowStatement(ctx.get_arena().alloc(ThrowStatementChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_break_statement(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::BreakStatement(ctx.get_arena().alloc(BreakStatementChildren {
            keyword,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_continue_statement(ctx: &C, keyword: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContinueStatement(ctx.get_arena().alloc(ContinueStatementChildren {
            keyword,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_echo_statement(ctx: &C, keyword: Self, expressions: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EchoStatement(ctx.get_arena().alloc(EchoStatementChildren {
            keyword,
            expressions,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_concurrent_statement(ctx: &C, keyword: Self, statement: Self) -> Self {
        let syntax = SyntaxVariant::ConcurrentStatement(ctx.get_arena().alloc(ConcurrentStatementChildren {
            keyword,
            statement,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_simple_initializer(ctx: &C, equal: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::SimpleInitializer(ctx.get_arena().alloc(SimpleInitializerChildren {
            equal,
            value,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_anonymous_function(ctx: &C, attribute_spec: Self, static_keyword: Self, async_keyword: Self, function_keyword: Self, left_paren: Self, parameters: Self, right_paren: Self, colon: Self, type_: Self, use_: Self, body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunction(ctx.get_arena().alloc(AnonymousFunctionChildren {
            attribute_spec,
            static_keyword,
            async_keyword,
            function_keyword,
            left_paren,
            parameters,
            right_paren,
            colon,
            type_,
            use_,
            body,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_anonymous_function_use_clause(ctx: &C, keyword: Self, left_paren: Self, variables: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunctionUseClause(ctx.get_arena().alloc(AnonymousFunctionUseClauseChildren {
            keyword,
            left_paren,
            variables,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_lambda_signature(ctx: &C, left_paren: Self, parameters: Self, right_paren: Self, capability: Self, colon: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::LambdaSignature(ctx.get_arena().alloc(LambdaSignatureChildren {
            left_paren,
            parameters,
            right_paren,
            capability,
            colon,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_cast_expression(ctx: &C, left_paren: Self, type_: Self, right_paren: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::CastExpression(ctx.get_arena().alloc(CastExpressionChildren {
            left_paren,
            type_,
            right_paren,
            operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_scope_resolution_expression(ctx: &C, qualifier: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::ScopeResolutionExpression(ctx.get_arena().alloc(ScopeResolutionExpressionChildren {
            qualifier,
            operator,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::MemberSelectionExpression(ctx.get_arena().alloc(MemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_safe_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::SafeMemberSelectionExpression(ctx.get_arena().alloc(SafeMemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_member_selection_expression(ctx: &C, object: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedMemberSelectionExpression(ctx.get_arena().alloc(EmbeddedMemberSelectionExpressionChildren {
            object,
            operator,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_yield_expression(ctx: &C, keyword: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::YieldExpression(ctx.get_arena().alloc(YieldExpressionChildren {
            keyword,
            operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_prefix_unary_expression(ctx: &C, operator: Self, operand: Self) -> Self {
        let syntax = SyntaxVariant::PrefixUnaryExpression(ctx.get_arena().alloc(PrefixUnaryExpressionChildren {
            operator,
            operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_postfix_unary_expression(ctx: &C, operand: Self, operator: Self) -> Self {
        let syntax = SyntaxVariant::PostfixUnaryExpression(ctx.get_arena().alloc(PostfixUnaryExpressionChildren {
            operand,
            operator,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_binary_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::BinaryExpression(ctx.get_arena().alloc(BinaryExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_is_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::IsExpression(ctx.get_arena().alloc(IsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_as_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::AsExpression(ctx.get_arena().alloc(AsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_nullable_as_expression(ctx: &C, left_operand: Self, operator: Self, right_operand: Self) -> Self {
        let syntax = SyntaxVariant::NullableAsExpression(ctx.get_arena().alloc(NullableAsExpressionChildren {
            left_operand,
            operator,
            right_operand,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_eval_expression(ctx: &C, keyword: Self, left_paren: Self, argument: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::EvalExpression(ctx.get_arena().alloc(EvalExpressionChildren {
            keyword,
            left_paren,
            argument,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_define_expression(ctx: &C, keyword: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::DefineExpression(ctx.get_arena().alloc(DefineExpressionChildren {
            keyword,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_isset_expression(ctx: &C, keyword: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IssetExpression(ctx.get_arena().alloc(IssetExpressionChildren {
            keyword,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_function_pointer_expression(ctx: &C, receiver: Self, type_args: Self) -> Self {
        let syntax = SyntaxVariant::FunctionPointerExpression(ctx.get_arena().alloc(FunctionPointerExpressionChildren {
            receiver,
            type_args,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_parenthesized_expression(ctx: &C, left_paren: Self, expression: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ParenthesizedExpression(ctx.get_arena().alloc(ParenthesizedExpressionChildren {
            left_paren,
            expression,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_braced_expression(ctx: &C, left_brace: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::BracedExpression(ctx.get_arena().alloc(BracedExpressionChildren {
            left_brace,
            expression,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_braced_expression(ctx: &C, left_brace: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedBracedExpression(ctx.get_arena().alloc(EmbeddedBracedExpressionChildren {
            left_brace,
            expression,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_list_expression(ctx: &C, keyword: Self, left_paren: Self, members: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ListExpression(ctx.get_arena().alloc(ListExpressionChildren {
            keyword,
            left_paren,
            members,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_collection_literal_expression(ctx: &C, name: Self, left_brace: Self, initializers: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CollectionLiteralExpression(ctx.get_arena().alloc(CollectionLiteralExpressionChildren {
            name,
            left_brace,
            initializers,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_object_creation_expression(ctx: &C, new_keyword: Self, object: Self) -> Self {
        let syntax = SyntaxVariant::ObjectCreationExpression(ctx.get_arena().alloc(ObjectCreationExpressionChildren {
            new_keyword,
            object,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_constructor_call(ctx: &C, type_: Self, left_paren: Self, argument_list: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ConstructorCall(ctx.get_arena().alloc(ConstructorCallChildren {
            type_,
            left_paren,
            argument_list,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_record_creation_expression(ctx: &C, type_: Self, left_bracket: Self, members: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::RecordCreationExpression(ctx.get_arena().alloc(RecordCreationExpressionChildren {
            type_,
            left_bracket,
            members,
            right_bracket,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_element_initializer(ctx: &C, key: Self, arrow: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::ElementInitializer(ctx.get_arena().alloc(ElementInitializerChildren {
            key,
            arrow,
            value,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_subscript_expression(ctx: &C, receiver: Self, left_bracket: Self, index: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::SubscriptExpression(ctx.get_arena().alloc(SubscriptExpressionChildren {
            receiver,
            left_bracket,
            index,
            right_bracket,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_embedded_subscript_expression(ctx: &C, receiver: Self, left_bracket: Self, index: Self, right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedSubscriptExpression(ctx.get_arena().alloc(EmbeddedSubscriptExpressionChildren {
            receiver,
            left_bracket,
            index,
            right_bracket,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_awaitable_creation_expression(ctx: &C, attribute_spec: Self, async_: Self, compound_statement: Self) -> Self {
        let syntax = SyntaxVariant::AwaitableCreationExpression(ctx.get_arena().alloc(AwaitableCreationExpressionChildren {
            attribute_spec,
            async_,
            compound_statement,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_children_declaration(ctx: &C, keyword: Self, expression: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenDeclaration(ctx.get_arena().alloc(XHPChildrenDeclarationChildren {
            keyword,
            expression,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_children_parenthesized_list(ctx: &C, left_paren: Self, xhp_children: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenParenthesizedList(ctx.get_arena().alloc(XHPChildrenParenthesizedListChildren {
            left_paren,
            xhp_children,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_category_declaration(ctx: &C, keyword: Self, categories: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPCategoryDeclaration(ctx.get_arena().alloc(XHPCategoryDeclarationChildren {
            keyword,
            categories,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_enum_type(ctx: &C, keyword: Self, left_brace: Self, values: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPEnumType(ctx.get_arena().alloc(XHPEnumTypeChildren {
            keyword,
            left_brace,
            values,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_lateinit(ctx: &C, at: Self, keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPLateinit(ctx.get_arena().alloc(XHPLateinitChildren {
            at,
            keyword,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_required(ctx: &C, at: Self, keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPRequired(ctx.get_arena().alloc(XHPRequiredChildren {
            at,
            keyword,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute_declaration(ctx: &C, keyword: Self, attributes: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttributeDeclaration(ctx.get_arena().alloc(XHPClassAttributeDeclarationChildren {
            keyword,
            attributes,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute(ctx: &C, type_: Self, name: Self, initializer: Self, required: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttribute(ctx.get_arena().alloc(XHPClassAttributeChildren {
            type_,
            name,
            initializer,
            required,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_simple_class_attribute(ctx: &C, type_: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleClassAttribute(ctx.get_arena().alloc(XHPSimpleClassAttributeChildren {
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_simple_attribute(ctx: &C, name: Self, equal: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleAttribute(ctx.get_arena().alloc(XHPSimpleAttributeChildren {
            name,
            equal,
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_spread_attribute(ctx: &C, left_brace: Self, spread_operator: Self, expression: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPSpreadAttribute(ctx.get_arena().alloc(XHPSpreadAttributeChildren {
            left_brace,
            spread_operator,
            expression,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_open(ctx: &C, left_angle: Self, name: Self, attributes: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPOpen(ctx.get_arena().alloc(XHPOpenChildren {
            left_angle,
            name,
            attributes,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_expression(ctx: &C, open: Self, body: Self, close: Self) -> Self {
        let syntax = SyntaxVariant::XHPExpression(ctx.get_arena().alloc(XHPExpressionChildren {
            open,
            body,
            close,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_xhp_close(ctx: &C, left_angle: Self, name: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPClose(ctx.get_arena().alloc(XHPCloseChildren {
            left_angle,
            name,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_constant(ctx: &C, left_type: Self, separator: Self, right_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstant(ctx.get_arena().alloc(TypeConstantChildren {
            left_type,
            separator,
            right_type,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pu_access(ctx: &C, left_type: Self, separator: Self, right_type: Self) -> Self {
        let syntax = SyntaxVariant::PUAccess(ctx.get_arena().alloc(PUAccessChildren {
            left_type,
            separator,
            right_type,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_type_explicit_specifier(ctx: &C, keyword: Self, left_angle: Self, types: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeExplicitSpecifier(ctx.get_arena().alloc(TupleTypeExplicitSpecifierChildren {
            keyword,
            left_angle,
            types,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_constraint(ctx: &C, keyword: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstraint(ctx.get_arena().alloc(TypeConstraintChildren {
            keyword,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_dictionary_type_specifier(ctx: &C, keyword: Self, left_angle: Self, members: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryTypeSpecifier(ctx.get_arena().alloc(DictionaryTypeSpecifierChildren {
            keyword,
            left_angle,
            members,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_closure_type_specifier(ctx: &C, outer_left_paren: Self, function_keyword: Self, inner_left_paren: Self, parameter_list: Self, inner_right_paren: Self, capability: Self, colon: Self, return_type: Self, outer_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ClosureTypeSpecifier(ctx.get_arena().alloc(ClosureTypeSpecifierChildren {
            outer_left_paren,
            function_keyword,
            inner_left_paren,
            parameter_list,
            inner_right_paren,
            capability,
            colon,
            return_type,
            outer_right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_closure_parameter_type_specifier(ctx: &C, call_convention: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::ClosureParameterTypeSpecifier(ctx.get_arena().alloc(ClosureParameterTypeSpecifierChildren {
            call_convention,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_field_specifier(ctx: &C, question: Self, name: Self, arrow: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::FieldSpecifier(ctx.get_arena().alloc(FieldSpecifierChildren {
            question,
            name,
            arrow,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_field_initializer(ctx: &C, name: Self, arrow: Self, value: Self) -> Self {
        let syntax = SyntaxVariant::FieldInitializer(ctx.get_arena().alloc(FieldInitializerChildren {
            name,
            arrow,
            value,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
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
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_shape_expression(ctx: &C, keyword: Self, left_paren: Self, fields: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeExpression(ctx.get_arena().alloc(ShapeExpressionChildren {
            keyword,
            left_paren,
            fields,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_expression(ctx: &C, keyword: Self, left_paren: Self, items: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleExpression(ctx.get_arena().alloc(TupleExpressionChildren {
            keyword,
            left_paren,
            items,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_generic_type_specifier(ctx: &C, class_type: Self, argument_list: Self) -> Self {
        let syntax = SyntaxVariant::GenericTypeSpecifier(ctx.get_arena().alloc(GenericTypeSpecifierChildren {
            class_type,
            argument_list,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_nullable_type_specifier(ctx: &C, question: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::NullableTypeSpecifier(ctx.get_arena().alloc(NullableTypeSpecifierChildren {
            question,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_like_type_specifier(ctx: &C, tilde: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::LikeTypeSpecifier(ctx.get_arena().alloc(LikeTypeSpecifierChildren {
            tilde,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_soft_type_specifier(ctx: &C, at: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::SoftTypeSpecifier(ctx.get_arena().alloc(SoftTypeSpecifierChildren {
            at,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_attributized_specifier(ctx: &C, attribute_spec: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::AttributizedSpecifier(ctx.get_arena().alloc(AttributizedSpecifierChildren {
            attribute_spec,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_reified_type_argument(ctx: &C, reified: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::ReifiedTypeArgument(ctx.get_arena().alloc(ReifiedTypeArgumentChildren {
            reified,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_arguments(ctx: &C, left_angle: Self, types: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeArguments(ctx.get_arena().alloc(TypeArgumentsChildren {
            left_angle,
            types,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_type_parameters(ctx: &C, left_angle: Self, parameters: Self, right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameters(ctx.get_arena().alloc(TypeParametersChildren {
            left_angle,
            parameters,
            right_angle,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_tuple_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeSpecifier(ctx.get_arena().alloc(TupleTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_union_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::UnionTypeSpecifier(ctx.get_arena().alloc(UnionTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_intersection_type_specifier(ctx: &C, left_paren: Self, types: Self, right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IntersectionTypeSpecifier(ctx.get_arena().alloc(IntersectionTypeSpecifierChildren {
            left_paren,
            types,
            right_paren,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_error(ctx: &C, error: Self) -> Self {
        let syntax = SyntaxVariant::ErrorSyntax(ctx.get_arena().alloc(ErrorSyntaxChildren {
            error,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_list_item(ctx: &C, item: Self, separator: Self) -> Self {
        let syntax = SyntaxVariant::ListItem(ctx.get_arena().alloc(ListItemChildren {
            item,
            separator,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_atom_expression(ctx: &C, glyph: Self, expression: Self) -> Self {
        let syntax = SyntaxVariant::PocketAtomExpression(ctx.get_arena().alloc(PocketAtomExpressionChildren {
            glyph,
            expression,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_identifier_expression(ctx: &C, qualifier: Self, pu_operator: Self, field: Self, operator: Self, name: Self) -> Self {
        let syntax = SyntaxVariant::PocketIdentifierExpression(ctx.get_arena().alloc(PocketIdentifierExpressionChildren {
            qualifier,
            pu_operator,
            field,
            operator,
            name,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_atom_mapping_declaration(ctx: &C, glyph: Self, name: Self, left_paren: Self, mappings: Self, right_paren: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketAtomMappingDeclaration(ctx.get_arena().alloc(PocketAtomMappingDeclarationChildren {
            glyph,
            name,
            left_paren,
            mappings,
            right_paren,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_enum_declaration(ctx: &C, attributes: Self, modifiers: Self, enum_: Self, name: Self, left_brace: Self, fields: Self, right_brace: Self) -> Self {
        let syntax = SyntaxVariant::PocketEnumDeclaration(ctx.get_arena().alloc(PocketEnumDeclarationChildren {
            attributes,
            modifiers,
            enum_,
            name,
            left_brace,
            fields,
            right_brace,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_field_type_expr_declaration(ctx: &C, case: Self, type_: Self, name: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeExprDeclaration(ctx.get_arena().alloc(PocketFieldTypeExprDeclarationChildren {
            case,
            type_,
            name,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_field_type_declaration(ctx: &C, case: Self, type_: Self, type_parameter: Self, semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeDeclaration(ctx.get_arena().alloc(PocketFieldTypeDeclarationChildren {
            case,
            type_,
            type_parameter,
            semicolon,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_id_declaration(ctx: &C, name: Self, initializer: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingIdDeclaration(ctx.get_arena().alloc(PocketMappingIdDeclarationChildren {
            name,
            initializer,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_type_declaration(ctx: &C, keyword: Self, name: Self, equal: Self, type_: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingTypeDeclaration(ctx.get_arena().alloc(PocketMappingTypeDeclarationChildren {
            keyword,
            name,
            equal,
            type_,
        }));
        let value = PositionedValue::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }

 }
