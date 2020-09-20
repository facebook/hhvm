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
use crate::lexable_token::LexableToken;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;

impl<T, V, C> SyntaxType<C> for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    fn make_end_of_file(_: &C, end_of_file_token: Self) -> Self {
        let syntax = SyntaxVariant::EndOfFile(Box::new(EndOfFileChildren {
            end_of_file_token,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_script(_: &C, script_declarations: Self) -> Self {
        let syntax = SyntaxVariant::Script(Box::new(ScriptChildren {
            script_declarations,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_qualified_name(_: &C, qualified_name_parts: Self) -> Self {
        let syntax = SyntaxVariant::QualifiedName(Box::new(QualifiedNameChildren {
            qualified_name_parts,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_simple_type_specifier(_: &C, simple_type_specifier: Self) -> Self {
        let syntax = SyntaxVariant::SimpleTypeSpecifier(Box::new(SimpleTypeSpecifierChildren {
            simple_type_specifier,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_literal_expression(_: &C, literal_expression: Self) -> Self {
        let syntax = SyntaxVariant::LiteralExpression(Box::new(LiteralExpressionChildren {
            literal_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_prefixed_string_expression(_: &C, prefixed_string_name: Self, prefixed_string_str: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedStringExpression(Box::new(PrefixedStringExpressionChildren {
            prefixed_string_name,
            prefixed_string_str,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_prefixed_code_expression(_: &C, prefixed_code_prefix: Self, prefixed_code_left_backtick: Self, prefixed_code_expression: Self, prefixed_code_right_backtick: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedCodeExpression(Box::new(PrefixedCodeExpressionChildren {
            prefixed_code_prefix,
            prefixed_code_left_backtick,
            prefixed_code_expression,
            prefixed_code_right_backtick,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_variable_expression(_: &C, variable_expression: Self) -> Self {
        let syntax = SyntaxVariant::VariableExpression(Box::new(VariableExpressionChildren {
            variable_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pipe_variable_expression(_: &C, pipe_variable_expression: Self) -> Self {
        let syntax = SyntaxVariant::PipeVariableExpression(Box::new(PipeVariableExpressionChildren {
            pipe_variable_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_file_attribute_specification(_: &C, file_attribute_specification_left_double_angle: Self, file_attribute_specification_keyword: Self, file_attribute_specification_colon: Self, file_attribute_specification_attributes: Self, file_attribute_specification_right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::FileAttributeSpecification(Box::new(FileAttributeSpecificationChildren {
            file_attribute_specification_left_double_angle,
            file_attribute_specification_keyword,
            file_attribute_specification_colon,
            file_attribute_specification_attributes,
            file_attribute_specification_right_double_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_enum_declaration(_: &C, enum_attribute_spec: Self, enum_keyword: Self, enum_name: Self, enum_colon: Self, enum_base: Self, enum_type: Self, enum_includes_keyword: Self, enum_includes_list: Self, enum_left_brace: Self, enum_enumerators: Self, enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumDeclaration(Box::new(EnumDeclarationChildren {
            enum_attribute_spec,
            enum_keyword,
            enum_name,
            enum_colon,
            enum_base,
            enum_type,
            enum_includes_keyword,
            enum_includes_list,
            enum_left_brace,
            enum_enumerators,
            enum_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_enumerator(_: &C, enumerator_name: Self, enumerator_equal: Self, enumerator_value: Self, enumerator_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::Enumerator(Box::new(EnumeratorChildren {
            enumerator_name,
            enumerator_equal,
            enumerator_value,
            enumerator_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_declaration(_: &C, record_attribute_spec: Self, record_modifier: Self, record_keyword: Self, record_name: Self, record_extends_keyword: Self, record_extends_opt: Self, record_left_brace: Self, record_fields: Self, record_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::RecordDeclaration(Box::new(RecordDeclarationChildren {
            record_attribute_spec,
            record_modifier,
            record_keyword,
            record_name,
            record_extends_keyword,
            record_extends_opt,
            record_left_brace,
            record_fields,
            record_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_field(_: &C, record_field_type: Self, record_field_name: Self, record_field_init: Self, record_field_semi: Self) -> Self {
        let syntax = SyntaxVariant::RecordField(Box::new(RecordFieldChildren {
            record_field_type,
            record_field_name,
            record_field_init,
            record_field_semi,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alias_declaration(_: &C, alias_attribute_spec: Self, alias_keyword: Self, alias_name: Self, alias_generic_parameter: Self, alias_constraint: Self, alias_equal: Self, alias_type: Self, alias_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AliasDeclaration(Box::new(AliasDeclarationChildren {
            alias_attribute_spec,
            alias_keyword,
            alias_name,
            alias_generic_parameter,
            alias_constraint,
            alias_equal,
            alias_type,
            alias_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_property_declaration(_: &C, property_attribute_spec: Self, property_modifiers: Self, property_type: Self, property_declarators: Self, property_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclaration(Box::new(PropertyDeclarationChildren {
            property_attribute_spec,
            property_modifiers,
            property_type,
            property_declarators,
            property_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_property_declarator(_: &C, property_name: Self, property_initializer: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclarator(Box::new(PropertyDeclaratorChildren {
            property_name,
            property_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_declaration(_: &C, namespace_header: Self, namespace_body: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclaration(Box::new(NamespaceDeclarationChildren {
            namespace_header,
            namespace_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_declaration_header(_: &C, namespace_keyword: Self, namespace_name: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclarationHeader(Box::new(NamespaceDeclarationHeaderChildren {
            namespace_keyword,
            namespace_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_body(_: &C, namespace_left_brace: Self, namespace_declarations: Self, namespace_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceBody(Box::new(NamespaceBodyChildren {
            namespace_left_brace,
            namespace_declarations,
            namespace_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_empty_body(_: &C, namespace_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceEmptyBody(Box::new(NamespaceEmptyBodyChildren {
            namespace_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_use_declaration(_: &C, namespace_use_keyword: Self, namespace_use_kind: Self, namespace_use_clauses: Self, namespace_use_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseDeclaration(Box::new(NamespaceUseDeclarationChildren {
            namespace_use_keyword,
            namespace_use_kind,
            namespace_use_clauses,
            namespace_use_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_group_use_declaration(_: &C, namespace_group_use_keyword: Self, namespace_group_use_kind: Self, namespace_group_use_prefix: Self, namespace_group_use_left_brace: Self, namespace_group_use_clauses: Self, namespace_group_use_right_brace: Self, namespace_group_use_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceGroupUseDeclaration(Box::new(NamespaceGroupUseDeclarationChildren {
            namespace_group_use_keyword,
            namespace_group_use_kind,
            namespace_group_use_prefix,
            namespace_group_use_left_brace,
            namespace_group_use_clauses,
            namespace_group_use_right_brace,
            namespace_group_use_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_use_clause(_: &C, namespace_use_clause_kind: Self, namespace_use_name: Self, namespace_use_as: Self, namespace_use_alias: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseClause(Box::new(NamespaceUseClauseChildren {
            namespace_use_clause_kind,
            namespace_use_name,
            namespace_use_as,
            namespace_use_alias,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_declaration(_: &C, function_attribute_spec: Self, function_declaration_header: Self, function_body: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclaration(Box::new(FunctionDeclarationChildren {
            function_attribute_spec,
            function_declaration_header,
            function_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_declaration_header(_: &C, function_modifiers: Self, function_keyword: Self, function_name: Self, function_type_parameter_list: Self, function_left_paren: Self, function_parameter_list: Self, function_right_paren: Self, function_capability_provisional: Self, function_colon: Self, function_type: Self, function_where_clause: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclarationHeader(Box::new(FunctionDeclarationHeaderChildren {
            function_modifiers,
            function_keyword,
            function_name,
            function_type_parameter_list,
            function_left_paren,
            function_parameter_list,
            function_right_paren,
            function_capability_provisional,
            function_colon,
            function_type,
            function_where_clause,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_capability_provisional(_: &C, capability_provisional_at: Self, capability_provisional_left_brace: Self, capability_provisional_type: Self, capability_provisional_unsafe_plus: Self, capability_provisional_unsafe_type: Self, capability_provisional_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CapabilityProvisional(Box::new(CapabilityProvisionalChildren {
            capability_provisional_at,
            capability_provisional_left_brace,
            capability_provisional_type,
            capability_provisional_unsafe_plus,
            capability_provisional_unsafe_type,
            capability_provisional_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_where_clause(_: &C, where_clause_keyword: Self, where_clause_constraints: Self) -> Self {
        let syntax = SyntaxVariant::WhereClause(Box::new(WhereClauseChildren {
            where_clause_keyword,
            where_clause_constraints,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_where_constraint(_: &C, where_constraint_left_type: Self, where_constraint_operator: Self, where_constraint_right_type: Self) -> Self {
        let syntax = SyntaxVariant::WhereConstraint(Box::new(WhereConstraintChildren {
            where_constraint_left_type,
            where_constraint_operator,
            where_constraint_right_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_methodish_declaration(_: &C, methodish_attribute: Self, methodish_function_decl_header: Self, methodish_function_body: Self, methodish_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishDeclaration(Box::new(MethodishDeclarationChildren {
            methodish_attribute,
            methodish_function_decl_header,
            methodish_function_body,
            methodish_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_methodish_trait_resolution(_: &C, methodish_trait_attribute: Self, methodish_trait_function_decl_header: Self, methodish_trait_equal: Self, methodish_trait_name: Self, methodish_trait_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishTraitResolution(Box::new(MethodishTraitResolutionChildren {
            methodish_trait_attribute,
            methodish_trait_function_decl_header,
            methodish_trait_equal,
            methodish_trait_name,
            methodish_trait_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_classish_declaration(_: &C, classish_attribute: Self, classish_modifiers: Self, classish_xhp: Self, classish_keyword: Self, classish_name: Self, classish_type_parameters: Self, classish_extends_keyword: Self, classish_extends_list: Self, classish_implements_keyword: Self, classish_implements_list: Self, classish_where_clause: Self, classish_body: Self) -> Self {
        let syntax = SyntaxVariant::ClassishDeclaration(Box::new(ClassishDeclarationChildren {
            classish_attribute,
            classish_modifiers,
            classish_xhp,
            classish_keyword,
            classish_name,
            classish_type_parameters,
            classish_extends_keyword,
            classish_extends_list,
            classish_implements_keyword,
            classish_implements_list,
            classish_where_clause,
            classish_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_classish_body(_: &C, classish_body_left_brace: Self, classish_body_elements: Self, classish_body_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ClassishBody(Box::new(ClassishBodyChildren {
            classish_body_left_brace,
            classish_body_elements,
            classish_body_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_precedence_item(_: &C, trait_use_precedence_item_name: Self, trait_use_precedence_item_keyword: Self, trait_use_precedence_item_removed_names: Self) -> Self {
        let syntax = SyntaxVariant::TraitUsePrecedenceItem(Box::new(TraitUsePrecedenceItemChildren {
            trait_use_precedence_item_name,
            trait_use_precedence_item_keyword,
            trait_use_precedence_item_removed_names,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_alias_item(_: &C, trait_use_alias_item_aliasing_name: Self, trait_use_alias_item_keyword: Self, trait_use_alias_item_modifiers: Self, trait_use_alias_item_aliased_name: Self) -> Self {
        let syntax = SyntaxVariant::TraitUseAliasItem(Box::new(TraitUseAliasItemChildren {
            trait_use_alias_item_aliasing_name,
            trait_use_alias_item_keyword,
            trait_use_alias_item_modifiers,
            trait_use_alias_item_aliased_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_conflict_resolution(_: &C, trait_use_conflict_resolution_keyword: Self, trait_use_conflict_resolution_names: Self, trait_use_conflict_resolution_left_brace: Self, trait_use_conflict_resolution_clauses: Self, trait_use_conflict_resolution_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::TraitUseConflictResolution(Box::new(TraitUseConflictResolutionChildren {
            trait_use_conflict_resolution_keyword,
            trait_use_conflict_resolution_names,
            trait_use_conflict_resolution_left_brace,
            trait_use_conflict_resolution_clauses,
            trait_use_conflict_resolution_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use(_: &C, trait_use_keyword: Self, trait_use_names: Self, trait_use_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TraitUse(Box::new(TraitUseChildren {
            trait_use_keyword,
            trait_use_names,
            trait_use_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_require_clause(_: &C, require_keyword: Self, require_kind: Self, require_name: Self, require_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::RequireClause(Box::new(RequireClauseChildren {
            require_keyword,
            require_kind,
            require_name,
            require_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_const_declaration(_: &C, const_modifiers: Self, const_keyword: Self, const_type_specifier: Self, const_declarators: Self, const_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ConstDeclaration(Box::new(ConstDeclarationChildren {
            const_modifiers,
            const_keyword,
            const_type_specifier,
            const_declarators,
            const_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_constant_declarator(_: &C, constant_declarator_name: Self, constant_declarator_initializer: Self) -> Self {
        let syntax = SyntaxVariant::ConstantDeclarator(Box::new(ConstantDeclaratorChildren {
            constant_declarator_name,
            constant_declarator_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_const_declaration(_: &C, type_const_attribute_spec: Self, type_const_modifiers: Self, type_const_keyword: Self, type_const_type_keyword: Self, type_const_name: Self, type_const_type_parameters: Self, type_const_type_constraint: Self, type_const_equal: Self, type_const_type_specifier: Self, type_const_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstDeclaration(Box::new(TypeConstDeclarationChildren {
            type_const_attribute_spec,
            type_const_modifiers,
            type_const_keyword,
            type_const_type_keyword,
            type_const_name,
            type_const_type_parameters,
            type_const_type_constraint,
            type_const_equal,
            type_const_type_specifier,
            type_const_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_decorated_expression(_: &C, decorated_expression_decorator: Self, decorated_expression_expression: Self) -> Self {
        let syntax = SyntaxVariant::DecoratedExpression(Box::new(DecoratedExpressionChildren {
            decorated_expression_decorator,
            decorated_expression_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_parameter_declaration(_: &C, parameter_attribute: Self, parameter_visibility: Self, parameter_call_convention: Self, parameter_type: Self, parameter_name: Self, parameter_default_value: Self) -> Self {
        let syntax = SyntaxVariant::ParameterDeclaration(Box::new(ParameterDeclarationChildren {
            parameter_attribute,
            parameter_visibility,
            parameter_call_convention,
            parameter_type,
            parameter_name,
            parameter_default_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_variadic_parameter(_: &C, variadic_parameter_call_convention: Self, variadic_parameter_type: Self, variadic_parameter_ellipsis: Self) -> Self {
        let syntax = SyntaxVariant::VariadicParameter(Box::new(VariadicParameterChildren {
            variadic_parameter_call_convention,
            variadic_parameter_type,
            variadic_parameter_ellipsis,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_old_attribute_specification(_: &C, old_attribute_specification_left_double_angle: Self, old_attribute_specification_attributes: Self, old_attribute_specification_right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::OldAttributeSpecification(Box::new(OldAttributeSpecificationChildren {
            old_attribute_specification_left_double_angle,
            old_attribute_specification_attributes,
            old_attribute_specification_right_double_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_attribute_specification(_: &C, attribute_specification_attributes: Self) -> Self {
        let syntax = SyntaxVariant::AttributeSpecification(Box::new(AttributeSpecificationChildren {
            attribute_specification_attributes,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_attribute(_: &C, attribute_at: Self, attribute_attribute_name: Self) -> Self {
        let syntax = SyntaxVariant::Attribute(Box::new(AttributeChildren {
            attribute_at,
            attribute_attribute_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_inclusion_expression(_: &C, inclusion_require: Self, inclusion_filename: Self) -> Self {
        let syntax = SyntaxVariant::InclusionExpression(Box::new(InclusionExpressionChildren {
            inclusion_require,
            inclusion_filename,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_inclusion_directive(_: &C, inclusion_expression: Self, inclusion_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::InclusionDirective(Box::new(InclusionDirectiveChildren {
            inclusion_expression,
            inclusion_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_compound_statement(_: &C, compound_left_brace: Self, compound_statements: Self, compound_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CompoundStatement(Box::new(CompoundStatementChildren {
            compound_left_brace,
            compound_statements,
            compound_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_expression_statement(_: &C, expression_statement_expression: Self, expression_statement_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ExpressionStatement(Box::new(ExpressionStatementChildren {
            expression_statement_expression,
            expression_statement_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_markup_section(_: &C, markup_text: Self, markup_suffix: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSection(Box::new(MarkupSectionChildren {
            markup_text,
            markup_suffix,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_markup_suffix(_: &C, markup_suffix_less_than_question: Self, markup_suffix_name: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSuffix(Box::new(MarkupSuffixChildren {
            markup_suffix_less_than_question,
            markup_suffix_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_unset_statement(_: &C, unset_keyword: Self, unset_left_paren: Self, unset_variables: Self, unset_right_paren: Self, unset_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UnsetStatement(Box::new(UnsetStatementChildren {
            unset_keyword,
            unset_left_paren,
            unset_variables,
            unset_right_paren,
            unset_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_using_statement_block_scoped(_: &C, using_block_await_keyword: Self, using_block_using_keyword: Self, using_block_left_paren: Self, using_block_expressions: Self, using_block_right_paren: Self, using_block_body: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementBlockScoped(Box::new(UsingStatementBlockScopedChildren {
            using_block_await_keyword,
            using_block_using_keyword,
            using_block_left_paren,
            using_block_expressions,
            using_block_right_paren,
            using_block_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_using_statement_function_scoped(_: &C, using_function_await_keyword: Self, using_function_using_keyword: Self, using_function_expression: Self, using_function_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementFunctionScoped(Box::new(UsingStatementFunctionScopedChildren {
            using_function_await_keyword,
            using_function_using_keyword,
            using_function_expression,
            using_function_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_while_statement(_: &C, while_keyword: Self, while_left_paren: Self, while_condition: Self, while_right_paren: Self, while_body: Self) -> Self {
        let syntax = SyntaxVariant::WhileStatement(Box::new(WhileStatementChildren {
            while_keyword,
            while_left_paren,
            while_condition,
            while_right_paren,
            while_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_if_statement(_: &C, if_keyword: Self, if_left_paren: Self, if_condition: Self, if_right_paren: Self, if_statement: Self, if_elseif_clauses: Self, if_else_clause: Self) -> Self {
        let syntax = SyntaxVariant::IfStatement(Box::new(IfStatementChildren {
            if_keyword,
            if_left_paren,
            if_condition,
            if_right_paren,
            if_statement,
            if_elseif_clauses,
            if_else_clause,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_elseif_clause(_: &C, elseif_keyword: Self, elseif_left_paren: Self, elseif_condition: Self, elseif_right_paren: Self, elseif_statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseifClause(Box::new(ElseifClauseChildren {
            elseif_keyword,
            elseif_left_paren,
            elseif_condition,
            elseif_right_paren,
            elseif_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_else_clause(_: &C, else_keyword: Self, else_statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseClause(Box::new(ElseClauseChildren {
            else_keyword,
            else_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_try_statement(_: &C, try_keyword: Self, try_compound_statement: Self, try_catch_clauses: Self, try_finally_clause: Self) -> Self {
        let syntax = SyntaxVariant::TryStatement(Box::new(TryStatementChildren {
            try_keyword,
            try_compound_statement,
            try_catch_clauses,
            try_finally_clause,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_catch_clause(_: &C, catch_keyword: Self, catch_left_paren: Self, catch_type: Self, catch_variable: Self, catch_right_paren: Self, catch_body: Self) -> Self {
        let syntax = SyntaxVariant::CatchClause(Box::new(CatchClauseChildren {
            catch_keyword,
            catch_left_paren,
            catch_type,
            catch_variable,
            catch_right_paren,
            catch_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_finally_clause(_: &C, finally_keyword: Self, finally_body: Self) -> Self {
        let syntax = SyntaxVariant::FinallyClause(Box::new(FinallyClauseChildren {
            finally_keyword,
            finally_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_do_statement(_: &C, do_keyword: Self, do_body: Self, do_while_keyword: Self, do_left_paren: Self, do_condition: Self, do_right_paren: Self, do_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::DoStatement(Box::new(DoStatementChildren {
            do_keyword,
            do_body,
            do_while_keyword,
            do_left_paren,
            do_condition,
            do_right_paren,
            do_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_for_statement(_: &C, for_keyword: Self, for_left_paren: Self, for_initializer: Self, for_first_semicolon: Self, for_control: Self, for_second_semicolon: Self, for_end_of_loop: Self, for_right_paren: Self, for_body: Self) -> Self {
        let syntax = SyntaxVariant::ForStatement(Box::new(ForStatementChildren {
            for_keyword,
            for_left_paren,
            for_initializer,
            for_first_semicolon,
            for_control,
            for_second_semicolon,
            for_end_of_loop,
            for_right_paren,
            for_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_foreach_statement(_: &C, foreach_keyword: Self, foreach_left_paren: Self, foreach_collection: Self, foreach_await_keyword: Self, foreach_as: Self, foreach_key: Self, foreach_arrow: Self, foreach_value: Self, foreach_right_paren: Self, foreach_body: Self) -> Self {
        let syntax = SyntaxVariant::ForeachStatement(Box::new(ForeachStatementChildren {
            foreach_keyword,
            foreach_left_paren,
            foreach_collection,
            foreach_await_keyword,
            foreach_as,
            foreach_key,
            foreach_arrow,
            foreach_value,
            foreach_right_paren,
            foreach_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_switch_statement(_: &C, switch_keyword: Self, switch_left_paren: Self, switch_expression: Self, switch_right_paren: Self, switch_left_brace: Self, switch_sections: Self, switch_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::SwitchStatement(Box::new(SwitchStatementChildren {
            switch_keyword,
            switch_left_paren,
            switch_expression,
            switch_right_paren,
            switch_left_brace,
            switch_sections,
            switch_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_switch_section(_: &C, switch_section_labels: Self, switch_section_statements: Self, switch_section_fallthrough: Self) -> Self {
        let syntax = SyntaxVariant::SwitchSection(Box::new(SwitchSectionChildren {
            switch_section_labels,
            switch_section_statements,
            switch_section_fallthrough,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_switch_fallthrough(_: &C, fallthrough_keyword: Self, fallthrough_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::SwitchFallthrough(Box::new(SwitchFallthroughChildren {
            fallthrough_keyword,
            fallthrough_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_case_label(_: &C, case_keyword: Self, case_expression: Self, case_colon: Self) -> Self {
        let syntax = SyntaxVariant::CaseLabel(Box::new(CaseLabelChildren {
            case_keyword,
            case_expression,
            case_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_default_label(_: &C, default_keyword: Self, default_colon: Self) -> Self {
        let syntax = SyntaxVariant::DefaultLabel(Box::new(DefaultLabelChildren {
            default_keyword,
            default_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_return_statement(_: &C, return_keyword: Self, return_expression: Self, return_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ReturnStatement(Box::new(ReturnStatementChildren {
            return_keyword,
            return_expression,
            return_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_goto_label(_: &C, goto_label_name: Self, goto_label_colon: Self) -> Self {
        let syntax = SyntaxVariant::GotoLabel(Box::new(GotoLabelChildren {
            goto_label_name,
            goto_label_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_goto_statement(_: &C, goto_statement_keyword: Self, goto_statement_label_name: Self, goto_statement_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::GotoStatement(Box::new(GotoStatementChildren {
            goto_statement_keyword,
            goto_statement_label_name,
            goto_statement_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_throw_statement(_: &C, throw_keyword: Self, throw_expression: Self, throw_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ThrowStatement(Box::new(ThrowStatementChildren {
            throw_keyword,
            throw_expression,
            throw_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_break_statement(_: &C, break_keyword: Self, break_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::BreakStatement(Box::new(BreakStatementChildren {
            break_keyword,
            break_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_continue_statement(_: &C, continue_keyword: Self, continue_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContinueStatement(Box::new(ContinueStatementChildren {
            continue_keyword,
            continue_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_echo_statement(_: &C, echo_keyword: Self, echo_expressions: Self, echo_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EchoStatement(Box::new(EchoStatementChildren {
            echo_keyword,
            echo_expressions,
            echo_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_concurrent_statement(_: &C, concurrent_keyword: Self, concurrent_statement: Self) -> Self {
        let syntax = SyntaxVariant::ConcurrentStatement(Box::new(ConcurrentStatementChildren {
            concurrent_keyword,
            concurrent_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_simple_initializer(_: &C, simple_initializer_equal: Self, simple_initializer_value: Self) -> Self {
        let syntax = SyntaxVariant::SimpleInitializer(Box::new(SimpleInitializerChildren {
            simple_initializer_equal,
            simple_initializer_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_anonymous_class(_: &C, anonymous_class_class_keyword: Self, anonymous_class_left_paren: Self, anonymous_class_argument_list: Self, anonymous_class_right_paren: Self, anonymous_class_extends_keyword: Self, anonymous_class_extends_list: Self, anonymous_class_implements_keyword: Self, anonymous_class_implements_list: Self, anonymous_class_body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousClass(Box::new(AnonymousClassChildren {
            anonymous_class_class_keyword,
            anonymous_class_left_paren,
            anonymous_class_argument_list,
            anonymous_class_right_paren,
            anonymous_class_extends_keyword,
            anonymous_class_extends_list,
            anonymous_class_implements_keyword,
            anonymous_class_implements_list,
            anonymous_class_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_anonymous_function(_: &C, anonymous_attribute_spec: Self, anonymous_static_keyword: Self, anonymous_async_keyword: Self, anonymous_function_keyword: Self, anonymous_left_paren: Self, anonymous_parameters: Self, anonymous_right_paren: Self, anonymous_colon: Self, anonymous_type: Self, anonymous_use: Self, anonymous_body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunction(Box::new(AnonymousFunctionChildren {
            anonymous_attribute_spec,
            anonymous_static_keyword,
            anonymous_async_keyword,
            anonymous_function_keyword,
            anonymous_left_paren,
            anonymous_parameters,
            anonymous_right_paren,
            anonymous_colon,
            anonymous_type,
            anonymous_use,
            anonymous_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_anonymous_function_use_clause(_: &C, anonymous_use_keyword: Self, anonymous_use_left_paren: Self, anonymous_use_variables: Self, anonymous_use_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunctionUseClause(Box::new(AnonymousFunctionUseClauseChildren {
            anonymous_use_keyword,
            anonymous_use_left_paren,
            anonymous_use_variables,
            anonymous_use_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_lambda_expression(_: &C, lambda_attribute_spec: Self, lambda_async: Self, lambda_signature: Self, lambda_arrow: Self, lambda_body: Self) -> Self {
        let syntax = SyntaxVariant::LambdaExpression(Box::new(LambdaExpressionChildren {
            lambda_attribute_spec,
            lambda_async,
            lambda_signature,
            lambda_arrow,
            lambda_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_lambda_signature(_: &C, lambda_left_paren: Self, lambda_parameters: Self, lambda_right_paren: Self, lambda_colon: Self, lambda_type: Self) -> Self {
        let syntax = SyntaxVariant::LambdaSignature(Box::new(LambdaSignatureChildren {
            lambda_left_paren,
            lambda_parameters,
            lambda_right_paren,
            lambda_colon,
            lambda_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_cast_expression(_: &C, cast_left_paren: Self, cast_type: Self, cast_right_paren: Self, cast_operand: Self) -> Self {
        let syntax = SyntaxVariant::CastExpression(Box::new(CastExpressionChildren {
            cast_left_paren,
            cast_type,
            cast_right_paren,
            cast_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_scope_resolution_expression(_: &C, scope_resolution_qualifier: Self, scope_resolution_operator: Self, scope_resolution_name: Self) -> Self {
        let syntax = SyntaxVariant::ScopeResolutionExpression(Box::new(ScopeResolutionExpressionChildren {
            scope_resolution_qualifier,
            scope_resolution_operator,
            scope_resolution_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_member_selection_expression(_: &C, member_object: Self, member_operator: Self, member_name: Self) -> Self {
        let syntax = SyntaxVariant::MemberSelectionExpression(Box::new(MemberSelectionExpressionChildren {
            member_object,
            member_operator,
            member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_safe_member_selection_expression(_: &C, safe_member_object: Self, safe_member_operator: Self, safe_member_name: Self) -> Self {
        let syntax = SyntaxVariant::SafeMemberSelectionExpression(Box::new(SafeMemberSelectionExpressionChildren {
            safe_member_object,
            safe_member_operator,
            safe_member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_member_selection_expression(_: &C, embedded_member_object: Self, embedded_member_operator: Self, embedded_member_name: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedMemberSelectionExpression(Box::new(EmbeddedMemberSelectionExpressionChildren {
            embedded_member_object,
            embedded_member_operator,
            embedded_member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_yield_expression(_: &C, yield_keyword: Self, yield_operand: Self) -> Self {
        let syntax = SyntaxVariant::YieldExpression(Box::new(YieldExpressionChildren {
            yield_keyword,
            yield_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_prefix_unary_expression(_: &C, prefix_unary_operator: Self, prefix_unary_operand: Self) -> Self {
        let syntax = SyntaxVariant::PrefixUnaryExpression(Box::new(PrefixUnaryExpressionChildren {
            prefix_unary_operator,
            prefix_unary_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_postfix_unary_expression(_: &C, postfix_unary_operand: Self, postfix_unary_operator: Self) -> Self {
        let syntax = SyntaxVariant::PostfixUnaryExpression(Box::new(PostfixUnaryExpressionChildren {
            postfix_unary_operand,
            postfix_unary_operator,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_binary_expression(_: &C, binary_left_operand: Self, binary_operator: Self, binary_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::BinaryExpression(Box::new(BinaryExpressionChildren {
            binary_left_operand,
            binary_operator,
            binary_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_is_expression(_: &C, is_left_operand: Self, is_operator: Self, is_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::IsExpression(Box::new(IsExpressionChildren {
            is_left_operand,
            is_operator,
            is_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_as_expression(_: &C, as_left_operand: Self, as_operator: Self, as_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::AsExpression(Box::new(AsExpressionChildren {
            as_left_operand,
            as_operator,
            as_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_nullable_as_expression(_: &C, nullable_as_left_operand: Self, nullable_as_operator: Self, nullable_as_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::NullableAsExpression(Box::new(NullableAsExpressionChildren {
            nullable_as_left_operand,
            nullable_as_operator,
            nullable_as_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_conditional_expression(_: &C, conditional_test: Self, conditional_question: Self, conditional_consequence: Self, conditional_colon: Self, conditional_alternative: Self) -> Self {
        let syntax = SyntaxVariant::ConditionalExpression(Box::new(ConditionalExpressionChildren {
            conditional_test,
            conditional_question,
            conditional_consequence,
            conditional_colon,
            conditional_alternative,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_eval_expression(_: &C, eval_keyword: Self, eval_left_paren: Self, eval_argument: Self, eval_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::EvalExpression(Box::new(EvalExpressionChildren {
            eval_keyword,
            eval_left_paren,
            eval_argument,
            eval_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_define_expression(_: &C, define_keyword: Self, define_left_paren: Self, define_argument_list: Self, define_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::DefineExpression(Box::new(DefineExpressionChildren {
            define_keyword,
            define_left_paren,
            define_argument_list,
            define_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_isset_expression(_: &C, isset_keyword: Self, isset_left_paren: Self, isset_argument_list: Self, isset_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IssetExpression(Box::new(IssetExpressionChildren {
            isset_keyword,
            isset_left_paren,
            isset_argument_list,
            isset_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_call_expression(_: &C, function_call_receiver: Self, function_call_type_args: Self, function_call_left_paren: Self, function_call_argument_list: Self, function_call_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::FunctionCallExpression(Box::new(FunctionCallExpressionChildren {
            function_call_receiver,
            function_call_type_args,
            function_call_left_paren,
            function_call_argument_list,
            function_call_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_pointer_expression(_: &C, function_pointer_receiver: Self, function_pointer_type_args: Self) -> Self {
        let syntax = SyntaxVariant::FunctionPointerExpression(Box::new(FunctionPointerExpressionChildren {
            function_pointer_receiver,
            function_pointer_type_args,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_parenthesized_expression(_: &C, parenthesized_expression_left_paren: Self, parenthesized_expression_expression: Self, parenthesized_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ParenthesizedExpression(Box::new(ParenthesizedExpressionChildren {
            parenthesized_expression_left_paren,
            parenthesized_expression_expression,
            parenthesized_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_braced_expression(_: &C, braced_expression_left_brace: Self, braced_expression_expression: Self, braced_expression_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::BracedExpression(Box::new(BracedExpressionChildren {
            braced_expression_left_brace,
            braced_expression_expression,
            braced_expression_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_braced_expression(_: &C, embedded_braced_expression_left_brace: Self, embedded_braced_expression_expression: Self, embedded_braced_expression_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedBracedExpression(Box::new(EmbeddedBracedExpressionChildren {
            embedded_braced_expression_left_brace,
            embedded_braced_expression_expression,
            embedded_braced_expression_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_list_expression(_: &C, list_keyword: Self, list_left_paren: Self, list_members: Self, list_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ListExpression(Box::new(ListExpressionChildren {
            list_keyword,
            list_left_paren,
            list_members,
            list_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_collection_literal_expression(_: &C, collection_literal_name: Self, collection_literal_left_brace: Self, collection_literal_initializers: Self, collection_literal_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CollectionLiteralExpression(Box::new(CollectionLiteralExpressionChildren {
            collection_literal_name,
            collection_literal_left_brace,
            collection_literal_initializers,
            collection_literal_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_object_creation_expression(_: &C, object_creation_new_keyword: Self, object_creation_object: Self) -> Self {
        let syntax = SyntaxVariant::ObjectCreationExpression(Box::new(ObjectCreationExpressionChildren {
            object_creation_new_keyword,
            object_creation_object,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_constructor_call(_: &C, constructor_call_type: Self, constructor_call_left_paren: Self, constructor_call_argument_list: Self, constructor_call_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ConstructorCall(Box::new(ConstructorCallChildren {
            constructor_call_type,
            constructor_call_left_paren,
            constructor_call_argument_list,
            constructor_call_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_creation_expression(_: &C, record_creation_type: Self, record_creation_left_bracket: Self, record_creation_members: Self, record_creation_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::RecordCreationExpression(Box::new(RecordCreationExpressionChildren {
            record_creation_type,
            record_creation_left_bracket,
            record_creation_members,
            record_creation_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_darray_intrinsic_expression(_: &C, darray_intrinsic_keyword: Self, darray_intrinsic_explicit_type: Self, darray_intrinsic_left_bracket: Self, darray_intrinsic_members: Self, darray_intrinsic_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::DarrayIntrinsicExpression(Box::new(DarrayIntrinsicExpressionChildren {
            darray_intrinsic_keyword,
            darray_intrinsic_explicit_type,
            darray_intrinsic_left_bracket,
            darray_intrinsic_members,
            darray_intrinsic_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_dictionary_intrinsic_expression(_: &C, dictionary_intrinsic_keyword: Self, dictionary_intrinsic_explicit_type: Self, dictionary_intrinsic_left_bracket: Self, dictionary_intrinsic_members: Self, dictionary_intrinsic_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryIntrinsicExpression(Box::new(DictionaryIntrinsicExpressionChildren {
            dictionary_intrinsic_keyword,
            dictionary_intrinsic_explicit_type,
            dictionary_intrinsic_left_bracket,
            dictionary_intrinsic_members,
            dictionary_intrinsic_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_keyset_intrinsic_expression(_: &C, keyset_intrinsic_keyword: Self, keyset_intrinsic_explicit_type: Self, keyset_intrinsic_left_bracket: Self, keyset_intrinsic_members: Self, keyset_intrinsic_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::KeysetIntrinsicExpression(Box::new(KeysetIntrinsicExpressionChildren {
            keyset_intrinsic_keyword,
            keyset_intrinsic_explicit_type,
            keyset_intrinsic_left_bracket,
            keyset_intrinsic_members,
            keyset_intrinsic_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_varray_intrinsic_expression(_: &C, varray_intrinsic_keyword: Self, varray_intrinsic_explicit_type: Self, varray_intrinsic_left_bracket: Self, varray_intrinsic_members: Self, varray_intrinsic_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::VarrayIntrinsicExpression(Box::new(VarrayIntrinsicExpressionChildren {
            varray_intrinsic_keyword,
            varray_intrinsic_explicit_type,
            varray_intrinsic_left_bracket,
            varray_intrinsic_members,
            varray_intrinsic_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_vector_intrinsic_expression(_: &C, vector_intrinsic_keyword: Self, vector_intrinsic_explicit_type: Self, vector_intrinsic_left_bracket: Self, vector_intrinsic_members: Self, vector_intrinsic_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::VectorIntrinsicExpression(Box::new(VectorIntrinsicExpressionChildren {
            vector_intrinsic_keyword,
            vector_intrinsic_explicit_type,
            vector_intrinsic_left_bracket,
            vector_intrinsic_members,
            vector_intrinsic_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_element_initializer(_: &C, element_key: Self, element_arrow: Self, element_value: Self) -> Self {
        let syntax = SyntaxVariant::ElementInitializer(Box::new(ElementInitializerChildren {
            element_key,
            element_arrow,
            element_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_subscript_expression(_: &C, subscript_receiver: Self, subscript_left_bracket: Self, subscript_index: Self, subscript_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::SubscriptExpression(Box::new(SubscriptExpressionChildren {
            subscript_receiver,
            subscript_left_bracket,
            subscript_index,
            subscript_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_subscript_expression(_: &C, embedded_subscript_receiver: Self, embedded_subscript_left_bracket: Self, embedded_subscript_index: Self, embedded_subscript_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedSubscriptExpression(Box::new(EmbeddedSubscriptExpressionChildren {
            embedded_subscript_receiver,
            embedded_subscript_left_bracket,
            embedded_subscript_index,
            embedded_subscript_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_awaitable_creation_expression(_: &C, awaitable_attribute_spec: Self, awaitable_async: Self, awaitable_compound_statement: Self) -> Self {
        let syntax = SyntaxVariant::AwaitableCreationExpression(Box::new(AwaitableCreationExpressionChildren {
            awaitable_attribute_spec,
            awaitable_async,
            awaitable_compound_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_children_declaration(_: &C, xhp_children_keyword: Self, xhp_children_expression: Self, xhp_children_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenDeclaration(Box::new(XHPChildrenDeclarationChildren {
            xhp_children_keyword,
            xhp_children_expression,
            xhp_children_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_children_parenthesized_list(_: &C, xhp_children_list_left_paren: Self, xhp_children_list_xhp_children: Self, xhp_children_list_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenParenthesizedList(Box::new(XHPChildrenParenthesizedListChildren {
            xhp_children_list_left_paren,
            xhp_children_list_xhp_children,
            xhp_children_list_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_category_declaration(_: &C, xhp_category_keyword: Self, xhp_category_categories: Self, xhp_category_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPCategoryDeclaration(Box::new(XHPCategoryDeclarationChildren {
            xhp_category_keyword,
            xhp_category_categories,
            xhp_category_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_enum_type(_: &C, xhp_enum_keyword: Self, xhp_enum_left_brace: Self, xhp_enum_values: Self, xhp_enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPEnumType(Box::new(XHPEnumTypeChildren {
            xhp_enum_keyword,
            xhp_enum_left_brace,
            xhp_enum_values,
            xhp_enum_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_lateinit(_: &C, xhp_lateinit_at: Self, xhp_lateinit_keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPLateinit(Box::new(XHPLateinitChildren {
            xhp_lateinit_at,
            xhp_lateinit_keyword,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_required(_: &C, xhp_required_at: Self, xhp_required_keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPRequired(Box::new(XHPRequiredChildren {
            xhp_required_at,
            xhp_required_keyword,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute_declaration(_: &C, xhp_attribute_keyword: Self, xhp_attribute_attributes: Self, xhp_attribute_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttributeDeclaration(Box::new(XHPClassAttributeDeclarationChildren {
            xhp_attribute_keyword,
            xhp_attribute_attributes,
            xhp_attribute_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute(_: &C, xhp_attribute_decl_type: Self, xhp_attribute_decl_name: Self, xhp_attribute_decl_initializer: Self, xhp_attribute_decl_required: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttribute(Box::new(XHPClassAttributeChildren {
            xhp_attribute_decl_type,
            xhp_attribute_decl_name,
            xhp_attribute_decl_initializer,
            xhp_attribute_decl_required,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_simple_class_attribute(_: &C, xhp_simple_class_attribute_type: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleClassAttribute(Box::new(XHPSimpleClassAttributeChildren {
            xhp_simple_class_attribute_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_simple_attribute(_: &C, xhp_simple_attribute_name: Self, xhp_simple_attribute_equal: Self, xhp_simple_attribute_expression: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleAttribute(Box::new(XHPSimpleAttributeChildren {
            xhp_simple_attribute_name,
            xhp_simple_attribute_equal,
            xhp_simple_attribute_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_spread_attribute(_: &C, xhp_spread_attribute_left_brace: Self, xhp_spread_attribute_spread_operator: Self, xhp_spread_attribute_expression: Self, xhp_spread_attribute_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPSpreadAttribute(Box::new(XHPSpreadAttributeChildren {
            xhp_spread_attribute_left_brace,
            xhp_spread_attribute_spread_operator,
            xhp_spread_attribute_expression,
            xhp_spread_attribute_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_open(_: &C, xhp_open_left_angle: Self, xhp_open_name: Self, xhp_open_attributes: Self, xhp_open_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPOpen(Box::new(XHPOpenChildren {
            xhp_open_left_angle,
            xhp_open_name,
            xhp_open_attributes,
            xhp_open_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_expression(_: &C, xhp_open: Self, xhp_body: Self, xhp_close: Self) -> Self {
        let syntax = SyntaxVariant::XHPExpression(Box::new(XHPExpressionChildren {
            xhp_open,
            xhp_body,
            xhp_close,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_close(_: &C, xhp_close_left_angle: Self, xhp_close_name: Self, xhp_close_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPClose(Box::new(XHPCloseChildren {
            xhp_close_left_angle,
            xhp_close_name,
            xhp_close_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_constant(_: &C, type_constant_left_type: Self, type_constant_separator: Self, type_constant_right_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstant(Box::new(TypeConstantChildren {
            type_constant_left_type,
            type_constant_separator,
            type_constant_right_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pu_access(_: &C, pu_access_left_type: Self, pu_access_separator: Self, pu_access_right_type: Self) -> Self {
        let syntax = SyntaxVariant::PUAccess(Box::new(PUAccessChildren {
            pu_access_left_type,
            pu_access_separator,
            pu_access_right_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_vector_type_specifier(_: &C, vector_type_keyword: Self, vector_type_left_angle: Self, vector_type_type: Self, vector_type_trailing_comma: Self, vector_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VectorTypeSpecifier(Box::new(VectorTypeSpecifierChildren {
            vector_type_keyword,
            vector_type_left_angle,
            vector_type_type,
            vector_type_trailing_comma,
            vector_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_keyset_type_specifier(_: &C, keyset_type_keyword: Self, keyset_type_left_angle: Self, keyset_type_type: Self, keyset_type_trailing_comma: Self, keyset_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::KeysetTypeSpecifier(Box::new(KeysetTypeSpecifierChildren {
            keyset_type_keyword,
            keyset_type_left_angle,
            keyset_type_type,
            keyset_type_trailing_comma,
            keyset_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_tuple_type_explicit_specifier(_: &C, tuple_type_keyword: Self, tuple_type_left_angle: Self, tuple_type_types: Self, tuple_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeExplicitSpecifier(Box::new(TupleTypeExplicitSpecifierChildren {
            tuple_type_keyword,
            tuple_type_left_angle,
            tuple_type_types,
            tuple_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_varray_type_specifier(_: &C, varray_keyword: Self, varray_left_angle: Self, varray_type: Self, varray_trailing_comma: Self, varray_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VarrayTypeSpecifier(Box::new(VarrayTypeSpecifierChildren {
            varray_keyword,
            varray_left_angle,
            varray_type,
            varray_trailing_comma,
            varray_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_vector_array_type_specifier(_: &C, vector_array_keyword: Self, vector_array_left_angle: Self, vector_array_type: Self, vector_array_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VectorArrayTypeSpecifier(Box::new(VectorArrayTypeSpecifierChildren {
            vector_array_keyword,
            vector_array_left_angle,
            vector_array_type,
            vector_array_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_parameter(_: &C, type_attribute_spec: Self, type_reified: Self, type_variance: Self, type_name: Self, type_param_params: Self, type_constraints: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameter(Box::new(TypeParameterChildren {
            type_attribute_spec,
            type_reified,
            type_variance,
            type_name,
            type_param_params,
            type_constraints,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_constraint(_: &C, constraint_keyword: Self, constraint_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstraint(Box::new(TypeConstraintChildren {
            constraint_keyword,
            constraint_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_darray_type_specifier(_: &C, darray_keyword: Self, darray_left_angle: Self, darray_key: Self, darray_comma: Self, darray_value: Self, darray_trailing_comma: Self, darray_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DarrayTypeSpecifier(Box::new(DarrayTypeSpecifierChildren {
            darray_keyword,
            darray_left_angle,
            darray_key,
            darray_comma,
            darray_value,
            darray_trailing_comma,
            darray_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_map_array_type_specifier(_: &C, map_array_keyword: Self, map_array_left_angle: Self, map_array_key: Self, map_array_comma: Self, map_array_value: Self, map_array_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::MapArrayTypeSpecifier(Box::new(MapArrayTypeSpecifierChildren {
            map_array_keyword,
            map_array_left_angle,
            map_array_key,
            map_array_comma,
            map_array_value,
            map_array_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_dictionary_type_specifier(_: &C, dictionary_type_keyword: Self, dictionary_type_left_angle: Self, dictionary_type_members: Self, dictionary_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryTypeSpecifier(Box::new(DictionaryTypeSpecifierChildren {
            dictionary_type_keyword,
            dictionary_type_left_angle,
            dictionary_type_members,
            dictionary_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_closure_type_specifier(_: &C, closure_outer_left_paren: Self, closure_function_keyword: Self, closure_inner_left_paren: Self, closure_parameter_list: Self, closure_inner_right_paren: Self, closure_colon: Self, closure_return_type: Self, closure_outer_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ClosureTypeSpecifier(Box::new(ClosureTypeSpecifierChildren {
            closure_outer_left_paren,
            closure_function_keyword,
            closure_inner_left_paren,
            closure_parameter_list,
            closure_inner_right_paren,
            closure_colon,
            closure_return_type,
            closure_outer_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_closure_parameter_type_specifier(_: &C, closure_parameter_call_convention: Self, closure_parameter_type: Self) -> Self {
        let syntax = SyntaxVariant::ClosureParameterTypeSpecifier(Box::new(ClosureParameterTypeSpecifierChildren {
            closure_parameter_call_convention,
            closure_parameter_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_classname_type_specifier(_: &C, classname_keyword: Self, classname_left_angle: Self, classname_type: Self, classname_trailing_comma: Self, classname_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::ClassnameTypeSpecifier(Box::new(ClassnameTypeSpecifierChildren {
            classname_keyword,
            classname_left_angle,
            classname_type,
            classname_trailing_comma,
            classname_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_field_specifier(_: &C, field_question: Self, field_name: Self, field_arrow: Self, field_type: Self) -> Self {
        let syntax = SyntaxVariant::FieldSpecifier(Box::new(FieldSpecifierChildren {
            field_question,
            field_name,
            field_arrow,
            field_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_field_initializer(_: &C, field_initializer_name: Self, field_initializer_arrow: Self, field_initializer_value: Self) -> Self {
        let syntax = SyntaxVariant::FieldInitializer(Box::new(FieldInitializerChildren {
            field_initializer_name,
            field_initializer_arrow,
            field_initializer_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_shape_type_specifier(_: &C, shape_type_keyword: Self, shape_type_left_paren: Self, shape_type_fields: Self, shape_type_ellipsis: Self, shape_type_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeTypeSpecifier(Box::new(ShapeTypeSpecifierChildren {
            shape_type_keyword,
            shape_type_left_paren,
            shape_type_fields,
            shape_type_ellipsis,
            shape_type_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_shape_expression(_: &C, shape_expression_keyword: Self, shape_expression_left_paren: Self, shape_expression_fields: Self, shape_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeExpression(Box::new(ShapeExpressionChildren {
            shape_expression_keyword,
            shape_expression_left_paren,
            shape_expression_fields,
            shape_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_tuple_expression(_: &C, tuple_expression_keyword: Self, tuple_expression_left_paren: Self, tuple_expression_items: Self, tuple_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleExpression(Box::new(TupleExpressionChildren {
            tuple_expression_keyword,
            tuple_expression_left_paren,
            tuple_expression_items,
            tuple_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_generic_type_specifier(_: &C, generic_class_type: Self, generic_argument_list: Self) -> Self {
        let syntax = SyntaxVariant::GenericTypeSpecifier(Box::new(GenericTypeSpecifierChildren {
            generic_class_type,
            generic_argument_list,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_nullable_type_specifier(_: &C, nullable_question: Self, nullable_type: Self) -> Self {
        let syntax = SyntaxVariant::NullableTypeSpecifier(Box::new(NullableTypeSpecifierChildren {
            nullable_question,
            nullable_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_like_type_specifier(_: &C, like_tilde: Self, like_type: Self) -> Self {
        let syntax = SyntaxVariant::LikeTypeSpecifier(Box::new(LikeTypeSpecifierChildren {
            like_tilde,
            like_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_soft_type_specifier(_: &C, soft_at: Self, soft_type: Self) -> Self {
        let syntax = SyntaxVariant::SoftTypeSpecifier(Box::new(SoftTypeSpecifierChildren {
            soft_at,
            soft_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_attributized_specifier(_: &C, attributized_specifier_attribute_spec: Self, attributized_specifier_type: Self) -> Self {
        let syntax = SyntaxVariant::AttributizedSpecifier(Box::new(AttributizedSpecifierChildren {
            attributized_specifier_attribute_spec,
            attributized_specifier_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_reified_type_argument(_: &C, reified_type_argument_reified: Self, reified_type_argument_type: Self) -> Self {
        let syntax = SyntaxVariant::ReifiedTypeArgument(Box::new(ReifiedTypeArgumentChildren {
            reified_type_argument_reified,
            reified_type_argument_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_arguments(_: &C, type_arguments_left_angle: Self, type_arguments_types: Self, type_arguments_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeArguments(Box::new(TypeArgumentsChildren {
            type_arguments_left_angle,
            type_arguments_types,
            type_arguments_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_parameters(_: &C, type_parameters_left_angle: Self, type_parameters_parameters: Self, type_parameters_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameters(Box::new(TypeParametersChildren {
            type_parameters_left_angle,
            type_parameters_parameters,
            type_parameters_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_tuple_type_specifier(_: &C, tuple_left_paren: Self, tuple_types: Self, tuple_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeSpecifier(Box::new(TupleTypeSpecifierChildren {
            tuple_left_paren,
            tuple_types,
            tuple_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_union_type_specifier(_: &C, union_left_paren: Self, union_types: Self, union_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::UnionTypeSpecifier(Box::new(UnionTypeSpecifierChildren {
            union_left_paren,
            union_types,
            union_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_intersection_type_specifier(_: &C, intersection_left_paren: Self, intersection_types: Self, intersection_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IntersectionTypeSpecifier(Box::new(IntersectionTypeSpecifierChildren {
            intersection_left_paren,
            intersection_types,
            intersection_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_error(_: &C, error_error: Self) -> Self {
        let syntax = SyntaxVariant::ErrorSyntax(Box::new(ErrorSyntaxChildren {
            error_error,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_list_item(_: &C, list_item: Self, list_separator: Self) -> Self {
        let syntax = SyntaxVariant::ListItem(Box::new(ListItemChildren {
            list_item,
            list_separator,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_atom_expression(_: &C, pocket_atom_glyph: Self, pocket_atom_expression: Self) -> Self {
        let syntax = SyntaxVariant::PocketAtomExpression(Box::new(PocketAtomExpressionChildren {
            pocket_atom_glyph,
            pocket_atom_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_identifier_expression(_: &C, pocket_identifier_qualifier: Self, pocket_identifier_pu_operator: Self, pocket_identifier_field: Self, pocket_identifier_operator: Self, pocket_identifier_name: Self) -> Self {
        let syntax = SyntaxVariant::PocketIdentifierExpression(Box::new(PocketIdentifierExpressionChildren {
            pocket_identifier_qualifier,
            pocket_identifier_pu_operator,
            pocket_identifier_field,
            pocket_identifier_operator,
            pocket_identifier_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_atom_mapping_declaration(_: &C, pocket_atom_mapping_glyph: Self, pocket_atom_mapping_name: Self, pocket_atom_mapping_left_paren: Self, pocket_atom_mapping_mappings: Self, pocket_atom_mapping_right_paren: Self, pocket_atom_mapping_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketAtomMappingDeclaration(Box::new(PocketAtomMappingDeclarationChildren {
            pocket_atom_mapping_glyph,
            pocket_atom_mapping_name,
            pocket_atom_mapping_left_paren,
            pocket_atom_mapping_mappings,
            pocket_atom_mapping_right_paren,
            pocket_atom_mapping_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_enum_declaration(_: &C, pocket_enum_attributes: Self, pocket_enum_modifiers: Self, pocket_enum_enum: Self, pocket_enum_name: Self, pocket_enum_left_brace: Self, pocket_enum_fields: Self, pocket_enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::PocketEnumDeclaration(Box::new(PocketEnumDeclarationChildren {
            pocket_enum_attributes,
            pocket_enum_modifiers,
            pocket_enum_enum,
            pocket_enum_name,
            pocket_enum_left_brace,
            pocket_enum_fields,
            pocket_enum_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_field_type_expr_declaration(_: &C, pocket_field_type_expr_case: Self, pocket_field_type_expr_type: Self, pocket_field_type_expr_name: Self, pocket_field_type_expr_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeExprDeclaration(Box::new(PocketFieldTypeExprDeclarationChildren {
            pocket_field_type_expr_case,
            pocket_field_type_expr_type,
            pocket_field_type_expr_name,
            pocket_field_type_expr_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_field_type_declaration(_: &C, pocket_field_type_case: Self, pocket_field_type_type: Self, pocket_field_type_type_parameter: Self, pocket_field_type_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeDeclaration(Box::new(PocketFieldTypeDeclarationChildren {
            pocket_field_type_case,
            pocket_field_type_type,
            pocket_field_type_type_parameter,
            pocket_field_type_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_id_declaration(_: &C, pocket_mapping_id_name: Self, pocket_mapping_id_initializer: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingIdDeclaration(Box::new(PocketMappingIdDeclarationChildren {
            pocket_mapping_id_name,
            pocket_mapping_id_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_type_declaration(_: &C, pocket_mapping_type_keyword: Self, pocket_mapping_type_name: Self, pocket_mapping_type_equal: Self, pocket_mapping_type_type: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingTypeDeclaration(Box::new(PocketMappingTypeDeclarationChildren {
            pocket_mapping_type_keyword,
            pocket_mapping_type_name,
            pocket_mapping_type_equal,
            pocket_mapping_type_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

 }

impl<T, V> Syntax<T, V>
where
    T: LexableToken,
{
    pub fn fold_over_children_owned<U>(
        f: &dyn Fn(Self, U) -> U,
        acc: U,
        syntax: SyntaxVariant<T, V>,
    ) -> U {
        match syntax {
            SyntaxVariant::Missing => acc,
            SyntaxVariant::Token (_) => acc,
            SyntaxVariant::SyntaxList(elems) => {
                let mut acc = acc;
                for item in elems {
                    acc = f(item, acc);
                }
                acc
            },
            SyntaxVariant::EndOfFile(x) => {
                let EndOfFileChildren { end_of_file_token } = *x;
                let acc = f(end_of_file_token, acc);
                acc
            },
            SyntaxVariant::Script(x) => {
                let ScriptChildren { script_declarations } = *x;
                let acc = f(script_declarations, acc);
                acc
            },
            SyntaxVariant::QualifiedName(x) => {
                let QualifiedNameChildren { qualified_name_parts } = *x;
                let acc = f(qualified_name_parts, acc);
                acc
            },
            SyntaxVariant::SimpleTypeSpecifier(x) => {
                let SimpleTypeSpecifierChildren { simple_type_specifier } = *x;
                let acc = f(simple_type_specifier, acc);
                acc
            },
            SyntaxVariant::LiteralExpression(x) => {
                let LiteralExpressionChildren { literal_expression } = *x;
                let acc = f(literal_expression, acc);
                acc
            },
            SyntaxVariant::PrefixedStringExpression(x) => {
                let PrefixedStringExpressionChildren { prefixed_string_name, prefixed_string_str } = *x;
                let acc = f(prefixed_string_name, acc);
                let acc = f(prefixed_string_str, acc);
                acc
            },
            SyntaxVariant::PrefixedCodeExpression(x) => {
                let PrefixedCodeExpressionChildren { prefixed_code_prefix, prefixed_code_left_backtick, prefixed_code_expression, prefixed_code_right_backtick } = *x;
                let acc = f(prefixed_code_prefix, acc);
                let acc = f(prefixed_code_left_backtick, acc);
                let acc = f(prefixed_code_expression, acc);
                let acc = f(prefixed_code_right_backtick, acc);
                acc
            },
            SyntaxVariant::VariableExpression(x) => {
                let VariableExpressionChildren { variable_expression } = *x;
                let acc = f(variable_expression, acc);
                acc
            },
            SyntaxVariant::PipeVariableExpression(x) => {
                let PipeVariableExpressionChildren { pipe_variable_expression } = *x;
                let acc = f(pipe_variable_expression, acc);
                acc
            },
            SyntaxVariant::FileAttributeSpecification(x) => {
                let FileAttributeSpecificationChildren { file_attribute_specification_left_double_angle, file_attribute_specification_keyword, file_attribute_specification_colon, file_attribute_specification_attributes, file_attribute_specification_right_double_angle } = *x;
                let acc = f(file_attribute_specification_left_double_angle, acc);
                let acc = f(file_attribute_specification_keyword, acc);
                let acc = f(file_attribute_specification_colon, acc);
                let acc = f(file_attribute_specification_attributes, acc);
                let acc = f(file_attribute_specification_right_double_angle, acc);
                acc
            },
            SyntaxVariant::EnumDeclaration(x) => {
                let EnumDeclarationChildren { enum_attribute_spec, enum_keyword, enum_name, enum_colon, enum_base, enum_type, enum_includes_keyword, enum_includes_list, enum_left_brace, enum_enumerators, enum_right_brace } = *x;
                let acc = f(enum_attribute_spec, acc);
                let acc = f(enum_keyword, acc);
                let acc = f(enum_name, acc);
                let acc = f(enum_colon, acc);
                let acc = f(enum_base, acc);
                let acc = f(enum_type, acc);
                let acc = f(enum_includes_keyword, acc);
                let acc = f(enum_includes_list, acc);
                let acc = f(enum_left_brace, acc);
                let acc = f(enum_enumerators, acc);
                let acc = f(enum_right_brace, acc);
                acc
            },
            SyntaxVariant::Enumerator(x) => {
                let EnumeratorChildren { enumerator_name, enumerator_equal, enumerator_value, enumerator_semicolon } = *x;
                let acc = f(enumerator_name, acc);
                let acc = f(enumerator_equal, acc);
                let acc = f(enumerator_value, acc);
                let acc = f(enumerator_semicolon, acc);
                acc
            },
            SyntaxVariant::RecordDeclaration(x) => {
                let RecordDeclarationChildren { record_attribute_spec, record_modifier, record_keyword, record_name, record_extends_keyword, record_extends_opt, record_left_brace, record_fields, record_right_brace } = *x;
                let acc = f(record_attribute_spec, acc);
                let acc = f(record_modifier, acc);
                let acc = f(record_keyword, acc);
                let acc = f(record_name, acc);
                let acc = f(record_extends_keyword, acc);
                let acc = f(record_extends_opt, acc);
                let acc = f(record_left_brace, acc);
                let acc = f(record_fields, acc);
                let acc = f(record_right_brace, acc);
                acc
            },
            SyntaxVariant::RecordField(x) => {
                let RecordFieldChildren { record_field_type, record_field_name, record_field_init, record_field_semi } = *x;
                let acc = f(record_field_type, acc);
                let acc = f(record_field_name, acc);
                let acc = f(record_field_init, acc);
                let acc = f(record_field_semi, acc);
                acc
            },
            SyntaxVariant::AliasDeclaration(x) => {
                let AliasDeclarationChildren { alias_attribute_spec, alias_keyword, alias_name, alias_generic_parameter, alias_constraint, alias_equal, alias_type, alias_semicolon } = *x;
                let acc = f(alias_attribute_spec, acc);
                let acc = f(alias_keyword, acc);
                let acc = f(alias_name, acc);
                let acc = f(alias_generic_parameter, acc);
                let acc = f(alias_constraint, acc);
                let acc = f(alias_equal, acc);
                let acc = f(alias_type, acc);
                let acc = f(alias_semicolon, acc);
                acc
            },
            SyntaxVariant::PropertyDeclaration(x) => {
                let PropertyDeclarationChildren { property_attribute_spec, property_modifiers, property_type, property_declarators, property_semicolon } = *x;
                let acc = f(property_attribute_spec, acc);
                let acc = f(property_modifiers, acc);
                let acc = f(property_type, acc);
                let acc = f(property_declarators, acc);
                let acc = f(property_semicolon, acc);
                acc
            },
            SyntaxVariant::PropertyDeclarator(x) => {
                let PropertyDeclaratorChildren { property_name, property_initializer } = *x;
                let acc = f(property_name, acc);
                let acc = f(property_initializer, acc);
                acc
            },
            SyntaxVariant::NamespaceDeclaration(x) => {
                let NamespaceDeclarationChildren { namespace_header, namespace_body } = *x;
                let acc = f(namespace_header, acc);
                let acc = f(namespace_body, acc);
                acc
            },
            SyntaxVariant::NamespaceDeclarationHeader(x) => {
                let NamespaceDeclarationHeaderChildren { namespace_keyword, namespace_name } = *x;
                let acc = f(namespace_keyword, acc);
                let acc = f(namespace_name, acc);
                acc
            },
            SyntaxVariant::NamespaceBody(x) => {
                let NamespaceBodyChildren { namespace_left_brace, namespace_declarations, namespace_right_brace } = *x;
                let acc = f(namespace_left_brace, acc);
                let acc = f(namespace_declarations, acc);
                let acc = f(namespace_right_brace, acc);
                acc
            },
            SyntaxVariant::NamespaceEmptyBody(x) => {
                let NamespaceEmptyBodyChildren { namespace_semicolon } = *x;
                let acc = f(namespace_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceUseDeclaration(x) => {
                let NamespaceUseDeclarationChildren { namespace_use_keyword, namespace_use_kind, namespace_use_clauses, namespace_use_semicolon } = *x;
                let acc = f(namespace_use_keyword, acc);
                let acc = f(namespace_use_kind, acc);
                let acc = f(namespace_use_clauses, acc);
                let acc = f(namespace_use_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceGroupUseDeclaration(x) => {
                let NamespaceGroupUseDeclarationChildren { namespace_group_use_keyword, namespace_group_use_kind, namespace_group_use_prefix, namespace_group_use_left_brace, namespace_group_use_clauses, namespace_group_use_right_brace, namespace_group_use_semicolon } = *x;
                let acc = f(namespace_group_use_keyword, acc);
                let acc = f(namespace_group_use_kind, acc);
                let acc = f(namespace_group_use_prefix, acc);
                let acc = f(namespace_group_use_left_brace, acc);
                let acc = f(namespace_group_use_clauses, acc);
                let acc = f(namespace_group_use_right_brace, acc);
                let acc = f(namespace_group_use_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceUseClause(x) => {
                let NamespaceUseClauseChildren { namespace_use_clause_kind, namespace_use_name, namespace_use_as, namespace_use_alias } = *x;
                let acc = f(namespace_use_clause_kind, acc);
                let acc = f(namespace_use_name, acc);
                let acc = f(namespace_use_as, acc);
                let acc = f(namespace_use_alias, acc);
                acc
            },
            SyntaxVariant::FunctionDeclaration(x) => {
                let FunctionDeclarationChildren { function_attribute_spec, function_declaration_header, function_body } = *x;
                let acc = f(function_attribute_spec, acc);
                let acc = f(function_declaration_header, acc);
                let acc = f(function_body, acc);
                acc
            },
            SyntaxVariant::FunctionDeclarationHeader(x) => {
                let FunctionDeclarationHeaderChildren { function_modifiers, function_keyword, function_name, function_type_parameter_list, function_left_paren, function_parameter_list, function_right_paren, function_capability_provisional, function_colon, function_type, function_where_clause } = *x;
                let acc = f(function_modifiers, acc);
                let acc = f(function_keyword, acc);
                let acc = f(function_name, acc);
                let acc = f(function_type_parameter_list, acc);
                let acc = f(function_left_paren, acc);
                let acc = f(function_parameter_list, acc);
                let acc = f(function_right_paren, acc);
                let acc = f(function_capability_provisional, acc);
                let acc = f(function_colon, acc);
                let acc = f(function_type, acc);
                let acc = f(function_where_clause, acc);
                acc
            },
            SyntaxVariant::CapabilityProvisional(x) => {
                let CapabilityProvisionalChildren { capability_provisional_at, capability_provisional_left_brace, capability_provisional_type, capability_provisional_unsafe_plus, capability_provisional_unsafe_type, capability_provisional_right_brace } = *x;
                let acc = f(capability_provisional_at, acc);
                let acc = f(capability_provisional_left_brace, acc);
                let acc = f(capability_provisional_type, acc);
                let acc = f(capability_provisional_unsafe_plus, acc);
                let acc = f(capability_provisional_unsafe_type, acc);
                let acc = f(capability_provisional_right_brace, acc);
                acc
            },
            SyntaxVariant::WhereClause(x) => {
                let WhereClauseChildren { where_clause_keyword, where_clause_constraints } = *x;
                let acc = f(where_clause_keyword, acc);
                let acc = f(where_clause_constraints, acc);
                acc
            },
            SyntaxVariant::WhereConstraint(x) => {
                let WhereConstraintChildren { where_constraint_left_type, where_constraint_operator, where_constraint_right_type } = *x;
                let acc = f(where_constraint_left_type, acc);
                let acc = f(where_constraint_operator, acc);
                let acc = f(where_constraint_right_type, acc);
                acc
            },
            SyntaxVariant::MethodishDeclaration(x) => {
                let MethodishDeclarationChildren { methodish_attribute, methodish_function_decl_header, methodish_function_body, methodish_semicolon } = *x;
                let acc = f(methodish_attribute, acc);
                let acc = f(methodish_function_decl_header, acc);
                let acc = f(methodish_function_body, acc);
                let acc = f(methodish_semicolon, acc);
                acc
            },
            SyntaxVariant::MethodishTraitResolution(x) => {
                let MethodishTraitResolutionChildren { methodish_trait_attribute, methodish_trait_function_decl_header, methodish_trait_equal, methodish_trait_name, methodish_trait_semicolon } = *x;
                let acc = f(methodish_trait_attribute, acc);
                let acc = f(methodish_trait_function_decl_header, acc);
                let acc = f(methodish_trait_equal, acc);
                let acc = f(methodish_trait_name, acc);
                let acc = f(methodish_trait_semicolon, acc);
                acc
            },
            SyntaxVariant::ClassishDeclaration(x) => {
                let ClassishDeclarationChildren { classish_attribute, classish_modifiers, classish_xhp, classish_keyword, classish_name, classish_type_parameters, classish_extends_keyword, classish_extends_list, classish_implements_keyword, classish_implements_list, classish_where_clause, classish_body } = *x;
                let acc = f(classish_attribute, acc);
                let acc = f(classish_modifiers, acc);
                let acc = f(classish_xhp, acc);
                let acc = f(classish_keyword, acc);
                let acc = f(classish_name, acc);
                let acc = f(classish_type_parameters, acc);
                let acc = f(classish_extends_keyword, acc);
                let acc = f(classish_extends_list, acc);
                let acc = f(classish_implements_keyword, acc);
                let acc = f(classish_implements_list, acc);
                let acc = f(classish_where_clause, acc);
                let acc = f(classish_body, acc);
                acc
            },
            SyntaxVariant::ClassishBody(x) => {
                let ClassishBodyChildren { classish_body_left_brace, classish_body_elements, classish_body_right_brace } = *x;
                let acc = f(classish_body_left_brace, acc);
                let acc = f(classish_body_elements, acc);
                let acc = f(classish_body_right_brace, acc);
                acc
            },
            SyntaxVariant::TraitUsePrecedenceItem(x) => {
                let TraitUsePrecedenceItemChildren { trait_use_precedence_item_name, trait_use_precedence_item_keyword, trait_use_precedence_item_removed_names } = *x;
                let acc = f(trait_use_precedence_item_name, acc);
                let acc = f(trait_use_precedence_item_keyword, acc);
                let acc = f(trait_use_precedence_item_removed_names, acc);
                acc
            },
            SyntaxVariant::TraitUseAliasItem(x) => {
                let TraitUseAliasItemChildren { trait_use_alias_item_aliasing_name, trait_use_alias_item_keyword, trait_use_alias_item_modifiers, trait_use_alias_item_aliased_name } = *x;
                let acc = f(trait_use_alias_item_aliasing_name, acc);
                let acc = f(trait_use_alias_item_keyword, acc);
                let acc = f(trait_use_alias_item_modifiers, acc);
                let acc = f(trait_use_alias_item_aliased_name, acc);
                acc
            },
            SyntaxVariant::TraitUseConflictResolution(x) => {
                let TraitUseConflictResolutionChildren { trait_use_conflict_resolution_keyword, trait_use_conflict_resolution_names, trait_use_conflict_resolution_left_brace, trait_use_conflict_resolution_clauses, trait_use_conflict_resolution_right_brace } = *x;
                let acc = f(trait_use_conflict_resolution_keyword, acc);
                let acc = f(trait_use_conflict_resolution_names, acc);
                let acc = f(trait_use_conflict_resolution_left_brace, acc);
                let acc = f(trait_use_conflict_resolution_clauses, acc);
                let acc = f(trait_use_conflict_resolution_right_brace, acc);
                acc
            },
            SyntaxVariant::TraitUse(x) => {
                let TraitUseChildren { trait_use_keyword, trait_use_names, trait_use_semicolon } = *x;
                let acc = f(trait_use_keyword, acc);
                let acc = f(trait_use_names, acc);
                let acc = f(trait_use_semicolon, acc);
                acc
            },
            SyntaxVariant::RequireClause(x) => {
                let RequireClauseChildren { require_keyword, require_kind, require_name, require_semicolon } = *x;
                let acc = f(require_keyword, acc);
                let acc = f(require_kind, acc);
                let acc = f(require_name, acc);
                let acc = f(require_semicolon, acc);
                acc
            },
            SyntaxVariant::ConstDeclaration(x) => {
                let ConstDeclarationChildren { const_modifiers, const_keyword, const_type_specifier, const_declarators, const_semicolon } = *x;
                let acc = f(const_modifiers, acc);
                let acc = f(const_keyword, acc);
                let acc = f(const_type_specifier, acc);
                let acc = f(const_declarators, acc);
                let acc = f(const_semicolon, acc);
                acc
            },
            SyntaxVariant::ConstantDeclarator(x) => {
                let ConstantDeclaratorChildren { constant_declarator_name, constant_declarator_initializer } = *x;
                let acc = f(constant_declarator_name, acc);
                let acc = f(constant_declarator_initializer, acc);
                acc
            },
            SyntaxVariant::TypeConstDeclaration(x) => {
                let TypeConstDeclarationChildren { type_const_attribute_spec, type_const_modifiers, type_const_keyword, type_const_type_keyword, type_const_name, type_const_type_parameters, type_const_type_constraint, type_const_equal, type_const_type_specifier, type_const_semicolon } = *x;
                let acc = f(type_const_attribute_spec, acc);
                let acc = f(type_const_modifiers, acc);
                let acc = f(type_const_keyword, acc);
                let acc = f(type_const_type_keyword, acc);
                let acc = f(type_const_name, acc);
                let acc = f(type_const_type_parameters, acc);
                let acc = f(type_const_type_constraint, acc);
                let acc = f(type_const_equal, acc);
                let acc = f(type_const_type_specifier, acc);
                let acc = f(type_const_semicolon, acc);
                acc
            },
            SyntaxVariant::DecoratedExpression(x) => {
                let DecoratedExpressionChildren { decorated_expression_decorator, decorated_expression_expression } = *x;
                let acc = f(decorated_expression_decorator, acc);
                let acc = f(decorated_expression_expression, acc);
                acc
            },
            SyntaxVariant::ParameterDeclaration(x) => {
                let ParameterDeclarationChildren { parameter_attribute, parameter_visibility, parameter_call_convention, parameter_type, parameter_name, parameter_default_value } = *x;
                let acc = f(parameter_attribute, acc);
                let acc = f(parameter_visibility, acc);
                let acc = f(parameter_call_convention, acc);
                let acc = f(parameter_type, acc);
                let acc = f(parameter_name, acc);
                let acc = f(parameter_default_value, acc);
                acc
            },
            SyntaxVariant::VariadicParameter(x) => {
                let VariadicParameterChildren { variadic_parameter_call_convention, variadic_parameter_type, variadic_parameter_ellipsis } = *x;
                let acc = f(variadic_parameter_call_convention, acc);
                let acc = f(variadic_parameter_type, acc);
                let acc = f(variadic_parameter_ellipsis, acc);
                acc
            },
            SyntaxVariant::OldAttributeSpecification(x) => {
                let OldAttributeSpecificationChildren { old_attribute_specification_left_double_angle, old_attribute_specification_attributes, old_attribute_specification_right_double_angle } = *x;
                let acc = f(old_attribute_specification_left_double_angle, acc);
                let acc = f(old_attribute_specification_attributes, acc);
                let acc = f(old_attribute_specification_right_double_angle, acc);
                acc
            },
            SyntaxVariant::AttributeSpecification(x) => {
                let AttributeSpecificationChildren { attribute_specification_attributes } = *x;
                let acc = f(attribute_specification_attributes, acc);
                acc
            },
            SyntaxVariant::Attribute(x) => {
                let AttributeChildren { attribute_at, attribute_attribute_name } = *x;
                let acc = f(attribute_at, acc);
                let acc = f(attribute_attribute_name, acc);
                acc
            },
            SyntaxVariant::InclusionExpression(x) => {
                let InclusionExpressionChildren { inclusion_require, inclusion_filename } = *x;
                let acc = f(inclusion_require, acc);
                let acc = f(inclusion_filename, acc);
                acc
            },
            SyntaxVariant::InclusionDirective(x) => {
                let InclusionDirectiveChildren { inclusion_expression, inclusion_semicolon } = *x;
                let acc = f(inclusion_expression, acc);
                let acc = f(inclusion_semicolon, acc);
                acc
            },
            SyntaxVariant::CompoundStatement(x) => {
                let CompoundStatementChildren { compound_left_brace, compound_statements, compound_right_brace } = *x;
                let acc = f(compound_left_brace, acc);
                let acc = f(compound_statements, acc);
                let acc = f(compound_right_brace, acc);
                acc
            },
            SyntaxVariant::ExpressionStatement(x) => {
                let ExpressionStatementChildren { expression_statement_expression, expression_statement_semicolon } = *x;
                let acc = f(expression_statement_expression, acc);
                let acc = f(expression_statement_semicolon, acc);
                acc
            },
            SyntaxVariant::MarkupSection(x) => {
                let MarkupSectionChildren { markup_text, markup_suffix } = *x;
                let acc = f(markup_text, acc);
                let acc = f(markup_suffix, acc);
                acc
            },
            SyntaxVariant::MarkupSuffix(x) => {
                let MarkupSuffixChildren { markup_suffix_less_than_question, markup_suffix_name } = *x;
                let acc = f(markup_suffix_less_than_question, acc);
                let acc = f(markup_suffix_name, acc);
                acc
            },
            SyntaxVariant::UnsetStatement(x) => {
                let UnsetStatementChildren { unset_keyword, unset_left_paren, unset_variables, unset_right_paren, unset_semicolon } = *x;
                let acc = f(unset_keyword, acc);
                let acc = f(unset_left_paren, acc);
                let acc = f(unset_variables, acc);
                let acc = f(unset_right_paren, acc);
                let acc = f(unset_semicolon, acc);
                acc
            },
            SyntaxVariant::UsingStatementBlockScoped(x) => {
                let UsingStatementBlockScopedChildren { using_block_await_keyword, using_block_using_keyword, using_block_left_paren, using_block_expressions, using_block_right_paren, using_block_body } = *x;
                let acc = f(using_block_await_keyword, acc);
                let acc = f(using_block_using_keyword, acc);
                let acc = f(using_block_left_paren, acc);
                let acc = f(using_block_expressions, acc);
                let acc = f(using_block_right_paren, acc);
                let acc = f(using_block_body, acc);
                acc
            },
            SyntaxVariant::UsingStatementFunctionScoped(x) => {
                let UsingStatementFunctionScopedChildren { using_function_await_keyword, using_function_using_keyword, using_function_expression, using_function_semicolon } = *x;
                let acc = f(using_function_await_keyword, acc);
                let acc = f(using_function_using_keyword, acc);
                let acc = f(using_function_expression, acc);
                let acc = f(using_function_semicolon, acc);
                acc
            },
            SyntaxVariant::WhileStatement(x) => {
                let WhileStatementChildren { while_keyword, while_left_paren, while_condition, while_right_paren, while_body } = *x;
                let acc = f(while_keyword, acc);
                let acc = f(while_left_paren, acc);
                let acc = f(while_condition, acc);
                let acc = f(while_right_paren, acc);
                let acc = f(while_body, acc);
                acc
            },
            SyntaxVariant::IfStatement(x) => {
                let IfStatementChildren { if_keyword, if_left_paren, if_condition, if_right_paren, if_statement, if_elseif_clauses, if_else_clause } = *x;
                let acc = f(if_keyword, acc);
                let acc = f(if_left_paren, acc);
                let acc = f(if_condition, acc);
                let acc = f(if_right_paren, acc);
                let acc = f(if_statement, acc);
                let acc = f(if_elseif_clauses, acc);
                let acc = f(if_else_clause, acc);
                acc
            },
            SyntaxVariant::ElseifClause(x) => {
                let ElseifClauseChildren { elseif_keyword, elseif_left_paren, elseif_condition, elseif_right_paren, elseif_statement } = *x;
                let acc = f(elseif_keyword, acc);
                let acc = f(elseif_left_paren, acc);
                let acc = f(elseif_condition, acc);
                let acc = f(elseif_right_paren, acc);
                let acc = f(elseif_statement, acc);
                acc
            },
            SyntaxVariant::ElseClause(x) => {
                let ElseClauseChildren { else_keyword, else_statement } = *x;
                let acc = f(else_keyword, acc);
                let acc = f(else_statement, acc);
                acc
            },
            SyntaxVariant::TryStatement(x) => {
                let TryStatementChildren { try_keyword, try_compound_statement, try_catch_clauses, try_finally_clause } = *x;
                let acc = f(try_keyword, acc);
                let acc = f(try_compound_statement, acc);
                let acc = f(try_catch_clauses, acc);
                let acc = f(try_finally_clause, acc);
                acc
            },
            SyntaxVariant::CatchClause(x) => {
                let CatchClauseChildren { catch_keyword, catch_left_paren, catch_type, catch_variable, catch_right_paren, catch_body } = *x;
                let acc = f(catch_keyword, acc);
                let acc = f(catch_left_paren, acc);
                let acc = f(catch_type, acc);
                let acc = f(catch_variable, acc);
                let acc = f(catch_right_paren, acc);
                let acc = f(catch_body, acc);
                acc
            },
            SyntaxVariant::FinallyClause(x) => {
                let FinallyClauseChildren { finally_keyword, finally_body } = *x;
                let acc = f(finally_keyword, acc);
                let acc = f(finally_body, acc);
                acc
            },
            SyntaxVariant::DoStatement(x) => {
                let DoStatementChildren { do_keyword, do_body, do_while_keyword, do_left_paren, do_condition, do_right_paren, do_semicolon } = *x;
                let acc = f(do_keyword, acc);
                let acc = f(do_body, acc);
                let acc = f(do_while_keyword, acc);
                let acc = f(do_left_paren, acc);
                let acc = f(do_condition, acc);
                let acc = f(do_right_paren, acc);
                let acc = f(do_semicolon, acc);
                acc
            },
            SyntaxVariant::ForStatement(x) => {
                let ForStatementChildren { for_keyword, for_left_paren, for_initializer, for_first_semicolon, for_control, for_second_semicolon, for_end_of_loop, for_right_paren, for_body } = *x;
                let acc = f(for_keyword, acc);
                let acc = f(for_left_paren, acc);
                let acc = f(for_initializer, acc);
                let acc = f(for_first_semicolon, acc);
                let acc = f(for_control, acc);
                let acc = f(for_second_semicolon, acc);
                let acc = f(for_end_of_loop, acc);
                let acc = f(for_right_paren, acc);
                let acc = f(for_body, acc);
                acc
            },
            SyntaxVariant::ForeachStatement(x) => {
                let ForeachStatementChildren { foreach_keyword, foreach_left_paren, foreach_collection, foreach_await_keyword, foreach_as, foreach_key, foreach_arrow, foreach_value, foreach_right_paren, foreach_body } = *x;
                let acc = f(foreach_keyword, acc);
                let acc = f(foreach_left_paren, acc);
                let acc = f(foreach_collection, acc);
                let acc = f(foreach_await_keyword, acc);
                let acc = f(foreach_as, acc);
                let acc = f(foreach_key, acc);
                let acc = f(foreach_arrow, acc);
                let acc = f(foreach_value, acc);
                let acc = f(foreach_right_paren, acc);
                let acc = f(foreach_body, acc);
                acc
            },
            SyntaxVariant::SwitchStatement(x) => {
                let SwitchStatementChildren { switch_keyword, switch_left_paren, switch_expression, switch_right_paren, switch_left_brace, switch_sections, switch_right_brace } = *x;
                let acc = f(switch_keyword, acc);
                let acc = f(switch_left_paren, acc);
                let acc = f(switch_expression, acc);
                let acc = f(switch_right_paren, acc);
                let acc = f(switch_left_brace, acc);
                let acc = f(switch_sections, acc);
                let acc = f(switch_right_brace, acc);
                acc
            },
            SyntaxVariant::SwitchSection(x) => {
                let SwitchSectionChildren { switch_section_labels, switch_section_statements, switch_section_fallthrough } = *x;
                let acc = f(switch_section_labels, acc);
                let acc = f(switch_section_statements, acc);
                let acc = f(switch_section_fallthrough, acc);
                acc
            },
            SyntaxVariant::SwitchFallthrough(x) => {
                let SwitchFallthroughChildren { fallthrough_keyword, fallthrough_semicolon } = *x;
                let acc = f(fallthrough_keyword, acc);
                let acc = f(fallthrough_semicolon, acc);
                acc
            },
            SyntaxVariant::CaseLabel(x) => {
                let CaseLabelChildren { case_keyword, case_expression, case_colon } = *x;
                let acc = f(case_keyword, acc);
                let acc = f(case_expression, acc);
                let acc = f(case_colon, acc);
                acc
            },
            SyntaxVariant::DefaultLabel(x) => {
                let DefaultLabelChildren { default_keyword, default_colon } = *x;
                let acc = f(default_keyword, acc);
                let acc = f(default_colon, acc);
                acc
            },
            SyntaxVariant::ReturnStatement(x) => {
                let ReturnStatementChildren { return_keyword, return_expression, return_semicolon } = *x;
                let acc = f(return_keyword, acc);
                let acc = f(return_expression, acc);
                let acc = f(return_semicolon, acc);
                acc
            },
            SyntaxVariant::GotoLabel(x) => {
                let GotoLabelChildren { goto_label_name, goto_label_colon } = *x;
                let acc = f(goto_label_name, acc);
                let acc = f(goto_label_colon, acc);
                acc
            },
            SyntaxVariant::GotoStatement(x) => {
                let GotoStatementChildren { goto_statement_keyword, goto_statement_label_name, goto_statement_semicolon } = *x;
                let acc = f(goto_statement_keyword, acc);
                let acc = f(goto_statement_label_name, acc);
                let acc = f(goto_statement_semicolon, acc);
                acc
            },
            SyntaxVariant::ThrowStatement(x) => {
                let ThrowStatementChildren { throw_keyword, throw_expression, throw_semicolon } = *x;
                let acc = f(throw_keyword, acc);
                let acc = f(throw_expression, acc);
                let acc = f(throw_semicolon, acc);
                acc
            },
            SyntaxVariant::BreakStatement(x) => {
                let BreakStatementChildren { break_keyword, break_semicolon } = *x;
                let acc = f(break_keyword, acc);
                let acc = f(break_semicolon, acc);
                acc
            },
            SyntaxVariant::ContinueStatement(x) => {
                let ContinueStatementChildren { continue_keyword, continue_semicolon } = *x;
                let acc = f(continue_keyword, acc);
                let acc = f(continue_semicolon, acc);
                acc
            },
            SyntaxVariant::EchoStatement(x) => {
                let EchoStatementChildren { echo_keyword, echo_expressions, echo_semicolon } = *x;
                let acc = f(echo_keyword, acc);
                let acc = f(echo_expressions, acc);
                let acc = f(echo_semicolon, acc);
                acc
            },
            SyntaxVariant::ConcurrentStatement(x) => {
                let ConcurrentStatementChildren { concurrent_keyword, concurrent_statement } = *x;
                let acc = f(concurrent_keyword, acc);
                let acc = f(concurrent_statement, acc);
                acc
            },
            SyntaxVariant::SimpleInitializer(x) => {
                let SimpleInitializerChildren { simple_initializer_equal, simple_initializer_value } = *x;
                let acc = f(simple_initializer_equal, acc);
                let acc = f(simple_initializer_value, acc);
                acc
            },
            SyntaxVariant::AnonymousClass(x) => {
                let AnonymousClassChildren { anonymous_class_class_keyword, anonymous_class_left_paren, anonymous_class_argument_list, anonymous_class_right_paren, anonymous_class_extends_keyword, anonymous_class_extends_list, anonymous_class_implements_keyword, anonymous_class_implements_list, anonymous_class_body } = *x;
                let acc = f(anonymous_class_class_keyword, acc);
                let acc = f(anonymous_class_left_paren, acc);
                let acc = f(anonymous_class_argument_list, acc);
                let acc = f(anonymous_class_right_paren, acc);
                let acc = f(anonymous_class_extends_keyword, acc);
                let acc = f(anonymous_class_extends_list, acc);
                let acc = f(anonymous_class_implements_keyword, acc);
                let acc = f(anonymous_class_implements_list, acc);
                let acc = f(anonymous_class_body, acc);
                acc
            },
            SyntaxVariant::AnonymousFunction(x) => {
                let AnonymousFunctionChildren { anonymous_attribute_spec, anonymous_static_keyword, anonymous_async_keyword, anonymous_function_keyword, anonymous_left_paren, anonymous_parameters, anonymous_right_paren, anonymous_colon, anonymous_type, anonymous_use, anonymous_body } = *x;
                let acc = f(anonymous_attribute_spec, acc);
                let acc = f(anonymous_static_keyword, acc);
                let acc = f(anonymous_async_keyword, acc);
                let acc = f(anonymous_function_keyword, acc);
                let acc = f(anonymous_left_paren, acc);
                let acc = f(anonymous_parameters, acc);
                let acc = f(anonymous_right_paren, acc);
                let acc = f(anonymous_colon, acc);
                let acc = f(anonymous_type, acc);
                let acc = f(anonymous_use, acc);
                let acc = f(anonymous_body, acc);
                acc
            },
            SyntaxVariant::AnonymousFunctionUseClause(x) => {
                let AnonymousFunctionUseClauseChildren { anonymous_use_keyword, anonymous_use_left_paren, anonymous_use_variables, anonymous_use_right_paren } = *x;
                let acc = f(anonymous_use_keyword, acc);
                let acc = f(anonymous_use_left_paren, acc);
                let acc = f(anonymous_use_variables, acc);
                let acc = f(anonymous_use_right_paren, acc);
                acc
            },
            SyntaxVariant::LambdaExpression(x) => {
                let LambdaExpressionChildren { lambda_attribute_spec, lambda_async, lambda_signature, lambda_arrow, lambda_body } = *x;
                let acc = f(lambda_attribute_spec, acc);
                let acc = f(lambda_async, acc);
                let acc = f(lambda_signature, acc);
                let acc = f(lambda_arrow, acc);
                let acc = f(lambda_body, acc);
                acc
            },
            SyntaxVariant::LambdaSignature(x) => {
                let LambdaSignatureChildren { lambda_left_paren, lambda_parameters, lambda_right_paren, lambda_colon, lambda_type } = *x;
                let acc = f(lambda_left_paren, acc);
                let acc = f(lambda_parameters, acc);
                let acc = f(lambda_right_paren, acc);
                let acc = f(lambda_colon, acc);
                let acc = f(lambda_type, acc);
                acc
            },
            SyntaxVariant::CastExpression(x) => {
                let CastExpressionChildren { cast_left_paren, cast_type, cast_right_paren, cast_operand } = *x;
                let acc = f(cast_left_paren, acc);
                let acc = f(cast_type, acc);
                let acc = f(cast_right_paren, acc);
                let acc = f(cast_operand, acc);
                acc
            },
            SyntaxVariant::ScopeResolutionExpression(x) => {
                let ScopeResolutionExpressionChildren { scope_resolution_qualifier, scope_resolution_operator, scope_resolution_name } = *x;
                let acc = f(scope_resolution_qualifier, acc);
                let acc = f(scope_resolution_operator, acc);
                let acc = f(scope_resolution_name, acc);
                acc
            },
            SyntaxVariant::MemberSelectionExpression(x) => {
                let MemberSelectionExpressionChildren { member_object, member_operator, member_name } = *x;
                let acc = f(member_object, acc);
                let acc = f(member_operator, acc);
                let acc = f(member_name, acc);
                acc
            },
            SyntaxVariant::SafeMemberSelectionExpression(x) => {
                let SafeMemberSelectionExpressionChildren { safe_member_object, safe_member_operator, safe_member_name } = *x;
                let acc = f(safe_member_object, acc);
                let acc = f(safe_member_operator, acc);
                let acc = f(safe_member_name, acc);
                acc
            },
            SyntaxVariant::EmbeddedMemberSelectionExpression(x) => {
                let EmbeddedMemberSelectionExpressionChildren { embedded_member_object, embedded_member_operator, embedded_member_name } = *x;
                let acc = f(embedded_member_object, acc);
                let acc = f(embedded_member_operator, acc);
                let acc = f(embedded_member_name, acc);
                acc
            },
            SyntaxVariant::YieldExpression(x) => {
                let YieldExpressionChildren { yield_keyword, yield_operand } = *x;
                let acc = f(yield_keyword, acc);
                let acc = f(yield_operand, acc);
                acc
            },
            SyntaxVariant::PrefixUnaryExpression(x) => {
                let PrefixUnaryExpressionChildren { prefix_unary_operator, prefix_unary_operand } = *x;
                let acc = f(prefix_unary_operator, acc);
                let acc = f(prefix_unary_operand, acc);
                acc
            },
            SyntaxVariant::PostfixUnaryExpression(x) => {
                let PostfixUnaryExpressionChildren { postfix_unary_operand, postfix_unary_operator } = *x;
                let acc = f(postfix_unary_operand, acc);
                let acc = f(postfix_unary_operator, acc);
                acc
            },
            SyntaxVariant::BinaryExpression(x) => {
                let BinaryExpressionChildren { binary_left_operand, binary_operator, binary_right_operand } = *x;
                let acc = f(binary_left_operand, acc);
                let acc = f(binary_operator, acc);
                let acc = f(binary_right_operand, acc);
                acc
            },
            SyntaxVariant::IsExpression(x) => {
                let IsExpressionChildren { is_left_operand, is_operator, is_right_operand } = *x;
                let acc = f(is_left_operand, acc);
                let acc = f(is_operator, acc);
                let acc = f(is_right_operand, acc);
                acc
            },
            SyntaxVariant::AsExpression(x) => {
                let AsExpressionChildren { as_left_operand, as_operator, as_right_operand } = *x;
                let acc = f(as_left_operand, acc);
                let acc = f(as_operator, acc);
                let acc = f(as_right_operand, acc);
                acc
            },
            SyntaxVariant::NullableAsExpression(x) => {
                let NullableAsExpressionChildren { nullable_as_left_operand, nullable_as_operator, nullable_as_right_operand } = *x;
                let acc = f(nullable_as_left_operand, acc);
                let acc = f(nullable_as_operator, acc);
                let acc = f(nullable_as_right_operand, acc);
                acc
            },
            SyntaxVariant::ConditionalExpression(x) => {
                let ConditionalExpressionChildren { conditional_test, conditional_question, conditional_consequence, conditional_colon, conditional_alternative } = *x;
                let acc = f(conditional_test, acc);
                let acc = f(conditional_question, acc);
                let acc = f(conditional_consequence, acc);
                let acc = f(conditional_colon, acc);
                let acc = f(conditional_alternative, acc);
                acc
            },
            SyntaxVariant::EvalExpression(x) => {
                let EvalExpressionChildren { eval_keyword, eval_left_paren, eval_argument, eval_right_paren } = *x;
                let acc = f(eval_keyword, acc);
                let acc = f(eval_left_paren, acc);
                let acc = f(eval_argument, acc);
                let acc = f(eval_right_paren, acc);
                acc
            },
            SyntaxVariant::DefineExpression(x) => {
                let DefineExpressionChildren { define_keyword, define_left_paren, define_argument_list, define_right_paren } = *x;
                let acc = f(define_keyword, acc);
                let acc = f(define_left_paren, acc);
                let acc = f(define_argument_list, acc);
                let acc = f(define_right_paren, acc);
                acc
            },
            SyntaxVariant::IssetExpression(x) => {
                let IssetExpressionChildren { isset_keyword, isset_left_paren, isset_argument_list, isset_right_paren } = *x;
                let acc = f(isset_keyword, acc);
                let acc = f(isset_left_paren, acc);
                let acc = f(isset_argument_list, acc);
                let acc = f(isset_right_paren, acc);
                acc
            },
            SyntaxVariant::FunctionCallExpression(x) => {
                let FunctionCallExpressionChildren { function_call_receiver, function_call_type_args, function_call_left_paren, function_call_argument_list, function_call_right_paren } = *x;
                let acc = f(function_call_receiver, acc);
                let acc = f(function_call_type_args, acc);
                let acc = f(function_call_left_paren, acc);
                let acc = f(function_call_argument_list, acc);
                let acc = f(function_call_right_paren, acc);
                acc
            },
            SyntaxVariant::FunctionPointerExpression(x) => {
                let FunctionPointerExpressionChildren { function_pointer_receiver, function_pointer_type_args } = *x;
                let acc = f(function_pointer_receiver, acc);
                let acc = f(function_pointer_type_args, acc);
                acc
            },
            SyntaxVariant::ParenthesizedExpression(x) => {
                let ParenthesizedExpressionChildren { parenthesized_expression_left_paren, parenthesized_expression_expression, parenthesized_expression_right_paren } = *x;
                let acc = f(parenthesized_expression_left_paren, acc);
                let acc = f(parenthesized_expression_expression, acc);
                let acc = f(parenthesized_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::BracedExpression(x) => {
                let BracedExpressionChildren { braced_expression_left_brace, braced_expression_expression, braced_expression_right_brace } = *x;
                let acc = f(braced_expression_left_brace, acc);
                let acc = f(braced_expression_expression, acc);
                let acc = f(braced_expression_right_brace, acc);
                acc
            },
            SyntaxVariant::EmbeddedBracedExpression(x) => {
                let EmbeddedBracedExpressionChildren { embedded_braced_expression_left_brace, embedded_braced_expression_expression, embedded_braced_expression_right_brace } = *x;
                let acc = f(embedded_braced_expression_left_brace, acc);
                let acc = f(embedded_braced_expression_expression, acc);
                let acc = f(embedded_braced_expression_right_brace, acc);
                acc
            },
            SyntaxVariant::ListExpression(x) => {
                let ListExpressionChildren { list_keyword, list_left_paren, list_members, list_right_paren } = *x;
                let acc = f(list_keyword, acc);
                let acc = f(list_left_paren, acc);
                let acc = f(list_members, acc);
                let acc = f(list_right_paren, acc);
                acc
            },
            SyntaxVariant::CollectionLiteralExpression(x) => {
                let CollectionLiteralExpressionChildren { collection_literal_name, collection_literal_left_brace, collection_literal_initializers, collection_literal_right_brace } = *x;
                let acc = f(collection_literal_name, acc);
                let acc = f(collection_literal_left_brace, acc);
                let acc = f(collection_literal_initializers, acc);
                let acc = f(collection_literal_right_brace, acc);
                acc
            },
            SyntaxVariant::ObjectCreationExpression(x) => {
                let ObjectCreationExpressionChildren { object_creation_new_keyword, object_creation_object } = *x;
                let acc = f(object_creation_new_keyword, acc);
                let acc = f(object_creation_object, acc);
                acc
            },
            SyntaxVariant::ConstructorCall(x) => {
                let ConstructorCallChildren { constructor_call_type, constructor_call_left_paren, constructor_call_argument_list, constructor_call_right_paren } = *x;
                let acc = f(constructor_call_type, acc);
                let acc = f(constructor_call_left_paren, acc);
                let acc = f(constructor_call_argument_list, acc);
                let acc = f(constructor_call_right_paren, acc);
                acc
            },
            SyntaxVariant::RecordCreationExpression(x) => {
                let RecordCreationExpressionChildren { record_creation_type, record_creation_left_bracket, record_creation_members, record_creation_right_bracket } = *x;
                let acc = f(record_creation_type, acc);
                let acc = f(record_creation_left_bracket, acc);
                let acc = f(record_creation_members, acc);
                let acc = f(record_creation_right_bracket, acc);
                acc
            },
            SyntaxVariant::DarrayIntrinsicExpression(x) => {
                let DarrayIntrinsicExpressionChildren { darray_intrinsic_keyword, darray_intrinsic_explicit_type, darray_intrinsic_left_bracket, darray_intrinsic_members, darray_intrinsic_right_bracket } = *x;
                let acc = f(darray_intrinsic_keyword, acc);
                let acc = f(darray_intrinsic_explicit_type, acc);
                let acc = f(darray_intrinsic_left_bracket, acc);
                let acc = f(darray_intrinsic_members, acc);
                let acc = f(darray_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::DictionaryIntrinsicExpression(x) => {
                let DictionaryIntrinsicExpressionChildren { dictionary_intrinsic_keyword, dictionary_intrinsic_explicit_type, dictionary_intrinsic_left_bracket, dictionary_intrinsic_members, dictionary_intrinsic_right_bracket } = *x;
                let acc = f(dictionary_intrinsic_keyword, acc);
                let acc = f(dictionary_intrinsic_explicit_type, acc);
                let acc = f(dictionary_intrinsic_left_bracket, acc);
                let acc = f(dictionary_intrinsic_members, acc);
                let acc = f(dictionary_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::KeysetIntrinsicExpression(x) => {
                let KeysetIntrinsicExpressionChildren { keyset_intrinsic_keyword, keyset_intrinsic_explicit_type, keyset_intrinsic_left_bracket, keyset_intrinsic_members, keyset_intrinsic_right_bracket } = *x;
                let acc = f(keyset_intrinsic_keyword, acc);
                let acc = f(keyset_intrinsic_explicit_type, acc);
                let acc = f(keyset_intrinsic_left_bracket, acc);
                let acc = f(keyset_intrinsic_members, acc);
                let acc = f(keyset_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::VarrayIntrinsicExpression(x) => {
                let VarrayIntrinsicExpressionChildren { varray_intrinsic_keyword, varray_intrinsic_explicit_type, varray_intrinsic_left_bracket, varray_intrinsic_members, varray_intrinsic_right_bracket } = *x;
                let acc = f(varray_intrinsic_keyword, acc);
                let acc = f(varray_intrinsic_explicit_type, acc);
                let acc = f(varray_intrinsic_left_bracket, acc);
                let acc = f(varray_intrinsic_members, acc);
                let acc = f(varray_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::VectorIntrinsicExpression(x) => {
                let VectorIntrinsicExpressionChildren { vector_intrinsic_keyword, vector_intrinsic_explicit_type, vector_intrinsic_left_bracket, vector_intrinsic_members, vector_intrinsic_right_bracket } = *x;
                let acc = f(vector_intrinsic_keyword, acc);
                let acc = f(vector_intrinsic_explicit_type, acc);
                let acc = f(vector_intrinsic_left_bracket, acc);
                let acc = f(vector_intrinsic_members, acc);
                let acc = f(vector_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::ElementInitializer(x) => {
                let ElementInitializerChildren { element_key, element_arrow, element_value } = *x;
                let acc = f(element_key, acc);
                let acc = f(element_arrow, acc);
                let acc = f(element_value, acc);
                acc
            },
            SyntaxVariant::SubscriptExpression(x) => {
                let SubscriptExpressionChildren { subscript_receiver, subscript_left_bracket, subscript_index, subscript_right_bracket } = *x;
                let acc = f(subscript_receiver, acc);
                let acc = f(subscript_left_bracket, acc);
                let acc = f(subscript_index, acc);
                let acc = f(subscript_right_bracket, acc);
                acc
            },
            SyntaxVariant::EmbeddedSubscriptExpression(x) => {
                let EmbeddedSubscriptExpressionChildren { embedded_subscript_receiver, embedded_subscript_left_bracket, embedded_subscript_index, embedded_subscript_right_bracket } = *x;
                let acc = f(embedded_subscript_receiver, acc);
                let acc = f(embedded_subscript_left_bracket, acc);
                let acc = f(embedded_subscript_index, acc);
                let acc = f(embedded_subscript_right_bracket, acc);
                acc
            },
            SyntaxVariant::AwaitableCreationExpression(x) => {
                let AwaitableCreationExpressionChildren { awaitable_attribute_spec, awaitable_async, awaitable_compound_statement } = *x;
                let acc = f(awaitable_attribute_spec, acc);
                let acc = f(awaitable_async, acc);
                let acc = f(awaitable_compound_statement, acc);
                acc
            },
            SyntaxVariant::XHPChildrenDeclaration(x) => {
                let XHPChildrenDeclarationChildren { xhp_children_keyword, xhp_children_expression, xhp_children_semicolon } = *x;
                let acc = f(xhp_children_keyword, acc);
                let acc = f(xhp_children_expression, acc);
                let acc = f(xhp_children_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPChildrenParenthesizedList(x) => {
                let XHPChildrenParenthesizedListChildren { xhp_children_list_left_paren, xhp_children_list_xhp_children, xhp_children_list_right_paren } = *x;
                let acc = f(xhp_children_list_left_paren, acc);
                let acc = f(xhp_children_list_xhp_children, acc);
                let acc = f(xhp_children_list_right_paren, acc);
                acc
            },
            SyntaxVariant::XHPCategoryDeclaration(x) => {
                let XHPCategoryDeclarationChildren { xhp_category_keyword, xhp_category_categories, xhp_category_semicolon } = *x;
                let acc = f(xhp_category_keyword, acc);
                let acc = f(xhp_category_categories, acc);
                let acc = f(xhp_category_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPEnumType(x) => {
                let XHPEnumTypeChildren { xhp_enum_keyword, xhp_enum_left_brace, xhp_enum_values, xhp_enum_right_brace } = *x;
                let acc = f(xhp_enum_keyword, acc);
                let acc = f(xhp_enum_left_brace, acc);
                let acc = f(xhp_enum_values, acc);
                let acc = f(xhp_enum_right_brace, acc);
                acc
            },
            SyntaxVariant::XHPLateinit(x) => {
                let XHPLateinitChildren { xhp_lateinit_at, xhp_lateinit_keyword } = *x;
                let acc = f(xhp_lateinit_at, acc);
                let acc = f(xhp_lateinit_keyword, acc);
                acc
            },
            SyntaxVariant::XHPRequired(x) => {
                let XHPRequiredChildren { xhp_required_at, xhp_required_keyword } = *x;
                let acc = f(xhp_required_at, acc);
                let acc = f(xhp_required_keyword, acc);
                acc
            },
            SyntaxVariant::XHPClassAttributeDeclaration(x) => {
                let XHPClassAttributeDeclarationChildren { xhp_attribute_keyword, xhp_attribute_attributes, xhp_attribute_semicolon } = *x;
                let acc = f(xhp_attribute_keyword, acc);
                let acc = f(xhp_attribute_attributes, acc);
                let acc = f(xhp_attribute_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPClassAttribute(x) => {
                let XHPClassAttributeChildren { xhp_attribute_decl_type, xhp_attribute_decl_name, xhp_attribute_decl_initializer, xhp_attribute_decl_required } = *x;
                let acc = f(xhp_attribute_decl_type, acc);
                let acc = f(xhp_attribute_decl_name, acc);
                let acc = f(xhp_attribute_decl_initializer, acc);
                let acc = f(xhp_attribute_decl_required, acc);
                acc
            },
            SyntaxVariant::XHPSimpleClassAttribute(x) => {
                let XHPSimpleClassAttributeChildren { xhp_simple_class_attribute_type } = *x;
                let acc = f(xhp_simple_class_attribute_type, acc);
                acc
            },
            SyntaxVariant::XHPSimpleAttribute(x) => {
                let XHPSimpleAttributeChildren { xhp_simple_attribute_name, xhp_simple_attribute_equal, xhp_simple_attribute_expression } = *x;
                let acc = f(xhp_simple_attribute_name, acc);
                let acc = f(xhp_simple_attribute_equal, acc);
                let acc = f(xhp_simple_attribute_expression, acc);
                acc
            },
            SyntaxVariant::XHPSpreadAttribute(x) => {
                let XHPSpreadAttributeChildren { xhp_spread_attribute_left_brace, xhp_spread_attribute_spread_operator, xhp_spread_attribute_expression, xhp_spread_attribute_right_brace } = *x;
                let acc = f(xhp_spread_attribute_left_brace, acc);
                let acc = f(xhp_spread_attribute_spread_operator, acc);
                let acc = f(xhp_spread_attribute_expression, acc);
                let acc = f(xhp_spread_attribute_right_brace, acc);
                acc
            },
            SyntaxVariant::XHPOpen(x) => {
                let XHPOpenChildren { xhp_open_left_angle, xhp_open_name, xhp_open_attributes, xhp_open_right_angle } = *x;
                let acc = f(xhp_open_left_angle, acc);
                let acc = f(xhp_open_name, acc);
                let acc = f(xhp_open_attributes, acc);
                let acc = f(xhp_open_right_angle, acc);
                acc
            },
            SyntaxVariant::XHPExpression(x) => {
                let XHPExpressionChildren { xhp_open, xhp_body, xhp_close } = *x;
                let acc = f(xhp_open, acc);
                let acc = f(xhp_body, acc);
                let acc = f(xhp_close, acc);
                acc
            },
            SyntaxVariant::XHPClose(x) => {
                let XHPCloseChildren { xhp_close_left_angle, xhp_close_name, xhp_close_right_angle } = *x;
                let acc = f(xhp_close_left_angle, acc);
                let acc = f(xhp_close_name, acc);
                let acc = f(xhp_close_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeConstant(x) => {
                let TypeConstantChildren { type_constant_left_type, type_constant_separator, type_constant_right_type } = *x;
                let acc = f(type_constant_left_type, acc);
                let acc = f(type_constant_separator, acc);
                let acc = f(type_constant_right_type, acc);
                acc
            },
            SyntaxVariant::PUAccess(x) => {
                let PUAccessChildren { pu_access_left_type, pu_access_separator, pu_access_right_type } = *x;
                let acc = f(pu_access_left_type, acc);
                let acc = f(pu_access_separator, acc);
                let acc = f(pu_access_right_type, acc);
                acc
            },
            SyntaxVariant::VectorTypeSpecifier(x) => {
                let VectorTypeSpecifierChildren { vector_type_keyword, vector_type_left_angle, vector_type_type, vector_type_trailing_comma, vector_type_right_angle } = *x;
                let acc = f(vector_type_keyword, acc);
                let acc = f(vector_type_left_angle, acc);
                let acc = f(vector_type_type, acc);
                let acc = f(vector_type_trailing_comma, acc);
                let acc = f(vector_type_right_angle, acc);
                acc
            },
            SyntaxVariant::KeysetTypeSpecifier(x) => {
                let KeysetTypeSpecifierChildren { keyset_type_keyword, keyset_type_left_angle, keyset_type_type, keyset_type_trailing_comma, keyset_type_right_angle } = *x;
                let acc = f(keyset_type_keyword, acc);
                let acc = f(keyset_type_left_angle, acc);
                let acc = f(keyset_type_type, acc);
                let acc = f(keyset_type_trailing_comma, acc);
                let acc = f(keyset_type_right_angle, acc);
                acc
            },
            SyntaxVariant::TupleTypeExplicitSpecifier(x) => {
                let TupleTypeExplicitSpecifierChildren { tuple_type_keyword, tuple_type_left_angle, tuple_type_types, tuple_type_right_angle } = *x;
                let acc = f(tuple_type_keyword, acc);
                let acc = f(tuple_type_left_angle, acc);
                let acc = f(tuple_type_types, acc);
                let acc = f(tuple_type_right_angle, acc);
                acc
            },
            SyntaxVariant::VarrayTypeSpecifier(x) => {
                let VarrayTypeSpecifierChildren { varray_keyword, varray_left_angle, varray_type, varray_trailing_comma, varray_right_angle } = *x;
                let acc = f(varray_keyword, acc);
                let acc = f(varray_left_angle, acc);
                let acc = f(varray_type, acc);
                let acc = f(varray_trailing_comma, acc);
                let acc = f(varray_right_angle, acc);
                acc
            },
            SyntaxVariant::VectorArrayTypeSpecifier(x) => {
                let VectorArrayTypeSpecifierChildren { vector_array_keyword, vector_array_left_angle, vector_array_type, vector_array_right_angle } = *x;
                let acc = f(vector_array_keyword, acc);
                let acc = f(vector_array_left_angle, acc);
                let acc = f(vector_array_type, acc);
                let acc = f(vector_array_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeParameter(x) => {
                let TypeParameterChildren { type_attribute_spec, type_reified, type_variance, type_name, type_param_params, type_constraints } = *x;
                let acc = f(type_attribute_spec, acc);
                let acc = f(type_reified, acc);
                let acc = f(type_variance, acc);
                let acc = f(type_name, acc);
                let acc = f(type_param_params, acc);
                let acc = f(type_constraints, acc);
                acc
            },
            SyntaxVariant::TypeConstraint(x) => {
                let TypeConstraintChildren { constraint_keyword, constraint_type } = *x;
                let acc = f(constraint_keyword, acc);
                let acc = f(constraint_type, acc);
                acc
            },
            SyntaxVariant::DarrayTypeSpecifier(x) => {
                let DarrayTypeSpecifierChildren { darray_keyword, darray_left_angle, darray_key, darray_comma, darray_value, darray_trailing_comma, darray_right_angle } = *x;
                let acc = f(darray_keyword, acc);
                let acc = f(darray_left_angle, acc);
                let acc = f(darray_key, acc);
                let acc = f(darray_comma, acc);
                let acc = f(darray_value, acc);
                let acc = f(darray_trailing_comma, acc);
                let acc = f(darray_right_angle, acc);
                acc
            },
            SyntaxVariant::MapArrayTypeSpecifier(x) => {
                let MapArrayTypeSpecifierChildren { map_array_keyword, map_array_left_angle, map_array_key, map_array_comma, map_array_value, map_array_right_angle } = *x;
                let acc = f(map_array_keyword, acc);
                let acc = f(map_array_left_angle, acc);
                let acc = f(map_array_key, acc);
                let acc = f(map_array_comma, acc);
                let acc = f(map_array_value, acc);
                let acc = f(map_array_right_angle, acc);
                acc
            },
            SyntaxVariant::DictionaryTypeSpecifier(x) => {
                let DictionaryTypeSpecifierChildren { dictionary_type_keyword, dictionary_type_left_angle, dictionary_type_members, dictionary_type_right_angle } = *x;
                let acc = f(dictionary_type_keyword, acc);
                let acc = f(dictionary_type_left_angle, acc);
                let acc = f(dictionary_type_members, acc);
                let acc = f(dictionary_type_right_angle, acc);
                acc
            },
            SyntaxVariant::ClosureTypeSpecifier(x) => {
                let ClosureTypeSpecifierChildren { closure_outer_left_paren, closure_function_keyword, closure_inner_left_paren, closure_parameter_list, closure_inner_right_paren, closure_colon, closure_return_type, closure_outer_right_paren } = *x;
                let acc = f(closure_outer_left_paren, acc);
                let acc = f(closure_function_keyword, acc);
                let acc = f(closure_inner_left_paren, acc);
                let acc = f(closure_parameter_list, acc);
                let acc = f(closure_inner_right_paren, acc);
                let acc = f(closure_colon, acc);
                let acc = f(closure_return_type, acc);
                let acc = f(closure_outer_right_paren, acc);
                acc
            },
            SyntaxVariant::ClosureParameterTypeSpecifier(x) => {
                let ClosureParameterTypeSpecifierChildren { closure_parameter_call_convention, closure_parameter_type } = *x;
                let acc = f(closure_parameter_call_convention, acc);
                let acc = f(closure_parameter_type, acc);
                acc
            },
            SyntaxVariant::ClassnameTypeSpecifier(x) => {
                let ClassnameTypeSpecifierChildren { classname_keyword, classname_left_angle, classname_type, classname_trailing_comma, classname_right_angle } = *x;
                let acc = f(classname_keyword, acc);
                let acc = f(classname_left_angle, acc);
                let acc = f(classname_type, acc);
                let acc = f(classname_trailing_comma, acc);
                let acc = f(classname_right_angle, acc);
                acc
            },
            SyntaxVariant::FieldSpecifier(x) => {
                let FieldSpecifierChildren { field_question, field_name, field_arrow, field_type } = *x;
                let acc = f(field_question, acc);
                let acc = f(field_name, acc);
                let acc = f(field_arrow, acc);
                let acc = f(field_type, acc);
                acc
            },
            SyntaxVariant::FieldInitializer(x) => {
                let FieldInitializerChildren { field_initializer_name, field_initializer_arrow, field_initializer_value } = *x;
                let acc = f(field_initializer_name, acc);
                let acc = f(field_initializer_arrow, acc);
                let acc = f(field_initializer_value, acc);
                acc
            },
            SyntaxVariant::ShapeTypeSpecifier(x) => {
                let ShapeTypeSpecifierChildren { shape_type_keyword, shape_type_left_paren, shape_type_fields, shape_type_ellipsis, shape_type_right_paren } = *x;
                let acc = f(shape_type_keyword, acc);
                let acc = f(shape_type_left_paren, acc);
                let acc = f(shape_type_fields, acc);
                let acc = f(shape_type_ellipsis, acc);
                let acc = f(shape_type_right_paren, acc);
                acc
            },
            SyntaxVariant::ShapeExpression(x) => {
                let ShapeExpressionChildren { shape_expression_keyword, shape_expression_left_paren, shape_expression_fields, shape_expression_right_paren } = *x;
                let acc = f(shape_expression_keyword, acc);
                let acc = f(shape_expression_left_paren, acc);
                let acc = f(shape_expression_fields, acc);
                let acc = f(shape_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::TupleExpression(x) => {
                let TupleExpressionChildren { tuple_expression_keyword, tuple_expression_left_paren, tuple_expression_items, tuple_expression_right_paren } = *x;
                let acc = f(tuple_expression_keyword, acc);
                let acc = f(tuple_expression_left_paren, acc);
                let acc = f(tuple_expression_items, acc);
                let acc = f(tuple_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::GenericTypeSpecifier(x) => {
                let GenericTypeSpecifierChildren { generic_class_type, generic_argument_list } = *x;
                let acc = f(generic_class_type, acc);
                let acc = f(generic_argument_list, acc);
                acc
            },
            SyntaxVariant::NullableTypeSpecifier(x) => {
                let NullableTypeSpecifierChildren { nullable_question, nullable_type } = *x;
                let acc = f(nullable_question, acc);
                let acc = f(nullable_type, acc);
                acc
            },
            SyntaxVariant::LikeTypeSpecifier(x) => {
                let LikeTypeSpecifierChildren { like_tilde, like_type } = *x;
                let acc = f(like_tilde, acc);
                let acc = f(like_type, acc);
                acc
            },
            SyntaxVariant::SoftTypeSpecifier(x) => {
                let SoftTypeSpecifierChildren { soft_at, soft_type } = *x;
                let acc = f(soft_at, acc);
                let acc = f(soft_type, acc);
                acc
            },
            SyntaxVariant::AttributizedSpecifier(x) => {
                let AttributizedSpecifierChildren { attributized_specifier_attribute_spec, attributized_specifier_type } = *x;
                let acc = f(attributized_specifier_attribute_spec, acc);
                let acc = f(attributized_specifier_type, acc);
                acc
            },
            SyntaxVariant::ReifiedTypeArgument(x) => {
                let ReifiedTypeArgumentChildren { reified_type_argument_reified, reified_type_argument_type } = *x;
                let acc = f(reified_type_argument_reified, acc);
                let acc = f(reified_type_argument_type, acc);
                acc
            },
            SyntaxVariant::TypeArguments(x) => {
                let TypeArgumentsChildren { type_arguments_left_angle, type_arguments_types, type_arguments_right_angle } = *x;
                let acc = f(type_arguments_left_angle, acc);
                let acc = f(type_arguments_types, acc);
                let acc = f(type_arguments_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeParameters(x) => {
                let TypeParametersChildren { type_parameters_left_angle, type_parameters_parameters, type_parameters_right_angle } = *x;
                let acc = f(type_parameters_left_angle, acc);
                let acc = f(type_parameters_parameters, acc);
                let acc = f(type_parameters_right_angle, acc);
                acc
            },
            SyntaxVariant::TupleTypeSpecifier(x) => {
                let TupleTypeSpecifierChildren { tuple_left_paren, tuple_types, tuple_right_paren } = *x;
                let acc = f(tuple_left_paren, acc);
                let acc = f(tuple_types, acc);
                let acc = f(tuple_right_paren, acc);
                acc
            },
            SyntaxVariant::UnionTypeSpecifier(x) => {
                let UnionTypeSpecifierChildren { union_left_paren, union_types, union_right_paren } = *x;
                let acc = f(union_left_paren, acc);
                let acc = f(union_types, acc);
                let acc = f(union_right_paren, acc);
                acc
            },
            SyntaxVariant::IntersectionTypeSpecifier(x) => {
                let IntersectionTypeSpecifierChildren { intersection_left_paren, intersection_types, intersection_right_paren } = *x;
                let acc = f(intersection_left_paren, acc);
                let acc = f(intersection_types, acc);
                let acc = f(intersection_right_paren, acc);
                acc
            },
            SyntaxVariant::ErrorSyntax(x) => {
                let ErrorSyntaxChildren { error_error } = *x;
                let acc = f(error_error, acc);
                acc
            },
            SyntaxVariant::ListItem(x) => {
                let ListItemChildren { list_item, list_separator } = *x;
                let acc = f(list_item, acc);
                let acc = f(list_separator, acc);
                acc
            },
            SyntaxVariant::PocketAtomExpression(x) => {
                let PocketAtomExpressionChildren { pocket_atom_glyph, pocket_atom_expression } = *x;
                let acc = f(pocket_atom_glyph, acc);
                let acc = f(pocket_atom_expression, acc);
                acc
            },
            SyntaxVariant::PocketIdentifierExpression(x) => {
                let PocketIdentifierExpressionChildren { pocket_identifier_qualifier, pocket_identifier_pu_operator, pocket_identifier_field, pocket_identifier_operator, pocket_identifier_name } = *x;
                let acc = f(pocket_identifier_qualifier, acc);
                let acc = f(pocket_identifier_pu_operator, acc);
                let acc = f(pocket_identifier_field, acc);
                let acc = f(pocket_identifier_operator, acc);
                let acc = f(pocket_identifier_name, acc);
                acc
            },
            SyntaxVariant::PocketAtomMappingDeclaration(x) => {
                let PocketAtomMappingDeclarationChildren { pocket_atom_mapping_glyph, pocket_atom_mapping_name, pocket_atom_mapping_left_paren, pocket_atom_mapping_mappings, pocket_atom_mapping_right_paren, pocket_atom_mapping_semicolon } = *x;
                let acc = f(pocket_atom_mapping_glyph, acc);
                let acc = f(pocket_atom_mapping_name, acc);
                let acc = f(pocket_atom_mapping_left_paren, acc);
                let acc = f(pocket_atom_mapping_mappings, acc);
                let acc = f(pocket_atom_mapping_right_paren, acc);
                let acc = f(pocket_atom_mapping_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketEnumDeclaration(x) => {
                let PocketEnumDeclarationChildren { pocket_enum_attributes, pocket_enum_modifiers, pocket_enum_enum, pocket_enum_name, pocket_enum_left_brace, pocket_enum_fields, pocket_enum_right_brace } = *x;
                let acc = f(pocket_enum_attributes, acc);
                let acc = f(pocket_enum_modifiers, acc);
                let acc = f(pocket_enum_enum, acc);
                let acc = f(pocket_enum_name, acc);
                let acc = f(pocket_enum_left_brace, acc);
                let acc = f(pocket_enum_fields, acc);
                let acc = f(pocket_enum_right_brace, acc);
                acc
            },
            SyntaxVariant::PocketFieldTypeExprDeclaration(x) => {
                let PocketFieldTypeExprDeclarationChildren { pocket_field_type_expr_case, pocket_field_type_expr_type, pocket_field_type_expr_name, pocket_field_type_expr_semicolon } = *x;
                let acc = f(pocket_field_type_expr_case, acc);
                let acc = f(pocket_field_type_expr_type, acc);
                let acc = f(pocket_field_type_expr_name, acc);
                let acc = f(pocket_field_type_expr_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketFieldTypeDeclaration(x) => {
                let PocketFieldTypeDeclarationChildren { pocket_field_type_case, pocket_field_type_type, pocket_field_type_type_parameter, pocket_field_type_semicolon } = *x;
                let acc = f(pocket_field_type_case, acc);
                let acc = f(pocket_field_type_type, acc);
                let acc = f(pocket_field_type_type_parameter, acc);
                let acc = f(pocket_field_type_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketMappingIdDeclaration(x) => {
                let PocketMappingIdDeclarationChildren { pocket_mapping_id_name, pocket_mapping_id_initializer } = *x;
                let acc = f(pocket_mapping_id_name, acc);
                let acc = f(pocket_mapping_id_initializer, acc);
                acc
            },
            SyntaxVariant::PocketMappingTypeDeclaration(x) => {
                let PocketMappingTypeDeclarationChildren { pocket_mapping_type_keyword, pocket_mapping_type_name, pocket_mapping_type_equal, pocket_mapping_type_type } = *x;
                let acc = f(pocket_mapping_type_keyword, acc);
                let acc = f(pocket_mapping_type_name, acc);
                let acc = f(pocket_mapping_type_equal, acc);
                let acc = f(pocket_mapping_type_type, acc);
                acc
            },

        }
    }

    pub fn kind(&self) -> SyntaxKind {
        match &self.syntax {
            SyntaxVariant::Missing => SyntaxKind::Missing,
            SyntaxVariant::Token (t) => SyntaxKind::Token(t.kind()),
            SyntaxVariant::SyntaxList (_) => SyntaxKind::SyntaxList,
            SyntaxVariant::EndOfFile {..} => SyntaxKind::EndOfFile,
            SyntaxVariant::Script {..} => SyntaxKind::Script,
            SyntaxVariant::QualifiedName {..} => SyntaxKind::QualifiedName,
            SyntaxVariant::SimpleTypeSpecifier {..} => SyntaxKind::SimpleTypeSpecifier,
            SyntaxVariant::LiteralExpression {..} => SyntaxKind::LiteralExpression,
            SyntaxVariant::PrefixedStringExpression {..} => SyntaxKind::PrefixedStringExpression,
            SyntaxVariant::PrefixedCodeExpression {..} => SyntaxKind::PrefixedCodeExpression,
            SyntaxVariant::VariableExpression {..} => SyntaxKind::VariableExpression,
            SyntaxVariant::PipeVariableExpression {..} => SyntaxKind::PipeVariableExpression,
            SyntaxVariant::FileAttributeSpecification {..} => SyntaxKind::FileAttributeSpecification,
            SyntaxVariant::EnumDeclaration {..} => SyntaxKind::EnumDeclaration,
            SyntaxVariant::Enumerator {..} => SyntaxKind::Enumerator,
            SyntaxVariant::RecordDeclaration {..} => SyntaxKind::RecordDeclaration,
            SyntaxVariant::RecordField {..} => SyntaxKind::RecordField,
            SyntaxVariant::AliasDeclaration {..} => SyntaxKind::AliasDeclaration,
            SyntaxVariant::PropertyDeclaration {..} => SyntaxKind::PropertyDeclaration,
            SyntaxVariant::PropertyDeclarator {..} => SyntaxKind::PropertyDeclarator,
            SyntaxVariant::NamespaceDeclaration {..} => SyntaxKind::NamespaceDeclaration,
            SyntaxVariant::NamespaceDeclarationHeader {..} => SyntaxKind::NamespaceDeclarationHeader,
            SyntaxVariant::NamespaceBody {..} => SyntaxKind::NamespaceBody,
            SyntaxVariant::NamespaceEmptyBody {..} => SyntaxKind::NamespaceEmptyBody,
            SyntaxVariant::NamespaceUseDeclaration {..} => SyntaxKind::NamespaceUseDeclaration,
            SyntaxVariant::NamespaceGroupUseDeclaration {..} => SyntaxKind::NamespaceGroupUseDeclaration,
            SyntaxVariant::NamespaceUseClause {..} => SyntaxKind::NamespaceUseClause,
            SyntaxVariant::FunctionDeclaration {..} => SyntaxKind::FunctionDeclaration,
            SyntaxVariant::FunctionDeclarationHeader {..} => SyntaxKind::FunctionDeclarationHeader,
            SyntaxVariant::CapabilityProvisional {..} => SyntaxKind::CapabilityProvisional,
            SyntaxVariant::WhereClause {..} => SyntaxKind::WhereClause,
            SyntaxVariant::WhereConstraint {..} => SyntaxKind::WhereConstraint,
            SyntaxVariant::MethodishDeclaration {..} => SyntaxKind::MethodishDeclaration,
            SyntaxVariant::MethodishTraitResolution {..} => SyntaxKind::MethodishTraitResolution,
            SyntaxVariant::ClassishDeclaration {..} => SyntaxKind::ClassishDeclaration,
            SyntaxVariant::ClassishBody {..} => SyntaxKind::ClassishBody,
            SyntaxVariant::TraitUsePrecedenceItem {..} => SyntaxKind::TraitUsePrecedenceItem,
            SyntaxVariant::TraitUseAliasItem {..} => SyntaxKind::TraitUseAliasItem,
            SyntaxVariant::TraitUseConflictResolution {..} => SyntaxKind::TraitUseConflictResolution,
            SyntaxVariant::TraitUse {..} => SyntaxKind::TraitUse,
            SyntaxVariant::RequireClause {..} => SyntaxKind::RequireClause,
            SyntaxVariant::ConstDeclaration {..} => SyntaxKind::ConstDeclaration,
            SyntaxVariant::ConstantDeclarator {..} => SyntaxKind::ConstantDeclarator,
            SyntaxVariant::TypeConstDeclaration {..} => SyntaxKind::TypeConstDeclaration,
            SyntaxVariant::DecoratedExpression {..} => SyntaxKind::DecoratedExpression,
            SyntaxVariant::ParameterDeclaration {..} => SyntaxKind::ParameterDeclaration,
            SyntaxVariant::VariadicParameter {..} => SyntaxKind::VariadicParameter,
            SyntaxVariant::OldAttributeSpecification {..} => SyntaxKind::OldAttributeSpecification,
            SyntaxVariant::AttributeSpecification {..} => SyntaxKind::AttributeSpecification,
            SyntaxVariant::Attribute {..} => SyntaxKind::Attribute,
            SyntaxVariant::InclusionExpression {..} => SyntaxKind::InclusionExpression,
            SyntaxVariant::InclusionDirective {..} => SyntaxKind::InclusionDirective,
            SyntaxVariant::CompoundStatement {..} => SyntaxKind::CompoundStatement,
            SyntaxVariant::ExpressionStatement {..} => SyntaxKind::ExpressionStatement,
            SyntaxVariant::MarkupSection {..} => SyntaxKind::MarkupSection,
            SyntaxVariant::MarkupSuffix {..} => SyntaxKind::MarkupSuffix,
            SyntaxVariant::UnsetStatement {..} => SyntaxKind::UnsetStatement,
            SyntaxVariant::UsingStatementBlockScoped {..} => SyntaxKind::UsingStatementBlockScoped,
            SyntaxVariant::UsingStatementFunctionScoped {..} => SyntaxKind::UsingStatementFunctionScoped,
            SyntaxVariant::WhileStatement {..} => SyntaxKind::WhileStatement,
            SyntaxVariant::IfStatement {..} => SyntaxKind::IfStatement,
            SyntaxVariant::ElseifClause {..} => SyntaxKind::ElseifClause,
            SyntaxVariant::ElseClause {..} => SyntaxKind::ElseClause,
            SyntaxVariant::TryStatement {..} => SyntaxKind::TryStatement,
            SyntaxVariant::CatchClause {..} => SyntaxKind::CatchClause,
            SyntaxVariant::FinallyClause {..} => SyntaxKind::FinallyClause,
            SyntaxVariant::DoStatement {..} => SyntaxKind::DoStatement,
            SyntaxVariant::ForStatement {..} => SyntaxKind::ForStatement,
            SyntaxVariant::ForeachStatement {..} => SyntaxKind::ForeachStatement,
            SyntaxVariant::SwitchStatement {..} => SyntaxKind::SwitchStatement,
            SyntaxVariant::SwitchSection {..} => SyntaxKind::SwitchSection,
            SyntaxVariant::SwitchFallthrough {..} => SyntaxKind::SwitchFallthrough,
            SyntaxVariant::CaseLabel {..} => SyntaxKind::CaseLabel,
            SyntaxVariant::DefaultLabel {..} => SyntaxKind::DefaultLabel,
            SyntaxVariant::ReturnStatement {..} => SyntaxKind::ReturnStatement,
            SyntaxVariant::GotoLabel {..} => SyntaxKind::GotoLabel,
            SyntaxVariant::GotoStatement {..} => SyntaxKind::GotoStatement,
            SyntaxVariant::ThrowStatement {..} => SyntaxKind::ThrowStatement,
            SyntaxVariant::BreakStatement {..} => SyntaxKind::BreakStatement,
            SyntaxVariant::ContinueStatement {..} => SyntaxKind::ContinueStatement,
            SyntaxVariant::EchoStatement {..} => SyntaxKind::EchoStatement,
            SyntaxVariant::ConcurrentStatement {..} => SyntaxKind::ConcurrentStatement,
            SyntaxVariant::SimpleInitializer {..} => SyntaxKind::SimpleInitializer,
            SyntaxVariant::AnonymousClass {..} => SyntaxKind::AnonymousClass,
            SyntaxVariant::AnonymousFunction {..} => SyntaxKind::AnonymousFunction,
            SyntaxVariant::AnonymousFunctionUseClause {..} => SyntaxKind::AnonymousFunctionUseClause,
            SyntaxVariant::LambdaExpression {..} => SyntaxKind::LambdaExpression,
            SyntaxVariant::LambdaSignature {..} => SyntaxKind::LambdaSignature,
            SyntaxVariant::CastExpression {..} => SyntaxKind::CastExpression,
            SyntaxVariant::ScopeResolutionExpression {..} => SyntaxKind::ScopeResolutionExpression,
            SyntaxVariant::MemberSelectionExpression {..} => SyntaxKind::MemberSelectionExpression,
            SyntaxVariant::SafeMemberSelectionExpression {..} => SyntaxKind::SafeMemberSelectionExpression,
            SyntaxVariant::EmbeddedMemberSelectionExpression {..} => SyntaxKind::EmbeddedMemberSelectionExpression,
            SyntaxVariant::YieldExpression {..} => SyntaxKind::YieldExpression,
            SyntaxVariant::PrefixUnaryExpression {..} => SyntaxKind::PrefixUnaryExpression,
            SyntaxVariant::PostfixUnaryExpression {..} => SyntaxKind::PostfixUnaryExpression,
            SyntaxVariant::BinaryExpression {..} => SyntaxKind::BinaryExpression,
            SyntaxVariant::IsExpression {..} => SyntaxKind::IsExpression,
            SyntaxVariant::AsExpression {..} => SyntaxKind::AsExpression,
            SyntaxVariant::NullableAsExpression {..} => SyntaxKind::NullableAsExpression,
            SyntaxVariant::ConditionalExpression {..} => SyntaxKind::ConditionalExpression,
            SyntaxVariant::EvalExpression {..} => SyntaxKind::EvalExpression,
            SyntaxVariant::DefineExpression {..} => SyntaxKind::DefineExpression,
            SyntaxVariant::IssetExpression {..} => SyntaxKind::IssetExpression,
            SyntaxVariant::FunctionCallExpression {..} => SyntaxKind::FunctionCallExpression,
            SyntaxVariant::FunctionPointerExpression {..} => SyntaxKind::FunctionPointerExpression,
            SyntaxVariant::ParenthesizedExpression {..} => SyntaxKind::ParenthesizedExpression,
            SyntaxVariant::BracedExpression {..} => SyntaxKind::BracedExpression,
            SyntaxVariant::EmbeddedBracedExpression {..} => SyntaxKind::EmbeddedBracedExpression,
            SyntaxVariant::ListExpression {..} => SyntaxKind::ListExpression,
            SyntaxVariant::CollectionLiteralExpression {..} => SyntaxKind::CollectionLiteralExpression,
            SyntaxVariant::ObjectCreationExpression {..} => SyntaxKind::ObjectCreationExpression,
            SyntaxVariant::ConstructorCall {..} => SyntaxKind::ConstructorCall,
            SyntaxVariant::RecordCreationExpression {..} => SyntaxKind::RecordCreationExpression,
            SyntaxVariant::DarrayIntrinsicExpression {..} => SyntaxKind::DarrayIntrinsicExpression,
            SyntaxVariant::DictionaryIntrinsicExpression {..} => SyntaxKind::DictionaryIntrinsicExpression,
            SyntaxVariant::KeysetIntrinsicExpression {..} => SyntaxKind::KeysetIntrinsicExpression,
            SyntaxVariant::VarrayIntrinsicExpression {..} => SyntaxKind::VarrayIntrinsicExpression,
            SyntaxVariant::VectorIntrinsicExpression {..} => SyntaxKind::VectorIntrinsicExpression,
            SyntaxVariant::ElementInitializer {..} => SyntaxKind::ElementInitializer,
            SyntaxVariant::SubscriptExpression {..} => SyntaxKind::SubscriptExpression,
            SyntaxVariant::EmbeddedSubscriptExpression {..} => SyntaxKind::EmbeddedSubscriptExpression,
            SyntaxVariant::AwaitableCreationExpression {..} => SyntaxKind::AwaitableCreationExpression,
            SyntaxVariant::XHPChildrenDeclaration {..} => SyntaxKind::XHPChildrenDeclaration,
            SyntaxVariant::XHPChildrenParenthesizedList {..} => SyntaxKind::XHPChildrenParenthesizedList,
            SyntaxVariant::XHPCategoryDeclaration {..} => SyntaxKind::XHPCategoryDeclaration,
            SyntaxVariant::XHPEnumType {..} => SyntaxKind::XHPEnumType,
            SyntaxVariant::XHPLateinit {..} => SyntaxKind::XHPLateinit,
            SyntaxVariant::XHPRequired {..} => SyntaxKind::XHPRequired,
            SyntaxVariant::XHPClassAttributeDeclaration {..} => SyntaxKind::XHPClassAttributeDeclaration,
            SyntaxVariant::XHPClassAttribute {..} => SyntaxKind::XHPClassAttribute,
            SyntaxVariant::XHPSimpleClassAttribute {..} => SyntaxKind::XHPSimpleClassAttribute,
            SyntaxVariant::XHPSimpleAttribute {..} => SyntaxKind::XHPSimpleAttribute,
            SyntaxVariant::XHPSpreadAttribute {..} => SyntaxKind::XHPSpreadAttribute,
            SyntaxVariant::XHPOpen {..} => SyntaxKind::XHPOpen,
            SyntaxVariant::XHPExpression {..} => SyntaxKind::XHPExpression,
            SyntaxVariant::XHPClose {..} => SyntaxKind::XHPClose,
            SyntaxVariant::TypeConstant {..} => SyntaxKind::TypeConstant,
            SyntaxVariant::PUAccess {..} => SyntaxKind::PUAccess,
            SyntaxVariant::VectorTypeSpecifier {..} => SyntaxKind::VectorTypeSpecifier,
            SyntaxVariant::KeysetTypeSpecifier {..} => SyntaxKind::KeysetTypeSpecifier,
            SyntaxVariant::TupleTypeExplicitSpecifier {..} => SyntaxKind::TupleTypeExplicitSpecifier,
            SyntaxVariant::VarrayTypeSpecifier {..} => SyntaxKind::VarrayTypeSpecifier,
            SyntaxVariant::VectorArrayTypeSpecifier {..} => SyntaxKind::VectorArrayTypeSpecifier,
            SyntaxVariant::TypeParameter {..} => SyntaxKind::TypeParameter,
            SyntaxVariant::TypeConstraint {..} => SyntaxKind::TypeConstraint,
            SyntaxVariant::DarrayTypeSpecifier {..} => SyntaxKind::DarrayTypeSpecifier,
            SyntaxVariant::MapArrayTypeSpecifier {..} => SyntaxKind::MapArrayTypeSpecifier,
            SyntaxVariant::DictionaryTypeSpecifier {..} => SyntaxKind::DictionaryTypeSpecifier,
            SyntaxVariant::ClosureTypeSpecifier {..} => SyntaxKind::ClosureTypeSpecifier,
            SyntaxVariant::ClosureParameterTypeSpecifier {..} => SyntaxKind::ClosureParameterTypeSpecifier,
            SyntaxVariant::ClassnameTypeSpecifier {..} => SyntaxKind::ClassnameTypeSpecifier,
            SyntaxVariant::FieldSpecifier {..} => SyntaxKind::FieldSpecifier,
            SyntaxVariant::FieldInitializer {..} => SyntaxKind::FieldInitializer,
            SyntaxVariant::ShapeTypeSpecifier {..} => SyntaxKind::ShapeTypeSpecifier,
            SyntaxVariant::ShapeExpression {..} => SyntaxKind::ShapeExpression,
            SyntaxVariant::TupleExpression {..} => SyntaxKind::TupleExpression,
            SyntaxVariant::GenericTypeSpecifier {..} => SyntaxKind::GenericTypeSpecifier,
            SyntaxVariant::NullableTypeSpecifier {..} => SyntaxKind::NullableTypeSpecifier,
            SyntaxVariant::LikeTypeSpecifier {..} => SyntaxKind::LikeTypeSpecifier,
            SyntaxVariant::SoftTypeSpecifier {..} => SyntaxKind::SoftTypeSpecifier,
            SyntaxVariant::AttributizedSpecifier {..} => SyntaxKind::AttributizedSpecifier,
            SyntaxVariant::ReifiedTypeArgument {..} => SyntaxKind::ReifiedTypeArgument,
            SyntaxVariant::TypeArguments {..} => SyntaxKind::TypeArguments,
            SyntaxVariant::TypeParameters {..} => SyntaxKind::TypeParameters,
            SyntaxVariant::TupleTypeSpecifier {..} => SyntaxKind::TupleTypeSpecifier,
            SyntaxVariant::UnionTypeSpecifier {..} => SyntaxKind::UnionTypeSpecifier,
            SyntaxVariant::IntersectionTypeSpecifier {..} => SyntaxKind::IntersectionTypeSpecifier,
            SyntaxVariant::ErrorSyntax {..} => SyntaxKind::ErrorSyntax,
            SyntaxVariant::ListItem {..} => SyntaxKind::ListItem,
            SyntaxVariant::PocketAtomExpression {..} => SyntaxKind::PocketAtomExpression,
            SyntaxVariant::PocketIdentifierExpression {..} => SyntaxKind::PocketIdentifierExpression,
            SyntaxVariant::PocketAtomMappingDeclaration {..} => SyntaxKind::PocketAtomMappingDeclaration,
            SyntaxVariant::PocketEnumDeclaration {..} => SyntaxKind::PocketEnumDeclaration,
            SyntaxVariant::PocketFieldTypeExprDeclaration {..} => SyntaxKind::PocketFieldTypeExprDeclaration,
            SyntaxVariant::PocketFieldTypeDeclaration {..} => SyntaxKind::PocketFieldTypeDeclaration,
            SyntaxVariant::PocketMappingIdDeclaration {..} => SyntaxKind::PocketMappingIdDeclaration,
            SyntaxVariant::PocketMappingTypeDeclaration {..} => SyntaxKind::PocketMappingTypeDeclaration,
        }
    }

    pub fn from_children(kind : SyntaxKind, mut ts : Vec<Self>) -> SyntaxVariant<T, V> {
         match (kind, ts.len()) {
             (SyntaxKind::Missing, 0) => SyntaxVariant::Missing,
             (SyntaxKind::SyntaxList, _) => SyntaxVariant::SyntaxList(ts),
             (SyntaxKind::EndOfFile, 1) => SyntaxVariant::EndOfFile(Box::new(EndOfFileChildren {
                 end_of_file_token: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::Script, 1) => SyntaxVariant::Script(Box::new(ScriptChildren {
                 script_declarations: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::QualifiedName, 1) => SyntaxVariant::QualifiedName(Box::new(QualifiedNameChildren {
                 qualified_name_parts: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SimpleTypeSpecifier, 1) => SyntaxVariant::SimpleTypeSpecifier(Box::new(SimpleTypeSpecifierChildren {
                 simple_type_specifier: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::LiteralExpression, 1) => SyntaxVariant::LiteralExpression(Box::new(LiteralExpressionChildren {
                 literal_expression: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PrefixedStringExpression, 2) => SyntaxVariant::PrefixedStringExpression(Box::new(PrefixedStringExpressionChildren {
                 prefixed_string_str: ts.pop().unwrap(),
                 prefixed_string_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PrefixedCodeExpression, 4) => SyntaxVariant::PrefixedCodeExpression(Box::new(PrefixedCodeExpressionChildren {
                 prefixed_code_right_backtick: ts.pop().unwrap(),
                 prefixed_code_expression: ts.pop().unwrap(),
                 prefixed_code_left_backtick: ts.pop().unwrap(),
                 prefixed_code_prefix: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VariableExpression, 1) => SyntaxVariant::VariableExpression(Box::new(VariableExpressionChildren {
                 variable_expression: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PipeVariableExpression, 1) => SyntaxVariant::PipeVariableExpression(Box::new(PipeVariableExpressionChildren {
                 pipe_variable_expression: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FileAttributeSpecification, 5) => SyntaxVariant::FileAttributeSpecification(Box::new(FileAttributeSpecificationChildren {
                 file_attribute_specification_right_double_angle: ts.pop().unwrap(),
                 file_attribute_specification_attributes: ts.pop().unwrap(),
                 file_attribute_specification_colon: ts.pop().unwrap(),
                 file_attribute_specification_keyword: ts.pop().unwrap(),
                 file_attribute_specification_left_double_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EnumDeclaration, 11) => SyntaxVariant::EnumDeclaration(Box::new(EnumDeclarationChildren {
                 enum_right_brace: ts.pop().unwrap(),
                 enum_enumerators: ts.pop().unwrap(),
                 enum_left_brace: ts.pop().unwrap(),
                 enum_includes_list: ts.pop().unwrap(),
                 enum_includes_keyword: ts.pop().unwrap(),
                 enum_type: ts.pop().unwrap(),
                 enum_base: ts.pop().unwrap(),
                 enum_colon: ts.pop().unwrap(),
                 enum_name: ts.pop().unwrap(),
                 enum_keyword: ts.pop().unwrap(),
                 enum_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::Enumerator, 4) => SyntaxVariant::Enumerator(Box::new(EnumeratorChildren {
                 enumerator_semicolon: ts.pop().unwrap(),
                 enumerator_value: ts.pop().unwrap(),
                 enumerator_equal: ts.pop().unwrap(),
                 enumerator_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::RecordDeclaration, 9) => SyntaxVariant::RecordDeclaration(Box::new(RecordDeclarationChildren {
                 record_right_brace: ts.pop().unwrap(),
                 record_fields: ts.pop().unwrap(),
                 record_left_brace: ts.pop().unwrap(),
                 record_extends_opt: ts.pop().unwrap(),
                 record_extends_keyword: ts.pop().unwrap(),
                 record_name: ts.pop().unwrap(),
                 record_keyword: ts.pop().unwrap(),
                 record_modifier: ts.pop().unwrap(),
                 record_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::RecordField, 4) => SyntaxVariant::RecordField(Box::new(RecordFieldChildren {
                 record_field_semi: ts.pop().unwrap(),
                 record_field_init: ts.pop().unwrap(),
                 record_field_name: ts.pop().unwrap(),
                 record_field_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AliasDeclaration, 8) => SyntaxVariant::AliasDeclaration(Box::new(AliasDeclarationChildren {
                 alias_semicolon: ts.pop().unwrap(),
                 alias_type: ts.pop().unwrap(),
                 alias_equal: ts.pop().unwrap(),
                 alias_constraint: ts.pop().unwrap(),
                 alias_generic_parameter: ts.pop().unwrap(),
                 alias_name: ts.pop().unwrap(),
                 alias_keyword: ts.pop().unwrap(),
                 alias_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PropertyDeclaration, 5) => SyntaxVariant::PropertyDeclaration(Box::new(PropertyDeclarationChildren {
                 property_semicolon: ts.pop().unwrap(),
                 property_declarators: ts.pop().unwrap(),
                 property_type: ts.pop().unwrap(),
                 property_modifiers: ts.pop().unwrap(),
                 property_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PropertyDeclarator, 2) => SyntaxVariant::PropertyDeclarator(Box::new(PropertyDeclaratorChildren {
                 property_initializer: ts.pop().unwrap(),
                 property_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceDeclaration, 2) => SyntaxVariant::NamespaceDeclaration(Box::new(NamespaceDeclarationChildren {
                 namespace_body: ts.pop().unwrap(),
                 namespace_header: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceDeclarationHeader, 2) => SyntaxVariant::NamespaceDeclarationHeader(Box::new(NamespaceDeclarationHeaderChildren {
                 namespace_name: ts.pop().unwrap(),
                 namespace_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceBody, 3) => SyntaxVariant::NamespaceBody(Box::new(NamespaceBodyChildren {
                 namespace_right_brace: ts.pop().unwrap(),
                 namespace_declarations: ts.pop().unwrap(),
                 namespace_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceEmptyBody, 1) => SyntaxVariant::NamespaceEmptyBody(Box::new(NamespaceEmptyBodyChildren {
                 namespace_semicolon: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceUseDeclaration, 4) => SyntaxVariant::NamespaceUseDeclaration(Box::new(NamespaceUseDeclarationChildren {
                 namespace_use_semicolon: ts.pop().unwrap(),
                 namespace_use_clauses: ts.pop().unwrap(),
                 namespace_use_kind: ts.pop().unwrap(),
                 namespace_use_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceGroupUseDeclaration, 7) => SyntaxVariant::NamespaceGroupUseDeclaration(Box::new(NamespaceGroupUseDeclarationChildren {
                 namespace_group_use_semicolon: ts.pop().unwrap(),
                 namespace_group_use_right_brace: ts.pop().unwrap(),
                 namespace_group_use_clauses: ts.pop().unwrap(),
                 namespace_group_use_left_brace: ts.pop().unwrap(),
                 namespace_group_use_prefix: ts.pop().unwrap(),
                 namespace_group_use_kind: ts.pop().unwrap(),
                 namespace_group_use_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NamespaceUseClause, 4) => SyntaxVariant::NamespaceUseClause(Box::new(NamespaceUseClauseChildren {
                 namespace_use_alias: ts.pop().unwrap(),
                 namespace_use_as: ts.pop().unwrap(),
                 namespace_use_name: ts.pop().unwrap(),
                 namespace_use_clause_kind: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FunctionDeclaration, 3) => SyntaxVariant::FunctionDeclaration(Box::new(FunctionDeclarationChildren {
                 function_body: ts.pop().unwrap(),
                 function_declaration_header: ts.pop().unwrap(),
                 function_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FunctionDeclarationHeader, 11) => SyntaxVariant::FunctionDeclarationHeader(Box::new(FunctionDeclarationHeaderChildren {
                 function_where_clause: ts.pop().unwrap(),
                 function_type: ts.pop().unwrap(),
                 function_colon: ts.pop().unwrap(),
                 function_capability_provisional: ts.pop().unwrap(),
                 function_right_paren: ts.pop().unwrap(),
                 function_parameter_list: ts.pop().unwrap(),
                 function_left_paren: ts.pop().unwrap(),
                 function_type_parameter_list: ts.pop().unwrap(),
                 function_name: ts.pop().unwrap(),
                 function_keyword: ts.pop().unwrap(),
                 function_modifiers: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CapabilityProvisional, 6) => SyntaxVariant::CapabilityProvisional(Box::new(CapabilityProvisionalChildren {
                 capability_provisional_right_brace: ts.pop().unwrap(),
                 capability_provisional_unsafe_type: ts.pop().unwrap(),
                 capability_provisional_unsafe_plus: ts.pop().unwrap(),
                 capability_provisional_type: ts.pop().unwrap(),
                 capability_provisional_left_brace: ts.pop().unwrap(),
                 capability_provisional_at: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::WhereClause, 2) => SyntaxVariant::WhereClause(Box::new(WhereClauseChildren {
                 where_clause_constraints: ts.pop().unwrap(),
                 where_clause_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::WhereConstraint, 3) => SyntaxVariant::WhereConstraint(Box::new(WhereConstraintChildren {
                 where_constraint_right_type: ts.pop().unwrap(),
                 where_constraint_operator: ts.pop().unwrap(),
                 where_constraint_left_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MethodishDeclaration, 4) => SyntaxVariant::MethodishDeclaration(Box::new(MethodishDeclarationChildren {
                 methodish_semicolon: ts.pop().unwrap(),
                 methodish_function_body: ts.pop().unwrap(),
                 methodish_function_decl_header: ts.pop().unwrap(),
                 methodish_attribute: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MethodishTraitResolution, 5) => SyntaxVariant::MethodishTraitResolution(Box::new(MethodishTraitResolutionChildren {
                 methodish_trait_semicolon: ts.pop().unwrap(),
                 methodish_trait_name: ts.pop().unwrap(),
                 methodish_trait_equal: ts.pop().unwrap(),
                 methodish_trait_function_decl_header: ts.pop().unwrap(),
                 methodish_trait_attribute: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ClassishDeclaration, 12) => SyntaxVariant::ClassishDeclaration(Box::new(ClassishDeclarationChildren {
                 classish_body: ts.pop().unwrap(),
                 classish_where_clause: ts.pop().unwrap(),
                 classish_implements_list: ts.pop().unwrap(),
                 classish_implements_keyword: ts.pop().unwrap(),
                 classish_extends_list: ts.pop().unwrap(),
                 classish_extends_keyword: ts.pop().unwrap(),
                 classish_type_parameters: ts.pop().unwrap(),
                 classish_name: ts.pop().unwrap(),
                 classish_keyword: ts.pop().unwrap(),
                 classish_xhp: ts.pop().unwrap(),
                 classish_modifiers: ts.pop().unwrap(),
                 classish_attribute: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ClassishBody, 3) => SyntaxVariant::ClassishBody(Box::new(ClassishBodyChildren {
                 classish_body_right_brace: ts.pop().unwrap(),
                 classish_body_elements: ts.pop().unwrap(),
                 classish_body_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TraitUsePrecedenceItem, 3) => SyntaxVariant::TraitUsePrecedenceItem(Box::new(TraitUsePrecedenceItemChildren {
                 trait_use_precedence_item_removed_names: ts.pop().unwrap(),
                 trait_use_precedence_item_keyword: ts.pop().unwrap(),
                 trait_use_precedence_item_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TraitUseAliasItem, 4) => SyntaxVariant::TraitUseAliasItem(Box::new(TraitUseAliasItemChildren {
                 trait_use_alias_item_aliased_name: ts.pop().unwrap(),
                 trait_use_alias_item_modifiers: ts.pop().unwrap(),
                 trait_use_alias_item_keyword: ts.pop().unwrap(),
                 trait_use_alias_item_aliasing_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TraitUseConflictResolution, 5) => SyntaxVariant::TraitUseConflictResolution(Box::new(TraitUseConflictResolutionChildren {
                 trait_use_conflict_resolution_right_brace: ts.pop().unwrap(),
                 trait_use_conflict_resolution_clauses: ts.pop().unwrap(),
                 trait_use_conflict_resolution_left_brace: ts.pop().unwrap(),
                 trait_use_conflict_resolution_names: ts.pop().unwrap(),
                 trait_use_conflict_resolution_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TraitUse, 3) => SyntaxVariant::TraitUse(Box::new(TraitUseChildren {
                 trait_use_semicolon: ts.pop().unwrap(),
                 trait_use_names: ts.pop().unwrap(),
                 trait_use_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::RequireClause, 4) => SyntaxVariant::RequireClause(Box::new(RequireClauseChildren {
                 require_semicolon: ts.pop().unwrap(),
                 require_name: ts.pop().unwrap(),
                 require_kind: ts.pop().unwrap(),
                 require_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ConstDeclaration, 5) => SyntaxVariant::ConstDeclaration(Box::new(ConstDeclarationChildren {
                 const_semicolon: ts.pop().unwrap(),
                 const_declarators: ts.pop().unwrap(),
                 const_type_specifier: ts.pop().unwrap(),
                 const_keyword: ts.pop().unwrap(),
                 const_modifiers: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ConstantDeclarator, 2) => SyntaxVariant::ConstantDeclarator(Box::new(ConstantDeclaratorChildren {
                 constant_declarator_initializer: ts.pop().unwrap(),
                 constant_declarator_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeConstDeclaration, 10) => SyntaxVariant::TypeConstDeclaration(Box::new(TypeConstDeclarationChildren {
                 type_const_semicolon: ts.pop().unwrap(),
                 type_const_type_specifier: ts.pop().unwrap(),
                 type_const_equal: ts.pop().unwrap(),
                 type_const_type_constraint: ts.pop().unwrap(),
                 type_const_type_parameters: ts.pop().unwrap(),
                 type_const_name: ts.pop().unwrap(),
                 type_const_type_keyword: ts.pop().unwrap(),
                 type_const_keyword: ts.pop().unwrap(),
                 type_const_modifiers: ts.pop().unwrap(),
                 type_const_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DecoratedExpression, 2) => SyntaxVariant::DecoratedExpression(Box::new(DecoratedExpressionChildren {
                 decorated_expression_expression: ts.pop().unwrap(),
                 decorated_expression_decorator: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ParameterDeclaration, 6) => SyntaxVariant::ParameterDeclaration(Box::new(ParameterDeclarationChildren {
                 parameter_default_value: ts.pop().unwrap(),
                 parameter_name: ts.pop().unwrap(),
                 parameter_type: ts.pop().unwrap(),
                 parameter_call_convention: ts.pop().unwrap(),
                 parameter_visibility: ts.pop().unwrap(),
                 parameter_attribute: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VariadicParameter, 3) => SyntaxVariant::VariadicParameter(Box::new(VariadicParameterChildren {
                 variadic_parameter_ellipsis: ts.pop().unwrap(),
                 variadic_parameter_type: ts.pop().unwrap(),
                 variadic_parameter_call_convention: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::OldAttributeSpecification, 3) => SyntaxVariant::OldAttributeSpecification(Box::new(OldAttributeSpecificationChildren {
                 old_attribute_specification_right_double_angle: ts.pop().unwrap(),
                 old_attribute_specification_attributes: ts.pop().unwrap(),
                 old_attribute_specification_left_double_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AttributeSpecification, 1) => SyntaxVariant::AttributeSpecification(Box::new(AttributeSpecificationChildren {
                 attribute_specification_attributes: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::Attribute, 2) => SyntaxVariant::Attribute(Box::new(AttributeChildren {
                 attribute_attribute_name: ts.pop().unwrap(),
                 attribute_at: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::InclusionExpression, 2) => SyntaxVariant::InclusionExpression(Box::new(InclusionExpressionChildren {
                 inclusion_filename: ts.pop().unwrap(),
                 inclusion_require: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::InclusionDirective, 2) => SyntaxVariant::InclusionDirective(Box::new(InclusionDirectiveChildren {
                 inclusion_semicolon: ts.pop().unwrap(),
                 inclusion_expression: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CompoundStatement, 3) => SyntaxVariant::CompoundStatement(Box::new(CompoundStatementChildren {
                 compound_right_brace: ts.pop().unwrap(),
                 compound_statements: ts.pop().unwrap(),
                 compound_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ExpressionStatement, 2) => SyntaxVariant::ExpressionStatement(Box::new(ExpressionStatementChildren {
                 expression_statement_semicolon: ts.pop().unwrap(),
                 expression_statement_expression: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MarkupSection, 2) => SyntaxVariant::MarkupSection(Box::new(MarkupSectionChildren {
                 markup_suffix: ts.pop().unwrap(),
                 markup_text: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MarkupSuffix, 2) => SyntaxVariant::MarkupSuffix(Box::new(MarkupSuffixChildren {
                 markup_suffix_name: ts.pop().unwrap(),
                 markup_suffix_less_than_question: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::UnsetStatement, 5) => SyntaxVariant::UnsetStatement(Box::new(UnsetStatementChildren {
                 unset_semicolon: ts.pop().unwrap(),
                 unset_right_paren: ts.pop().unwrap(),
                 unset_variables: ts.pop().unwrap(),
                 unset_left_paren: ts.pop().unwrap(),
                 unset_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::UsingStatementBlockScoped, 6) => SyntaxVariant::UsingStatementBlockScoped(Box::new(UsingStatementBlockScopedChildren {
                 using_block_body: ts.pop().unwrap(),
                 using_block_right_paren: ts.pop().unwrap(),
                 using_block_expressions: ts.pop().unwrap(),
                 using_block_left_paren: ts.pop().unwrap(),
                 using_block_using_keyword: ts.pop().unwrap(),
                 using_block_await_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::UsingStatementFunctionScoped, 4) => SyntaxVariant::UsingStatementFunctionScoped(Box::new(UsingStatementFunctionScopedChildren {
                 using_function_semicolon: ts.pop().unwrap(),
                 using_function_expression: ts.pop().unwrap(),
                 using_function_using_keyword: ts.pop().unwrap(),
                 using_function_await_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::WhileStatement, 5) => SyntaxVariant::WhileStatement(Box::new(WhileStatementChildren {
                 while_body: ts.pop().unwrap(),
                 while_right_paren: ts.pop().unwrap(),
                 while_condition: ts.pop().unwrap(),
                 while_left_paren: ts.pop().unwrap(),
                 while_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::IfStatement, 7) => SyntaxVariant::IfStatement(Box::new(IfStatementChildren {
                 if_else_clause: ts.pop().unwrap(),
                 if_elseif_clauses: ts.pop().unwrap(),
                 if_statement: ts.pop().unwrap(),
                 if_right_paren: ts.pop().unwrap(),
                 if_condition: ts.pop().unwrap(),
                 if_left_paren: ts.pop().unwrap(),
                 if_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ElseifClause, 5) => SyntaxVariant::ElseifClause(Box::new(ElseifClauseChildren {
                 elseif_statement: ts.pop().unwrap(),
                 elseif_right_paren: ts.pop().unwrap(),
                 elseif_condition: ts.pop().unwrap(),
                 elseif_left_paren: ts.pop().unwrap(),
                 elseif_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ElseClause, 2) => SyntaxVariant::ElseClause(Box::new(ElseClauseChildren {
                 else_statement: ts.pop().unwrap(),
                 else_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TryStatement, 4) => SyntaxVariant::TryStatement(Box::new(TryStatementChildren {
                 try_finally_clause: ts.pop().unwrap(),
                 try_catch_clauses: ts.pop().unwrap(),
                 try_compound_statement: ts.pop().unwrap(),
                 try_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CatchClause, 6) => SyntaxVariant::CatchClause(Box::new(CatchClauseChildren {
                 catch_body: ts.pop().unwrap(),
                 catch_right_paren: ts.pop().unwrap(),
                 catch_variable: ts.pop().unwrap(),
                 catch_type: ts.pop().unwrap(),
                 catch_left_paren: ts.pop().unwrap(),
                 catch_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FinallyClause, 2) => SyntaxVariant::FinallyClause(Box::new(FinallyClauseChildren {
                 finally_body: ts.pop().unwrap(),
                 finally_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DoStatement, 7) => SyntaxVariant::DoStatement(Box::new(DoStatementChildren {
                 do_semicolon: ts.pop().unwrap(),
                 do_right_paren: ts.pop().unwrap(),
                 do_condition: ts.pop().unwrap(),
                 do_left_paren: ts.pop().unwrap(),
                 do_while_keyword: ts.pop().unwrap(),
                 do_body: ts.pop().unwrap(),
                 do_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ForStatement, 9) => SyntaxVariant::ForStatement(Box::new(ForStatementChildren {
                 for_body: ts.pop().unwrap(),
                 for_right_paren: ts.pop().unwrap(),
                 for_end_of_loop: ts.pop().unwrap(),
                 for_second_semicolon: ts.pop().unwrap(),
                 for_control: ts.pop().unwrap(),
                 for_first_semicolon: ts.pop().unwrap(),
                 for_initializer: ts.pop().unwrap(),
                 for_left_paren: ts.pop().unwrap(),
                 for_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ForeachStatement, 10) => SyntaxVariant::ForeachStatement(Box::new(ForeachStatementChildren {
                 foreach_body: ts.pop().unwrap(),
                 foreach_right_paren: ts.pop().unwrap(),
                 foreach_value: ts.pop().unwrap(),
                 foreach_arrow: ts.pop().unwrap(),
                 foreach_key: ts.pop().unwrap(),
                 foreach_as: ts.pop().unwrap(),
                 foreach_await_keyword: ts.pop().unwrap(),
                 foreach_collection: ts.pop().unwrap(),
                 foreach_left_paren: ts.pop().unwrap(),
                 foreach_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SwitchStatement, 7) => SyntaxVariant::SwitchStatement(Box::new(SwitchStatementChildren {
                 switch_right_brace: ts.pop().unwrap(),
                 switch_sections: ts.pop().unwrap(),
                 switch_left_brace: ts.pop().unwrap(),
                 switch_right_paren: ts.pop().unwrap(),
                 switch_expression: ts.pop().unwrap(),
                 switch_left_paren: ts.pop().unwrap(),
                 switch_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SwitchSection, 3) => SyntaxVariant::SwitchSection(Box::new(SwitchSectionChildren {
                 switch_section_fallthrough: ts.pop().unwrap(),
                 switch_section_statements: ts.pop().unwrap(),
                 switch_section_labels: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SwitchFallthrough, 2) => SyntaxVariant::SwitchFallthrough(Box::new(SwitchFallthroughChildren {
                 fallthrough_semicolon: ts.pop().unwrap(),
                 fallthrough_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CaseLabel, 3) => SyntaxVariant::CaseLabel(Box::new(CaseLabelChildren {
                 case_colon: ts.pop().unwrap(),
                 case_expression: ts.pop().unwrap(),
                 case_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DefaultLabel, 2) => SyntaxVariant::DefaultLabel(Box::new(DefaultLabelChildren {
                 default_colon: ts.pop().unwrap(),
                 default_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ReturnStatement, 3) => SyntaxVariant::ReturnStatement(Box::new(ReturnStatementChildren {
                 return_semicolon: ts.pop().unwrap(),
                 return_expression: ts.pop().unwrap(),
                 return_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::GotoLabel, 2) => SyntaxVariant::GotoLabel(Box::new(GotoLabelChildren {
                 goto_label_colon: ts.pop().unwrap(),
                 goto_label_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::GotoStatement, 3) => SyntaxVariant::GotoStatement(Box::new(GotoStatementChildren {
                 goto_statement_semicolon: ts.pop().unwrap(),
                 goto_statement_label_name: ts.pop().unwrap(),
                 goto_statement_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ThrowStatement, 3) => SyntaxVariant::ThrowStatement(Box::new(ThrowStatementChildren {
                 throw_semicolon: ts.pop().unwrap(),
                 throw_expression: ts.pop().unwrap(),
                 throw_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::BreakStatement, 2) => SyntaxVariant::BreakStatement(Box::new(BreakStatementChildren {
                 break_semicolon: ts.pop().unwrap(),
                 break_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ContinueStatement, 2) => SyntaxVariant::ContinueStatement(Box::new(ContinueStatementChildren {
                 continue_semicolon: ts.pop().unwrap(),
                 continue_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EchoStatement, 3) => SyntaxVariant::EchoStatement(Box::new(EchoStatementChildren {
                 echo_semicolon: ts.pop().unwrap(),
                 echo_expressions: ts.pop().unwrap(),
                 echo_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ConcurrentStatement, 2) => SyntaxVariant::ConcurrentStatement(Box::new(ConcurrentStatementChildren {
                 concurrent_statement: ts.pop().unwrap(),
                 concurrent_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SimpleInitializer, 2) => SyntaxVariant::SimpleInitializer(Box::new(SimpleInitializerChildren {
                 simple_initializer_value: ts.pop().unwrap(),
                 simple_initializer_equal: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AnonymousClass, 9) => SyntaxVariant::AnonymousClass(Box::new(AnonymousClassChildren {
                 anonymous_class_body: ts.pop().unwrap(),
                 anonymous_class_implements_list: ts.pop().unwrap(),
                 anonymous_class_implements_keyword: ts.pop().unwrap(),
                 anonymous_class_extends_list: ts.pop().unwrap(),
                 anonymous_class_extends_keyword: ts.pop().unwrap(),
                 anonymous_class_right_paren: ts.pop().unwrap(),
                 anonymous_class_argument_list: ts.pop().unwrap(),
                 anonymous_class_left_paren: ts.pop().unwrap(),
                 anonymous_class_class_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AnonymousFunction, 11) => SyntaxVariant::AnonymousFunction(Box::new(AnonymousFunctionChildren {
                 anonymous_body: ts.pop().unwrap(),
                 anonymous_use: ts.pop().unwrap(),
                 anonymous_type: ts.pop().unwrap(),
                 anonymous_colon: ts.pop().unwrap(),
                 anonymous_right_paren: ts.pop().unwrap(),
                 anonymous_parameters: ts.pop().unwrap(),
                 anonymous_left_paren: ts.pop().unwrap(),
                 anonymous_function_keyword: ts.pop().unwrap(),
                 anonymous_async_keyword: ts.pop().unwrap(),
                 anonymous_static_keyword: ts.pop().unwrap(),
                 anonymous_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AnonymousFunctionUseClause, 4) => SyntaxVariant::AnonymousFunctionUseClause(Box::new(AnonymousFunctionUseClauseChildren {
                 anonymous_use_right_paren: ts.pop().unwrap(),
                 anonymous_use_variables: ts.pop().unwrap(),
                 anonymous_use_left_paren: ts.pop().unwrap(),
                 anonymous_use_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::LambdaExpression, 5) => SyntaxVariant::LambdaExpression(Box::new(LambdaExpressionChildren {
                 lambda_body: ts.pop().unwrap(),
                 lambda_arrow: ts.pop().unwrap(),
                 lambda_signature: ts.pop().unwrap(),
                 lambda_async: ts.pop().unwrap(),
                 lambda_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::LambdaSignature, 5) => SyntaxVariant::LambdaSignature(Box::new(LambdaSignatureChildren {
                 lambda_type: ts.pop().unwrap(),
                 lambda_colon: ts.pop().unwrap(),
                 lambda_right_paren: ts.pop().unwrap(),
                 lambda_parameters: ts.pop().unwrap(),
                 lambda_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CastExpression, 4) => SyntaxVariant::CastExpression(Box::new(CastExpressionChildren {
                 cast_operand: ts.pop().unwrap(),
                 cast_right_paren: ts.pop().unwrap(),
                 cast_type: ts.pop().unwrap(),
                 cast_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ScopeResolutionExpression, 3) => SyntaxVariant::ScopeResolutionExpression(Box::new(ScopeResolutionExpressionChildren {
                 scope_resolution_name: ts.pop().unwrap(),
                 scope_resolution_operator: ts.pop().unwrap(),
                 scope_resolution_qualifier: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MemberSelectionExpression, 3) => SyntaxVariant::MemberSelectionExpression(Box::new(MemberSelectionExpressionChildren {
                 member_name: ts.pop().unwrap(),
                 member_operator: ts.pop().unwrap(),
                 member_object: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SafeMemberSelectionExpression, 3) => SyntaxVariant::SafeMemberSelectionExpression(Box::new(SafeMemberSelectionExpressionChildren {
                 safe_member_name: ts.pop().unwrap(),
                 safe_member_operator: ts.pop().unwrap(),
                 safe_member_object: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EmbeddedMemberSelectionExpression, 3) => SyntaxVariant::EmbeddedMemberSelectionExpression(Box::new(EmbeddedMemberSelectionExpressionChildren {
                 embedded_member_name: ts.pop().unwrap(),
                 embedded_member_operator: ts.pop().unwrap(),
                 embedded_member_object: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::YieldExpression, 2) => SyntaxVariant::YieldExpression(Box::new(YieldExpressionChildren {
                 yield_operand: ts.pop().unwrap(),
                 yield_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PrefixUnaryExpression, 2) => SyntaxVariant::PrefixUnaryExpression(Box::new(PrefixUnaryExpressionChildren {
                 prefix_unary_operand: ts.pop().unwrap(),
                 prefix_unary_operator: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PostfixUnaryExpression, 2) => SyntaxVariant::PostfixUnaryExpression(Box::new(PostfixUnaryExpressionChildren {
                 postfix_unary_operator: ts.pop().unwrap(),
                 postfix_unary_operand: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::BinaryExpression, 3) => SyntaxVariant::BinaryExpression(Box::new(BinaryExpressionChildren {
                 binary_right_operand: ts.pop().unwrap(),
                 binary_operator: ts.pop().unwrap(),
                 binary_left_operand: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::IsExpression, 3) => SyntaxVariant::IsExpression(Box::new(IsExpressionChildren {
                 is_right_operand: ts.pop().unwrap(),
                 is_operator: ts.pop().unwrap(),
                 is_left_operand: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AsExpression, 3) => SyntaxVariant::AsExpression(Box::new(AsExpressionChildren {
                 as_right_operand: ts.pop().unwrap(),
                 as_operator: ts.pop().unwrap(),
                 as_left_operand: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NullableAsExpression, 3) => SyntaxVariant::NullableAsExpression(Box::new(NullableAsExpressionChildren {
                 nullable_as_right_operand: ts.pop().unwrap(),
                 nullable_as_operator: ts.pop().unwrap(),
                 nullable_as_left_operand: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ConditionalExpression, 5) => SyntaxVariant::ConditionalExpression(Box::new(ConditionalExpressionChildren {
                 conditional_alternative: ts.pop().unwrap(),
                 conditional_colon: ts.pop().unwrap(),
                 conditional_consequence: ts.pop().unwrap(),
                 conditional_question: ts.pop().unwrap(),
                 conditional_test: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EvalExpression, 4) => SyntaxVariant::EvalExpression(Box::new(EvalExpressionChildren {
                 eval_right_paren: ts.pop().unwrap(),
                 eval_argument: ts.pop().unwrap(),
                 eval_left_paren: ts.pop().unwrap(),
                 eval_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DefineExpression, 4) => SyntaxVariant::DefineExpression(Box::new(DefineExpressionChildren {
                 define_right_paren: ts.pop().unwrap(),
                 define_argument_list: ts.pop().unwrap(),
                 define_left_paren: ts.pop().unwrap(),
                 define_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::IssetExpression, 4) => SyntaxVariant::IssetExpression(Box::new(IssetExpressionChildren {
                 isset_right_paren: ts.pop().unwrap(),
                 isset_argument_list: ts.pop().unwrap(),
                 isset_left_paren: ts.pop().unwrap(),
                 isset_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FunctionCallExpression, 5) => SyntaxVariant::FunctionCallExpression(Box::new(FunctionCallExpressionChildren {
                 function_call_right_paren: ts.pop().unwrap(),
                 function_call_argument_list: ts.pop().unwrap(),
                 function_call_left_paren: ts.pop().unwrap(),
                 function_call_type_args: ts.pop().unwrap(),
                 function_call_receiver: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FunctionPointerExpression, 2) => SyntaxVariant::FunctionPointerExpression(Box::new(FunctionPointerExpressionChildren {
                 function_pointer_type_args: ts.pop().unwrap(),
                 function_pointer_receiver: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ParenthesizedExpression, 3) => SyntaxVariant::ParenthesizedExpression(Box::new(ParenthesizedExpressionChildren {
                 parenthesized_expression_right_paren: ts.pop().unwrap(),
                 parenthesized_expression_expression: ts.pop().unwrap(),
                 parenthesized_expression_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::BracedExpression, 3) => SyntaxVariant::BracedExpression(Box::new(BracedExpressionChildren {
                 braced_expression_right_brace: ts.pop().unwrap(),
                 braced_expression_expression: ts.pop().unwrap(),
                 braced_expression_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EmbeddedBracedExpression, 3) => SyntaxVariant::EmbeddedBracedExpression(Box::new(EmbeddedBracedExpressionChildren {
                 embedded_braced_expression_right_brace: ts.pop().unwrap(),
                 embedded_braced_expression_expression: ts.pop().unwrap(),
                 embedded_braced_expression_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ListExpression, 4) => SyntaxVariant::ListExpression(Box::new(ListExpressionChildren {
                 list_right_paren: ts.pop().unwrap(),
                 list_members: ts.pop().unwrap(),
                 list_left_paren: ts.pop().unwrap(),
                 list_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::CollectionLiteralExpression, 4) => SyntaxVariant::CollectionLiteralExpression(Box::new(CollectionLiteralExpressionChildren {
                 collection_literal_right_brace: ts.pop().unwrap(),
                 collection_literal_initializers: ts.pop().unwrap(),
                 collection_literal_left_brace: ts.pop().unwrap(),
                 collection_literal_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ObjectCreationExpression, 2) => SyntaxVariant::ObjectCreationExpression(Box::new(ObjectCreationExpressionChildren {
                 object_creation_object: ts.pop().unwrap(),
                 object_creation_new_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ConstructorCall, 4) => SyntaxVariant::ConstructorCall(Box::new(ConstructorCallChildren {
                 constructor_call_right_paren: ts.pop().unwrap(),
                 constructor_call_argument_list: ts.pop().unwrap(),
                 constructor_call_left_paren: ts.pop().unwrap(),
                 constructor_call_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::RecordCreationExpression, 4) => SyntaxVariant::RecordCreationExpression(Box::new(RecordCreationExpressionChildren {
                 record_creation_right_bracket: ts.pop().unwrap(),
                 record_creation_members: ts.pop().unwrap(),
                 record_creation_left_bracket: ts.pop().unwrap(),
                 record_creation_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DarrayIntrinsicExpression, 5) => SyntaxVariant::DarrayIntrinsicExpression(Box::new(DarrayIntrinsicExpressionChildren {
                 darray_intrinsic_right_bracket: ts.pop().unwrap(),
                 darray_intrinsic_members: ts.pop().unwrap(),
                 darray_intrinsic_left_bracket: ts.pop().unwrap(),
                 darray_intrinsic_explicit_type: ts.pop().unwrap(),
                 darray_intrinsic_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DictionaryIntrinsicExpression, 5) => SyntaxVariant::DictionaryIntrinsicExpression(Box::new(DictionaryIntrinsicExpressionChildren {
                 dictionary_intrinsic_right_bracket: ts.pop().unwrap(),
                 dictionary_intrinsic_members: ts.pop().unwrap(),
                 dictionary_intrinsic_left_bracket: ts.pop().unwrap(),
                 dictionary_intrinsic_explicit_type: ts.pop().unwrap(),
                 dictionary_intrinsic_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::KeysetIntrinsicExpression, 5) => SyntaxVariant::KeysetIntrinsicExpression(Box::new(KeysetIntrinsicExpressionChildren {
                 keyset_intrinsic_right_bracket: ts.pop().unwrap(),
                 keyset_intrinsic_members: ts.pop().unwrap(),
                 keyset_intrinsic_left_bracket: ts.pop().unwrap(),
                 keyset_intrinsic_explicit_type: ts.pop().unwrap(),
                 keyset_intrinsic_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VarrayIntrinsicExpression, 5) => SyntaxVariant::VarrayIntrinsicExpression(Box::new(VarrayIntrinsicExpressionChildren {
                 varray_intrinsic_right_bracket: ts.pop().unwrap(),
                 varray_intrinsic_members: ts.pop().unwrap(),
                 varray_intrinsic_left_bracket: ts.pop().unwrap(),
                 varray_intrinsic_explicit_type: ts.pop().unwrap(),
                 varray_intrinsic_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VectorIntrinsicExpression, 5) => SyntaxVariant::VectorIntrinsicExpression(Box::new(VectorIntrinsicExpressionChildren {
                 vector_intrinsic_right_bracket: ts.pop().unwrap(),
                 vector_intrinsic_members: ts.pop().unwrap(),
                 vector_intrinsic_left_bracket: ts.pop().unwrap(),
                 vector_intrinsic_explicit_type: ts.pop().unwrap(),
                 vector_intrinsic_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ElementInitializer, 3) => SyntaxVariant::ElementInitializer(Box::new(ElementInitializerChildren {
                 element_value: ts.pop().unwrap(),
                 element_arrow: ts.pop().unwrap(),
                 element_key: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SubscriptExpression, 4) => SyntaxVariant::SubscriptExpression(Box::new(SubscriptExpressionChildren {
                 subscript_right_bracket: ts.pop().unwrap(),
                 subscript_index: ts.pop().unwrap(),
                 subscript_left_bracket: ts.pop().unwrap(),
                 subscript_receiver: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::EmbeddedSubscriptExpression, 4) => SyntaxVariant::EmbeddedSubscriptExpression(Box::new(EmbeddedSubscriptExpressionChildren {
                 embedded_subscript_right_bracket: ts.pop().unwrap(),
                 embedded_subscript_index: ts.pop().unwrap(),
                 embedded_subscript_left_bracket: ts.pop().unwrap(),
                 embedded_subscript_receiver: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AwaitableCreationExpression, 3) => SyntaxVariant::AwaitableCreationExpression(Box::new(AwaitableCreationExpressionChildren {
                 awaitable_compound_statement: ts.pop().unwrap(),
                 awaitable_async: ts.pop().unwrap(),
                 awaitable_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPChildrenDeclaration, 3) => SyntaxVariant::XHPChildrenDeclaration(Box::new(XHPChildrenDeclarationChildren {
                 xhp_children_semicolon: ts.pop().unwrap(),
                 xhp_children_expression: ts.pop().unwrap(),
                 xhp_children_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPChildrenParenthesizedList, 3) => SyntaxVariant::XHPChildrenParenthesizedList(Box::new(XHPChildrenParenthesizedListChildren {
                 xhp_children_list_right_paren: ts.pop().unwrap(),
                 xhp_children_list_xhp_children: ts.pop().unwrap(),
                 xhp_children_list_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPCategoryDeclaration, 3) => SyntaxVariant::XHPCategoryDeclaration(Box::new(XHPCategoryDeclarationChildren {
                 xhp_category_semicolon: ts.pop().unwrap(),
                 xhp_category_categories: ts.pop().unwrap(),
                 xhp_category_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPEnumType, 4) => SyntaxVariant::XHPEnumType(Box::new(XHPEnumTypeChildren {
                 xhp_enum_right_brace: ts.pop().unwrap(),
                 xhp_enum_values: ts.pop().unwrap(),
                 xhp_enum_left_brace: ts.pop().unwrap(),
                 xhp_enum_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPLateinit, 2) => SyntaxVariant::XHPLateinit(Box::new(XHPLateinitChildren {
                 xhp_lateinit_keyword: ts.pop().unwrap(),
                 xhp_lateinit_at: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPRequired, 2) => SyntaxVariant::XHPRequired(Box::new(XHPRequiredChildren {
                 xhp_required_keyword: ts.pop().unwrap(),
                 xhp_required_at: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPClassAttributeDeclaration, 3) => SyntaxVariant::XHPClassAttributeDeclaration(Box::new(XHPClassAttributeDeclarationChildren {
                 xhp_attribute_semicolon: ts.pop().unwrap(),
                 xhp_attribute_attributes: ts.pop().unwrap(),
                 xhp_attribute_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPClassAttribute, 4) => SyntaxVariant::XHPClassAttribute(Box::new(XHPClassAttributeChildren {
                 xhp_attribute_decl_required: ts.pop().unwrap(),
                 xhp_attribute_decl_initializer: ts.pop().unwrap(),
                 xhp_attribute_decl_name: ts.pop().unwrap(),
                 xhp_attribute_decl_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPSimpleClassAttribute, 1) => SyntaxVariant::XHPSimpleClassAttribute(Box::new(XHPSimpleClassAttributeChildren {
                 xhp_simple_class_attribute_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPSimpleAttribute, 3) => SyntaxVariant::XHPSimpleAttribute(Box::new(XHPSimpleAttributeChildren {
                 xhp_simple_attribute_expression: ts.pop().unwrap(),
                 xhp_simple_attribute_equal: ts.pop().unwrap(),
                 xhp_simple_attribute_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPSpreadAttribute, 4) => SyntaxVariant::XHPSpreadAttribute(Box::new(XHPSpreadAttributeChildren {
                 xhp_spread_attribute_right_brace: ts.pop().unwrap(),
                 xhp_spread_attribute_expression: ts.pop().unwrap(),
                 xhp_spread_attribute_spread_operator: ts.pop().unwrap(),
                 xhp_spread_attribute_left_brace: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPOpen, 4) => SyntaxVariant::XHPOpen(Box::new(XHPOpenChildren {
                 xhp_open_right_angle: ts.pop().unwrap(),
                 xhp_open_attributes: ts.pop().unwrap(),
                 xhp_open_name: ts.pop().unwrap(),
                 xhp_open_left_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPExpression, 3) => SyntaxVariant::XHPExpression(Box::new(XHPExpressionChildren {
                 xhp_close: ts.pop().unwrap(),
                 xhp_body: ts.pop().unwrap(),
                 xhp_open: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::XHPClose, 3) => SyntaxVariant::XHPClose(Box::new(XHPCloseChildren {
                 xhp_close_right_angle: ts.pop().unwrap(),
                 xhp_close_name: ts.pop().unwrap(),
                 xhp_close_left_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeConstant, 3) => SyntaxVariant::TypeConstant(Box::new(TypeConstantChildren {
                 type_constant_right_type: ts.pop().unwrap(),
                 type_constant_separator: ts.pop().unwrap(),
                 type_constant_left_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PUAccess, 3) => SyntaxVariant::PUAccess(Box::new(PUAccessChildren {
                 pu_access_right_type: ts.pop().unwrap(),
                 pu_access_separator: ts.pop().unwrap(),
                 pu_access_left_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VectorTypeSpecifier, 5) => SyntaxVariant::VectorTypeSpecifier(Box::new(VectorTypeSpecifierChildren {
                 vector_type_right_angle: ts.pop().unwrap(),
                 vector_type_trailing_comma: ts.pop().unwrap(),
                 vector_type_type: ts.pop().unwrap(),
                 vector_type_left_angle: ts.pop().unwrap(),
                 vector_type_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::KeysetTypeSpecifier, 5) => SyntaxVariant::KeysetTypeSpecifier(Box::new(KeysetTypeSpecifierChildren {
                 keyset_type_right_angle: ts.pop().unwrap(),
                 keyset_type_trailing_comma: ts.pop().unwrap(),
                 keyset_type_type: ts.pop().unwrap(),
                 keyset_type_left_angle: ts.pop().unwrap(),
                 keyset_type_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TupleTypeExplicitSpecifier, 4) => SyntaxVariant::TupleTypeExplicitSpecifier(Box::new(TupleTypeExplicitSpecifierChildren {
                 tuple_type_right_angle: ts.pop().unwrap(),
                 tuple_type_types: ts.pop().unwrap(),
                 tuple_type_left_angle: ts.pop().unwrap(),
                 tuple_type_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VarrayTypeSpecifier, 5) => SyntaxVariant::VarrayTypeSpecifier(Box::new(VarrayTypeSpecifierChildren {
                 varray_right_angle: ts.pop().unwrap(),
                 varray_trailing_comma: ts.pop().unwrap(),
                 varray_type: ts.pop().unwrap(),
                 varray_left_angle: ts.pop().unwrap(),
                 varray_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::VectorArrayTypeSpecifier, 4) => SyntaxVariant::VectorArrayTypeSpecifier(Box::new(VectorArrayTypeSpecifierChildren {
                 vector_array_right_angle: ts.pop().unwrap(),
                 vector_array_type: ts.pop().unwrap(),
                 vector_array_left_angle: ts.pop().unwrap(),
                 vector_array_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeParameter, 6) => SyntaxVariant::TypeParameter(Box::new(TypeParameterChildren {
                 type_constraints: ts.pop().unwrap(),
                 type_param_params: ts.pop().unwrap(),
                 type_name: ts.pop().unwrap(),
                 type_variance: ts.pop().unwrap(),
                 type_reified: ts.pop().unwrap(),
                 type_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeConstraint, 2) => SyntaxVariant::TypeConstraint(Box::new(TypeConstraintChildren {
                 constraint_type: ts.pop().unwrap(),
                 constraint_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DarrayTypeSpecifier, 7) => SyntaxVariant::DarrayTypeSpecifier(Box::new(DarrayTypeSpecifierChildren {
                 darray_right_angle: ts.pop().unwrap(),
                 darray_trailing_comma: ts.pop().unwrap(),
                 darray_value: ts.pop().unwrap(),
                 darray_comma: ts.pop().unwrap(),
                 darray_key: ts.pop().unwrap(),
                 darray_left_angle: ts.pop().unwrap(),
                 darray_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::MapArrayTypeSpecifier, 6) => SyntaxVariant::MapArrayTypeSpecifier(Box::new(MapArrayTypeSpecifierChildren {
                 map_array_right_angle: ts.pop().unwrap(),
                 map_array_value: ts.pop().unwrap(),
                 map_array_comma: ts.pop().unwrap(),
                 map_array_key: ts.pop().unwrap(),
                 map_array_left_angle: ts.pop().unwrap(),
                 map_array_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::DictionaryTypeSpecifier, 4) => SyntaxVariant::DictionaryTypeSpecifier(Box::new(DictionaryTypeSpecifierChildren {
                 dictionary_type_right_angle: ts.pop().unwrap(),
                 dictionary_type_members: ts.pop().unwrap(),
                 dictionary_type_left_angle: ts.pop().unwrap(),
                 dictionary_type_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ClosureTypeSpecifier, 8) => SyntaxVariant::ClosureTypeSpecifier(Box::new(ClosureTypeSpecifierChildren {
                 closure_outer_right_paren: ts.pop().unwrap(),
                 closure_return_type: ts.pop().unwrap(),
                 closure_colon: ts.pop().unwrap(),
                 closure_inner_right_paren: ts.pop().unwrap(),
                 closure_parameter_list: ts.pop().unwrap(),
                 closure_inner_left_paren: ts.pop().unwrap(),
                 closure_function_keyword: ts.pop().unwrap(),
                 closure_outer_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ClosureParameterTypeSpecifier, 2) => SyntaxVariant::ClosureParameterTypeSpecifier(Box::new(ClosureParameterTypeSpecifierChildren {
                 closure_parameter_type: ts.pop().unwrap(),
                 closure_parameter_call_convention: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ClassnameTypeSpecifier, 5) => SyntaxVariant::ClassnameTypeSpecifier(Box::new(ClassnameTypeSpecifierChildren {
                 classname_right_angle: ts.pop().unwrap(),
                 classname_trailing_comma: ts.pop().unwrap(),
                 classname_type: ts.pop().unwrap(),
                 classname_left_angle: ts.pop().unwrap(),
                 classname_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FieldSpecifier, 4) => SyntaxVariant::FieldSpecifier(Box::new(FieldSpecifierChildren {
                 field_type: ts.pop().unwrap(),
                 field_arrow: ts.pop().unwrap(),
                 field_name: ts.pop().unwrap(),
                 field_question: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::FieldInitializer, 3) => SyntaxVariant::FieldInitializer(Box::new(FieldInitializerChildren {
                 field_initializer_value: ts.pop().unwrap(),
                 field_initializer_arrow: ts.pop().unwrap(),
                 field_initializer_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ShapeTypeSpecifier, 5) => SyntaxVariant::ShapeTypeSpecifier(Box::new(ShapeTypeSpecifierChildren {
                 shape_type_right_paren: ts.pop().unwrap(),
                 shape_type_ellipsis: ts.pop().unwrap(),
                 shape_type_fields: ts.pop().unwrap(),
                 shape_type_left_paren: ts.pop().unwrap(),
                 shape_type_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ShapeExpression, 4) => SyntaxVariant::ShapeExpression(Box::new(ShapeExpressionChildren {
                 shape_expression_right_paren: ts.pop().unwrap(),
                 shape_expression_fields: ts.pop().unwrap(),
                 shape_expression_left_paren: ts.pop().unwrap(),
                 shape_expression_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TupleExpression, 4) => SyntaxVariant::TupleExpression(Box::new(TupleExpressionChildren {
                 tuple_expression_right_paren: ts.pop().unwrap(),
                 tuple_expression_items: ts.pop().unwrap(),
                 tuple_expression_left_paren: ts.pop().unwrap(),
                 tuple_expression_keyword: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::GenericTypeSpecifier, 2) => SyntaxVariant::GenericTypeSpecifier(Box::new(GenericTypeSpecifierChildren {
                 generic_argument_list: ts.pop().unwrap(),
                 generic_class_type: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::NullableTypeSpecifier, 2) => SyntaxVariant::NullableTypeSpecifier(Box::new(NullableTypeSpecifierChildren {
                 nullable_type: ts.pop().unwrap(),
                 nullable_question: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::LikeTypeSpecifier, 2) => SyntaxVariant::LikeTypeSpecifier(Box::new(LikeTypeSpecifierChildren {
                 like_type: ts.pop().unwrap(),
                 like_tilde: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::SoftTypeSpecifier, 2) => SyntaxVariant::SoftTypeSpecifier(Box::new(SoftTypeSpecifierChildren {
                 soft_type: ts.pop().unwrap(),
                 soft_at: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::AttributizedSpecifier, 2) => SyntaxVariant::AttributizedSpecifier(Box::new(AttributizedSpecifierChildren {
                 attributized_specifier_type: ts.pop().unwrap(),
                 attributized_specifier_attribute_spec: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ReifiedTypeArgument, 2) => SyntaxVariant::ReifiedTypeArgument(Box::new(ReifiedTypeArgumentChildren {
                 reified_type_argument_type: ts.pop().unwrap(),
                 reified_type_argument_reified: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeArguments, 3) => SyntaxVariant::TypeArguments(Box::new(TypeArgumentsChildren {
                 type_arguments_right_angle: ts.pop().unwrap(),
                 type_arguments_types: ts.pop().unwrap(),
                 type_arguments_left_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TypeParameters, 3) => SyntaxVariant::TypeParameters(Box::new(TypeParametersChildren {
                 type_parameters_right_angle: ts.pop().unwrap(),
                 type_parameters_parameters: ts.pop().unwrap(),
                 type_parameters_left_angle: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::TupleTypeSpecifier, 3) => SyntaxVariant::TupleTypeSpecifier(Box::new(TupleTypeSpecifierChildren {
                 tuple_right_paren: ts.pop().unwrap(),
                 tuple_types: ts.pop().unwrap(),
                 tuple_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::UnionTypeSpecifier, 3) => SyntaxVariant::UnionTypeSpecifier(Box::new(UnionTypeSpecifierChildren {
                 union_right_paren: ts.pop().unwrap(),
                 union_types: ts.pop().unwrap(),
                 union_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::IntersectionTypeSpecifier, 3) => SyntaxVariant::IntersectionTypeSpecifier(Box::new(IntersectionTypeSpecifierChildren {
                 intersection_right_paren: ts.pop().unwrap(),
                 intersection_types: ts.pop().unwrap(),
                 intersection_left_paren: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ErrorSyntax, 1) => SyntaxVariant::ErrorSyntax(Box::new(ErrorSyntaxChildren {
                 error_error: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::ListItem, 2) => SyntaxVariant::ListItem(Box::new(ListItemChildren {
                 list_separator: ts.pop().unwrap(),
                 list_item: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketAtomExpression, 2) => SyntaxVariant::PocketAtomExpression(Box::new(PocketAtomExpressionChildren {
                 pocket_atom_expression: ts.pop().unwrap(),
                 pocket_atom_glyph: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketIdentifierExpression, 5) => SyntaxVariant::PocketIdentifierExpression(Box::new(PocketIdentifierExpressionChildren {
                 pocket_identifier_name: ts.pop().unwrap(),
                 pocket_identifier_operator: ts.pop().unwrap(),
                 pocket_identifier_field: ts.pop().unwrap(),
                 pocket_identifier_pu_operator: ts.pop().unwrap(),
                 pocket_identifier_qualifier: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketAtomMappingDeclaration, 6) => SyntaxVariant::PocketAtomMappingDeclaration(Box::new(PocketAtomMappingDeclarationChildren {
                 pocket_atom_mapping_semicolon: ts.pop().unwrap(),
                 pocket_atom_mapping_right_paren: ts.pop().unwrap(),
                 pocket_atom_mapping_mappings: ts.pop().unwrap(),
                 pocket_atom_mapping_left_paren: ts.pop().unwrap(),
                 pocket_atom_mapping_name: ts.pop().unwrap(),
                 pocket_atom_mapping_glyph: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketEnumDeclaration, 7) => SyntaxVariant::PocketEnumDeclaration(Box::new(PocketEnumDeclarationChildren {
                 pocket_enum_right_brace: ts.pop().unwrap(),
                 pocket_enum_fields: ts.pop().unwrap(),
                 pocket_enum_left_brace: ts.pop().unwrap(),
                 pocket_enum_name: ts.pop().unwrap(),
                 pocket_enum_enum: ts.pop().unwrap(),
                 pocket_enum_modifiers: ts.pop().unwrap(),
                 pocket_enum_attributes: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketFieldTypeExprDeclaration, 4) => SyntaxVariant::PocketFieldTypeExprDeclaration(Box::new(PocketFieldTypeExprDeclarationChildren {
                 pocket_field_type_expr_semicolon: ts.pop().unwrap(),
                 pocket_field_type_expr_name: ts.pop().unwrap(),
                 pocket_field_type_expr_type: ts.pop().unwrap(),
                 pocket_field_type_expr_case: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketFieldTypeDeclaration, 4) => SyntaxVariant::PocketFieldTypeDeclaration(Box::new(PocketFieldTypeDeclarationChildren {
                 pocket_field_type_semicolon: ts.pop().unwrap(),
                 pocket_field_type_type_parameter: ts.pop().unwrap(),
                 pocket_field_type_type: ts.pop().unwrap(),
                 pocket_field_type_case: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketMappingIdDeclaration, 2) => SyntaxVariant::PocketMappingIdDeclaration(Box::new(PocketMappingIdDeclarationChildren {
                 pocket_mapping_id_initializer: ts.pop().unwrap(),
                 pocket_mapping_id_name: ts.pop().unwrap(),
                 
             })),
             (SyntaxKind::PocketMappingTypeDeclaration, 4) => SyntaxVariant::PocketMappingTypeDeclaration(Box::new(PocketMappingTypeDeclarationChildren {
                 pocket_mapping_type_type: ts.pop().unwrap(),
                 pocket_mapping_type_equal: ts.pop().unwrap(),
                 pocket_mapping_type_name: ts.pop().unwrap(),
                 pocket_mapping_type_keyword: ts.pop().unwrap(),
                 
             })),
             _ => panic!("from_children called with wrong number of children"),
         }
    }
}

#[derive(Debug, Clone)]
pub struct EndOfFileChildren<T, V> {
    pub end_of_file_token: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ScriptChildren<T, V> {
    pub script_declarations: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct QualifiedNameChildren<T, V> {
    pub qualified_name_parts: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SimpleTypeSpecifierChildren<T, V> {
    pub simple_type_specifier: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct LiteralExpressionChildren<T, V> {
    pub literal_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixedStringExpressionChildren<T, V> {
    pub prefixed_string_name: Syntax<T, V>,
    pub prefixed_string_str: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixedCodeExpressionChildren<T, V> {
    pub prefixed_code_prefix: Syntax<T, V>,
    pub prefixed_code_left_backtick: Syntax<T, V>,
    pub prefixed_code_expression: Syntax<T, V>,
    pub prefixed_code_right_backtick: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VariableExpressionChildren<T, V> {
    pub variable_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PipeVariableExpressionChildren<T, V> {
    pub pipe_variable_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FileAttributeSpecificationChildren<T, V> {
    pub file_attribute_specification_left_double_angle: Syntax<T, V>,
    pub file_attribute_specification_keyword: Syntax<T, V>,
    pub file_attribute_specification_colon: Syntax<T, V>,
    pub file_attribute_specification_attributes: Syntax<T, V>,
    pub file_attribute_specification_right_double_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumDeclarationChildren<T, V> {
    pub enum_attribute_spec: Syntax<T, V>,
    pub enum_keyword: Syntax<T, V>,
    pub enum_name: Syntax<T, V>,
    pub enum_colon: Syntax<T, V>,
    pub enum_base: Syntax<T, V>,
    pub enum_type: Syntax<T, V>,
    pub enum_includes_keyword: Syntax<T, V>,
    pub enum_includes_list: Syntax<T, V>,
    pub enum_left_brace: Syntax<T, V>,
    pub enum_enumerators: Syntax<T, V>,
    pub enum_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EnumeratorChildren<T, V> {
    pub enumerator_name: Syntax<T, V>,
    pub enumerator_equal: Syntax<T, V>,
    pub enumerator_value: Syntax<T, V>,
    pub enumerator_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct RecordDeclarationChildren<T, V> {
    pub record_attribute_spec: Syntax<T, V>,
    pub record_modifier: Syntax<T, V>,
    pub record_keyword: Syntax<T, V>,
    pub record_name: Syntax<T, V>,
    pub record_extends_keyword: Syntax<T, V>,
    pub record_extends_opt: Syntax<T, V>,
    pub record_left_brace: Syntax<T, V>,
    pub record_fields: Syntax<T, V>,
    pub record_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct RecordFieldChildren<T, V> {
    pub record_field_type: Syntax<T, V>,
    pub record_field_name: Syntax<T, V>,
    pub record_field_init: Syntax<T, V>,
    pub record_field_semi: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AliasDeclarationChildren<T, V> {
    pub alias_attribute_spec: Syntax<T, V>,
    pub alias_keyword: Syntax<T, V>,
    pub alias_name: Syntax<T, V>,
    pub alias_generic_parameter: Syntax<T, V>,
    pub alias_constraint: Syntax<T, V>,
    pub alias_equal: Syntax<T, V>,
    pub alias_type: Syntax<T, V>,
    pub alias_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PropertyDeclarationChildren<T, V> {
    pub property_attribute_spec: Syntax<T, V>,
    pub property_modifiers: Syntax<T, V>,
    pub property_type: Syntax<T, V>,
    pub property_declarators: Syntax<T, V>,
    pub property_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PropertyDeclaratorChildren<T, V> {
    pub property_name: Syntax<T, V>,
    pub property_initializer: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceDeclarationChildren<T, V> {
    pub namespace_header: Syntax<T, V>,
    pub namespace_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceDeclarationHeaderChildren<T, V> {
    pub namespace_keyword: Syntax<T, V>,
    pub namespace_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceBodyChildren<T, V> {
    pub namespace_left_brace: Syntax<T, V>,
    pub namespace_declarations: Syntax<T, V>,
    pub namespace_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceEmptyBodyChildren<T, V> {
    pub namespace_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceUseDeclarationChildren<T, V> {
    pub namespace_use_keyword: Syntax<T, V>,
    pub namespace_use_kind: Syntax<T, V>,
    pub namespace_use_clauses: Syntax<T, V>,
    pub namespace_use_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceGroupUseDeclarationChildren<T, V> {
    pub namespace_group_use_keyword: Syntax<T, V>,
    pub namespace_group_use_kind: Syntax<T, V>,
    pub namespace_group_use_prefix: Syntax<T, V>,
    pub namespace_group_use_left_brace: Syntax<T, V>,
    pub namespace_group_use_clauses: Syntax<T, V>,
    pub namespace_group_use_right_brace: Syntax<T, V>,
    pub namespace_group_use_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NamespaceUseClauseChildren<T, V> {
    pub namespace_use_clause_kind: Syntax<T, V>,
    pub namespace_use_name: Syntax<T, V>,
    pub namespace_use_as: Syntax<T, V>,
    pub namespace_use_alias: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionDeclarationChildren<T, V> {
    pub function_attribute_spec: Syntax<T, V>,
    pub function_declaration_header: Syntax<T, V>,
    pub function_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionDeclarationHeaderChildren<T, V> {
    pub function_modifiers: Syntax<T, V>,
    pub function_keyword: Syntax<T, V>,
    pub function_name: Syntax<T, V>,
    pub function_type_parameter_list: Syntax<T, V>,
    pub function_left_paren: Syntax<T, V>,
    pub function_parameter_list: Syntax<T, V>,
    pub function_right_paren: Syntax<T, V>,
    pub function_capability_provisional: Syntax<T, V>,
    pub function_colon: Syntax<T, V>,
    pub function_type: Syntax<T, V>,
    pub function_where_clause: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CapabilityProvisionalChildren<T, V> {
    pub capability_provisional_at: Syntax<T, V>,
    pub capability_provisional_left_brace: Syntax<T, V>,
    pub capability_provisional_type: Syntax<T, V>,
    pub capability_provisional_unsafe_plus: Syntax<T, V>,
    pub capability_provisional_unsafe_type: Syntax<T, V>,
    pub capability_provisional_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct WhereClauseChildren<T, V> {
    pub where_clause_keyword: Syntax<T, V>,
    pub where_clause_constraints: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct WhereConstraintChildren<T, V> {
    pub where_constraint_left_type: Syntax<T, V>,
    pub where_constraint_operator: Syntax<T, V>,
    pub where_constraint_right_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MethodishDeclarationChildren<T, V> {
    pub methodish_attribute: Syntax<T, V>,
    pub methodish_function_decl_header: Syntax<T, V>,
    pub methodish_function_body: Syntax<T, V>,
    pub methodish_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MethodishTraitResolutionChildren<T, V> {
    pub methodish_trait_attribute: Syntax<T, V>,
    pub methodish_trait_function_decl_header: Syntax<T, V>,
    pub methodish_trait_equal: Syntax<T, V>,
    pub methodish_trait_name: Syntax<T, V>,
    pub methodish_trait_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassishDeclarationChildren<T, V> {
    pub classish_attribute: Syntax<T, V>,
    pub classish_modifiers: Syntax<T, V>,
    pub classish_xhp: Syntax<T, V>,
    pub classish_keyword: Syntax<T, V>,
    pub classish_name: Syntax<T, V>,
    pub classish_type_parameters: Syntax<T, V>,
    pub classish_extends_keyword: Syntax<T, V>,
    pub classish_extends_list: Syntax<T, V>,
    pub classish_implements_keyword: Syntax<T, V>,
    pub classish_implements_list: Syntax<T, V>,
    pub classish_where_clause: Syntax<T, V>,
    pub classish_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassishBodyChildren<T, V> {
    pub classish_body_left_brace: Syntax<T, V>,
    pub classish_body_elements: Syntax<T, V>,
    pub classish_body_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TraitUsePrecedenceItemChildren<T, V> {
    pub trait_use_precedence_item_name: Syntax<T, V>,
    pub trait_use_precedence_item_keyword: Syntax<T, V>,
    pub trait_use_precedence_item_removed_names: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TraitUseAliasItemChildren<T, V> {
    pub trait_use_alias_item_aliasing_name: Syntax<T, V>,
    pub trait_use_alias_item_keyword: Syntax<T, V>,
    pub trait_use_alias_item_modifiers: Syntax<T, V>,
    pub trait_use_alias_item_aliased_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TraitUseConflictResolutionChildren<T, V> {
    pub trait_use_conflict_resolution_keyword: Syntax<T, V>,
    pub trait_use_conflict_resolution_names: Syntax<T, V>,
    pub trait_use_conflict_resolution_left_brace: Syntax<T, V>,
    pub trait_use_conflict_resolution_clauses: Syntax<T, V>,
    pub trait_use_conflict_resolution_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TraitUseChildren<T, V> {
    pub trait_use_keyword: Syntax<T, V>,
    pub trait_use_names: Syntax<T, V>,
    pub trait_use_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct RequireClauseChildren<T, V> {
    pub require_keyword: Syntax<T, V>,
    pub require_kind: Syntax<T, V>,
    pub require_name: Syntax<T, V>,
    pub require_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstDeclarationChildren<T, V> {
    pub const_modifiers: Syntax<T, V>,
    pub const_keyword: Syntax<T, V>,
    pub const_type_specifier: Syntax<T, V>,
    pub const_declarators: Syntax<T, V>,
    pub const_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstantDeclaratorChildren<T, V> {
    pub constant_declarator_name: Syntax<T, V>,
    pub constant_declarator_initializer: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstDeclarationChildren<T, V> {
    pub type_const_attribute_spec: Syntax<T, V>,
    pub type_const_modifiers: Syntax<T, V>,
    pub type_const_keyword: Syntax<T, V>,
    pub type_const_type_keyword: Syntax<T, V>,
    pub type_const_name: Syntax<T, V>,
    pub type_const_type_parameters: Syntax<T, V>,
    pub type_const_type_constraint: Syntax<T, V>,
    pub type_const_equal: Syntax<T, V>,
    pub type_const_type_specifier: Syntax<T, V>,
    pub type_const_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DecoratedExpressionChildren<T, V> {
    pub decorated_expression_decorator: Syntax<T, V>,
    pub decorated_expression_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ParameterDeclarationChildren<T, V> {
    pub parameter_attribute: Syntax<T, V>,
    pub parameter_visibility: Syntax<T, V>,
    pub parameter_call_convention: Syntax<T, V>,
    pub parameter_type: Syntax<T, V>,
    pub parameter_name: Syntax<T, V>,
    pub parameter_default_value: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VariadicParameterChildren<T, V> {
    pub variadic_parameter_call_convention: Syntax<T, V>,
    pub variadic_parameter_type: Syntax<T, V>,
    pub variadic_parameter_ellipsis: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct OldAttributeSpecificationChildren<T, V> {
    pub old_attribute_specification_left_double_angle: Syntax<T, V>,
    pub old_attribute_specification_attributes: Syntax<T, V>,
    pub old_attribute_specification_right_double_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributeSpecificationChildren<T, V> {
    pub attribute_specification_attributes: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributeChildren<T, V> {
    pub attribute_at: Syntax<T, V>,
    pub attribute_attribute_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct InclusionExpressionChildren<T, V> {
    pub inclusion_require: Syntax<T, V>,
    pub inclusion_filename: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct InclusionDirectiveChildren<T, V> {
    pub inclusion_expression: Syntax<T, V>,
    pub inclusion_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CompoundStatementChildren<T, V> {
    pub compound_left_brace: Syntax<T, V>,
    pub compound_statements: Syntax<T, V>,
    pub compound_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ExpressionStatementChildren<T, V> {
    pub expression_statement_expression: Syntax<T, V>,
    pub expression_statement_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MarkupSectionChildren<T, V> {
    pub markup_text: Syntax<T, V>,
    pub markup_suffix: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MarkupSuffixChildren<T, V> {
    pub markup_suffix_less_than_question: Syntax<T, V>,
    pub markup_suffix_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct UnsetStatementChildren<T, V> {
    pub unset_keyword: Syntax<T, V>,
    pub unset_left_paren: Syntax<T, V>,
    pub unset_variables: Syntax<T, V>,
    pub unset_right_paren: Syntax<T, V>,
    pub unset_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct UsingStatementBlockScopedChildren<T, V> {
    pub using_block_await_keyword: Syntax<T, V>,
    pub using_block_using_keyword: Syntax<T, V>,
    pub using_block_left_paren: Syntax<T, V>,
    pub using_block_expressions: Syntax<T, V>,
    pub using_block_right_paren: Syntax<T, V>,
    pub using_block_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct UsingStatementFunctionScopedChildren<T, V> {
    pub using_function_await_keyword: Syntax<T, V>,
    pub using_function_using_keyword: Syntax<T, V>,
    pub using_function_expression: Syntax<T, V>,
    pub using_function_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct WhileStatementChildren<T, V> {
    pub while_keyword: Syntax<T, V>,
    pub while_left_paren: Syntax<T, V>,
    pub while_condition: Syntax<T, V>,
    pub while_right_paren: Syntax<T, V>,
    pub while_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct IfStatementChildren<T, V> {
    pub if_keyword: Syntax<T, V>,
    pub if_left_paren: Syntax<T, V>,
    pub if_condition: Syntax<T, V>,
    pub if_right_paren: Syntax<T, V>,
    pub if_statement: Syntax<T, V>,
    pub if_elseif_clauses: Syntax<T, V>,
    pub if_else_clause: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ElseifClauseChildren<T, V> {
    pub elseif_keyword: Syntax<T, V>,
    pub elseif_left_paren: Syntax<T, V>,
    pub elseif_condition: Syntax<T, V>,
    pub elseif_right_paren: Syntax<T, V>,
    pub elseif_statement: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ElseClauseChildren<T, V> {
    pub else_keyword: Syntax<T, V>,
    pub else_statement: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TryStatementChildren<T, V> {
    pub try_keyword: Syntax<T, V>,
    pub try_compound_statement: Syntax<T, V>,
    pub try_catch_clauses: Syntax<T, V>,
    pub try_finally_clause: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CatchClauseChildren<T, V> {
    pub catch_keyword: Syntax<T, V>,
    pub catch_left_paren: Syntax<T, V>,
    pub catch_type: Syntax<T, V>,
    pub catch_variable: Syntax<T, V>,
    pub catch_right_paren: Syntax<T, V>,
    pub catch_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FinallyClauseChildren<T, V> {
    pub finally_keyword: Syntax<T, V>,
    pub finally_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DoStatementChildren<T, V> {
    pub do_keyword: Syntax<T, V>,
    pub do_body: Syntax<T, V>,
    pub do_while_keyword: Syntax<T, V>,
    pub do_left_paren: Syntax<T, V>,
    pub do_condition: Syntax<T, V>,
    pub do_right_paren: Syntax<T, V>,
    pub do_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ForStatementChildren<T, V> {
    pub for_keyword: Syntax<T, V>,
    pub for_left_paren: Syntax<T, V>,
    pub for_initializer: Syntax<T, V>,
    pub for_first_semicolon: Syntax<T, V>,
    pub for_control: Syntax<T, V>,
    pub for_second_semicolon: Syntax<T, V>,
    pub for_end_of_loop: Syntax<T, V>,
    pub for_right_paren: Syntax<T, V>,
    pub for_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ForeachStatementChildren<T, V> {
    pub foreach_keyword: Syntax<T, V>,
    pub foreach_left_paren: Syntax<T, V>,
    pub foreach_collection: Syntax<T, V>,
    pub foreach_await_keyword: Syntax<T, V>,
    pub foreach_as: Syntax<T, V>,
    pub foreach_key: Syntax<T, V>,
    pub foreach_arrow: Syntax<T, V>,
    pub foreach_value: Syntax<T, V>,
    pub foreach_right_paren: Syntax<T, V>,
    pub foreach_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchStatementChildren<T, V> {
    pub switch_keyword: Syntax<T, V>,
    pub switch_left_paren: Syntax<T, V>,
    pub switch_expression: Syntax<T, V>,
    pub switch_right_paren: Syntax<T, V>,
    pub switch_left_brace: Syntax<T, V>,
    pub switch_sections: Syntax<T, V>,
    pub switch_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchSectionChildren<T, V> {
    pub switch_section_labels: Syntax<T, V>,
    pub switch_section_statements: Syntax<T, V>,
    pub switch_section_fallthrough: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SwitchFallthroughChildren<T, V> {
    pub fallthrough_keyword: Syntax<T, V>,
    pub fallthrough_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CaseLabelChildren<T, V> {
    pub case_keyword: Syntax<T, V>,
    pub case_expression: Syntax<T, V>,
    pub case_colon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DefaultLabelChildren<T, V> {
    pub default_keyword: Syntax<T, V>,
    pub default_colon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ReturnStatementChildren<T, V> {
    pub return_keyword: Syntax<T, V>,
    pub return_expression: Syntax<T, V>,
    pub return_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct GotoLabelChildren<T, V> {
    pub goto_label_name: Syntax<T, V>,
    pub goto_label_colon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct GotoStatementChildren<T, V> {
    pub goto_statement_keyword: Syntax<T, V>,
    pub goto_statement_label_name: Syntax<T, V>,
    pub goto_statement_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ThrowStatementChildren<T, V> {
    pub throw_keyword: Syntax<T, V>,
    pub throw_expression: Syntax<T, V>,
    pub throw_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct BreakStatementChildren<T, V> {
    pub break_keyword: Syntax<T, V>,
    pub break_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ContinueStatementChildren<T, V> {
    pub continue_keyword: Syntax<T, V>,
    pub continue_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EchoStatementChildren<T, V> {
    pub echo_keyword: Syntax<T, V>,
    pub echo_expressions: Syntax<T, V>,
    pub echo_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ConcurrentStatementChildren<T, V> {
    pub concurrent_keyword: Syntax<T, V>,
    pub concurrent_statement: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SimpleInitializerChildren<T, V> {
    pub simple_initializer_equal: Syntax<T, V>,
    pub simple_initializer_value: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousClassChildren<T, V> {
    pub anonymous_class_class_keyword: Syntax<T, V>,
    pub anonymous_class_left_paren: Syntax<T, V>,
    pub anonymous_class_argument_list: Syntax<T, V>,
    pub anonymous_class_right_paren: Syntax<T, V>,
    pub anonymous_class_extends_keyword: Syntax<T, V>,
    pub anonymous_class_extends_list: Syntax<T, V>,
    pub anonymous_class_implements_keyword: Syntax<T, V>,
    pub anonymous_class_implements_list: Syntax<T, V>,
    pub anonymous_class_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousFunctionChildren<T, V> {
    pub anonymous_attribute_spec: Syntax<T, V>,
    pub anonymous_static_keyword: Syntax<T, V>,
    pub anonymous_async_keyword: Syntax<T, V>,
    pub anonymous_function_keyword: Syntax<T, V>,
    pub anonymous_left_paren: Syntax<T, V>,
    pub anonymous_parameters: Syntax<T, V>,
    pub anonymous_right_paren: Syntax<T, V>,
    pub anonymous_colon: Syntax<T, V>,
    pub anonymous_type: Syntax<T, V>,
    pub anonymous_use: Syntax<T, V>,
    pub anonymous_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AnonymousFunctionUseClauseChildren<T, V> {
    pub anonymous_use_keyword: Syntax<T, V>,
    pub anonymous_use_left_paren: Syntax<T, V>,
    pub anonymous_use_variables: Syntax<T, V>,
    pub anonymous_use_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct LambdaExpressionChildren<T, V> {
    pub lambda_attribute_spec: Syntax<T, V>,
    pub lambda_async: Syntax<T, V>,
    pub lambda_signature: Syntax<T, V>,
    pub lambda_arrow: Syntax<T, V>,
    pub lambda_body: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct LambdaSignatureChildren<T, V> {
    pub lambda_left_paren: Syntax<T, V>,
    pub lambda_parameters: Syntax<T, V>,
    pub lambda_right_paren: Syntax<T, V>,
    pub lambda_colon: Syntax<T, V>,
    pub lambda_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CastExpressionChildren<T, V> {
    pub cast_left_paren: Syntax<T, V>,
    pub cast_type: Syntax<T, V>,
    pub cast_right_paren: Syntax<T, V>,
    pub cast_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ScopeResolutionExpressionChildren<T, V> {
    pub scope_resolution_qualifier: Syntax<T, V>,
    pub scope_resolution_operator: Syntax<T, V>,
    pub scope_resolution_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MemberSelectionExpressionChildren<T, V> {
    pub member_object: Syntax<T, V>,
    pub member_operator: Syntax<T, V>,
    pub member_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SafeMemberSelectionExpressionChildren<T, V> {
    pub safe_member_object: Syntax<T, V>,
    pub safe_member_operator: Syntax<T, V>,
    pub safe_member_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedMemberSelectionExpressionChildren<T, V> {
    pub embedded_member_object: Syntax<T, V>,
    pub embedded_member_operator: Syntax<T, V>,
    pub embedded_member_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct YieldExpressionChildren<T, V> {
    pub yield_keyword: Syntax<T, V>,
    pub yield_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PrefixUnaryExpressionChildren<T, V> {
    pub prefix_unary_operator: Syntax<T, V>,
    pub prefix_unary_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PostfixUnaryExpressionChildren<T, V> {
    pub postfix_unary_operand: Syntax<T, V>,
    pub postfix_unary_operator: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct BinaryExpressionChildren<T, V> {
    pub binary_left_operand: Syntax<T, V>,
    pub binary_operator: Syntax<T, V>,
    pub binary_right_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct IsExpressionChildren<T, V> {
    pub is_left_operand: Syntax<T, V>,
    pub is_operator: Syntax<T, V>,
    pub is_right_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AsExpressionChildren<T, V> {
    pub as_left_operand: Syntax<T, V>,
    pub as_operator: Syntax<T, V>,
    pub as_right_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NullableAsExpressionChildren<T, V> {
    pub nullable_as_left_operand: Syntax<T, V>,
    pub nullable_as_operator: Syntax<T, V>,
    pub nullable_as_right_operand: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ConditionalExpressionChildren<T, V> {
    pub conditional_test: Syntax<T, V>,
    pub conditional_question: Syntax<T, V>,
    pub conditional_consequence: Syntax<T, V>,
    pub conditional_colon: Syntax<T, V>,
    pub conditional_alternative: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EvalExpressionChildren<T, V> {
    pub eval_keyword: Syntax<T, V>,
    pub eval_left_paren: Syntax<T, V>,
    pub eval_argument: Syntax<T, V>,
    pub eval_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DefineExpressionChildren<T, V> {
    pub define_keyword: Syntax<T, V>,
    pub define_left_paren: Syntax<T, V>,
    pub define_argument_list: Syntax<T, V>,
    pub define_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct IssetExpressionChildren<T, V> {
    pub isset_keyword: Syntax<T, V>,
    pub isset_left_paren: Syntax<T, V>,
    pub isset_argument_list: Syntax<T, V>,
    pub isset_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionCallExpressionChildren<T, V> {
    pub function_call_receiver: Syntax<T, V>,
    pub function_call_type_args: Syntax<T, V>,
    pub function_call_left_paren: Syntax<T, V>,
    pub function_call_argument_list: Syntax<T, V>,
    pub function_call_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FunctionPointerExpressionChildren<T, V> {
    pub function_pointer_receiver: Syntax<T, V>,
    pub function_pointer_type_args: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ParenthesizedExpressionChildren<T, V> {
    pub parenthesized_expression_left_paren: Syntax<T, V>,
    pub parenthesized_expression_expression: Syntax<T, V>,
    pub parenthesized_expression_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct BracedExpressionChildren<T, V> {
    pub braced_expression_left_brace: Syntax<T, V>,
    pub braced_expression_expression: Syntax<T, V>,
    pub braced_expression_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedBracedExpressionChildren<T, V> {
    pub embedded_braced_expression_left_brace: Syntax<T, V>,
    pub embedded_braced_expression_expression: Syntax<T, V>,
    pub embedded_braced_expression_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ListExpressionChildren<T, V> {
    pub list_keyword: Syntax<T, V>,
    pub list_left_paren: Syntax<T, V>,
    pub list_members: Syntax<T, V>,
    pub list_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct CollectionLiteralExpressionChildren<T, V> {
    pub collection_literal_name: Syntax<T, V>,
    pub collection_literal_left_brace: Syntax<T, V>,
    pub collection_literal_initializers: Syntax<T, V>,
    pub collection_literal_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ObjectCreationExpressionChildren<T, V> {
    pub object_creation_new_keyword: Syntax<T, V>,
    pub object_creation_object: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ConstructorCallChildren<T, V> {
    pub constructor_call_type: Syntax<T, V>,
    pub constructor_call_left_paren: Syntax<T, V>,
    pub constructor_call_argument_list: Syntax<T, V>,
    pub constructor_call_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct RecordCreationExpressionChildren<T, V> {
    pub record_creation_type: Syntax<T, V>,
    pub record_creation_left_bracket: Syntax<T, V>,
    pub record_creation_members: Syntax<T, V>,
    pub record_creation_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DarrayIntrinsicExpressionChildren<T, V> {
    pub darray_intrinsic_keyword: Syntax<T, V>,
    pub darray_intrinsic_explicit_type: Syntax<T, V>,
    pub darray_intrinsic_left_bracket: Syntax<T, V>,
    pub darray_intrinsic_members: Syntax<T, V>,
    pub darray_intrinsic_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DictionaryIntrinsicExpressionChildren<T, V> {
    pub dictionary_intrinsic_keyword: Syntax<T, V>,
    pub dictionary_intrinsic_explicit_type: Syntax<T, V>,
    pub dictionary_intrinsic_left_bracket: Syntax<T, V>,
    pub dictionary_intrinsic_members: Syntax<T, V>,
    pub dictionary_intrinsic_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct KeysetIntrinsicExpressionChildren<T, V> {
    pub keyset_intrinsic_keyword: Syntax<T, V>,
    pub keyset_intrinsic_explicit_type: Syntax<T, V>,
    pub keyset_intrinsic_left_bracket: Syntax<T, V>,
    pub keyset_intrinsic_members: Syntax<T, V>,
    pub keyset_intrinsic_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VarrayIntrinsicExpressionChildren<T, V> {
    pub varray_intrinsic_keyword: Syntax<T, V>,
    pub varray_intrinsic_explicit_type: Syntax<T, V>,
    pub varray_intrinsic_left_bracket: Syntax<T, V>,
    pub varray_intrinsic_members: Syntax<T, V>,
    pub varray_intrinsic_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VectorIntrinsicExpressionChildren<T, V> {
    pub vector_intrinsic_keyword: Syntax<T, V>,
    pub vector_intrinsic_explicit_type: Syntax<T, V>,
    pub vector_intrinsic_left_bracket: Syntax<T, V>,
    pub vector_intrinsic_members: Syntax<T, V>,
    pub vector_intrinsic_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ElementInitializerChildren<T, V> {
    pub element_key: Syntax<T, V>,
    pub element_arrow: Syntax<T, V>,
    pub element_value: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SubscriptExpressionChildren<T, V> {
    pub subscript_receiver: Syntax<T, V>,
    pub subscript_left_bracket: Syntax<T, V>,
    pub subscript_index: Syntax<T, V>,
    pub subscript_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct EmbeddedSubscriptExpressionChildren<T, V> {
    pub embedded_subscript_receiver: Syntax<T, V>,
    pub embedded_subscript_left_bracket: Syntax<T, V>,
    pub embedded_subscript_index: Syntax<T, V>,
    pub embedded_subscript_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AwaitableCreationExpressionChildren<T, V> {
    pub awaitable_attribute_spec: Syntax<T, V>,
    pub awaitable_async: Syntax<T, V>,
    pub awaitable_compound_statement: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPChildrenDeclarationChildren<T, V> {
    pub xhp_children_keyword: Syntax<T, V>,
    pub xhp_children_expression: Syntax<T, V>,
    pub xhp_children_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPChildrenParenthesizedListChildren<T, V> {
    pub xhp_children_list_left_paren: Syntax<T, V>,
    pub xhp_children_list_xhp_children: Syntax<T, V>,
    pub xhp_children_list_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPCategoryDeclarationChildren<T, V> {
    pub xhp_category_keyword: Syntax<T, V>,
    pub xhp_category_categories: Syntax<T, V>,
    pub xhp_category_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPEnumTypeChildren<T, V> {
    pub xhp_enum_keyword: Syntax<T, V>,
    pub xhp_enum_left_brace: Syntax<T, V>,
    pub xhp_enum_values: Syntax<T, V>,
    pub xhp_enum_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPLateinitChildren<T, V> {
    pub xhp_lateinit_at: Syntax<T, V>,
    pub xhp_lateinit_keyword: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPRequiredChildren<T, V> {
    pub xhp_required_at: Syntax<T, V>,
    pub xhp_required_keyword: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPClassAttributeDeclarationChildren<T, V> {
    pub xhp_attribute_keyword: Syntax<T, V>,
    pub xhp_attribute_attributes: Syntax<T, V>,
    pub xhp_attribute_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPClassAttributeChildren<T, V> {
    pub xhp_attribute_decl_type: Syntax<T, V>,
    pub xhp_attribute_decl_name: Syntax<T, V>,
    pub xhp_attribute_decl_initializer: Syntax<T, V>,
    pub xhp_attribute_decl_required: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSimpleClassAttributeChildren<T, V> {
    pub xhp_simple_class_attribute_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSimpleAttributeChildren<T, V> {
    pub xhp_simple_attribute_name: Syntax<T, V>,
    pub xhp_simple_attribute_equal: Syntax<T, V>,
    pub xhp_simple_attribute_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPSpreadAttributeChildren<T, V> {
    pub xhp_spread_attribute_left_brace: Syntax<T, V>,
    pub xhp_spread_attribute_spread_operator: Syntax<T, V>,
    pub xhp_spread_attribute_expression: Syntax<T, V>,
    pub xhp_spread_attribute_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPOpenChildren<T, V> {
    pub xhp_open_left_angle: Syntax<T, V>,
    pub xhp_open_name: Syntax<T, V>,
    pub xhp_open_attributes: Syntax<T, V>,
    pub xhp_open_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPExpressionChildren<T, V> {
    pub xhp_open: Syntax<T, V>,
    pub xhp_body: Syntax<T, V>,
    pub xhp_close: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct XHPCloseChildren<T, V> {
    pub xhp_close_left_angle: Syntax<T, V>,
    pub xhp_close_name: Syntax<T, V>,
    pub xhp_close_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstantChildren<T, V> {
    pub type_constant_left_type: Syntax<T, V>,
    pub type_constant_separator: Syntax<T, V>,
    pub type_constant_right_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PUAccessChildren<T, V> {
    pub pu_access_left_type: Syntax<T, V>,
    pub pu_access_separator: Syntax<T, V>,
    pub pu_access_right_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VectorTypeSpecifierChildren<T, V> {
    pub vector_type_keyword: Syntax<T, V>,
    pub vector_type_left_angle: Syntax<T, V>,
    pub vector_type_type: Syntax<T, V>,
    pub vector_type_trailing_comma: Syntax<T, V>,
    pub vector_type_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct KeysetTypeSpecifierChildren<T, V> {
    pub keyset_type_keyword: Syntax<T, V>,
    pub keyset_type_left_angle: Syntax<T, V>,
    pub keyset_type_type: Syntax<T, V>,
    pub keyset_type_trailing_comma: Syntax<T, V>,
    pub keyset_type_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleTypeExplicitSpecifierChildren<T, V> {
    pub tuple_type_keyword: Syntax<T, V>,
    pub tuple_type_left_angle: Syntax<T, V>,
    pub tuple_type_types: Syntax<T, V>,
    pub tuple_type_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VarrayTypeSpecifierChildren<T, V> {
    pub varray_keyword: Syntax<T, V>,
    pub varray_left_angle: Syntax<T, V>,
    pub varray_type: Syntax<T, V>,
    pub varray_trailing_comma: Syntax<T, V>,
    pub varray_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct VectorArrayTypeSpecifierChildren<T, V> {
    pub vector_array_keyword: Syntax<T, V>,
    pub vector_array_left_angle: Syntax<T, V>,
    pub vector_array_type: Syntax<T, V>,
    pub vector_array_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeParameterChildren<T, V> {
    pub type_attribute_spec: Syntax<T, V>,
    pub type_reified: Syntax<T, V>,
    pub type_variance: Syntax<T, V>,
    pub type_name: Syntax<T, V>,
    pub type_param_params: Syntax<T, V>,
    pub type_constraints: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeConstraintChildren<T, V> {
    pub constraint_keyword: Syntax<T, V>,
    pub constraint_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DarrayTypeSpecifierChildren<T, V> {
    pub darray_keyword: Syntax<T, V>,
    pub darray_left_angle: Syntax<T, V>,
    pub darray_key: Syntax<T, V>,
    pub darray_comma: Syntax<T, V>,
    pub darray_value: Syntax<T, V>,
    pub darray_trailing_comma: Syntax<T, V>,
    pub darray_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MapArrayTypeSpecifierChildren<T, V> {
    pub map_array_keyword: Syntax<T, V>,
    pub map_array_left_angle: Syntax<T, V>,
    pub map_array_key: Syntax<T, V>,
    pub map_array_comma: Syntax<T, V>,
    pub map_array_value: Syntax<T, V>,
    pub map_array_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DictionaryTypeSpecifierChildren<T, V> {
    pub dictionary_type_keyword: Syntax<T, V>,
    pub dictionary_type_left_angle: Syntax<T, V>,
    pub dictionary_type_members: Syntax<T, V>,
    pub dictionary_type_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ClosureTypeSpecifierChildren<T, V> {
    pub closure_outer_left_paren: Syntax<T, V>,
    pub closure_function_keyword: Syntax<T, V>,
    pub closure_inner_left_paren: Syntax<T, V>,
    pub closure_parameter_list: Syntax<T, V>,
    pub closure_inner_right_paren: Syntax<T, V>,
    pub closure_colon: Syntax<T, V>,
    pub closure_return_type: Syntax<T, V>,
    pub closure_outer_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ClosureParameterTypeSpecifierChildren<T, V> {
    pub closure_parameter_call_convention: Syntax<T, V>,
    pub closure_parameter_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ClassnameTypeSpecifierChildren<T, V> {
    pub classname_keyword: Syntax<T, V>,
    pub classname_left_angle: Syntax<T, V>,
    pub classname_type: Syntax<T, V>,
    pub classname_trailing_comma: Syntax<T, V>,
    pub classname_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FieldSpecifierChildren<T, V> {
    pub field_question: Syntax<T, V>,
    pub field_name: Syntax<T, V>,
    pub field_arrow: Syntax<T, V>,
    pub field_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct FieldInitializerChildren<T, V> {
    pub field_initializer_name: Syntax<T, V>,
    pub field_initializer_arrow: Syntax<T, V>,
    pub field_initializer_value: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ShapeTypeSpecifierChildren<T, V> {
    pub shape_type_keyword: Syntax<T, V>,
    pub shape_type_left_paren: Syntax<T, V>,
    pub shape_type_fields: Syntax<T, V>,
    pub shape_type_ellipsis: Syntax<T, V>,
    pub shape_type_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ShapeExpressionChildren<T, V> {
    pub shape_expression_keyword: Syntax<T, V>,
    pub shape_expression_left_paren: Syntax<T, V>,
    pub shape_expression_fields: Syntax<T, V>,
    pub shape_expression_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleExpressionChildren<T, V> {
    pub tuple_expression_keyword: Syntax<T, V>,
    pub tuple_expression_left_paren: Syntax<T, V>,
    pub tuple_expression_items: Syntax<T, V>,
    pub tuple_expression_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct GenericTypeSpecifierChildren<T, V> {
    pub generic_class_type: Syntax<T, V>,
    pub generic_argument_list: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct NullableTypeSpecifierChildren<T, V> {
    pub nullable_question: Syntax<T, V>,
    pub nullable_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct LikeTypeSpecifierChildren<T, V> {
    pub like_tilde: Syntax<T, V>,
    pub like_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct SoftTypeSpecifierChildren<T, V> {
    pub soft_at: Syntax<T, V>,
    pub soft_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AttributizedSpecifierChildren<T, V> {
    pub attributized_specifier_attribute_spec: Syntax<T, V>,
    pub attributized_specifier_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ReifiedTypeArgumentChildren<T, V> {
    pub reified_type_argument_reified: Syntax<T, V>,
    pub reified_type_argument_type: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeArgumentsChildren<T, V> {
    pub type_arguments_left_angle: Syntax<T, V>,
    pub type_arguments_types: Syntax<T, V>,
    pub type_arguments_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TypeParametersChildren<T, V> {
    pub type_parameters_left_angle: Syntax<T, V>,
    pub type_parameters_parameters: Syntax<T, V>,
    pub type_parameters_right_angle: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct TupleTypeSpecifierChildren<T, V> {
    pub tuple_left_paren: Syntax<T, V>,
    pub tuple_types: Syntax<T, V>,
    pub tuple_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct UnionTypeSpecifierChildren<T, V> {
    pub union_left_paren: Syntax<T, V>,
    pub union_types: Syntax<T, V>,
    pub union_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct IntersectionTypeSpecifierChildren<T, V> {
    pub intersection_left_paren: Syntax<T, V>,
    pub intersection_types: Syntax<T, V>,
    pub intersection_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ErrorSyntaxChildren<T, V> {
    pub error_error: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ListItemChildren<T, V> {
    pub list_item: Syntax<T, V>,
    pub list_separator: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketAtomExpressionChildren<T, V> {
    pub pocket_atom_glyph: Syntax<T, V>,
    pub pocket_atom_expression: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketIdentifierExpressionChildren<T, V> {
    pub pocket_identifier_qualifier: Syntax<T, V>,
    pub pocket_identifier_pu_operator: Syntax<T, V>,
    pub pocket_identifier_field: Syntax<T, V>,
    pub pocket_identifier_operator: Syntax<T, V>,
    pub pocket_identifier_name: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketAtomMappingDeclarationChildren<T, V> {
    pub pocket_atom_mapping_glyph: Syntax<T, V>,
    pub pocket_atom_mapping_name: Syntax<T, V>,
    pub pocket_atom_mapping_left_paren: Syntax<T, V>,
    pub pocket_atom_mapping_mappings: Syntax<T, V>,
    pub pocket_atom_mapping_right_paren: Syntax<T, V>,
    pub pocket_atom_mapping_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketEnumDeclarationChildren<T, V> {
    pub pocket_enum_attributes: Syntax<T, V>,
    pub pocket_enum_modifiers: Syntax<T, V>,
    pub pocket_enum_enum: Syntax<T, V>,
    pub pocket_enum_name: Syntax<T, V>,
    pub pocket_enum_left_brace: Syntax<T, V>,
    pub pocket_enum_fields: Syntax<T, V>,
    pub pocket_enum_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketFieldTypeExprDeclarationChildren<T, V> {
    pub pocket_field_type_expr_case: Syntax<T, V>,
    pub pocket_field_type_expr_type: Syntax<T, V>,
    pub pocket_field_type_expr_name: Syntax<T, V>,
    pub pocket_field_type_expr_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketFieldTypeDeclarationChildren<T, V> {
    pub pocket_field_type_case: Syntax<T, V>,
    pub pocket_field_type_type: Syntax<T, V>,
    pub pocket_field_type_type_parameter: Syntax<T, V>,
    pub pocket_field_type_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketMappingIdDeclarationChildren<T, V> {
    pub pocket_mapping_id_name: Syntax<T, V>,
    pub pocket_mapping_id_initializer: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct PocketMappingTypeDeclarationChildren<T, V> {
    pub pocket_mapping_type_keyword: Syntax<T, V>,
    pub pocket_mapping_type_name: Syntax<T, V>,
    pub pocket_mapping_type_equal: Syntax<T, V>,
    pub pocket_mapping_type_type: Syntax<T, V>,
}


#[derive(Debug, Clone)]
pub enum SyntaxVariant<T, V> {
    Token(Box<T>),
    Missing,
    SyntaxList(Vec<Syntax<T, V>>),
    EndOfFile(Box<EndOfFileChildren<T, V>>),
    Script(Box<ScriptChildren<T, V>>),
    QualifiedName(Box<QualifiedNameChildren<T, V>>),
    SimpleTypeSpecifier(Box<SimpleTypeSpecifierChildren<T, V>>),
    LiteralExpression(Box<LiteralExpressionChildren<T, V>>),
    PrefixedStringExpression(Box<PrefixedStringExpressionChildren<T, V>>),
    PrefixedCodeExpression(Box<PrefixedCodeExpressionChildren<T, V>>),
    VariableExpression(Box<VariableExpressionChildren<T, V>>),
    PipeVariableExpression(Box<PipeVariableExpressionChildren<T, V>>),
    FileAttributeSpecification(Box<FileAttributeSpecificationChildren<T, V>>),
    EnumDeclaration(Box<EnumDeclarationChildren<T, V>>),
    Enumerator(Box<EnumeratorChildren<T, V>>),
    RecordDeclaration(Box<RecordDeclarationChildren<T, V>>),
    RecordField(Box<RecordFieldChildren<T, V>>),
    AliasDeclaration(Box<AliasDeclarationChildren<T, V>>),
    PropertyDeclaration(Box<PropertyDeclarationChildren<T, V>>),
    PropertyDeclarator(Box<PropertyDeclaratorChildren<T, V>>),
    NamespaceDeclaration(Box<NamespaceDeclarationChildren<T, V>>),
    NamespaceDeclarationHeader(Box<NamespaceDeclarationHeaderChildren<T, V>>),
    NamespaceBody(Box<NamespaceBodyChildren<T, V>>),
    NamespaceEmptyBody(Box<NamespaceEmptyBodyChildren<T, V>>),
    NamespaceUseDeclaration(Box<NamespaceUseDeclarationChildren<T, V>>),
    NamespaceGroupUseDeclaration(Box<NamespaceGroupUseDeclarationChildren<T, V>>),
    NamespaceUseClause(Box<NamespaceUseClauseChildren<T, V>>),
    FunctionDeclaration(Box<FunctionDeclarationChildren<T, V>>),
    FunctionDeclarationHeader(Box<FunctionDeclarationHeaderChildren<T, V>>),
    CapabilityProvisional(Box<CapabilityProvisionalChildren<T, V>>),
    WhereClause(Box<WhereClauseChildren<T, V>>),
    WhereConstraint(Box<WhereConstraintChildren<T, V>>),
    MethodishDeclaration(Box<MethodishDeclarationChildren<T, V>>),
    MethodishTraitResolution(Box<MethodishTraitResolutionChildren<T, V>>),
    ClassishDeclaration(Box<ClassishDeclarationChildren<T, V>>),
    ClassishBody(Box<ClassishBodyChildren<T, V>>),
    TraitUsePrecedenceItem(Box<TraitUsePrecedenceItemChildren<T, V>>),
    TraitUseAliasItem(Box<TraitUseAliasItemChildren<T, V>>),
    TraitUseConflictResolution(Box<TraitUseConflictResolutionChildren<T, V>>),
    TraitUse(Box<TraitUseChildren<T, V>>),
    RequireClause(Box<RequireClauseChildren<T, V>>),
    ConstDeclaration(Box<ConstDeclarationChildren<T, V>>),
    ConstantDeclarator(Box<ConstantDeclaratorChildren<T, V>>),
    TypeConstDeclaration(Box<TypeConstDeclarationChildren<T, V>>),
    DecoratedExpression(Box<DecoratedExpressionChildren<T, V>>),
    ParameterDeclaration(Box<ParameterDeclarationChildren<T, V>>),
    VariadicParameter(Box<VariadicParameterChildren<T, V>>),
    OldAttributeSpecification(Box<OldAttributeSpecificationChildren<T, V>>),
    AttributeSpecification(Box<AttributeSpecificationChildren<T, V>>),
    Attribute(Box<AttributeChildren<T, V>>),
    InclusionExpression(Box<InclusionExpressionChildren<T, V>>),
    InclusionDirective(Box<InclusionDirectiveChildren<T, V>>),
    CompoundStatement(Box<CompoundStatementChildren<T, V>>),
    ExpressionStatement(Box<ExpressionStatementChildren<T, V>>),
    MarkupSection(Box<MarkupSectionChildren<T, V>>),
    MarkupSuffix(Box<MarkupSuffixChildren<T, V>>),
    UnsetStatement(Box<UnsetStatementChildren<T, V>>),
    UsingStatementBlockScoped(Box<UsingStatementBlockScopedChildren<T, V>>),
    UsingStatementFunctionScoped(Box<UsingStatementFunctionScopedChildren<T, V>>),
    WhileStatement(Box<WhileStatementChildren<T, V>>),
    IfStatement(Box<IfStatementChildren<T, V>>),
    ElseifClause(Box<ElseifClauseChildren<T, V>>),
    ElseClause(Box<ElseClauseChildren<T, V>>),
    TryStatement(Box<TryStatementChildren<T, V>>),
    CatchClause(Box<CatchClauseChildren<T, V>>),
    FinallyClause(Box<FinallyClauseChildren<T, V>>),
    DoStatement(Box<DoStatementChildren<T, V>>),
    ForStatement(Box<ForStatementChildren<T, V>>),
    ForeachStatement(Box<ForeachStatementChildren<T, V>>),
    SwitchStatement(Box<SwitchStatementChildren<T, V>>),
    SwitchSection(Box<SwitchSectionChildren<T, V>>),
    SwitchFallthrough(Box<SwitchFallthroughChildren<T, V>>),
    CaseLabel(Box<CaseLabelChildren<T, V>>),
    DefaultLabel(Box<DefaultLabelChildren<T, V>>),
    ReturnStatement(Box<ReturnStatementChildren<T, V>>),
    GotoLabel(Box<GotoLabelChildren<T, V>>),
    GotoStatement(Box<GotoStatementChildren<T, V>>),
    ThrowStatement(Box<ThrowStatementChildren<T, V>>),
    BreakStatement(Box<BreakStatementChildren<T, V>>),
    ContinueStatement(Box<ContinueStatementChildren<T, V>>),
    EchoStatement(Box<EchoStatementChildren<T, V>>),
    ConcurrentStatement(Box<ConcurrentStatementChildren<T, V>>),
    SimpleInitializer(Box<SimpleInitializerChildren<T, V>>),
    AnonymousClass(Box<AnonymousClassChildren<T, V>>),
    AnonymousFunction(Box<AnonymousFunctionChildren<T, V>>),
    AnonymousFunctionUseClause(Box<AnonymousFunctionUseClauseChildren<T, V>>),
    LambdaExpression(Box<LambdaExpressionChildren<T, V>>),
    LambdaSignature(Box<LambdaSignatureChildren<T, V>>),
    CastExpression(Box<CastExpressionChildren<T, V>>),
    ScopeResolutionExpression(Box<ScopeResolutionExpressionChildren<T, V>>),
    MemberSelectionExpression(Box<MemberSelectionExpressionChildren<T, V>>),
    SafeMemberSelectionExpression(Box<SafeMemberSelectionExpressionChildren<T, V>>),
    EmbeddedMemberSelectionExpression(Box<EmbeddedMemberSelectionExpressionChildren<T, V>>),
    YieldExpression(Box<YieldExpressionChildren<T, V>>),
    PrefixUnaryExpression(Box<PrefixUnaryExpressionChildren<T, V>>),
    PostfixUnaryExpression(Box<PostfixUnaryExpressionChildren<T, V>>),
    BinaryExpression(Box<BinaryExpressionChildren<T, V>>),
    IsExpression(Box<IsExpressionChildren<T, V>>),
    AsExpression(Box<AsExpressionChildren<T, V>>),
    NullableAsExpression(Box<NullableAsExpressionChildren<T, V>>),
    ConditionalExpression(Box<ConditionalExpressionChildren<T, V>>),
    EvalExpression(Box<EvalExpressionChildren<T, V>>),
    DefineExpression(Box<DefineExpressionChildren<T, V>>),
    IssetExpression(Box<IssetExpressionChildren<T, V>>),
    FunctionCallExpression(Box<FunctionCallExpressionChildren<T, V>>),
    FunctionPointerExpression(Box<FunctionPointerExpressionChildren<T, V>>),
    ParenthesizedExpression(Box<ParenthesizedExpressionChildren<T, V>>),
    BracedExpression(Box<BracedExpressionChildren<T, V>>),
    EmbeddedBracedExpression(Box<EmbeddedBracedExpressionChildren<T, V>>),
    ListExpression(Box<ListExpressionChildren<T, V>>),
    CollectionLiteralExpression(Box<CollectionLiteralExpressionChildren<T, V>>),
    ObjectCreationExpression(Box<ObjectCreationExpressionChildren<T, V>>),
    ConstructorCall(Box<ConstructorCallChildren<T, V>>),
    RecordCreationExpression(Box<RecordCreationExpressionChildren<T, V>>),
    DarrayIntrinsicExpression(Box<DarrayIntrinsicExpressionChildren<T, V>>),
    DictionaryIntrinsicExpression(Box<DictionaryIntrinsicExpressionChildren<T, V>>),
    KeysetIntrinsicExpression(Box<KeysetIntrinsicExpressionChildren<T, V>>),
    VarrayIntrinsicExpression(Box<VarrayIntrinsicExpressionChildren<T, V>>),
    VectorIntrinsicExpression(Box<VectorIntrinsicExpressionChildren<T, V>>),
    ElementInitializer(Box<ElementInitializerChildren<T, V>>),
    SubscriptExpression(Box<SubscriptExpressionChildren<T, V>>),
    EmbeddedSubscriptExpression(Box<EmbeddedSubscriptExpressionChildren<T, V>>),
    AwaitableCreationExpression(Box<AwaitableCreationExpressionChildren<T, V>>),
    XHPChildrenDeclaration(Box<XHPChildrenDeclarationChildren<T, V>>),
    XHPChildrenParenthesizedList(Box<XHPChildrenParenthesizedListChildren<T, V>>),
    XHPCategoryDeclaration(Box<XHPCategoryDeclarationChildren<T, V>>),
    XHPEnumType(Box<XHPEnumTypeChildren<T, V>>),
    XHPLateinit(Box<XHPLateinitChildren<T, V>>),
    XHPRequired(Box<XHPRequiredChildren<T, V>>),
    XHPClassAttributeDeclaration(Box<XHPClassAttributeDeclarationChildren<T, V>>),
    XHPClassAttribute(Box<XHPClassAttributeChildren<T, V>>),
    XHPSimpleClassAttribute(Box<XHPSimpleClassAttributeChildren<T, V>>),
    XHPSimpleAttribute(Box<XHPSimpleAttributeChildren<T, V>>),
    XHPSpreadAttribute(Box<XHPSpreadAttributeChildren<T, V>>),
    XHPOpen(Box<XHPOpenChildren<T, V>>),
    XHPExpression(Box<XHPExpressionChildren<T, V>>),
    XHPClose(Box<XHPCloseChildren<T, V>>),
    TypeConstant(Box<TypeConstantChildren<T, V>>),
    PUAccess(Box<PUAccessChildren<T, V>>),
    VectorTypeSpecifier(Box<VectorTypeSpecifierChildren<T, V>>),
    KeysetTypeSpecifier(Box<KeysetTypeSpecifierChildren<T, V>>),
    TupleTypeExplicitSpecifier(Box<TupleTypeExplicitSpecifierChildren<T, V>>),
    VarrayTypeSpecifier(Box<VarrayTypeSpecifierChildren<T, V>>),
    VectorArrayTypeSpecifier(Box<VectorArrayTypeSpecifierChildren<T, V>>),
    TypeParameter(Box<TypeParameterChildren<T, V>>),
    TypeConstraint(Box<TypeConstraintChildren<T, V>>),
    DarrayTypeSpecifier(Box<DarrayTypeSpecifierChildren<T, V>>),
    MapArrayTypeSpecifier(Box<MapArrayTypeSpecifierChildren<T, V>>),
    DictionaryTypeSpecifier(Box<DictionaryTypeSpecifierChildren<T, V>>),
    ClosureTypeSpecifier(Box<ClosureTypeSpecifierChildren<T, V>>),
    ClosureParameterTypeSpecifier(Box<ClosureParameterTypeSpecifierChildren<T, V>>),
    ClassnameTypeSpecifier(Box<ClassnameTypeSpecifierChildren<T, V>>),
    FieldSpecifier(Box<FieldSpecifierChildren<T, V>>),
    FieldInitializer(Box<FieldInitializerChildren<T, V>>),
    ShapeTypeSpecifier(Box<ShapeTypeSpecifierChildren<T, V>>),
    ShapeExpression(Box<ShapeExpressionChildren<T, V>>),
    TupleExpression(Box<TupleExpressionChildren<T, V>>),
    GenericTypeSpecifier(Box<GenericTypeSpecifierChildren<T, V>>),
    NullableTypeSpecifier(Box<NullableTypeSpecifierChildren<T, V>>),
    LikeTypeSpecifier(Box<LikeTypeSpecifierChildren<T, V>>),
    SoftTypeSpecifier(Box<SoftTypeSpecifierChildren<T, V>>),
    AttributizedSpecifier(Box<AttributizedSpecifierChildren<T, V>>),
    ReifiedTypeArgument(Box<ReifiedTypeArgumentChildren<T, V>>),
    TypeArguments(Box<TypeArgumentsChildren<T, V>>),
    TypeParameters(Box<TypeParametersChildren<T, V>>),
    TupleTypeSpecifier(Box<TupleTypeSpecifierChildren<T, V>>),
    UnionTypeSpecifier(Box<UnionTypeSpecifierChildren<T, V>>),
    IntersectionTypeSpecifier(Box<IntersectionTypeSpecifierChildren<T, V>>),
    ErrorSyntax(Box<ErrorSyntaxChildren<T, V>>),
    ListItem(Box<ListItemChildren<T, V>>),
    PocketAtomExpression(Box<PocketAtomExpressionChildren<T, V>>),
    PocketIdentifierExpression(Box<PocketIdentifierExpressionChildren<T, V>>),
    PocketAtomMappingDeclaration(Box<PocketAtomMappingDeclarationChildren<T, V>>),
    PocketEnumDeclaration(Box<PocketEnumDeclarationChildren<T, V>>),
    PocketFieldTypeExprDeclaration(Box<PocketFieldTypeExprDeclarationChildren<T, V>>),
    PocketFieldTypeDeclaration(Box<PocketFieldTypeDeclarationChildren<T, V>>),
    PocketMappingIdDeclaration(Box<PocketMappingIdDeclarationChildren<T, V>>),
    PocketMappingTypeDeclaration(Box<PocketMappingTypeDeclarationChildren<T, V>>),
}

impl<'a, T, V> SyntaxChildrenIterator<'a, T, V> {
    pub fn next_impl(&mut self, direction : bool) -> Option<&'a Syntax<T, V>> {
        use SyntaxVariant::*;
        let get_index = |len| {
            let back_index_plus_1 = len - self.index_back;
            if back_index_plus_1 <= self.index {
                return None
            }
            if direction {
                Some (self.index)
            } else {
                Some (back_index_plus_1 - 1)
            }
        };
        let res = match &self.syntax {
            Missing => None,
            Token (_) => None,
            SyntaxList(elems) => {
                get_index(elems.len()).and_then(|x| elems.get(x))
            },
            EndOfFile(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.end_of_file_token),
                        _ => None,
                    }
                })
            },
            Script(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.script_declarations),
                        _ => None,
                    }
                })
            },
            QualifiedName(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.qualified_name_parts),
                        _ => None,
                    }
                })
            },
            SimpleTypeSpecifier(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.simple_type_specifier),
                        _ => None,
                    }
                })
            },
            LiteralExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.literal_expression),
                        _ => None,
                    }
                })
            },
            PrefixedStringExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.prefixed_string_name),
                    1 => Some(&x.prefixed_string_str),
                        _ => None,
                    }
                })
            },
            PrefixedCodeExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.prefixed_code_prefix),
                    1 => Some(&x.prefixed_code_left_backtick),
                    2 => Some(&x.prefixed_code_expression),
                    3 => Some(&x.prefixed_code_right_backtick),
                        _ => None,
                    }
                })
            },
            VariableExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.variable_expression),
                        _ => None,
                    }
                })
            },
            PipeVariableExpression(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.pipe_variable_expression),
                        _ => None,
                    }
                })
            },
            FileAttributeSpecification(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.file_attribute_specification_left_double_angle),
                    1 => Some(&x.file_attribute_specification_keyword),
                    2 => Some(&x.file_attribute_specification_colon),
                    3 => Some(&x.file_attribute_specification_attributes),
                    4 => Some(&x.file_attribute_specification_right_double_angle),
                        _ => None,
                    }
                })
            },
            EnumDeclaration(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.enum_attribute_spec),
                    1 => Some(&x.enum_keyword),
                    2 => Some(&x.enum_name),
                    3 => Some(&x.enum_colon),
                    4 => Some(&x.enum_base),
                    5 => Some(&x.enum_type),
                    6 => Some(&x.enum_includes_keyword),
                    7 => Some(&x.enum_includes_list),
                    8 => Some(&x.enum_left_brace),
                    9 => Some(&x.enum_enumerators),
                    10 => Some(&x.enum_right_brace),
                        _ => None,
                    }
                })
            },
            Enumerator(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.enumerator_name),
                    1 => Some(&x.enumerator_equal),
                    2 => Some(&x.enumerator_value),
                    3 => Some(&x.enumerator_semicolon),
                        _ => None,
                    }
                })
            },
            RecordDeclaration(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.record_attribute_spec),
                    1 => Some(&x.record_modifier),
                    2 => Some(&x.record_keyword),
                    3 => Some(&x.record_name),
                    4 => Some(&x.record_extends_keyword),
                    5 => Some(&x.record_extends_opt),
                    6 => Some(&x.record_left_brace),
                    7 => Some(&x.record_fields),
                    8 => Some(&x.record_right_brace),
                        _ => None,
                    }
                })
            },
            RecordField(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.record_field_type),
                    1 => Some(&x.record_field_name),
                    2 => Some(&x.record_field_init),
                    3 => Some(&x.record_field_semi),
                        _ => None,
                    }
                })
            },
            AliasDeclaration(x) => {
                get_index(8).and_then(|index| { match index {
                        0 => Some(&x.alias_attribute_spec),
                    1 => Some(&x.alias_keyword),
                    2 => Some(&x.alias_name),
                    3 => Some(&x.alias_generic_parameter),
                    4 => Some(&x.alias_constraint),
                    5 => Some(&x.alias_equal),
                    6 => Some(&x.alias_type),
                    7 => Some(&x.alias_semicolon),
                        _ => None,
                    }
                })
            },
            PropertyDeclaration(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.property_attribute_spec),
                    1 => Some(&x.property_modifiers),
                    2 => Some(&x.property_type),
                    3 => Some(&x.property_declarators),
                    4 => Some(&x.property_semicolon),
                        _ => None,
                    }
                })
            },
            PropertyDeclarator(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.property_name),
                    1 => Some(&x.property_initializer),
                        _ => None,
                    }
                })
            },
            NamespaceDeclaration(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.namespace_header),
                    1 => Some(&x.namespace_body),
                        _ => None,
                    }
                })
            },
            NamespaceDeclarationHeader(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.namespace_keyword),
                    1 => Some(&x.namespace_name),
                        _ => None,
                    }
                })
            },
            NamespaceBody(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.namespace_left_brace),
                    1 => Some(&x.namespace_declarations),
                    2 => Some(&x.namespace_right_brace),
                        _ => None,
                    }
                })
            },
            NamespaceEmptyBody(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.namespace_semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceUseDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.namespace_use_keyword),
                    1 => Some(&x.namespace_use_kind),
                    2 => Some(&x.namespace_use_clauses),
                    3 => Some(&x.namespace_use_semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceGroupUseDeclaration(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.namespace_group_use_keyword),
                    1 => Some(&x.namespace_group_use_kind),
                    2 => Some(&x.namespace_group_use_prefix),
                    3 => Some(&x.namespace_group_use_left_brace),
                    4 => Some(&x.namespace_group_use_clauses),
                    5 => Some(&x.namespace_group_use_right_brace),
                    6 => Some(&x.namespace_group_use_semicolon),
                        _ => None,
                    }
                })
            },
            NamespaceUseClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.namespace_use_clause_kind),
                    1 => Some(&x.namespace_use_name),
                    2 => Some(&x.namespace_use_as),
                    3 => Some(&x.namespace_use_alias),
                        _ => None,
                    }
                })
            },
            FunctionDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.function_attribute_spec),
                    1 => Some(&x.function_declaration_header),
                    2 => Some(&x.function_body),
                        _ => None,
                    }
                })
            },
            FunctionDeclarationHeader(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.function_modifiers),
                    1 => Some(&x.function_keyword),
                    2 => Some(&x.function_name),
                    3 => Some(&x.function_type_parameter_list),
                    4 => Some(&x.function_left_paren),
                    5 => Some(&x.function_parameter_list),
                    6 => Some(&x.function_right_paren),
                    7 => Some(&x.function_capability_provisional),
                    8 => Some(&x.function_colon),
                    9 => Some(&x.function_type),
                    10 => Some(&x.function_where_clause),
                        _ => None,
                    }
                })
            },
            CapabilityProvisional(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.capability_provisional_at),
                    1 => Some(&x.capability_provisional_left_brace),
                    2 => Some(&x.capability_provisional_type),
                    3 => Some(&x.capability_provisional_unsafe_plus),
                    4 => Some(&x.capability_provisional_unsafe_type),
                    5 => Some(&x.capability_provisional_right_brace),
                        _ => None,
                    }
                })
            },
            WhereClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.where_clause_keyword),
                    1 => Some(&x.where_clause_constraints),
                        _ => None,
                    }
                })
            },
            WhereConstraint(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.where_constraint_left_type),
                    1 => Some(&x.where_constraint_operator),
                    2 => Some(&x.where_constraint_right_type),
                        _ => None,
                    }
                })
            },
            MethodishDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.methodish_attribute),
                    1 => Some(&x.methodish_function_decl_header),
                    2 => Some(&x.methodish_function_body),
                    3 => Some(&x.methodish_semicolon),
                        _ => None,
                    }
                })
            },
            MethodishTraitResolution(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.methodish_trait_attribute),
                    1 => Some(&x.methodish_trait_function_decl_header),
                    2 => Some(&x.methodish_trait_equal),
                    3 => Some(&x.methodish_trait_name),
                    4 => Some(&x.methodish_trait_semicolon),
                        _ => None,
                    }
                })
            },
            ClassishDeclaration(x) => {
                get_index(12).and_then(|index| { match index {
                        0 => Some(&x.classish_attribute),
                    1 => Some(&x.classish_modifiers),
                    2 => Some(&x.classish_xhp),
                    3 => Some(&x.classish_keyword),
                    4 => Some(&x.classish_name),
                    5 => Some(&x.classish_type_parameters),
                    6 => Some(&x.classish_extends_keyword),
                    7 => Some(&x.classish_extends_list),
                    8 => Some(&x.classish_implements_keyword),
                    9 => Some(&x.classish_implements_list),
                    10 => Some(&x.classish_where_clause),
                    11 => Some(&x.classish_body),
                        _ => None,
                    }
                })
            },
            ClassishBody(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.classish_body_left_brace),
                    1 => Some(&x.classish_body_elements),
                    2 => Some(&x.classish_body_right_brace),
                        _ => None,
                    }
                })
            },
            TraitUsePrecedenceItem(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.trait_use_precedence_item_name),
                    1 => Some(&x.trait_use_precedence_item_keyword),
                    2 => Some(&x.trait_use_precedence_item_removed_names),
                        _ => None,
                    }
                })
            },
            TraitUseAliasItem(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.trait_use_alias_item_aliasing_name),
                    1 => Some(&x.trait_use_alias_item_keyword),
                    2 => Some(&x.trait_use_alias_item_modifiers),
                    3 => Some(&x.trait_use_alias_item_aliased_name),
                        _ => None,
                    }
                })
            },
            TraitUseConflictResolution(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.trait_use_conflict_resolution_keyword),
                    1 => Some(&x.trait_use_conflict_resolution_names),
                    2 => Some(&x.trait_use_conflict_resolution_left_brace),
                    3 => Some(&x.trait_use_conflict_resolution_clauses),
                    4 => Some(&x.trait_use_conflict_resolution_right_brace),
                        _ => None,
                    }
                })
            },
            TraitUse(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.trait_use_keyword),
                    1 => Some(&x.trait_use_names),
                    2 => Some(&x.trait_use_semicolon),
                        _ => None,
                    }
                })
            },
            RequireClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.require_keyword),
                    1 => Some(&x.require_kind),
                    2 => Some(&x.require_name),
                    3 => Some(&x.require_semicolon),
                        _ => None,
                    }
                })
            },
            ConstDeclaration(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.const_modifiers),
                    1 => Some(&x.const_keyword),
                    2 => Some(&x.const_type_specifier),
                    3 => Some(&x.const_declarators),
                    4 => Some(&x.const_semicolon),
                        _ => None,
                    }
                })
            },
            ConstantDeclarator(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.constant_declarator_name),
                    1 => Some(&x.constant_declarator_initializer),
                        _ => None,
                    }
                })
            },
            TypeConstDeclaration(x) => {
                get_index(10).and_then(|index| { match index {
                        0 => Some(&x.type_const_attribute_spec),
                    1 => Some(&x.type_const_modifiers),
                    2 => Some(&x.type_const_keyword),
                    3 => Some(&x.type_const_type_keyword),
                    4 => Some(&x.type_const_name),
                    5 => Some(&x.type_const_type_parameters),
                    6 => Some(&x.type_const_type_constraint),
                    7 => Some(&x.type_const_equal),
                    8 => Some(&x.type_const_type_specifier),
                    9 => Some(&x.type_const_semicolon),
                        _ => None,
                    }
                })
            },
            DecoratedExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.decorated_expression_decorator),
                    1 => Some(&x.decorated_expression_expression),
                        _ => None,
                    }
                })
            },
            ParameterDeclaration(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.parameter_attribute),
                    1 => Some(&x.parameter_visibility),
                    2 => Some(&x.parameter_call_convention),
                    3 => Some(&x.parameter_type),
                    4 => Some(&x.parameter_name),
                    5 => Some(&x.parameter_default_value),
                        _ => None,
                    }
                })
            },
            VariadicParameter(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.variadic_parameter_call_convention),
                    1 => Some(&x.variadic_parameter_type),
                    2 => Some(&x.variadic_parameter_ellipsis),
                        _ => None,
                    }
                })
            },
            OldAttributeSpecification(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.old_attribute_specification_left_double_angle),
                    1 => Some(&x.old_attribute_specification_attributes),
                    2 => Some(&x.old_attribute_specification_right_double_angle),
                        _ => None,
                    }
                })
            },
            AttributeSpecification(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.attribute_specification_attributes),
                        _ => None,
                    }
                })
            },
            Attribute(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.attribute_at),
                    1 => Some(&x.attribute_attribute_name),
                        _ => None,
                    }
                })
            },
            InclusionExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.inclusion_require),
                    1 => Some(&x.inclusion_filename),
                        _ => None,
                    }
                })
            },
            InclusionDirective(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.inclusion_expression),
                    1 => Some(&x.inclusion_semicolon),
                        _ => None,
                    }
                })
            },
            CompoundStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.compound_left_brace),
                    1 => Some(&x.compound_statements),
                    2 => Some(&x.compound_right_brace),
                        _ => None,
                    }
                })
            },
            ExpressionStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.expression_statement_expression),
                    1 => Some(&x.expression_statement_semicolon),
                        _ => None,
                    }
                })
            },
            MarkupSection(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.markup_text),
                    1 => Some(&x.markup_suffix),
                        _ => None,
                    }
                })
            },
            MarkupSuffix(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.markup_suffix_less_than_question),
                    1 => Some(&x.markup_suffix_name),
                        _ => None,
                    }
                })
            },
            UnsetStatement(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.unset_keyword),
                    1 => Some(&x.unset_left_paren),
                    2 => Some(&x.unset_variables),
                    3 => Some(&x.unset_right_paren),
                    4 => Some(&x.unset_semicolon),
                        _ => None,
                    }
                })
            },
            UsingStatementBlockScoped(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.using_block_await_keyword),
                    1 => Some(&x.using_block_using_keyword),
                    2 => Some(&x.using_block_left_paren),
                    3 => Some(&x.using_block_expressions),
                    4 => Some(&x.using_block_right_paren),
                    5 => Some(&x.using_block_body),
                        _ => None,
                    }
                })
            },
            UsingStatementFunctionScoped(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.using_function_await_keyword),
                    1 => Some(&x.using_function_using_keyword),
                    2 => Some(&x.using_function_expression),
                    3 => Some(&x.using_function_semicolon),
                        _ => None,
                    }
                })
            },
            WhileStatement(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.while_keyword),
                    1 => Some(&x.while_left_paren),
                    2 => Some(&x.while_condition),
                    3 => Some(&x.while_right_paren),
                    4 => Some(&x.while_body),
                        _ => None,
                    }
                })
            },
            IfStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.if_keyword),
                    1 => Some(&x.if_left_paren),
                    2 => Some(&x.if_condition),
                    3 => Some(&x.if_right_paren),
                    4 => Some(&x.if_statement),
                    5 => Some(&x.if_elseif_clauses),
                    6 => Some(&x.if_else_clause),
                        _ => None,
                    }
                })
            },
            ElseifClause(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.elseif_keyword),
                    1 => Some(&x.elseif_left_paren),
                    2 => Some(&x.elseif_condition),
                    3 => Some(&x.elseif_right_paren),
                    4 => Some(&x.elseif_statement),
                        _ => None,
                    }
                })
            },
            ElseClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.else_keyword),
                    1 => Some(&x.else_statement),
                        _ => None,
                    }
                })
            },
            TryStatement(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.try_keyword),
                    1 => Some(&x.try_compound_statement),
                    2 => Some(&x.try_catch_clauses),
                    3 => Some(&x.try_finally_clause),
                        _ => None,
                    }
                })
            },
            CatchClause(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.catch_keyword),
                    1 => Some(&x.catch_left_paren),
                    2 => Some(&x.catch_type),
                    3 => Some(&x.catch_variable),
                    4 => Some(&x.catch_right_paren),
                    5 => Some(&x.catch_body),
                        _ => None,
                    }
                })
            },
            FinallyClause(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.finally_keyword),
                    1 => Some(&x.finally_body),
                        _ => None,
                    }
                })
            },
            DoStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.do_keyword),
                    1 => Some(&x.do_body),
                    2 => Some(&x.do_while_keyword),
                    3 => Some(&x.do_left_paren),
                    4 => Some(&x.do_condition),
                    5 => Some(&x.do_right_paren),
                    6 => Some(&x.do_semicolon),
                        _ => None,
                    }
                })
            },
            ForStatement(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.for_keyword),
                    1 => Some(&x.for_left_paren),
                    2 => Some(&x.for_initializer),
                    3 => Some(&x.for_first_semicolon),
                    4 => Some(&x.for_control),
                    5 => Some(&x.for_second_semicolon),
                    6 => Some(&x.for_end_of_loop),
                    7 => Some(&x.for_right_paren),
                    8 => Some(&x.for_body),
                        _ => None,
                    }
                })
            },
            ForeachStatement(x) => {
                get_index(10).and_then(|index| { match index {
                        0 => Some(&x.foreach_keyword),
                    1 => Some(&x.foreach_left_paren),
                    2 => Some(&x.foreach_collection),
                    3 => Some(&x.foreach_await_keyword),
                    4 => Some(&x.foreach_as),
                    5 => Some(&x.foreach_key),
                    6 => Some(&x.foreach_arrow),
                    7 => Some(&x.foreach_value),
                    8 => Some(&x.foreach_right_paren),
                    9 => Some(&x.foreach_body),
                        _ => None,
                    }
                })
            },
            SwitchStatement(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.switch_keyword),
                    1 => Some(&x.switch_left_paren),
                    2 => Some(&x.switch_expression),
                    3 => Some(&x.switch_right_paren),
                    4 => Some(&x.switch_left_brace),
                    5 => Some(&x.switch_sections),
                    6 => Some(&x.switch_right_brace),
                        _ => None,
                    }
                })
            },
            SwitchSection(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.switch_section_labels),
                    1 => Some(&x.switch_section_statements),
                    2 => Some(&x.switch_section_fallthrough),
                        _ => None,
                    }
                })
            },
            SwitchFallthrough(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.fallthrough_keyword),
                    1 => Some(&x.fallthrough_semicolon),
                        _ => None,
                    }
                })
            },
            CaseLabel(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.case_keyword),
                    1 => Some(&x.case_expression),
                    2 => Some(&x.case_colon),
                        _ => None,
                    }
                })
            },
            DefaultLabel(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.default_keyword),
                    1 => Some(&x.default_colon),
                        _ => None,
                    }
                })
            },
            ReturnStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.return_keyword),
                    1 => Some(&x.return_expression),
                    2 => Some(&x.return_semicolon),
                        _ => None,
                    }
                })
            },
            GotoLabel(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.goto_label_name),
                    1 => Some(&x.goto_label_colon),
                        _ => None,
                    }
                })
            },
            GotoStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.goto_statement_keyword),
                    1 => Some(&x.goto_statement_label_name),
                    2 => Some(&x.goto_statement_semicolon),
                        _ => None,
                    }
                })
            },
            ThrowStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.throw_keyword),
                    1 => Some(&x.throw_expression),
                    2 => Some(&x.throw_semicolon),
                        _ => None,
                    }
                })
            },
            BreakStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.break_keyword),
                    1 => Some(&x.break_semicolon),
                        _ => None,
                    }
                })
            },
            ContinueStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.continue_keyword),
                    1 => Some(&x.continue_semicolon),
                        _ => None,
                    }
                })
            },
            EchoStatement(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.echo_keyword),
                    1 => Some(&x.echo_expressions),
                    2 => Some(&x.echo_semicolon),
                        _ => None,
                    }
                })
            },
            ConcurrentStatement(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.concurrent_keyword),
                    1 => Some(&x.concurrent_statement),
                        _ => None,
                    }
                })
            },
            SimpleInitializer(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.simple_initializer_equal),
                    1 => Some(&x.simple_initializer_value),
                        _ => None,
                    }
                })
            },
            AnonymousClass(x) => {
                get_index(9).and_then(|index| { match index {
                        0 => Some(&x.anonymous_class_class_keyword),
                    1 => Some(&x.anonymous_class_left_paren),
                    2 => Some(&x.anonymous_class_argument_list),
                    3 => Some(&x.anonymous_class_right_paren),
                    4 => Some(&x.anonymous_class_extends_keyword),
                    5 => Some(&x.anonymous_class_extends_list),
                    6 => Some(&x.anonymous_class_implements_keyword),
                    7 => Some(&x.anonymous_class_implements_list),
                    8 => Some(&x.anonymous_class_body),
                        _ => None,
                    }
                })
            },
            AnonymousFunction(x) => {
                get_index(11).and_then(|index| { match index {
                        0 => Some(&x.anonymous_attribute_spec),
                    1 => Some(&x.anonymous_static_keyword),
                    2 => Some(&x.anonymous_async_keyword),
                    3 => Some(&x.anonymous_function_keyword),
                    4 => Some(&x.anonymous_left_paren),
                    5 => Some(&x.anonymous_parameters),
                    6 => Some(&x.anonymous_right_paren),
                    7 => Some(&x.anonymous_colon),
                    8 => Some(&x.anonymous_type),
                    9 => Some(&x.anonymous_use),
                    10 => Some(&x.anonymous_body),
                        _ => None,
                    }
                })
            },
            AnonymousFunctionUseClause(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.anonymous_use_keyword),
                    1 => Some(&x.anonymous_use_left_paren),
                    2 => Some(&x.anonymous_use_variables),
                    3 => Some(&x.anonymous_use_right_paren),
                        _ => None,
                    }
                })
            },
            LambdaExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.lambda_attribute_spec),
                    1 => Some(&x.lambda_async),
                    2 => Some(&x.lambda_signature),
                    3 => Some(&x.lambda_arrow),
                    4 => Some(&x.lambda_body),
                        _ => None,
                    }
                })
            },
            LambdaSignature(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.lambda_left_paren),
                    1 => Some(&x.lambda_parameters),
                    2 => Some(&x.lambda_right_paren),
                    3 => Some(&x.lambda_colon),
                    4 => Some(&x.lambda_type),
                        _ => None,
                    }
                })
            },
            CastExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.cast_left_paren),
                    1 => Some(&x.cast_type),
                    2 => Some(&x.cast_right_paren),
                    3 => Some(&x.cast_operand),
                        _ => None,
                    }
                })
            },
            ScopeResolutionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.scope_resolution_qualifier),
                    1 => Some(&x.scope_resolution_operator),
                    2 => Some(&x.scope_resolution_name),
                        _ => None,
                    }
                })
            },
            MemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.member_object),
                    1 => Some(&x.member_operator),
                    2 => Some(&x.member_name),
                        _ => None,
                    }
                })
            },
            SafeMemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.safe_member_object),
                    1 => Some(&x.safe_member_operator),
                    2 => Some(&x.safe_member_name),
                        _ => None,
                    }
                })
            },
            EmbeddedMemberSelectionExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.embedded_member_object),
                    1 => Some(&x.embedded_member_operator),
                    2 => Some(&x.embedded_member_name),
                        _ => None,
                    }
                })
            },
            YieldExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.yield_keyword),
                    1 => Some(&x.yield_operand),
                        _ => None,
                    }
                })
            },
            PrefixUnaryExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.prefix_unary_operator),
                    1 => Some(&x.prefix_unary_operand),
                        _ => None,
                    }
                })
            },
            PostfixUnaryExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.postfix_unary_operand),
                    1 => Some(&x.postfix_unary_operator),
                        _ => None,
                    }
                })
            },
            BinaryExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.binary_left_operand),
                    1 => Some(&x.binary_operator),
                    2 => Some(&x.binary_right_operand),
                        _ => None,
                    }
                })
            },
            IsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.is_left_operand),
                    1 => Some(&x.is_operator),
                    2 => Some(&x.is_right_operand),
                        _ => None,
                    }
                })
            },
            AsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.as_left_operand),
                    1 => Some(&x.as_operator),
                    2 => Some(&x.as_right_operand),
                        _ => None,
                    }
                })
            },
            NullableAsExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.nullable_as_left_operand),
                    1 => Some(&x.nullable_as_operator),
                    2 => Some(&x.nullable_as_right_operand),
                        _ => None,
                    }
                })
            },
            ConditionalExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.conditional_test),
                    1 => Some(&x.conditional_question),
                    2 => Some(&x.conditional_consequence),
                    3 => Some(&x.conditional_colon),
                    4 => Some(&x.conditional_alternative),
                        _ => None,
                    }
                })
            },
            EvalExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.eval_keyword),
                    1 => Some(&x.eval_left_paren),
                    2 => Some(&x.eval_argument),
                    3 => Some(&x.eval_right_paren),
                        _ => None,
                    }
                })
            },
            DefineExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.define_keyword),
                    1 => Some(&x.define_left_paren),
                    2 => Some(&x.define_argument_list),
                    3 => Some(&x.define_right_paren),
                        _ => None,
                    }
                })
            },
            IssetExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.isset_keyword),
                    1 => Some(&x.isset_left_paren),
                    2 => Some(&x.isset_argument_list),
                    3 => Some(&x.isset_right_paren),
                        _ => None,
                    }
                })
            },
            FunctionCallExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.function_call_receiver),
                    1 => Some(&x.function_call_type_args),
                    2 => Some(&x.function_call_left_paren),
                    3 => Some(&x.function_call_argument_list),
                    4 => Some(&x.function_call_right_paren),
                        _ => None,
                    }
                })
            },
            FunctionPointerExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.function_pointer_receiver),
                    1 => Some(&x.function_pointer_type_args),
                        _ => None,
                    }
                })
            },
            ParenthesizedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.parenthesized_expression_left_paren),
                    1 => Some(&x.parenthesized_expression_expression),
                    2 => Some(&x.parenthesized_expression_right_paren),
                        _ => None,
                    }
                })
            },
            BracedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.braced_expression_left_brace),
                    1 => Some(&x.braced_expression_expression),
                    2 => Some(&x.braced_expression_right_brace),
                        _ => None,
                    }
                })
            },
            EmbeddedBracedExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.embedded_braced_expression_left_brace),
                    1 => Some(&x.embedded_braced_expression_expression),
                    2 => Some(&x.embedded_braced_expression_right_brace),
                        _ => None,
                    }
                })
            },
            ListExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.list_keyword),
                    1 => Some(&x.list_left_paren),
                    2 => Some(&x.list_members),
                    3 => Some(&x.list_right_paren),
                        _ => None,
                    }
                })
            },
            CollectionLiteralExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.collection_literal_name),
                    1 => Some(&x.collection_literal_left_brace),
                    2 => Some(&x.collection_literal_initializers),
                    3 => Some(&x.collection_literal_right_brace),
                        _ => None,
                    }
                })
            },
            ObjectCreationExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.object_creation_new_keyword),
                    1 => Some(&x.object_creation_object),
                        _ => None,
                    }
                })
            },
            ConstructorCall(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.constructor_call_type),
                    1 => Some(&x.constructor_call_left_paren),
                    2 => Some(&x.constructor_call_argument_list),
                    3 => Some(&x.constructor_call_right_paren),
                        _ => None,
                    }
                })
            },
            RecordCreationExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.record_creation_type),
                    1 => Some(&x.record_creation_left_bracket),
                    2 => Some(&x.record_creation_members),
                    3 => Some(&x.record_creation_right_bracket),
                        _ => None,
                    }
                })
            },
            DarrayIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.darray_intrinsic_keyword),
                    1 => Some(&x.darray_intrinsic_explicit_type),
                    2 => Some(&x.darray_intrinsic_left_bracket),
                    3 => Some(&x.darray_intrinsic_members),
                    4 => Some(&x.darray_intrinsic_right_bracket),
                        _ => None,
                    }
                })
            },
            DictionaryIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.dictionary_intrinsic_keyword),
                    1 => Some(&x.dictionary_intrinsic_explicit_type),
                    2 => Some(&x.dictionary_intrinsic_left_bracket),
                    3 => Some(&x.dictionary_intrinsic_members),
                    4 => Some(&x.dictionary_intrinsic_right_bracket),
                        _ => None,
                    }
                })
            },
            KeysetIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyset_intrinsic_keyword),
                    1 => Some(&x.keyset_intrinsic_explicit_type),
                    2 => Some(&x.keyset_intrinsic_left_bracket),
                    3 => Some(&x.keyset_intrinsic_members),
                    4 => Some(&x.keyset_intrinsic_right_bracket),
                        _ => None,
                    }
                })
            },
            VarrayIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.varray_intrinsic_keyword),
                    1 => Some(&x.varray_intrinsic_explicit_type),
                    2 => Some(&x.varray_intrinsic_left_bracket),
                    3 => Some(&x.varray_intrinsic_members),
                    4 => Some(&x.varray_intrinsic_right_bracket),
                        _ => None,
                    }
                })
            },
            VectorIntrinsicExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.vector_intrinsic_keyword),
                    1 => Some(&x.vector_intrinsic_explicit_type),
                    2 => Some(&x.vector_intrinsic_left_bracket),
                    3 => Some(&x.vector_intrinsic_members),
                    4 => Some(&x.vector_intrinsic_right_bracket),
                        _ => None,
                    }
                })
            },
            ElementInitializer(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.element_key),
                    1 => Some(&x.element_arrow),
                    2 => Some(&x.element_value),
                        _ => None,
                    }
                })
            },
            SubscriptExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.subscript_receiver),
                    1 => Some(&x.subscript_left_bracket),
                    2 => Some(&x.subscript_index),
                    3 => Some(&x.subscript_right_bracket),
                        _ => None,
                    }
                })
            },
            EmbeddedSubscriptExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.embedded_subscript_receiver),
                    1 => Some(&x.embedded_subscript_left_bracket),
                    2 => Some(&x.embedded_subscript_index),
                    3 => Some(&x.embedded_subscript_right_bracket),
                        _ => None,
                    }
                })
            },
            AwaitableCreationExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.awaitable_attribute_spec),
                    1 => Some(&x.awaitable_async),
                    2 => Some(&x.awaitable_compound_statement),
                        _ => None,
                    }
                })
            },
            XHPChildrenDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_children_keyword),
                    1 => Some(&x.xhp_children_expression),
                    2 => Some(&x.xhp_children_semicolon),
                        _ => None,
                    }
                })
            },
            XHPChildrenParenthesizedList(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_children_list_left_paren),
                    1 => Some(&x.xhp_children_list_xhp_children),
                    2 => Some(&x.xhp_children_list_right_paren),
                        _ => None,
                    }
                })
            },
            XHPCategoryDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_category_keyword),
                    1 => Some(&x.xhp_category_categories),
                    2 => Some(&x.xhp_category_semicolon),
                        _ => None,
                    }
                })
            },
            XHPEnumType(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.xhp_enum_keyword),
                    1 => Some(&x.xhp_enum_left_brace),
                    2 => Some(&x.xhp_enum_values),
                    3 => Some(&x.xhp_enum_right_brace),
                        _ => None,
                    }
                })
            },
            XHPLateinit(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.xhp_lateinit_at),
                    1 => Some(&x.xhp_lateinit_keyword),
                        _ => None,
                    }
                })
            },
            XHPRequired(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.xhp_required_at),
                    1 => Some(&x.xhp_required_keyword),
                        _ => None,
                    }
                })
            },
            XHPClassAttributeDeclaration(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_attribute_keyword),
                    1 => Some(&x.xhp_attribute_attributes),
                    2 => Some(&x.xhp_attribute_semicolon),
                        _ => None,
                    }
                })
            },
            XHPClassAttribute(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.xhp_attribute_decl_type),
                    1 => Some(&x.xhp_attribute_decl_name),
                    2 => Some(&x.xhp_attribute_decl_initializer),
                    3 => Some(&x.xhp_attribute_decl_required),
                        _ => None,
                    }
                })
            },
            XHPSimpleClassAttribute(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.xhp_simple_class_attribute_type),
                        _ => None,
                    }
                })
            },
            XHPSimpleAttribute(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_simple_attribute_name),
                    1 => Some(&x.xhp_simple_attribute_equal),
                    2 => Some(&x.xhp_simple_attribute_expression),
                        _ => None,
                    }
                })
            },
            XHPSpreadAttribute(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.xhp_spread_attribute_left_brace),
                    1 => Some(&x.xhp_spread_attribute_spread_operator),
                    2 => Some(&x.xhp_spread_attribute_expression),
                    3 => Some(&x.xhp_spread_attribute_right_brace),
                        _ => None,
                    }
                })
            },
            XHPOpen(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.xhp_open_left_angle),
                    1 => Some(&x.xhp_open_name),
                    2 => Some(&x.xhp_open_attributes),
                    3 => Some(&x.xhp_open_right_angle),
                        _ => None,
                    }
                })
            },
            XHPExpression(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_open),
                    1 => Some(&x.xhp_body),
                    2 => Some(&x.xhp_close),
                        _ => None,
                    }
                })
            },
            XHPClose(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.xhp_close_left_angle),
                    1 => Some(&x.xhp_close_name),
                    2 => Some(&x.xhp_close_right_angle),
                        _ => None,
                    }
                })
            },
            TypeConstant(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.type_constant_left_type),
                    1 => Some(&x.type_constant_separator),
                    2 => Some(&x.type_constant_right_type),
                        _ => None,
                    }
                })
            },
            PUAccess(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.pu_access_left_type),
                    1 => Some(&x.pu_access_separator),
                    2 => Some(&x.pu_access_right_type),
                        _ => None,
                    }
                })
            },
            VectorTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.vector_type_keyword),
                    1 => Some(&x.vector_type_left_angle),
                    2 => Some(&x.vector_type_type),
                    3 => Some(&x.vector_type_trailing_comma),
                    4 => Some(&x.vector_type_right_angle),
                        _ => None,
                    }
                })
            },
            KeysetTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.keyset_type_keyword),
                    1 => Some(&x.keyset_type_left_angle),
                    2 => Some(&x.keyset_type_type),
                    3 => Some(&x.keyset_type_trailing_comma),
                    4 => Some(&x.keyset_type_right_angle),
                        _ => None,
                    }
                })
            },
            TupleTypeExplicitSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.tuple_type_keyword),
                    1 => Some(&x.tuple_type_left_angle),
                    2 => Some(&x.tuple_type_types),
                    3 => Some(&x.tuple_type_right_angle),
                        _ => None,
                    }
                })
            },
            VarrayTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.varray_keyword),
                    1 => Some(&x.varray_left_angle),
                    2 => Some(&x.varray_type),
                    3 => Some(&x.varray_trailing_comma),
                    4 => Some(&x.varray_right_angle),
                        _ => None,
                    }
                })
            },
            VectorArrayTypeSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.vector_array_keyword),
                    1 => Some(&x.vector_array_left_angle),
                    2 => Some(&x.vector_array_type),
                    3 => Some(&x.vector_array_right_angle),
                        _ => None,
                    }
                })
            },
            TypeParameter(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.type_attribute_spec),
                    1 => Some(&x.type_reified),
                    2 => Some(&x.type_variance),
                    3 => Some(&x.type_name),
                    4 => Some(&x.type_param_params),
                    5 => Some(&x.type_constraints),
                        _ => None,
                    }
                })
            },
            TypeConstraint(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.constraint_keyword),
                    1 => Some(&x.constraint_type),
                        _ => None,
                    }
                })
            },
            DarrayTypeSpecifier(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.darray_keyword),
                    1 => Some(&x.darray_left_angle),
                    2 => Some(&x.darray_key),
                    3 => Some(&x.darray_comma),
                    4 => Some(&x.darray_value),
                    5 => Some(&x.darray_trailing_comma),
                    6 => Some(&x.darray_right_angle),
                        _ => None,
                    }
                })
            },
            MapArrayTypeSpecifier(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.map_array_keyword),
                    1 => Some(&x.map_array_left_angle),
                    2 => Some(&x.map_array_key),
                    3 => Some(&x.map_array_comma),
                    4 => Some(&x.map_array_value),
                    5 => Some(&x.map_array_right_angle),
                        _ => None,
                    }
                })
            },
            DictionaryTypeSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.dictionary_type_keyword),
                    1 => Some(&x.dictionary_type_left_angle),
                    2 => Some(&x.dictionary_type_members),
                    3 => Some(&x.dictionary_type_right_angle),
                        _ => None,
                    }
                })
            },
            ClosureTypeSpecifier(x) => {
                get_index(8).and_then(|index| { match index {
                        0 => Some(&x.closure_outer_left_paren),
                    1 => Some(&x.closure_function_keyword),
                    2 => Some(&x.closure_inner_left_paren),
                    3 => Some(&x.closure_parameter_list),
                    4 => Some(&x.closure_inner_right_paren),
                    5 => Some(&x.closure_colon),
                    6 => Some(&x.closure_return_type),
                    7 => Some(&x.closure_outer_right_paren),
                        _ => None,
                    }
                })
            },
            ClosureParameterTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.closure_parameter_call_convention),
                    1 => Some(&x.closure_parameter_type),
                        _ => None,
                    }
                })
            },
            ClassnameTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.classname_keyword),
                    1 => Some(&x.classname_left_angle),
                    2 => Some(&x.classname_type),
                    3 => Some(&x.classname_trailing_comma),
                    4 => Some(&x.classname_right_angle),
                        _ => None,
                    }
                })
            },
            FieldSpecifier(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.field_question),
                    1 => Some(&x.field_name),
                    2 => Some(&x.field_arrow),
                    3 => Some(&x.field_type),
                        _ => None,
                    }
                })
            },
            FieldInitializer(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.field_initializer_name),
                    1 => Some(&x.field_initializer_arrow),
                    2 => Some(&x.field_initializer_value),
                        _ => None,
                    }
                })
            },
            ShapeTypeSpecifier(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.shape_type_keyword),
                    1 => Some(&x.shape_type_left_paren),
                    2 => Some(&x.shape_type_fields),
                    3 => Some(&x.shape_type_ellipsis),
                    4 => Some(&x.shape_type_right_paren),
                        _ => None,
                    }
                })
            },
            ShapeExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.shape_expression_keyword),
                    1 => Some(&x.shape_expression_left_paren),
                    2 => Some(&x.shape_expression_fields),
                    3 => Some(&x.shape_expression_right_paren),
                        _ => None,
                    }
                })
            },
            TupleExpression(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.tuple_expression_keyword),
                    1 => Some(&x.tuple_expression_left_paren),
                    2 => Some(&x.tuple_expression_items),
                    3 => Some(&x.tuple_expression_right_paren),
                        _ => None,
                    }
                })
            },
            GenericTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.generic_class_type),
                    1 => Some(&x.generic_argument_list),
                        _ => None,
                    }
                })
            },
            NullableTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.nullable_question),
                    1 => Some(&x.nullable_type),
                        _ => None,
                    }
                })
            },
            LikeTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.like_tilde),
                    1 => Some(&x.like_type),
                        _ => None,
                    }
                })
            },
            SoftTypeSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.soft_at),
                    1 => Some(&x.soft_type),
                        _ => None,
                    }
                })
            },
            AttributizedSpecifier(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.attributized_specifier_attribute_spec),
                    1 => Some(&x.attributized_specifier_type),
                        _ => None,
                    }
                })
            },
            ReifiedTypeArgument(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.reified_type_argument_reified),
                    1 => Some(&x.reified_type_argument_type),
                        _ => None,
                    }
                })
            },
            TypeArguments(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.type_arguments_left_angle),
                    1 => Some(&x.type_arguments_types),
                    2 => Some(&x.type_arguments_right_angle),
                        _ => None,
                    }
                })
            },
            TypeParameters(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.type_parameters_left_angle),
                    1 => Some(&x.type_parameters_parameters),
                    2 => Some(&x.type_parameters_right_angle),
                        _ => None,
                    }
                })
            },
            TupleTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.tuple_left_paren),
                    1 => Some(&x.tuple_types),
                    2 => Some(&x.tuple_right_paren),
                        _ => None,
                    }
                })
            },
            UnionTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.union_left_paren),
                    1 => Some(&x.union_types),
                    2 => Some(&x.union_right_paren),
                        _ => None,
                    }
                })
            },
            IntersectionTypeSpecifier(x) => {
                get_index(3).and_then(|index| { match index {
                        0 => Some(&x.intersection_left_paren),
                    1 => Some(&x.intersection_types),
                    2 => Some(&x.intersection_right_paren),
                        _ => None,
                    }
                })
            },
            ErrorSyntax(x) => {
                get_index(1).and_then(|index| { match index {
                        0 => Some(&x.error_error),
                        _ => None,
                    }
                })
            },
            ListItem(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.list_item),
                    1 => Some(&x.list_separator),
                        _ => None,
                    }
                })
            },
            PocketAtomExpression(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.pocket_atom_glyph),
                    1 => Some(&x.pocket_atom_expression),
                        _ => None,
                    }
                })
            },
            PocketIdentifierExpression(x) => {
                get_index(5).and_then(|index| { match index {
                        0 => Some(&x.pocket_identifier_qualifier),
                    1 => Some(&x.pocket_identifier_pu_operator),
                    2 => Some(&x.pocket_identifier_field),
                    3 => Some(&x.pocket_identifier_operator),
                    4 => Some(&x.pocket_identifier_name),
                        _ => None,
                    }
                })
            },
            PocketAtomMappingDeclaration(x) => {
                get_index(6).and_then(|index| { match index {
                        0 => Some(&x.pocket_atom_mapping_glyph),
                    1 => Some(&x.pocket_atom_mapping_name),
                    2 => Some(&x.pocket_atom_mapping_left_paren),
                    3 => Some(&x.pocket_atom_mapping_mappings),
                    4 => Some(&x.pocket_atom_mapping_right_paren),
                    5 => Some(&x.pocket_atom_mapping_semicolon),
                        _ => None,
                    }
                })
            },
            PocketEnumDeclaration(x) => {
                get_index(7).and_then(|index| { match index {
                        0 => Some(&x.pocket_enum_attributes),
                    1 => Some(&x.pocket_enum_modifiers),
                    2 => Some(&x.pocket_enum_enum),
                    3 => Some(&x.pocket_enum_name),
                    4 => Some(&x.pocket_enum_left_brace),
                    5 => Some(&x.pocket_enum_fields),
                    6 => Some(&x.pocket_enum_right_brace),
                        _ => None,
                    }
                })
            },
            PocketFieldTypeExprDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.pocket_field_type_expr_case),
                    1 => Some(&x.pocket_field_type_expr_type),
                    2 => Some(&x.pocket_field_type_expr_name),
                    3 => Some(&x.pocket_field_type_expr_semicolon),
                        _ => None,
                    }
                })
            },
            PocketFieldTypeDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.pocket_field_type_case),
                    1 => Some(&x.pocket_field_type_type),
                    2 => Some(&x.pocket_field_type_type_parameter),
                    3 => Some(&x.pocket_field_type_semicolon),
                        _ => None,
                    }
                })
            },
            PocketMappingIdDeclaration(x) => {
                get_index(2).and_then(|index| { match index {
                        0 => Some(&x.pocket_mapping_id_name),
                    1 => Some(&x.pocket_mapping_id_initializer),
                        _ => None,
                    }
                })
            },
            PocketMappingTypeDeclaration(x) => {
                get_index(4).and_then(|index| { match index {
                        0 => Some(&x.pocket_mapping_type_keyword),
                    1 => Some(&x.pocket_mapping_type_name),
                    2 => Some(&x.pocket_mapping_type_equal),
                    3 => Some(&x.pocket_mapping_type_type),
                        _ => None,
                    }
                })
            },

        };
        if res.is_some() {
            if direction {
                self.index = self.index + 1
            } else {
                self.index_back = self.index_back + 1
            }
        }
        res
    }
}
