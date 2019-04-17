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
use crate::syntax_type::SyntaxType;

impl<T, V> SyntaxType<T, V> for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
    fn make_end_of_file(end_of_file_token: Self) -> Self {
        let syntax = SyntaxVariant::EndOfFile(Box::new(EndOfFileChildren {
            end_of_file_token,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_script(script_declarations: Self) -> Self {
        let syntax = SyntaxVariant::Script(Box::new(ScriptChildren {
            script_declarations,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_qualified_name(qualified_name_parts: Self) -> Self {
        let syntax = SyntaxVariant::QualifiedName(Box::new(QualifiedNameChildren {
            qualified_name_parts,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_simple_type_specifier(simple_type_specifier: Self) -> Self {
        let syntax = SyntaxVariant::SimpleTypeSpecifier(Box::new(SimpleTypeSpecifierChildren {
            simple_type_specifier,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_literal_expression(literal_expression: Self) -> Self {
        let syntax = SyntaxVariant::LiteralExpression(Box::new(LiteralExpressionChildren {
            literal_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_prefixed_string_expression(prefixed_string_name: Self, prefixed_string_str: Self) -> Self {
        let syntax = SyntaxVariant::PrefixedStringExpression(Box::new(PrefixedStringExpressionChildren {
            prefixed_string_name,
            prefixed_string_str,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_variable_expression(variable_expression: Self) -> Self {
        let syntax = SyntaxVariant::VariableExpression(Box::new(VariableExpressionChildren {
            variable_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pipe_variable_expression(pipe_variable_expression: Self) -> Self {
        let syntax = SyntaxVariant::PipeVariableExpression(Box::new(PipeVariableExpressionChildren {
            pipe_variable_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_file_attribute_specification(file_attribute_specification_left_double_angle: Self, file_attribute_specification_keyword: Self, file_attribute_specification_colon: Self, file_attribute_specification_attributes: Self, file_attribute_specification_right_double_angle: Self) -> Self {
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

    fn make_enum_declaration(enum_attribute_spec: Self, enum_keyword: Self, enum_name: Self, enum_colon: Self, enum_base: Self, enum_type: Self, enum_left_brace: Self, enum_enumerators: Self, enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EnumDeclaration(Box::new(EnumDeclarationChildren {
            enum_attribute_spec,
            enum_keyword,
            enum_name,
            enum_colon,
            enum_base,
            enum_type,
            enum_left_brace,
            enum_enumerators,
            enum_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_enumerator(enumerator_name: Self, enumerator_equal: Self, enumerator_value: Self, enumerator_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::Enumerator(Box::new(EnumeratorChildren {
            enumerator_name,
            enumerator_equal,
            enumerator_value,
            enumerator_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_declaration(record_attribute_spec: Self, record_keyword: Self, record_name: Self, record_left_brace: Self, record_fields: Self, record_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::RecordDeclaration(Box::new(RecordDeclarationChildren {
            record_attribute_spec,
            record_keyword,
            record_name,
            record_left_brace,
            record_fields,
            record_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_field(record_field_name: Self, record_field_colon: Self, record_field_type: Self, record_field_init: Self, record_field_comma: Self) -> Self {
        let syntax = SyntaxVariant::RecordField(Box::new(RecordFieldChildren {
            record_field_name,
            record_field_colon,
            record_field_type,
            record_field_init,
            record_field_comma,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alias_declaration(alias_attribute_spec: Self, alias_keyword: Self, alias_name: Self, alias_generic_parameter: Self, alias_constraint: Self, alias_equal: Self, alias_type: Self, alias_semicolon: Self) -> Self {
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

    fn make_property_declaration(property_attribute_spec: Self, property_modifiers: Self, property_type: Self, property_declarators: Self, property_semicolon: Self) -> Self {
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

    fn make_property_declarator(property_name: Self, property_initializer: Self) -> Self {
        let syntax = SyntaxVariant::PropertyDeclarator(Box::new(PropertyDeclaratorChildren {
            property_name,
            property_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_declaration(namespace_keyword: Self, namespace_name: Self, namespace_body: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceDeclaration(Box::new(NamespaceDeclarationChildren {
            namespace_keyword,
            namespace_name,
            namespace_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_body(namespace_left_brace: Self, namespace_declarations: Self, namespace_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceBody(Box::new(NamespaceBodyChildren {
            namespace_left_brace,
            namespace_declarations,
            namespace_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_empty_body(namespace_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceEmptyBody(Box::new(NamespaceEmptyBodyChildren {
            namespace_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_use_declaration(namespace_use_keyword: Self, namespace_use_kind: Self, namespace_use_clauses: Self, namespace_use_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseDeclaration(Box::new(NamespaceUseDeclarationChildren {
            namespace_use_keyword,
            namespace_use_kind,
            namespace_use_clauses,
            namespace_use_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_namespace_group_use_declaration(namespace_group_use_keyword: Self, namespace_group_use_kind: Self, namespace_group_use_prefix: Self, namespace_group_use_left_brace: Self, namespace_group_use_clauses: Self, namespace_group_use_right_brace: Self, namespace_group_use_semicolon: Self) -> Self {
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

    fn make_namespace_use_clause(namespace_use_clause_kind: Self, namespace_use_name: Self, namespace_use_as: Self, namespace_use_alias: Self) -> Self {
        let syntax = SyntaxVariant::NamespaceUseClause(Box::new(NamespaceUseClauseChildren {
            namespace_use_clause_kind,
            namespace_use_name,
            namespace_use_as,
            namespace_use_alias,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_declaration(function_attribute_spec: Self, function_declaration_header: Self, function_body: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclaration(Box::new(FunctionDeclarationChildren {
            function_attribute_spec,
            function_declaration_header,
            function_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_declaration_header(function_modifiers: Self, function_keyword: Self, function_name: Self, function_type_parameter_list: Self, function_left_paren: Self, function_parameter_list: Self, function_right_paren: Self, function_colon: Self, function_type: Self, function_where_clause: Self) -> Self {
        let syntax = SyntaxVariant::FunctionDeclarationHeader(Box::new(FunctionDeclarationHeaderChildren {
            function_modifiers,
            function_keyword,
            function_name,
            function_type_parameter_list,
            function_left_paren,
            function_parameter_list,
            function_right_paren,
            function_colon,
            function_type,
            function_where_clause,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_where_clause(where_clause_keyword: Self, where_clause_constraints: Self) -> Self {
        let syntax = SyntaxVariant::WhereClause(Box::new(WhereClauseChildren {
            where_clause_keyword,
            where_clause_constraints,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_where_constraint(where_constraint_left_type: Self, where_constraint_operator: Self, where_constraint_right_type: Self) -> Self {
        let syntax = SyntaxVariant::WhereConstraint(Box::new(WhereConstraintChildren {
            where_constraint_left_type,
            where_constraint_operator,
            where_constraint_right_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_methodish_declaration(methodish_attribute: Self, methodish_function_decl_header: Self, methodish_function_body: Self, methodish_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::MethodishDeclaration(Box::new(MethodishDeclarationChildren {
            methodish_attribute,
            methodish_function_decl_header,
            methodish_function_body,
            methodish_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_methodish_trait_resolution(methodish_trait_attribute: Self, methodish_trait_function_decl_header: Self, methodish_trait_equal: Self, methodish_trait_name: Self, methodish_trait_semicolon: Self) -> Self {
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

    fn make_classish_declaration(classish_attribute: Self, classish_modifiers: Self, classish_keyword: Self, classish_name: Self, classish_type_parameters: Self, classish_extends_keyword: Self, classish_extends_list: Self, classish_implements_keyword: Self, classish_implements_list: Self, classish_body: Self) -> Self {
        let syntax = SyntaxVariant::ClassishDeclaration(Box::new(ClassishDeclarationChildren {
            classish_attribute,
            classish_modifiers,
            classish_keyword,
            classish_name,
            classish_type_parameters,
            classish_extends_keyword,
            classish_extends_list,
            classish_implements_keyword,
            classish_implements_list,
            classish_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_classish_body(classish_body_left_brace: Self, classish_body_elements: Self, classish_body_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::ClassishBody(Box::new(ClassishBodyChildren {
            classish_body_left_brace,
            classish_body_elements,
            classish_body_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_precedence_item(trait_use_precedence_item_name: Self, trait_use_precedence_item_keyword: Self, trait_use_precedence_item_removed_names: Self) -> Self {
        let syntax = SyntaxVariant::TraitUsePrecedenceItem(Box::new(TraitUsePrecedenceItemChildren {
            trait_use_precedence_item_name,
            trait_use_precedence_item_keyword,
            trait_use_precedence_item_removed_names,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_alias_item(trait_use_alias_item_aliasing_name: Self, trait_use_alias_item_keyword: Self, trait_use_alias_item_modifiers: Self, trait_use_alias_item_aliased_name: Self) -> Self {
        let syntax = SyntaxVariant::TraitUseAliasItem(Box::new(TraitUseAliasItemChildren {
            trait_use_alias_item_aliasing_name,
            trait_use_alias_item_keyword,
            trait_use_alias_item_modifiers,
            trait_use_alias_item_aliased_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_trait_use_conflict_resolution(trait_use_conflict_resolution_keyword: Self, trait_use_conflict_resolution_names: Self, trait_use_conflict_resolution_left_brace: Self, trait_use_conflict_resolution_clauses: Self, trait_use_conflict_resolution_right_brace: Self) -> Self {
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

    fn make_trait_use(trait_use_keyword: Self, trait_use_names: Self, trait_use_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TraitUse(Box::new(TraitUseChildren {
            trait_use_keyword,
            trait_use_names,
            trait_use_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_require_clause(require_keyword: Self, require_kind: Self, require_name: Self, require_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::RequireClause(Box::new(RequireClauseChildren {
            require_keyword,
            require_kind,
            require_name,
            require_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_const_declaration(const_visibility: Self, const_abstract: Self, const_keyword: Self, const_type_specifier: Self, const_declarators: Self, const_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ConstDeclaration(Box::new(ConstDeclarationChildren {
            const_visibility,
            const_abstract,
            const_keyword,
            const_type_specifier,
            const_declarators,
            const_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_constant_declarator(constant_declarator_name: Self, constant_declarator_initializer: Self) -> Self {
        let syntax = SyntaxVariant::ConstantDeclarator(Box::new(ConstantDeclaratorChildren {
            constant_declarator_name,
            constant_declarator_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_const_declaration(type_const_attribute_spec: Self, type_const_abstract: Self, type_const_keyword: Self, type_const_type_keyword: Self, type_const_name: Self, type_const_type_parameters: Self, type_const_type_constraint: Self, type_const_equal: Self, type_const_type_specifier: Self, type_const_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstDeclaration(Box::new(TypeConstDeclarationChildren {
            type_const_attribute_spec,
            type_const_abstract,
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

    fn make_decorated_expression(decorated_expression_decorator: Self, decorated_expression_expression: Self) -> Self {
        let syntax = SyntaxVariant::DecoratedExpression(Box::new(DecoratedExpressionChildren {
            decorated_expression_decorator,
            decorated_expression_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_parameter_declaration(parameter_attribute: Self, parameter_visibility: Self, parameter_call_convention: Self, parameter_type: Self, parameter_name: Self, parameter_default_value: Self) -> Self {
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

    fn make_variadic_parameter(variadic_parameter_call_convention: Self, variadic_parameter_type: Self, variadic_parameter_ellipsis: Self) -> Self {
        let syntax = SyntaxVariant::VariadicParameter(Box::new(VariadicParameterChildren {
            variadic_parameter_call_convention,
            variadic_parameter_type,
            variadic_parameter_ellipsis,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_attribute_specification(attribute_specification_left_double_angle: Self, attribute_specification_attributes: Self, attribute_specification_right_double_angle: Self) -> Self {
        let syntax = SyntaxVariant::AttributeSpecification(Box::new(AttributeSpecificationChildren {
            attribute_specification_left_double_angle,
            attribute_specification_attributes,
            attribute_specification_right_double_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_inclusion_expression(inclusion_require: Self, inclusion_filename: Self) -> Self {
        let syntax = SyntaxVariant::InclusionExpression(Box::new(InclusionExpressionChildren {
            inclusion_require,
            inclusion_filename,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_inclusion_directive(inclusion_expression: Self, inclusion_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::InclusionDirective(Box::new(InclusionDirectiveChildren {
            inclusion_expression,
            inclusion_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_compound_statement(compound_left_brace: Self, compound_statements: Self, compound_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CompoundStatement(Box::new(CompoundStatementChildren {
            compound_left_brace,
            compound_statements,
            compound_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alternate_loop_statement(alternate_loop_opening_colon: Self, alternate_loop_statements: Self, alternate_loop_closing_keyword: Self, alternate_loop_closing_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AlternateLoopStatement(Box::new(AlternateLoopStatementChildren {
            alternate_loop_opening_colon,
            alternate_loop_statements,
            alternate_loop_closing_keyword,
            alternate_loop_closing_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_expression_statement(expression_statement_expression: Self, expression_statement_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ExpressionStatement(Box::new(ExpressionStatementChildren {
            expression_statement_expression,
            expression_statement_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_markup_section(markup_prefix: Self, markup_text: Self, markup_suffix: Self, markup_expression: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSection(Box::new(MarkupSectionChildren {
            markup_prefix,
            markup_text,
            markup_suffix,
            markup_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_markup_suffix(markup_suffix_less_than_question: Self, markup_suffix_name: Self) -> Self {
        let syntax = SyntaxVariant::MarkupSuffix(Box::new(MarkupSuffixChildren {
            markup_suffix_less_than_question,
            markup_suffix_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_unset_statement(unset_keyword: Self, unset_left_paren: Self, unset_variables: Self, unset_right_paren: Self, unset_semicolon: Self) -> Self {
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

    fn make_let_statement(let_statement_keyword: Self, let_statement_name: Self, let_statement_colon: Self, let_statement_type: Self, let_statement_initializer: Self, let_statement_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::LetStatement(Box::new(LetStatementChildren {
            let_statement_keyword,
            let_statement_name,
            let_statement_colon,
            let_statement_type,
            let_statement_initializer,
            let_statement_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_using_statement_block_scoped(using_block_await_keyword: Self, using_block_using_keyword: Self, using_block_left_paren: Self, using_block_expressions: Self, using_block_right_paren: Self, using_block_body: Self) -> Self {
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

    fn make_using_statement_function_scoped(using_function_await_keyword: Self, using_function_using_keyword: Self, using_function_expression: Self, using_function_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::UsingStatementFunctionScoped(Box::new(UsingStatementFunctionScopedChildren {
            using_function_await_keyword,
            using_function_using_keyword,
            using_function_expression,
            using_function_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_declare_directive_statement(declare_directive_keyword: Self, declare_directive_left_paren: Self, declare_directive_expression: Self, declare_directive_right_paren: Self, declare_directive_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::DeclareDirectiveStatement(Box::new(DeclareDirectiveStatementChildren {
            declare_directive_keyword,
            declare_directive_left_paren,
            declare_directive_expression,
            declare_directive_right_paren,
            declare_directive_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_declare_block_statement(declare_block_keyword: Self, declare_block_left_paren: Self, declare_block_expression: Self, declare_block_right_paren: Self, declare_block_body: Self) -> Self {
        let syntax = SyntaxVariant::DeclareBlockStatement(Box::new(DeclareBlockStatementChildren {
            declare_block_keyword,
            declare_block_left_paren,
            declare_block_expression,
            declare_block_right_paren,
            declare_block_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_while_statement(while_keyword: Self, while_left_paren: Self, while_condition: Self, while_right_paren: Self, while_body: Self) -> Self {
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

    fn make_if_statement(if_keyword: Self, if_left_paren: Self, if_condition: Self, if_right_paren: Self, if_statement: Self, if_elseif_clauses: Self, if_else_clause: Self) -> Self {
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

    fn make_elseif_clause(elseif_keyword: Self, elseif_left_paren: Self, elseif_condition: Self, elseif_right_paren: Self, elseif_statement: Self) -> Self {
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

    fn make_else_clause(else_keyword: Self, else_statement: Self) -> Self {
        let syntax = SyntaxVariant::ElseClause(Box::new(ElseClauseChildren {
            else_keyword,
            else_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alternate_if_statement(alternate_if_keyword: Self, alternate_if_left_paren: Self, alternate_if_condition: Self, alternate_if_right_paren: Self, alternate_if_colon: Self, alternate_if_statement: Self, alternate_if_elseif_clauses: Self, alternate_if_else_clause: Self, alternate_if_endif_keyword: Self, alternate_if_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AlternateIfStatement(Box::new(AlternateIfStatementChildren {
            alternate_if_keyword,
            alternate_if_left_paren,
            alternate_if_condition,
            alternate_if_right_paren,
            alternate_if_colon,
            alternate_if_statement,
            alternate_if_elseif_clauses,
            alternate_if_else_clause,
            alternate_if_endif_keyword,
            alternate_if_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alternate_elseif_clause(alternate_elseif_keyword: Self, alternate_elseif_left_paren: Self, alternate_elseif_condition: Self, alternate_elseif_right_paren: Self, alternate_elseif_colon: Self, alternate_elseif_statement: Self) -> Self {
        let syntax = SyntaxVariant::AlternateElseifClause(Box::new(AlternateElseifClauseChildren {
            alternate_elseif_keyword,
            alternate_elseif_left_paren,
            alternate_elseif_condition,
            alternate_elseif_right_paren,
            alternate_elseif_colon,
            alternate_elseif_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_alternate_else_clause(alternate_else_keyword: Self, alternate_else_colon: Self, alternate_else_statement: Self) -> Self {
        let syntax = SyntaxVariant::AlternateElseClause(Box::new(AlternateElseClauseChildren {
            alternate_else_keyword,
            alternate_else_colon,
            alternate_else_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_try_statement(try_keyword: Self, try_compound_statement: Self, try_catch_clauses: Self, try_finally_clause: Self) -> Self {
        let syntax = SyntaxVariant::TryStatement(Box::new(TryStatementChildren {
            try_keyword,
            try_compound_statement,
            try_catch_clauses,
            try_finally_clause,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_catch_clause(catch_keyword: Self, catch_left_paren: Self, catch_type: Self, catch_variable: Self, catch_right_paren: Self, catch_body: Self) -> Self {
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

    fn make_finally_clause(finally_keyword: Self, finally_body: Self) -> Self {
        let syntax = SyntaxVariant::FinallyClause(Box::new(FinallyClauseChildren {
            finally_keyword,
            finally_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_do_statement(do_keyword: Self, do_body: Self, do_while_keyword: Self, do_left_paren: Self, do_condition: Self, do_right_paren: Self, do_semicolon: Self) -> Self {
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

    fn make_for_statement(for_keyword: Self, for_left_paren: Self, for_initializer: Self, for_first_semicolon: Self, for_control: Self, for_second_semicolon: Self, for_end_of_loop: Self, for_right_paren: Self, for_body: Self) -> Self {
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

    fn make_foreach_statement(foreach_keyword: Self, foreach_left_paren: Self, foreach_collection: Self, foreach_await_keyword: Self, foreach_as: Self, foreach_key: Self, foreach_arrow: Self, foreach_value: Self, foreach_right_paren: Self, foreach_body: Self) -> Self {
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

    fn make_switch_statement(switch_keyword: Self, switch_left_paren: Self, switch_expression: Self, switch_right_paren: Self, switch_left_brace: Self, switch_sections: Self, switch_right_brace: Self) -> Self {
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

    fn make_alternate_switch_statement(alternate_switch_keyword: Self, alternate_switch_left_paren: Self, alternate_switch_expression: Self, alternate_switch_right_paren: Self, alternate_switch_opening_colon: Self, alternate_switch_sections: Self, alternate_switch_closing_endswitch: Self, alternate_switch_closing_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::AlternateSwitchStatement(Box::new(AlternateSwitchStatementChildren {
            alternate_switch_keyword,
            alternate_switch_left_paren,
            alternate_switch_expression,
            alternate_switch_right_paren,
            alternate_switch_opening_colon,
            alternate_switch_sections,
            alternate_switch_closing_endswitch,
            alternate_switch_closing_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_switch_section(switch_section_labels: Self, switch_section_statements: Self, switch_section_fallthrough: Self) -> Self {
        let syntax = SyntaxVariant::SwitchSection(Box::new(SwitchSectionChildren {
            switch_section_labels,
            switch_section_statements,
            switch_section_fallthrough,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_switch_fallthrough(fallthrough_keyword: Self, fallthrough_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::SwitchFallthrough(Box::new(SwitchFallthroughChildren {
            fallthrough_keyword,
            fallthrough_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_case_label(case_keyword: Self, case_expression: Self, case_colon: Self) -> Self {
        let syntax = SyntaxVariant::CaseLabel(Box::new(CaseLabelChildren {
            case_keyword,
            case_expression,
            case_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_default_label(default_keyword: Self, default_colon: Self) -> Self {
        let syntax = SyntaxVariant::DefaultLabel(Box::new(DefaultLabelChildren {
            default_keyword,
            default_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_return_statement(return_keyword: Self, return_expression: Self, return_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ReturnStatement(Box::new(ReturnStatementChildren {
            return_keyword,
            return_expression,
            return_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_goto_label(goto_label_name: Self, goto_label_colon: Self) -> Self {
        let syntax = SyntaxVariant::GotoLabel(Box::new(GotoLabelChildren {
            goto_label_name,
            goto_label_colon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_goto_statement(goto_statement_keyword: Self, goto_statement_label_name: Self, goto_statement_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::GotoStatement(Box::new(GotoStatementChildren {
            goto_statement_keyword,
            goto_statement_label_name,
            goto_statement_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_throw_statement(throw_keyword: Self, throw_expression: Self, throw_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ThrowStatement(Box::new(ThrowStatementChildren {
            throw_keyword,
            throw_expression,
            throw_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_break_statement(break_keyword: Self, break_level: Self, break_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::BreakStatement(Box::new(BreakStatementChildren {
            break_keyword,
            break_level,
            break_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_continue_statement(continue_keyword: Self, continue_level: Self, continue_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::ContinueStatement(Box::new(ContinueStatementChildren {
            continue_keyword,
            continue_level,
            continue_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_echo_statement(echo_keyword: Self, echo_expressions: Self, echo_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::EchoStatement(Box::new(EchoStatementChildren {
            echo_keyword,
            echo_expressions,
            echo_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_concurrent_statement(concurrent_keyword: Self, concurrent_statement: Self) -> Self {
        let syntax = SyntaxVariant::ConcurrentStatement(Box::new(ConcurrentStatementChildren {
            concurrent_keyword,
            concurrent_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_simple_initializer(simple_initializer_equal: Self, simple_initializer_value: Self) -> Self {
        let syntax = SyntaxVariant::SimpleInitializer(Box::new(SimpleInitializerChildren {
            simple_initializer_equal,
            simple_initializer_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_anonymous_class(anonymous_class_class_keyword: Self, anonymous_class_left_paren: Self, anonymous_class_argument_list: Self, anonymous_class_right_paren: Self, anonymous_class_extends_keyword: Self, anonymous_class_extends_list: Self, anonymous_class_implements_keyword: Self, anonymous_class_implements_list: Self, anonymous_class_body: Self) -> Self {
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

    fn make_anonymous_function(anonymous_attribute_spec: Self, anonymous_static_keyword: Self, anonymous_async_keyword: Self, anonymous_coroutine_keyword: Self, anonymous_function_keyword: Self, anonymous_left_paren: Self, anonymous_parameters: Self, anonymous_right_paren: Self, anonymous_colon: Self, anonymous_type: Self, anonymous_use: Self, anonymous_body: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunction(Box::new(AnonymousFunctionChildren {
            anonymous_attribute_spec,
            anonymous_static_keyword,
            anonymous_async_keyword,
            anonymous_coroutine_keyword,
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

    fn make_php7_anonymous_function(php7_anonymous_attribute_spec: Self, php7_anonymous_static_keyword: Self, php7_anonymous_async_keyword: Self, php7_anonymous_coroutine_keyword: Self, php7_anonymous_function_keyword: Self, php7_anonymous_left_paren: Self, php7_anonymous_parameters: Self, php7_anonymous_right_paren: Self, php7_anonymous_use: Self, php7_anonymous_colon: Self, php7_anonymous_type: Self, php7_anonymous_body: Self) -> Self {
        let syntax = SyntaxVariant::Php7AnonymousFunction(Box::new(Php7AnonymousFunctionChildren {
            php7_anonymous_attribute_spec,
            php7_anonymous_static_keyword,
            php7_anonymous_async_keyword,
            php7_anonymous_coroutine_keyword,
            php7_anonymous_function_keyword,
            php7_anonymous_left_paren,
            php7_anonymous_parameters,
            php7_anonymous_right_paren,
            php7_anonymous_use,
            php7_anonymous_colon,
            php7_anonymous_type,
            php7_anonymous_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_anonymous_function_use_clause(anonymous_use_keyword: Self, anonymous_use_left_paren: Self, anonymous_use_variables: Self, anonymous_use_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::AnonymousFunctionUseClause(Box::new(AnonymousFunctionUseClauseChildren {
            anonymous_use_keyword,
            anonymous_use_left_paren,
            anonymous_use_variables,
            anonymous_use_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_lambda_expression(lambda_attribute_spec: Self, lambda_async: Self, lambda_coroutine: Self, lambda_signature: Self, lambda_arrow: Self, lambda_body: Self) -> Self {
        let syntax = SyntaxVariant::LambdaExpression(Box::new(LambdaExpressionChildren {
            lambda_attribute_spec,
            lambda_async,
            lambda_coroutine,
            lambda_signature,
            lambda_arrow,
            lambda_body,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_lambda_signature(lambda_left_paren: Self, lambda_parameters: Self, lambda_right_paren: Self, lambda_colon: Self, lambda_type: Self) -> Self {
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

    fn make_cast_expression(cast_left_paren: Self, cast_type: Self, cast_right_paren: Self, cast_operand: Self) -> Self {
        let syntax = SyntaxVariant::CastExpression(Box::new(CastExpressionChildren {
            cast_left_paren,
            cast_type,
            cast_right_paren,
            cast_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_scope_resolution_expression(scope_resolution_qualifier: Self, scope_resolution_operator: Self, scope_resolution_name: Self) -> Self {
        let syntax = SyntaxVariant::ScopeResolutionExpression(Box::new(ScopeResolutionExpressionChildren {
            scope_resolution_qualifier,
            scope_resolution_operator,
            scope_resolution_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_member_selection_expression(member_object: Self, member_operator: Self, member_name: Self) -> Self {
        let syntax = SyntaxVariant::MemberSelectionExpression(Box::new(MemberSelectionExpressionChildren {
            member_object,
            member_operator,
            member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_safe_member_selection_expression(safe_member_object: Self, safe_member_operator: Self, safe_member_name: Self) -> Self {
        let syntax = SyntaxVariant::SafeMemberSelectionExpression(Box::new(SafeMemberSelectionExpressionChildren {
            safe_member_object,
            safe_member_operator,
            safe_member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_member_selection_expression(embedded_member_object: Self, embedded_member_operator: Self, embedded_member_name: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedMemberSelectionExpression(Box::new(EmbeddedMemberSelectionExpressionChildren {
            embedded_member_object,
            embedded_member_operator,
            embedded_member_name,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_yield_expression(yield_keyword: Self, yield_operand: Self) -> Self {
        let syntax = SyntaxVariant::YieldExpression(Box::new(YieldExpressionChildren {
            yield_keyword,
            yield_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_yield_from_expression(yield_from_yield_keyword: Self, yield_from_from_keyword: Self, yield_from_operand: Self) -> Self {
        let syntax = SyntaxVariant::YieldFromExpression(Box::new(YieldFromExpressionChildren {
            yield_from_yield_keyword,
            yield_from_from_keyword,
            yield_from_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_prefix_unary_expression(prefix_unary_operator: Self, prefix_unary_operand: Self) -> Self {
        let syntax = SyntaxVariant::PrefixUnaryExpression(Box::new(PrefixUnaryExpressionChildren {
            prefix_unary_operator,
            prefix_unary_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_postfix_unary_expression(postfix_unary_operand: Self, postfix_unary_operator: Self) -> Self {
        let syntax = SyntaxVariant::PostfixUnaryExpression(Box::new(PostfixUnaryExpressionChildren {
            postfix_unary_operand,
            postfix_unary_operator,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_binary_expression(binary_left_operand: Self, binary_operator: Self, binary_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::BinaryExpression(Box::new(BinaryExpressionChildren {
            binary_left_operand,
            binary_operator,
            binary_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_instanceof_expression(instanceof_left_operand: Self, instanceof_operator: Self, instanceof_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::InstanceofExpression(Box::new(InstanceofExpressionChildren {
            instanceof_left_operand,
            instanceof_operator,
            instanceof_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_is_expression(is_left_operand: Self, is_operator: Self, is_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::IsExpression(Box::new(IsExpressionChildren {
            is_left_operand,
            is_operator,
            is_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_as_expression(as_left_operand: Self, as_operator: Self, as_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::AsExpression(Box::new(AsExpressionChildren {
            as_left_operand,
            as_operator,
            as_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_nullable_as_expression(nullable_as_left_operand: Self, nullable_as_operator: Self, nullable_as_right_operand: Self) -> Self {
        let syntax = SyntaxVariant::NullableAsExpression(Box::new(NullableAsExpressionChildren {
            nullable_as_left_operand,
            nullable_as_operator,
            nullable_as_right_operand,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_conditional_expression(conditional_test: Self, conditional_question: Self, conditional_consequence: Self, conditional_colon: Self, conditional_alternative: Self) -> Self {
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

    fn make_eval_expression(eval_keyword: Self, eval_left_paren: Self, eval_argument: Self, eval_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::EvalExpression(Box::new(EvalExpressionChildren {
            eval_keyword,
            eval_left_paren,
            eval_argument,
            eval_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_empty_expression(empty_keyword: Self, empty_left_paren: Self, empty_argument: Self, empty_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::EmptyExpression(Box::new(EmptyExpressionChildren {
            empty_keyword,
            empty_left_paren,
            empty_argument,
            empty_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_define_expression(define_keyword: Self, define_left_paren: Self, define_argument_list: Self, define_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::DefineExpression(Box::new(DefineExpressionChildren {
            define_keyword,
            define_left_paren,
            define_argument_list,
            define_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_halt_compiler_expression(halt_compiler_keyword: Self, halt_compiler_left_paren: Self, halt_compiler_argument_list: Self, halt_compiler_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::HaltCompilerExpression(Box::new(HaltCompilerExpressionChildren {
            halt_compiler_keyword,
            halt_compiler_left_paren,
            halt_compiler_argument_list,
            halt_compiler_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_isset_expression(isset_keyword: Self, isset_left_paren: Self, isset_argument_list: Self, isset_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::IssetExpression(Box::new(IssetExpressionChildren {
            isset_keyword,
            isset_left_paren,
            isset_argument_list,
            isset_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_function_call_expression(function_call_receiver: Self, function_call_type_args: Self, function_call_left_paren: Self, function_call_argument_list: Self, function_call_right_paren: Self) -> Self {
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

    fn make_parenthesized_expression(parenthesized_expression_left_paren: Self, parenthesized_expression_expression: Self, parenthesized_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ParenthesizedExpression(Box::new(ParenthesizedExpressionChildren {
            parenthesized_expression_left_paren,
            parenthesized_expression_expression,
            parenthesized_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_braced_expression(braced_expression_left_brace: Self, braced_expression_expression: Self, braced_expression_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::BracedExpression(Box::new(BracedExpressionChildren {
            braced_expression_left_brace,
            braced_expression_expression,
            braced_expression_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_braced_expression(embedded_braced_expression_left_brace: Self, embedded_braced_expression_expression: Self, embedded_braced_expression_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedBracedExpression(Box::new(EmbeddedBracedExpressionChildren {
            embedded_braced_expression_left_brace,
            embedded_braced_expression_expression,
            embedded_braced_expression_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_list_expression(list_keyword: Self, list_left_paren: Self, list_members: Self, list_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ListExpression(Box::new(ListExpressionChildren {
            list_keyword,
            list_left_paren,
            list_members,
            list_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_collection_literal_expression(collection_literal_name: Self, collection_literal_left_brace: Self, collection_literal_initializers: Self, collection_literal_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::CollectionLiteralExpression(Box::new(CollectionLiteralExpressionChildren {
            collection_literal_name,
            collection_literal_left_brace,
            collection_literal_initializers,
            collection_literal_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_object_creation_expression(object_creation_new_keyword: Self, object_creation_object: Self) -> Self {
        let syntax = SyntaxVariant::ObjectCreationExpression(Box::new(ObjectCreationExpressionChildren {
            object_creation_new_keyword,
            object_creation_object,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_constructor_call(constructor_call_type: Self, constructor_call_left_paren: Self, constructor_call_argument_list: Self, constructor_call_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ConstructorCall(Box::new(ConstructorCallChildren {
            constructor_call_type,
            constructor_call_left_paren,
            constructor_call_argument_list,
            constructor_call_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_record_creation_expression(record_creation_type: Self, record_creation_left_bracket: Self, record_creation_members: Self, record_creation_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::RecordCreationExpression(Box::new(RecordCreationExpressionChildren {
            record_creation_type,
            record_creation_left_bracket,
            record_creation_members,
            record_creation_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_array_creation_expression(array_creation_left_bracket: Self, array_creation_members: Self, array_creation_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::ArrayCreationExpression(Box::new(ArrayCreationExpressionChildren {
            array_creation_left_bracket,
            array_creation_members,
            array_creation_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_array_intrinsic_expression(array_intrinsic_keyword: Self, array_intrinsic_left_paren: Self, array_intrinsic_members: Self, array_intrinsic_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ArrayIntrinsicExpression(Box::new(ArrayIntrinsicExpressionChildren {
            array_intrinsic_keyword,
            array_intrinsic_left_paren,
            array_intrinsic_members,
            array_intrinsic_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_darray_intrinsic_expression(darray_intrinsic_keyword: Self, darray_intrinsic_explicit_type: Self, darray_intrinsic_left_bracket: Self, darray_intrinsic_members: Self, darray_intrinsic_right_bracket: Self) -> Self {
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

    fn make_dictionary_intrinsic_expression(dictionary_intrinsic_keyword: Self, dictionary_intrinsic_explicit_type: Self, dictionary_intrinsic_left_bracket: Self, dictionary_intrinsic_members: Self, dictionary_intrinsic_right_bracket: Self) -> Self {
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

    fn make_keyset_intrinsic_expression(keyset_intrinsic_keyword: Self, keyset_intrinsic_explicit_type: Self, keyset_intrinsic_left_bracket: Self, keyset_intrinsic_members: Self, keyset_intrinsic_right_bracket: Self) -> Self {
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

    fn make_varray_intrinsic_expression(varray_intrinsic_keyword: Self, varray_intrinsic_explicit_type: Self, varray_intrinsic_left_bracket: Self, varray_intrinsic_members: Self, varray_intrinsic_right_bracket: Self) -> Self {
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

    fn make_vector_intrinsic_expression(vector_intrinsic_keyword: Self, vector_intrinsic_explicit_type: Self, vector_intrinsic_left_bracket: Self, vector_intrinsic_members: Self, vector_intrinsic_right_bracket: Self) -> Self {
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

    fn make_element_initializer(element_key: Self, element_arrow: Self, element_value: Self) -> Self {
        let syntax = SyntaxVariant::ElementInitializer(Box::new(ElementInitializerChildren {
            element_key,
            element_arrow,
            element_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_subscript_expression(subscript_receiver: Self, subscript_left_bracket: Self, subscript_index: Self, subscript_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::SubscriptExpression(Box::new(SubscriptExpressionChildren {
            subscript_receiver,
            subscript_left_bracket,
            subscript_index,
            subscript_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_embedded_subscript_expression(embedded_subscript_receiver: Self, embedded_subscript_left_bracket: Self, embedded_subscript_index: Self, embedded_subscript_right_bracket: Self) -> Self {
        let syntax = SyntaxVariant::EmbeddedSubscriptExpression(Box::new(EmbeddedSubscriptExpressionChildren {
            embedded_subscript_receiver,
            embedded_subscript_left_bracket,
            embedded_subscript_index,
            embedded_subscript_right_bracket,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_awaitable_creation_expression(awaitable_attribute_spec: Self, awaitable_async: Self, awaitable_coroutine: Self, awaitable_compound_statement: Self) -> Self {
        let syntax = SyntaxVariant::AwaitableCreationExpression(Box::new(AwaitableCreationExpressionChildren {
            awaitable_attribute_spec,
            awaitable_async,
            awaitable_coroutine,
            awaitable_compound_statement,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_children_declaration(xhp_children_keyword: Self, xhp_children_expression: Self, xhp_children_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenDeclaration(Box::new(XHPChildrenDeclarationChildren {
            xhp_children_keyword,
            xhp_children_expression,
            xhp_children_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_children_parenthesized_list(xhp_children_list_left_paren: Self, xhp_children_list_xhp_children: Self, xhp_children_list_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::XHPChildrenParenthesizedList(Box::new(XHPChildrenParenthesizedListChildren {
            xhp_children_list_left_paren,
            xhp_children_list_xhp_children,
            xhp_children_list_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_category_declaration(xhp_category_keyword: Self, xhp_category_categories: Self, xhp_category_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPCategoryDeclaration(Box::new(XHPCategoryDeclarationChildren {
            xhp_category_keyword,
            xhp_category_categories,
            xhp_category_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_enum_type(xhp_enum_optional: Self, xhp_enum_keyword: Self, xhp_enum_left_brace: Self, xhp_enum_values: Self, xhp_enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPEnumType(Box::new(XHPEnumTypeChildren {
            xhp_enum_optional,
            xhp_enum_keyword,
            xhp_enum_left_brace,
            xhp_enum_values,
            xhp_enum_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_required(xhp_required_at: Self, xhp_required_keyword: Self) -> Self {
        let syntax = SyntaxVariant::XHPRequired(Box::new(XHPRequiredChildren {
            xhp_required_at,
            xhp_required_keyword,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute_declaration(xhp_attribute_keyword: Self, xhp_attribute_attributes: Self, xhp_attribute_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttributeDeclaration(Box::new(XHPClassAttributeDeclarationChildren {
            xhp_attribute_keyword,
            xhp_attribute_attributes,
            xhp_attribute_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_class_attribute(xhp_attribute_decl_type: Self, xhp_attribute_decl_name: Self, xhp_attribute_decl_initializer: Self, xhp_attribute_decl_required: Self) -> Self {
        let syntax = SyntaxVariant::XHPClassAttribute(Box::new(XHPClassAttributeChildren {
            xhp_attribute_decl_type,
            xhp_attribute_decl_name,
            xhp_attribute_decl_initializer,
            xhp_attribute_decl_required,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_simple_class_attribute(xhp_simple_class_attribute_type: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleClassAttribute(Box::new(XHPSimpleClassAttributeChildren {
            xhp_simple_class_attribute_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_simple_attribute(xhp_simple_attribute_name: Self, xhp_simple_attribute_equal: Self, xhp_simple_attribute_expression: Self) -> Self {
        let syntax = SyntaxVariant::XHPSimpleAttribute(Box::new(XHPSimpleAttributeChildren {
            xhp_simple_attribute_name,
            xhp_simple_attribute_equal,
            xhp_simple_attribute_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_spread_attribute(xhp_spread_attribute_left_brace: Self, xhp_spread_attribute_spread_operator: Self, xhp_spread_attribute_expression: Self, xhp_spread_attribute_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::XHPSpreadAttribute(Box::new(XHPSpreadAttributeChildren {
            xhp_spread_attribute_left_brace,
            xhp_spread_attribute_spread_operator,
            xhp_spread_attribute_expression,
            xhp_spread_attribute_right_brace,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_open(xhp_open_left_angle: Self, xhp_open_name: Self, xhp_open_attributes: Self, xhp_open_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPOpen(Box::new(XHPOpenChildren {
            xhp_open_left_angle,
            xhp_open_name,
            xhp_open_attributes,
            xhp_open_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_expression(xhp_open: Self, xhp_body: Self, xhp_close: Self) -> Self {
        let syntax = SyntaxVariant::XHPExpression(Box::new(XHPExpressionChildren {
            xhp_open,
            xhp_body,
            xhp_close,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_xhp_close(xhp_close_left_angle: Self, xhp_close_name: Self, xhp_close_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::XHPClose(Box::new(XHPCloseChildren {
            xhp_close_left_angle,
            xhp_close_name,
            xhp_close_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_constant(type_constant_left_type: Self, type_constant_separator: Self, type_constant_right_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstant(Box::new(TypeConstantChildren {
            type_constant_left_type,
            type_constant_separator,
            type_constant_right_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_vector_type_specifier(vector_type_keyword: Self, vector_type_left_angle: Self, vector_type_type: Self, vector_type_trailing_comma: Self, vector_type_right_angle: Self) -> Self {
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

    fn make_keyset_type_specifier(keyset_type_keyword: Self, keyset_type_left_angle: Self, keyset_type_type: Self, keyset_type_trailing_comma: Self, keyset_type_right_angle: Self) -> Self {
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

    fn make_tuple_type_explicit_specifier(tuple_type_keyword: Self, tuple_type_left_angle: Self, tuple_type_types: Self, tuple_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeExplicitSpecifier(Box::new(TupleTypeExplicitSpecifierChildren {
            tuple_type_keyword,
            tuple_type_left_angle,
            tuple_type_types,
            tuple_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_varray_type_specifier(varray_keyword: Self, varray_left_angle: Self, varray_type: Self, varray_trailing_comma: Self, varray_right_angle: Self) -> Self {
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

    fn make_vector_array_type_specifier(vector_array_keyword: Self, vector_array_left_angle: Self, vector_array_type: Self, vector_array_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::VectorArrayTypeSpecifier(Box::new(VectorArrayTypeSpecifierChildren {
            vector_array_keyword,
            vector_array_left_angle,
            vector_array_type,
            vector_array_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_parameter(type_attribute_spec: Self, type_reified: Self, type_variance: Self, type_name: Self, type_constraints: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameter(Box::new(TypeParameterChildren {
            type_attribute_spec,
            type_reified,
            type_variance,
            type_name,
            type_constraints,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_constraint(constraint_keyword: Self, constraint_type: Self) -> Self {
        let syntax = SyntaxVariant::TypeConstraint(Box::new(TypeConstraintChildren {
            constraint_keyword,
            constraint_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_darray_type_specifier(darray_keyword: Self, darray_left_angle: Self, darray_key: Self, darray_comma: Self, darray_value: Self, darray_trailing_comma: Self, darray_right_angle: Self) -> Self {
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

    fn make_map_array_type_specifier(map_array_keyword: Self, map_array_left_angle: Self, map_array_key: Self, map_array_comma: Self, map_array_value: Self, map_array_right_angle: Self) -> Self {
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

    fn make_dictionary_type_specifier(dictionary_type_keyword: Self, dictionary_type_left_angle: Self, dictionary_type_members: Self, dictionary_type_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::DictionaryTypeSpecifier(Box::new(DictionaryTypeSpecifierChildren {
            dictionary_type_keyword,
            dictionary_type_left_angle,
            dictionary_type_members,
            dictionary_type_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_closure_type_specifier(closure_outer_left_paren: Self, closure_coroutine: Self, closure_function_keyword: Self, closure_inner_left_paren: Self, closure_parameter_list: Self, closure_inner_right_paren: Self, closure_colon: Self, closure_return_type: Self, closure_outer_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ClosureTypeSpecifier(Box::new(ClosureTypeSpecifierChildren {
            closure_outer_left_paren,
            closure_coroutine,
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

    fn make_closure_parameter_type_specifier(closure_parameter_call_convention: Self, closure_parameter_type: Self) -> Self {
        let syntax = SyntaxVariant::ClosureParameterTypeSpecifier(Box::new(ClosureParameterTypeSpecifierChildren {
            closure_parameter_call_convention,
            closure_parameter_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_classname_type_specifier(classname_keyword: Self, classname_left_angle: Self, classname_type: Self, classname_trailing_comma: Self, classname_right_angle: Self) -> Self {
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

    fn make_field_specifier(field_question: Self, field_name: Self, field_arrow: Self, field_type: Self) -> Self {
        let syntax = SyntaxVariant::FieldSpecifier(Box::new(FieldSpecifierChildren {
            field_question,
            field_name,
            field_arrow,
            field_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_field_initializer(field_initializer_name: Self, field_initializer_arrow: Self, field_initializer_value: Self) -> Self {
        let syntax = SyntaxVariant::FieldInitializer(Box::new(FieldInitializerChildren {
            field_initializer_name,
            field_initializer_arrow,
            field_initializer_value,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_shape_type_specifier(shape_type_keyword: Self, shape_type_left_paren: Self, shape_type_fields: Self, shape_type_ellipsis: Self, shape_type_right_paren: Self) -> Self {
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

    fn make_shape_expression(shape_expression_keyword: Self, shape_expression_left_paren: Self, shape_expression_fields: Self, shape_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::ShapeExpression(Box::new(ShapeExpressionChildren {
            shape_expression_keyword,
            shape_expression_left_paren,
            shape_expression_fields,
            shape_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_tuple_expression(tuple_expression_keyword: Self, tuple_expression_left_paren: Self, tuple_expression_items: Self, tuple_expression_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleExpression(Box::new(TupleExpressionChildren {
            tuple_expression_keyword,
            tuple_expression_left_paren,
            tuple_expression_items,
            tuple_expression_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_generic_type_specifier(generic_class_type: Self, generic_argument_list: Self) -> Self {
        let syntax = SyntaxVariant::GenericTypeSpecifier(Box::new(GenericTypeSpecifierChildren {
            generic_class_type,
            generic_argument_list,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_nullable_type_specifier(nullable_question: Self, nullable_type: Self) -> Self {
        let syntax = SyntaxVariant::NullableTypeSpecifier(Box::new(NullableTypeSpecifierChildren {
            nullable_question,
            nullable_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_like_type_specifier(like_tilde: Self, like_type: Self) -> Self {
        let syntax = SyntaxVariant::LikeTypeSpecifier(Box::new(LikeTypeSpecifierChildren {
            like_tilde,
            like_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_soft_type_specifier(soft_at: Self, soft_type: Self) -> Self {
        let syntax = SyntaxVariant::SoftTypeSpecifier(Box::new(SoftTypeSpecifierChildren {
            soft_at,
            soft_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_reified_type_argument(reified_type_argument_reified: Self, reified_type_argument_type: Self) -> Self {
        let syntax = SyntaxVariant::ReifiedTypeArgument(Box::new(ReifiedTypeArgumentChildren {
            reified_type_argument_reified,
            reified_type_argument_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_arguments(type_arguments_left_angle: Self, type_arguments_types: Self, type_arguments_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeArguments(Box::new(TypeArgumentsChildren {
            type_arguments_left_angle,
            type_arguments_types,
            type_arguments_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_type_parameters(type_parameters_left_angle: Self, type_parameters_parameters: Self, type_parameters_right_angle: Self) -> Self {
        let syntax = SyntaxVariant::TypeParameters(Box::new(TypeParametersChildren {
            type_parameters_left_angle,
            type_parameters_parameters,
            type_parameters_right_angle,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_tuple_type_specifier(tuple_left_paren: Self, tuple_types: Self, tuple_right_paren: Self) -> Self {
        let syntax = SyntaxVariant::TupleTypeSpecifier(Box::new(TupleTypeSpecifierChildren {
            tuple_left_paren,
            tuple_types,
            tuple_right_paren,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_error(error_error: Self) -> Self {
        let syntax = SyntaxVariant::ErrorSyntax(Box::new(ErrorSyntaxChildren {
            error_error,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_list_item(list_item: Self, list_separator: Self) -> Self {
        let syntax = SyntaxVariant::ListItem(Box::new(ListItemChildren {
            list_item,
            list_separator,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_atom_expression(pocket_atom_glyph: Self, pocket_atom_expression: Self) -> Self {
        let syntax = SyntaxVariant::PocketAtomExpression(Box::new(PocketAtomExpressionChildren {
            pocket_atom_glyph,
            pocket_atom_expression,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_identifier_expression(pocket_identifier_qualifier: Self, pocket_identifier_pu_operator: Self, pocket_identifier_field: Self, pocket_identifier_operator: Self, pocket_identifier_name: Self) -> Self {
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

    fn make_pocket_atom_mapping_declaration(pocket_atom_mapping_glyph: Self, pocket_atom_mapping_name: Self, pocket_atom_mapping_left_paren: Self, pocket_atom_mapping_mappings: Self, pocket_atom_mapping_right_paren: Self, pocket_atom_mapping_semicolon: Self) -> Self {
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

    fn make_pocket_enum_declaration(pocket_enum_modifiers: Self, pocket_enum_enum: Self, pocket_enum_name: Self, pocket_enum_left_brace: Self, pocket_enum_fields: Self, pocket_enum_right_brace: Self) -> Self {
        let syntax = SyntaxVariant::PocketEnumDeclaration(Box::new(PocketEnumDeclarationChildren {
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

    fn make_pocket_field_type_expr_declaration(pocket_field_type_expr_case: Self, pocket_field_type_expr_type: Self, pocket_field_type_expr_name: Self, pocket_field_type_expr_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeExprDeclaration(Box::new(PocketFieldTypeExprDeclarationChildren {
            pocket_field_type_expr_case,
            pocket_field_type_expr_type,
            pocket_field_type_expr_name,
            pocket_field_type_expr_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_field_type_declaration(pocket_field_type_case: Self, pocket_field_type_type: Self, pocket_field_type_name: Self, pocket_field_type_semicolon: Self) -> Self {
        let syntax = SyntaxVariant::PocketFieldTypeDeclaration(Box::new(PocketFieldTypeDeclarationChildren {
            pocket_field_type_case,
            pocket_field_type_type,
            pocket_field_type_name,
            pocket_field_type_semicolon,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_id_declaration(pocket_mapping_id_name: Self, pocket_mapping_id_initializer: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingIdDeclaration(Box::new(PocketMappingIdDeclarationChildren {
            pocket_mapping_id_name,
            pocket_mapping_id_initializer,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn make_pocket_mapping_type_declaration(pocket_mapping_type_keyword: Self, pocket_mapping_type_name: Self, pocket_mapping_type_equal: Self, pocket_mapping_type_type: Self) -> Self {
        let syntax = SyntaxVariant::PocketMappingTypeDeclaration(Box::new(PocketMappingTypeDeclarationChildren {
            pocket_mapping_type_keyword,
            pocket_mapping_type_name,
            pocket_mapping_type_equal,
            pocket_mapping_type_type,
        }));
        let value = V::from_syntax(&syntax);
        Self::make(syntax, value)
    }

    fn fold_over_children<'a, U>(
        f: &Fn(&'a Self, U) -> U,
        acc: U,
        syntax: &'a SyntaxVariant<T, V>,
    ) -> U {
        match syntax {
            SyntaxVariant::Missing => acc,
            SyntaxVariant::Token (_) => acc,
            SyntaxVariant::SyntaxList(elems) => {
                let mut acc = acc;
                for item in elems.iter() {
                    acc = f(item, acc)
                }
                acc
            },
            SyntaxVariant::EndOfFile(x) => {
                let acc = f(&x.end_of_file_token, acc);
                acc
            },
            SyntaxVariant::Script(x) => {
                let acc = f(&x.script_declarations, acc);
                acc
            },
            SyntaxVariant::QualifiedName(x) => {
                let acc = f(&x.qualified_name_parts, acc);
                acc
            },
            SyntaxVariant::SimpleTypeSpecifier(x) => {
                let acc = f(&x.simple_type_specifier, acc);
                acc
            },
            SyntaxVariant::LiteralExpression(x) => {
                let acc = f(&x.literal_expression, acc);
                acc
            },
            SyntaxVariant::PrefixedStringExpression(x) => {
                let acc = f(&x.prefixed_string_name, acc);
                let acc = f(&x.prefixed_string_str, acc);
                acc
            },
            SyntaxVariant::VariableExpression(x) => {
                let acc = f(&x.variable_expression, acc);
                acc
            },
            SyntaxVariant::PipeVariableExpression(x) => {
                let acc = f(&x.pipe_variable_expression, acc);
                acc
            },
            SyntaxVariant::FileAttributeSpecification(x) => {
                let acc = f(&x.file_attribute_specification_left_double_angle, acc);
                let acc = f(&x.file_attribute_specification_keyword, acc);
                let acc = f(&x.file_attribute_specification_colon, acc);
                let acc = f(&x.file_attribute_specification_attributes, acc);
                let acc = f(&x.file_attribute_specification_right_double_angle, acc);
                acc
            },
            SyntaxVariant::EnumDeclaration(x) => {
                let acc = f(&x.enum_attribute_spec, acc);
                let acc = f(&x.enum_keyword, acc);
                let acc = f(&x.enum_name, acc);
                let acc = f(&x.enum_colon, acc);
                let acc = f(&x.enum_base, acc);
                let acc = f(&x.enum_type, acc);
                let acc = f(&x.enum_left_brace, acc);
                let acc = f(&x.enum_enumerators, acc);
                let acc = f(&x.enum_right_brace, acc);
                acc
            },
            SyntaxVariant::Enumerator(x) => {
                let acc = f(&x.enumerator_name, acc);
                let acc = f(&x.enumerator_equal, acc);
                let acc = f(&x.enumerator_value, acc);
                let acc = f(&x.enumerator_semicolon, acc);
                acc
            },
            SyntaxVariant::RecordDeclaration(x) => {
                let acc = f(&x.record_attribute_spec, acc);
                let acc = f(&x.record_keyword, acc);
                let acc = f(&x.record_name, acc);
                let acc = f(&x.record_left_brace, acc);
                let acc = f(&x.record_fields, acc);
                let acc = f(&x.record_right_brace, acc);
                acc
            },
            SyntaxVariant::RecordField(x) => {
                let acc = f(&x.record_field_name, acc);
                let acc = f(&x.record_field_colon, acc);
                let acc = f(&x.record_field_type, acc);
                let acc = f(&x.record_field_init, acc);
                let acc = f(&x.record_field_comma, acc);
                acc
            },
            SyntaxVariant::AliasDeclaration(x) => {
                let acc = f(&x.alias_attribute_spec, acc);
                let acc = f(&x.alias_keyword, acc);
                let acc = f(&x.alias_name, acc);
                let acc = f(&x.alias_generic_parameter, acc);
                let acc = f(&x.alias_constraint, acc);
                let acc = f(&x.alias_equal, acc);
                let acc = f(&x.alias_type, acc);
                let acc = f(&x.alias_semicolon, acc);
                acc
            },
            SyntaxVariant::PropertyDeclaration(x) => {
                let acc = f(&x.property_attribute_spec, acc);
                let acc = f(&x.property_modifiers, acc);
                let acc = f(&x.property_type, acc);
                let acc = f(&x.property_declarators, acc);
                let acc = f(&x.property_semicolon, acc);
                acc
            },
            SyntaxVariant::PropertyDeclarator(x) => {
                let acc = f(&x.property_name, acc);
                let acc = f(&x.property_initializer, acc);
                acc
            },
            SyntaxVariant::NamespaceDeclaration(x) => {
                let acc = f(&x.namespace_keyword, acc);
                let acc = f(&x.namespace_name, acc);
                let acc = f(&x.namespace_body, acc);
                acc
            },
            SyntaxVariant::NamespaceBody(x) => {
                let acc = f(&x.namespace_left_brace, acc);
                let acc = f(&x.namespace_declarations, acc);
                let acc = f(&x.namespace_right_brace, acc);
                acc
            },
            SyntaxVariant::NamespaceEmptyBody(x) => {
                let acc = f(&x.namespace_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceUseDeclaration(x) => {
                let acc = f(&x.namespace_use_keyword, acc);
                let acc = f(&x.namespace_use_kind, acc);
                let acc = f(&x.namespace_use_clauses, acc);
                let acc = f(&x.namespace_use_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceGroupUseDeclaration(x) => {
                let acc = f(&x.namespace_group_use_keyword, acc);
                let acc = f(&x.namespace_group_use_kind, acc);
                let acc = f(&x.namespace_group_use_prefix, acc);
                let acc = f(&x.namespace_group_use_left_brace, acc);
                let acc = f(&x.namespace_group_use_clauses, acc);
                let acc = f(&x.namespace_group_use_right_brace, acc);
                let acc = f(&x.namespace_group_use_semicolon, acc);
                acc
            },
            SyntaxVariant::NamespaceUseClause(x) => {
                let acc = f(&x.namespace_use_clause_kind, acc);
                let acc = f(&x.namespace_use_name, acc);
                let acc = f(&x.namespace_use_as, acc);
                let acc = f(&x.namespace_use_alias, acc);
                acc
            },
            SyntaxVariant::FunctionDeclaration(x) => {
                let acc = f(&x.function_attribute_spec, acc);
                let acc = f(&x.function_declaration_header, acc);
                let acc = f(&x.function_body, acc);
                acc
            },
            SyntaxVariant::FunctionDeclarationHeader(x) => {
                let acc = f(&x.function_modifiers, acc);
                let acc = f(&x.function_keyword, acc);
                let acc = f(&x.function_name, acc);
                let acc = f(&x.function_type_parameter_list, acc);
                let acc = f(&x.function_left_paren, acc);
                let acc = f(&x.function_parameter_list, acc);
                let acc = f(&x.function_right_paren, acc);
                let acc = f(&x.function_colon, acc);
                let acc = f(&x.function_type, acc);
                let acc = f(&x.function_where_clause, acc);
                acc
            },
            SyntaxVariant::WhereClause(x) => {
                let acc = f(&x.where_clause_keyword, acc);
                let acc = f(&x.where_clause_constraints, acc);
                acc
            },
            SyntaxVariant::WhereConstraint(x) => {
                let acc = f(&x.where_constraint_left_type, acc);
                let acc = f(&x.where_constraint_operator, acc);
                let acc = f(&x.where_constraint_right_type, acc);
                acc
            },
            SyntaxVariant::MethodishDeclaration(x) => {
                let acc = f(&x.methodish_attribute, acc);
                let acc = f(&x.methodish_function_decl_header, acc);
                let acc = f(&x.methodish_function_body, acc);
                let acc = f(&x.methodish_semicolon, acc);
                acc
            },
            SyntaxVariant::MethodishTraitResolution(x) => {
                let acc = f(&x.methodish_trait_attribute, acc);
                let acc = f(&x.methodish_trait_function_decl_header, acc);
                let acc = f(&x.methodish_trait_equal, acc);
                let acc = f(&x.methodish_trait_name, acc);
                let acc = f(&x.methodish_trait_semicolon, acc);
                acc
            },
            SyntaxVariant::ClassishDeclaration(x) => {
                let acc = f(&x.classish_attribute, acc);
                let acc = f(&x.classish_modifiers, acc);
                let acc = f(&x.classish_keyword, acc);
                let acc = f(&x.classish_name, acc);
                let acc = f(&x.classish_type_parameters, acc);
                let acc = f(&x.classish_extends_keyword, acc);
                let acc = f(&x.classish_extends_list, acc);
                let acc = f(&x.classish_implements_keyword, acc);
                let acc = f(&x.classish_implements_list, acc);
                let acc = f(&x.classish_body, acc);
                acc
            },
            SyntaxVariant::ClassishBody(x) => {
                let acc = f(&x.classish_body_left_brace, acc);
                let acc = f(&x.classish_body_elements, acc);
                let acc = f(&x.classish_body_right_brace, acc);
                acc
            },
            SyntaxVariant::TraitUsePrecedenceItem(x) => {
                let acc = f(&x.trait_use_precedence_item_name, acc);
                let acc = f(&x.trait_use_precedence_item_keyword, acc);
                let acc = f(&x.trait_use_precedence_item_removed_names, acc);
                acc
            },
            SyntaxVariant::TraitUseAliasItem(x) => {
                let acc = f(&x.trait_use_alias_item_aliasing_name, acc);
                let acc = f(&x.trait_use_alias_item_keyword, acc);
                let acc = f(&x.trait_use_alias_item_modifiers, acc);
                let acc = f(&x.trait_use_alias_item_aliased_name, acc);
                acc
            },
            SyntaxVariant::TraitUseConflictResolution(x) => {
                let acc = f(&x.trait_use_conflict_resolution_keyword, acc);
                let acc = f(&x.trait_use_conflict_resolution_names, acc);
                let acc = f(&x.trait_use_conflict_resolution_left_brace, acc);
                let acc = f(&x.trait_use_conflict_resolution_clauses, acc);
                let acc = f(&x.trait_use_conflict_resolution_right_brace, acc);
                acc
            },
            SyntaxVariant::TraitUse(x) => {
                let acc = f(&x.trait_use_keyword, acc);
                let acc = f(&x.trait_use_names, acc);
                let acc = f(&x.trait_use_semicolon, acc);
                acc
            },
            SyntaxVariant::RequireClause(x) => {
                let acc = f(&x.require_keyword, acc);
                let acc = f(&x.require_kind, acc);
                let acc = f(&x.require_name, acc);
                let acc = f(&x.require_semicolon, acc);
                acc
            },
            SyntaxVariant::ConstDeclaration(x) => {
                let acc = f(&x.const_visibility, acc);
                let acc = f(&x.const_abstract, acc);
                let acc = f(&x.const_keyword, acc);
                let acc = f(&x.const_type_specifier, acc);
                let acc = f(&x.const_declarators, acc);
                let acc = f(&x.const_semicolon, acc);
                acc
            },
            SyntaxVariant::ConstantDeclarator(x) => {
                let acc = f(&x.constant_declarator_name, acc);
                let acc = f(&x.constant_declarator_initializer, acc);
                acc
            },
            SyntaxVariant::TypeConstDeclaration(x) => {
                let acc = f(&x.type_const_attribute_spec, acc);
                let acc = f(&x.type_const_abstract, acc);
                let acc = f(&x.type_const_keyword, acc);
                let acc = f(&x.type_const_type_keyword, acc);
                let acc = f(&x.type_const_name, acc);
                let acc = f(&x.type_const_type_parameters, acc);
                let acc = f(&x.type_const_type_constraint, acc);
                let acc = f(&x.type_const_equal, acc);
                let acc = f(&x.type_const_type_specifier, acc);
                let acc = f(&x.type_const_semicolon, acc);
                acc
            },
            SyntaxVariant::DecoratedExpression(x) => {
                let acc = f(&x.decorated_expression_decorator, acc);
                let acc = f(&x.decorated_expression_expression, acc);
                acc
            },
            SyntaxVariant::ParameterDeclaration(x) => {
                let acc = f(&x.parameter_attribute, acc);
                let acc = f(&x.parameter_visibility, acc);
                let acc = f(&x.parameter_call_convention, acc);
                let acc = f(&x.parameter_type, acc);
                let acc = f(&x.parameter_name, acc);
                let acc = f(&x.parameter_default_value, acc);
                acc
            },
            SyntaxVariant::VariadicParameter(x) => {
                let acc = f(&x.variadic_parameter_call_convention, acc);
                let acc = f(&x.variadic_parameter_type, acc);
                let acc = f(&x.variadic_parameter_ellipsis, acc);
                acc
            },
            SyntaxVariant::AttributeSpecification(x) => {
                let acc = f(&x.attribute_specification_left_double_angle, acc);
                let acc = f(&x.attribute_specification_attributes, acc);
                let acc = f(&x.attribute_specification_right_double_angle, acc);
                acc
            },
            SyntaxVariant::InclusionExpression(x) => {
                let acc = f(&x.inclusion_require, acc);
                let acc = f(&x.inclusion_filename, acc);
                acc
            },
            SyntaxVariant::InclusionDirective(x) => {
                let acc = f(&x.inclusion_expression, acc);
                let acc = f(&x.inclusion_semicolon, acc);
                acc
            },
            SyntaxVariant::CompoundStatement(x) => {
                let acc = f(&x.compound_left_brace, acc);
                let acc = f(&x.compound_statements, acc);
                let acc = f(&x.compound_right_brace, acc);
                acc
            },
            SyntaxVariant::AlternateLoopStatement(x) => {
                let acc = f(&x.alternate_loop_opening_colon, acc);
                let acc = f(&x.alternate_loop_statements, acc);
                let acc = f(&x.alternate_loop_closing_keyword, acc);
                let acc = f(&x.alternate_loop_closing_semicolon, acc);
                acc
            },
            SyntaxVariant::ExpressionStatement(x) => {
                let acc = f(&x.expression_statement_expression, acc);
                let acc = f(&x.expression_statement_semicolon, acc);
                acc
            },
            SyntaxVariant::MarkupSection(x) => {
                let acc = f(&x.markup_prefix, acc);
                let acc = f(&x.markup_text, acc);
                let acc = f(&x.markup_suffix, acc);
                let acc = f(&x.markup_expression, acc);
                acc
            },
            SyntaxVariant::MarkupSuffix(x) => {
                let acc = f(&x.markup_suffix_less_than_question, acc);
                let acc = f(&x.markup_suffix_name, acc);
                acc
            },
            SyntaxVariant::UnsetStatement(x) => {
                let acc = f(&x.unset_keyword, acc);
                let acc = f(&x.unset_left_paren, acc);
                let acc = f(&x.unset_variables, acc);
                let acc = f(&x.unset_right_paren, acc);
                let acc = f(&x.unset_semicolon, acc);
                acc
            },
            SyntaxVariant::LetStatement(x) => {
                let acc = f(&x.let_statement_keyword, acc);
                let acc = f(&x.let_statement_name, acc);
                let acc = f(&x.let_statement_colon, acc);
                let acc = f(&x.let_statement_type, acc);
                let acc = f(&x.let_statement_initializer, acc);
                let acc = f(&x.let_statement_semicolon, acc);
                acc
            },
            SyntaxVariant::UsingStatementBlockScoped(x) => {
                let acc = f(&x.using_block_await_keyword, acc);
                let acc = f(&x.using_block_using_keyword, acc);
                let acc = f(&x.using_block_left_paren, acc);
                let acc = f(&x.using_block_expressions, acc);
                let acc = f(&x.using_block_right_paren, acc);
                let acc = f(&x.using_block_body, acc);
                acc
            },
            SyntaxVariant::UsingStatementFunctionScoped(x) => {
                let acc = f(&x.using_function_await_keyword, acc);
                let acc = f(&x.using_function_using_keyword, acc);
                let acc = f(&x.using_function_expression, acc);
                let acc = f(&x.using_function_semicolon, acc);
                acc
            },
            SyntaxVariant::DeclareDirectiveStatement(x) => {
                let acc = f(&x.declare_directive_keyword, acc);
                let acc = f(&x.declare_directive_left_paren, acc);
                let acc = f(&x.declare_directive_expression, acc);
                let acc = f(&x.declare_directive_right_paren, acc);
                let acc = f(&x.declare_directive_semicolon, acc);
                acc
            },
            SyntaxVariant::DeclareBlockStatement(x) => {
                let acc = f(&x.declare_block_keyword, acc);
                let acc = f(&x.declare_block_left_paren, acc);
                let acc = f(&x.declare_block_expression, acc);
                let acc = f(&x.declare_block_right_paren, acc);
                let acc = f(&x.declare_block_body, acc);
                acc
            },
            SyntaxVariant::WhileStatement(x) => {
                let acc = f(&x.while_keyword, acc);
                let acc = f(&x.while_left_paren, acc);
                let acc = f(&x.while_condition, acc);
                let acc = f(&x.while_right_paren, acc);
                let acc = f(&x.while_body, acc);
                acc
            },
            SyntaxVariant::IfStatement(x) => {
                let acc = f(&x.if_keyword, acc);
                let acc = f(&x.if_left_paren, acc);
                let acc = f(&x.if_condition, acc);
                let acc = f(&x.if_right_paren, acc);
                let acc = f(&x.if_statement, acc);
                let acc = f(&x.if_elseif_clauses, acc);
                let acc = f(&x.if_else_clause, acc);
                acc
            },
            SyntaxVariant::ElseifClause(x) => {
                let acc = f(&x.elseif_keyword, acc);
                let acc = f(&x.elseif_left_paren, acc);
                let acc = f(&x.elseif_condition, acc);
                let acc = f(&x.elseif_right_paren, acc);
                let acc = f(&x.elseif_statement, acc);
                acc
            },
            SyntaxVariant::ElseClause(x) => {
                let acc = f(&x.else_keyword, acc);
                let acc = f(&x.else_statement, acc);
                acc
            },
            SyntaxVariant::AlternateIfStatement(x) => {
                let acc = f(&x.alternate_if_keyword, acc);
                let acc = f(&x.alternate_if_left_paren, acc);
                let acc = f(&x.alternate_if_condition, acc);
                let acc = f(&x.alternate_if_right_paren, acc);
                let acc = f(&x.alternate_if_colon, acc);
                let acc = f(&x.alternate_if_statement, acc);
                let acc = f(&x.alternate_if_elseif_clauses, acc);
                let acc = f(&x.alternate_if_else_clause, acc);
                let acc = f(&x.alternate_if_endif_keyword, acc);
                let acc = f(&x.alternate_if_semicolon, acc);
                acc
            },
            SyntaxVariant::AlternateElseifClause(x) => {
                let acc = f(&x.alternate_elseif_keyword, acc);
                let acc = f(&x.alternate_elseif_left_paren, acc);
                let acc = f(&x.alternate_elseif_condition, acc);
                let acc = f(&x.alternate_elseif_right_paren, acc);
                let acc = f(&x.alternate_elseif_colon, acc);
                let acc = f(&x.alternate_elseif_statement, acc);
                acc
            },
            SyntaxVariant::AlternateElseClause(x) => {
                let acc = f(&x.alternate_else_keyword, acc);
                let acc = f(&x.alternate_else_colon, acc);
                let acc = f(&x.alternate_else_statement, acc);
                acc
            },
            SyntaxVariant::TryStatement(x) => {
                let acc = f(&x.try_keyword, acc);
                let acc = f(&x.try_compound_statement, acc);
                let acc = f(&x.try_catch_clauses, acc);
                let acc = f(&x.try_finally_clause, acc);
                acc
            },
            SyntaxVariant::CatchClause(x) => {
                let acc = f(&x.catch_keyword, acc);
                let acc = f(&x.catch_left_paren, acc);
                let acc = f(&x.catch_type, acc);
                let acc = f(&x.catch_variable, acc);
                let acc = f(&x.catch_right_paren, acc);
                let acc = f(&x.catch_body, acc);
                acc
            },
            SyntaxVariant::FinallyClause(x) => {
                let acc = f(&x.finally_keyword, acc);
                let acc = f(&x.finally_body, acc);
                acc
            },
            SyntaxVariant::DoStatement(x) => {
                let acc = f(&x.do_keyword, acc);
                let acc = f(&x.do_body, acc);
                let acc = f(&x.do_while_keyword, acc);
                let acc = f(&x.do_left_paren, acc);
                let acc = f(&x.do_condition, acc);
                let acc = f(&x.do_right_paren, acc);
                let acc = f(&x.do_semicolon, acc);
                acc
            },
            SyntaxVariant::ForStatement(x) => {
                let acc = f(&x.for_keyword, acc);
                let acc = f(&x.for_left_paren, acc);
                let acc = f(&x.for_initializer, acc);
                let acc = f(&x.for_first_semicolon, acc);
                let acc = f(&x.for_control, acc);
                let acc = f(&x.for_second_semicolon, acc);
                let acc = f(&x.for_end_of_loop, acc);
                let acc = f(&x.for_right_paren, acc);
                let acc = f(&x.for_body, acc);
                acc
            },
            SyntaxVariant::ForeachStatement(x) => {
                let acc = f(&x.foreach_keyword, acc);
                let acc = f(&x.foreach_left_paren, acc);
                let acc = f(&x.foreach_collection, acc);
                let acc = f(&x.foreach_await_keyword, acc);
                let acc = f(&x.foreach_as, acc);
                let acc = f(&x.foreach_key, acc);
                let acc = f(&x.foreach_arrow, acc);
                let acc = f(&x.foreach_value, acc);
                let acc = f(&x.foreach_right_paren, acc);
                let acc = f(&x.foreach_body, acc);
                acc
            },
            SyntaxVariant::SwitchStatement(x) => {
                let acc = f(&x.switch_keyword, acc);
                let acc = f(&x.switch_left_paren, acc);
                let acc = f(&x.switch_expression, acc);
                let acc = f(&x.switch_right_paren, acc);
                let acc = f(&x.switch_left_brace, acc);
                let acc = f(&x.switch_sections, acc);
                let acc = f(&x.switch_right_brace, acc);
                acc
            },
            SyntaxVariant::AlternateSwitchStatement(x) => {
                let acc = f(&x.alternate_switch_keyword, acc);
                let acc = f(&x.alternate_switch_left_paren, acc);
                let acc = f(&x.alternate_switch_expression, acc);
                let acc = f(&x.alternate_switch_right_paren, acc);
                let acc = f(&x.alternate_switch_opening_colon, acc);
                let acc = f(&x.alternate_switch_sections, acc);
                let acc = f(&x.alternate_switch_closing_endswitch, acc);
                let acc = f(&x.alternate_switch_closing_semicolon, acc);
                acc
            },
            SyntaxVariant::SwitchSection(x) => {
                let acc = f(&x.switch_section_labels, acc);
                let acc = f(&x.switch_section_statements, acc);
                let acc = f(&x.switch_section_fallthrough, acc);
                acc
            },
            SyntaxVariant::SwitchFallthrough(x) => {
                let acc = f(&x.fallthrough_keyword, acc);
                let acc = f(&x.fallthrough_semicolon, acc);
                acc
            },
            SyntaxVariant::CaseLabel(x) => {
                let acc = f(&x.case_keyword, acc);
                let acc = f(&x.case_expression, acc);
                let acc = f(&x.case_colon, acc);
                acc
            },
            SyntaxVariant::DefaultLabel(x) => {
                let acc = f(&x.default_keyword, acc);
                let acc = f(&x.default_colon, acc);
                acc
            },
            SyntaxVariant::ReturnStatement(x) => {
                let acc = f(&x.return_keyword, acc);
                let acc = f(&x.return_expression, acc);
                let acc = f(&x.return_semicolon, acc);
                acc
            },
            SyntaxVariant::GotoLabel(x) => {
                let acc = f(&x.goto_label_name, acc);
                let acc = f(&x.goto_label_colon, acc);
                acc
            },
            SyntaxVariant::GotoStatement(x) => {
                let acc = f(&x.goto_statement_keyword, acc);
                let acc = f(&x.goto_statement_label_name, acc);
                let acc = f(&x.goto_statement_semicolon, acc);
                acc
            },
            SyntaxVariant::ThrowStatement(x) => {
                let acc = f(&x.throw_keyword, acc);
                let acc = f(&x.throw_expression, acc);
                let acc = f(&x.throw_semicolon, acc);
                acc
            },
            SyntaxVariant::BreakStatement(x) => {
                let acc = f(&x.break_keyword, acc);
                let acc = f(&x.break_level, acc);
                let acc = f(&x.break_semicolon, acc);
                acc
            },
            SyntaxVariant::ContinueStatement(x) => {
                let acc = f(&x.continue_keyword, acc);
                let acc = f(&x.continue_level, acc);
                let acc = f(&x.continue_semicolon, acc);
                acc
            },
            SyntaxVariant::EchoStatement(x) => {
                let acc = f(&x.echo_keyword, acc);
                let acc = f(&x.echo_expressions, acc);
                let acc = f(&x.echo_semicolon, acc);
                acc
            },
            SyntaxVariant::ConcurrentStatement(x) => {
                let acc = f(&x.concurrent_keyword, acc);
                let acc = f(&x.concurrent_statement, acc);
                acc
            },
            SyntaxVariant::SimpleInitializer(x) => {
                let acc = f(&x.simple_initializer_equal, acc);
                let acc = f(&x.simple_initializer_value, acc);
                acc
            },
            SyntaxVariant::AnonymousClass(x) => {
                let acc = f(&x.anonymous_class_class_keyword, acc);
                let acc = f(&x.anonymous_class_left_paren, acc);
                let acc = f(&x.anonymous_class_argument_list, acc);
                let acc = f(&x.anonymous_class_right_paren, acc);
                let acc = f(&x.anonymous_class_extends_keyword, acc);
                let acc = f(&x.anonymous_class_extends_list, acc);
                let acc = f(&x.anonymous_class_implements_keyword, acc);
                let acc = f(&x.anonymous_class_implements_list, acc);
                let acc = f(&x.anonymous_class_body, acc);
                acc
            },
            SyntaxVariant::AnonymousFunction(x) => {
                let acc = f(&x.anonymous_attribute_spec, acc);
                let acc = f(&x.anonymous_static_keyword, acc);
                let acc = f(&x.anonymous_async_keyword, acc);
                let acc = f(&x.anonymous_coroutine_keyword, acc);
                let acc = f(&x.anonymous_function_keyword, acc);
                let acc = f(&x.anonymous_left_paren, acc);
                let acc = f(&x.anonymous_parameters, acc);
                let acc = f(&x.anonymous_right_paren, acc);
                let acc = f(&x.anonymous_colon, acc);
                let acc = f(&x.anonymous_type, acc);
                let acc = f(&x.anonymous_use, acc);
                let acc = f(&x.anonymous_body, acc);
                acc
            },
            SyntaxVariant::Php7AnonymousFunction(x) => {
                let acc = f(&x.php7_anonymous_attribute_spec, acc);
                let acc = f(&x.php7_anonymous_static_keyword, acc);
                let acc = f(&x.php7_anonymous_async_keyword, acc);
                let acc = f(&x.php7_anonymous_coroutine_keyword, acc);
                let acc = f(&x.php7_anonymous_function_keyword, acc);
                let acc = f(&x.php7_anonymous_left_paren, acc);
                let acc = f(&x.php7_anonymous_parameters, acc);
                let acc = f(&x.php7_anonymous_right_paren, acc);
                let acc = f(&x.php7_anonymous_use, acc);
                let acc = f(&x.php7_anonymous_colon, acc);
                let acc = f(&x.php7_anonymous_type, acc);
                let acc = f(&x.php7_anonymous_body, acc);
                acc
            },
            SyntaxVariant::AnonymousFunctionUseClause(x) => {
                let acc = f(&x.anonymous_use_keyword, acc);
                let acc = f(&x.anonymous_use_left_paren, acc);
                let acc = f(&x.anonymous_use_variables, acc);
                let acc = f(&x.anonymous_use_right_paren, acc);
                acc
            },
            SyntaxVariant::LambdaExpression(x) => {
                let acc = f(&x.lambda_attribute_spec, acc);
                let acc = f(&x.lambda_async, acc);
                let acc = f(&x.lambda_coroutine, acc);
                let acc = f(&x.lambda_signature, acc);
                let acc = f(&x.lambda_arrow, acc);
                let acc = f(&x.lambda_body, acc);
                acc
            },
            SyntaxVariant::LambdaSignature(x) => {
                let acc = f(&x.lambda_left_paren, acc);
                let acc = f(&x.lambda_parameters, acc);
                let acc = f(&x.lambda_right_paren, acc);
                let acc = f(&x.lambda_colon, acc);
                let acc = f(&x.lambda_type, acc);
                acc
            },
            SyntaxVariant::CastExpression(x) => {
                let acc = f(&x.cast_left_paren, acc);
                let acc = f(&x.cast_type, acc);
                let acc = f(&x.cast_right_paren, acc);
                let acc = f(&x.cast_operand, acc);
                acc
            },
            SyntaxVariant::ScopeResolutionExpression(x) => {
                let acc = f(&x.scope_resolution_qualifier, acc);
                let acc = f(&x.scope_resolution_operator, acc);
                let acc = f(&x.scope_resolution_name, acc);
                acc
            },
            SyntaxVariant::MemberSelectionExpression(x) => {
                let acc = f(&x.member_object, acc);
                let acc = f(&x.member_operator, acc);
                let acc = f(&x.member_name, acc);
                acc
            },
            SyntaxVariant::SafeMemberSelectionExpression(x) => {
                let acc = f(&x.safe_member_object, acc);
                let acc = f(&x.safe_member_operator, acc);
                let acc = f(&x.safe_member_name, acc);
                acc
            },
            SyntaxVariant::EmbeddedMemberSelectionExpression(x) => {
                let acc = f(&x.embedded_member_object, acc);
                let acc = f(&x.embedded_member_operator, acc);
                let acc = f(&x.embedded_member_name, acc);
                acc
            },
            SyntaxVariant::YieldExpression(x) => {
                let acc = f(&x.yield_keyword, acc);
                let acc = f(&x.yield_operand, acc);
                acc
            },
            SyntaxVariant::YieldFromExpression(x) => {
                let acc = f(&x.yield_from_yield_keyword, acc);
                let acc = f(&x.yield_from_from_keyword, acc);
                let acc = f(&x.yield_from_operand, acc);
                acc
            },
            SyntaxVariant::PrefixUnaryExpression(x) => {
                let acc = f(&x.prefix_unary_operator, acc);
                let acc = f(&x.prefix_unary_operand, acc);
                acc
            },
            SyntaxVariant::PostfixUnaryExpression(x) => {
                let acc = f(&x.postfix_unary_operand, acc);
                let acc = f(&x.postfix_unary_operator, acc);
                acc
            },
            SyntaxVariant::BinaryExpression(x) => {
                let acc = f(&x.binary_left_operand, acc);
                let acc = f(&x.binary_operator, acc);
                let acc = f(&x.binary_right_operand, acc);
                acc
            },
            SyntaxVariant::InstanceofExpression(x) => {
                let acc = f(&x.instanceof_left_operand, acc);
                let acc = f(&x.instanceof_operator, acc);
                let acc = f(&x.instanceof_right_operand, acc);
                acc
            },
            SyntaxVariant::IsExpression(x) => {
                let acc = f(&x.is_left_operand, acc);
                let acc = f(&x.is_operator, acc);
                let acc = f(&x.is_right_operand, acc);
                acc
            },
            SyntaxVariant::AsExpression(x) => {
                let acc = f(&x.as_left_operand, acc);
                let acc = f(&x.as_operator, acc);
                let acc = f(&x.as_right_operand, acc);
                acc
            },
            SyntaxVariant::NullableAsExpression(x) => {
                let acc = f(&x.nullable_as_left_operand, acc);
                let acc = f(&x.nullable_as_operator, acc);
                let acc = f(&x.nullable_as_right_operand, acc);
                acc
            },
            SyntaxVariant::ConditionalExpression(x) => {
                let acc = f(&x.conditional_test, acc);
                let acc = f(&x.conditional_question, acc);
                let acc = f(&x.conditional_consequence, acc);
                let acc = f(&x.conditional_colon, acc);
                let acc = f(&x.conditional_alternative, acc);
                acc
            },
            SyntaxVariant::EvalExpression(x) => {
                let acc = f(&x.eval_keyword, acc);
                let acc = f(&x.eval_left_paren, acc);
                let acc = f(&x.eval_argument, acc);
                let acc = f(&x.eval_right_paren, acc);
                acc
            },
            SyntaxVariant::EmptyExpression(x) => {
                let acc = f(&x.empty_keyword, acc);
                let acc = f(&x.empty_left_paren, acc);
                let acc = f(&x.empty_argument, acc);
                let acc = f(&x.empty_right_paren, acc);
                acc
            },
            SyntaxVariant::DefineExpression(x) => {
                let acc = f(&x.define_keyword, acc);
                let acc = f(&x.define_left_paren, acc);
                let acc = f(&x.define_argument_list, acc);
                let acc = f(&x.define_right_paren, acc);
                acc
            },
            SyntaxVariant::HaltCompilerExpression(x) => {
                let acc = f(&x.halt_compiler_keyword, acc);
                let acc = f(&x.halt_compiler_left_paren, acc);
                let acc = f(&x.halt_compiler_argument_list, acc);
                let acc = f(&x.halt_compiler_right_paren, acc);
                acc
            },
            SyntaxVariant::IssetExpression(x) => {
                let acc = f(&x.isset_keyword, acc);
                let acc = f(&x.isset_left_paren, acc);
                let acc = f(&x.isset_argument_list, acc);
                let acc = f(&x.isset_right_paren, acc);
                acc
            },
            SyntaxVariant::FunctionCallExpression(x) => {
                let acc = f(&x.function_call_receiver, acc);
                let acc = f(&x.function_call_type_args, acc);
                let acc = f(&x.function_call_left_paren, acc);
                let acc = f(&x.function_call_argument_list, acc);
                let acc = f(&x.function_call_right_paren, acc);
                acc
            },
            SyntaxVariant::ParenthesizedExpression(x) => {
                let acc = f(&x.parenthesized_expression_left_paren, acc);
                let acc = f(&x.parenthesized_expression_expression, acc);
                let acc = f(&x.parenthesized_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::BracedExpression(x) => {
                let acc = f(&x.braced_expression_left_brace, acc);
                let acc = f(&x.braced_expression_expression, acc);
                let acc = f(&x.braced_expression_right_brace, acc);
                acc
            },
            SyntaxVariant::EmbeddedBracedExpression(x) => {
                let acc = f(&x.embedded_braced_expression_left_brace, acc);
                let acc = f(&x.embedded_braced_expression_expression, acc);
                let acc = f(&x.embedded_braced_expression_right_brace, acc);
                acc
            },
            SyntaxVariant::ListExpression(x) => {
                let acc = f(&x.list_keyword, acc);
                let acc = f(&x.list_left_paren, acc);
                let acc = f(&x.list_members, acc);
                let acc = f(&x.list_right_paren, acc);
                acc
            },
            SyntaxVariant::CollectionLiteralExpression(x) => {
                let acc = f(&x.collection_literal_name, acc);
                let acc = f(&x.collection_literal_left_brace, acc);
                let acc = f(&x.collection_literal_initializers, acc);
                let acc = f(&x.collection_literal_right_brace, acc);
                acc
            },
            SyntaxVariant::ObjectCreationExpression(x) => {
                let acc = f(&x.object_creation_new_keyword, acc);
                let acc = f(&x.object_creation_object, acc);
                acc
            },
            SyntaxVariant::ConstructorCall(x) => {
                let acc = f(&x.constructor_call_type, acc);
                let acc = f(&x.constructor_call_left_paren, acc);
                let acc = f(&x.constructor_call_argument_list, acc);
                let acc = f(&x.constructor_call_right_paren, acc);
                acc
            },
            SyntaxVariant::RecordCreationExpression(x) => {
                let acc = f(&x.record_creation_type, acc);
                let acc = f(&x.record_creation_left_bracket, acc);
                let acc = f(&x.record_creation_members, acc);
                let acc = f(&x.record_creation_right_bracket, acc);
                acc
            },
            SyntaxVariant::ArrayCreationExpression(x) => {
                let acc = f(&x.array_creation_left_bracket, acc);
                let acc = f(&x.array_creation_members, acc);
                let acc = f(&x.array_creation_right_bracket, acc);
                acc
            },
            SyntaxVariant::ArrayIntrinsicExpression(x) => {
                let acc = f(&x.array_intrinsic_keyword, acc);
                let acc = f(&x.array_intrinsic_left_paren, acc);
                let acc = f(&x.array_intrinsic_members, acc);
                let acc = f(&x.array_intrinsic_right_paren, acc);
                acc
            },
            SyntaxVariant::DarrayIntrinsicExpression(x) => {
                let acc = f(&x.darray_intrinsic_keyword, acc);
                let acc = f(&x.darray_intrinsic_explicit_type, acc);
                let acc = f(&x.darray_intrinsic_left_bracket, acc);
                let acc = f(&x.darray_intrinsic_members, acc);
                let acc = f(&x.darray_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::DictionaryIntrinsicExpression(x) => {
                let acc = f(&x.dictionary_intrinsic_keyword, acc);
                let acc = f(&x.dictionary_intrinsic_explicit_type, acc);
                let acc = f(&x.dictionary_intrinsic_left_bracket, acc);
                let acc = f(&x.dictionary_intrinsic_members, acc);
                let acc = f(&x.dictionary_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::KeysetIntrinsicExpression(x) => {
                let acc = f(&x.keyset_intrinsic_keyword, acc);
                let acc = f(&x.keyset_intrinsic_explicit_type, acc);
                let acc = f(&x.keyset_intrinsic_left_bracket, acc);
                let acc = f(&x.keyset_intrinsic_members, acc);
                let acc = f(&x.keyset_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::VarrayIntrinsicExpression(x) => {
                let acc = f(&x.varray_intrinsic_keyword, acc);
                let acc = f(&x.varray_intrinsic_explicit_type, acc);
                let acc = f(&x.varray_intrinsic_left_bracket, acc);
                let acc = f(&x.varray_intrinsic_members, acc);
                let acc = f(&x.varray_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::VectorIntrinsicExpression(x) => {
                let acc = f(&x.vector_intrinsic_keyword, acc);
                let acc = f(&x.vector_intrinsic_explicit_type, acc);
                let acc = f(&x.vector_intrinsic_left_bracket, acc);
                let acc = f(&x.vector_intrinsic_members, acc);
                let acc = f(&x.vector_intrinsic_right_bracket, acc);
                acc
            },
            SyntaxVariant::ElementInitializer(x) => {
                let acc = f(&x.element_key, acc);
                let acc = f(&x.element_arrow, acc);
                let acc = f(&x.element_value, acc);
                acc
            },
            SyntaxVariant::SubscriptExpression(x) => {
                let acc = f(&x.subscript_receiver, acc);
                let acc = f(&x.subscript_left_bracket, acc);
                let acc = f(&x.subscript_index, acc);
                let acc = f(&x.subscript_right_bracket, acc);
                acc
            },
            SyntaxVariant::EmbeddedSubscriptExpression(x) => {
                let acc = f(&x.embedded_subscript_receiver, acc);
                let acc = f(&x.embedded_subscript_left_bracket, acc);
                let acc = f(&x.embedded_subscript_index, acc);
                let acc = f(&x.embedded_subscript_right_bracket, acc);
                acc
            },
            SyntaxVariant::AwaitableCreationExpression(x) => {
                let acc = f(&x.awaitable_attribute_spec, acc);
                let acc = f(&x.awaitable_async, acc);
                let acc = f(&x.awaitable_coroutine, acc);
                let acc = f(&x.awaitable_compound_statement, acc);
                acc
            },
            SyntaxVariant::XHPChildrenDeclaration(x) => {
                let acc = f(&x.xhp_children_keyword, acc);
                let acc = f(&x.xhp_children_expression, acc);
                let acc = f(&x.xhp_children_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPChildrenParenthesizedList(x) => {
                let acc = f(&x.xhp_children_list_left_paren, acc);
                let acc = f(&x.xhp_children_list_xhp_children, acc);
                let acc = f(&x.xhp_children_list_right_paren, acc);
                acc
            },
            SyntaxVariant::XHPCategoryDeclaration(x) => {
                let acc = f(&x.xhp_category_keyword, acc);
                let acc = f(&x.xhp_category_categories, acc);
                let acc = f(&x.xhp_category_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPEnumType(x) => {
                let acc = f(&x.xhp_enum_optional, acc);
                let acc = f(&x.xhp_enum_keyword, acc);
                let acc = f(&x.xhp_enum_left_brace, acc);
                let acc = f(&x.xhp_enum_values, acc);
                let acc = f(&x.xhp_enum_right_brace, acc);
                acc
            },
            SyntaxVariant::XHPRequired(x) => {
                let acc = f(&x.xhp_required_at, acc);
                let acc = f(&x.xhp_required_keyword, acc);
                acc
            },
            SyntaxVariant::XHPClassAttributeDeclaration(x) => {
                let acc = f(&x.xhp_attribute_keyword, acc);
                let acc = f(&x.xhp_attribute_attributes, acc);
                let acc = f(&x.xhp_attribute_semicolon, acc);
                acc
            },
            SyntaxVariant::XHPClassAttribute(x) => {
                let acc = f(&x.xhp_attribute_decl_type, acc);
                let acc = f(&x.xhp_attribute_decl_name, acc);
                let acc = f(&x.xhp_attribute_decl_initializer, acc);
                let acc = f(&x.xhp_attribute_decl_required, acc);
                acc
            },
            SyntaxVariant::XHPSimpleClassAttribute(x) => {
                let acc = f(&x.xhp_simple_class_attribute_type, acc);
                acc
            },
            SyntaxVariant::XHPSimpleAttribute(x) => {
                let acc = f(&x.xhp_simple_attribute_name, acc);
                let acc = f(&x.xhp_simple_attribute_equal, acc);
                let acc = f(&x.xhp_simple_attribute_expression, acc);
                acc
            },
            SyntaxVariant::XHPSpreadAttribute(x) => {
                let acc = f(&x.xhp_spread_attribute_left_brace, acc);
                let acc = f(&x.xhp_spread_attribute_spread_operator, acc);
                let acc = f(&x.xhp_spread_attribute_expression, acc);
                let acc = f(&x.xhp_spread_attribute_right_brace, acc);
                acc
            },
            SyntaxVariant::XHPOpen(x) => {
                let acc = f(&x.xhp_open_left_angle, acc);
                let acc = f(&x.xhp_open_name, acc);
                let acc = f(&x.xhp_open_attributes, acc);
                let acc = f(&x.xhp_open_right_angle, acc);
                acc
            },
            SyntaxVariant::XHPExpression(x) => {
                let acc = f(&x.xhp_open, acc);
                let acc = f(&x.xhp_body, acc);
                let acc = f(&x.xhp_close, acc);
                acc
            },
            SyntaxVariant::XHPClose(x) => {
                let acc = f(&x.xhp_close_left_angle, acc);
                let acc = f(&x.xhp_close_name, acc);
                let acc = f(&x.xhp_close_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeConstant(x) => {
                let acc = f(&x.type_constant_left_type, acc);
                let acc = f(&x.type_constant_separator, acc);
                let acc = f(&x.type_constant_right_type, acc);
                acc
            },
            SyntaxVariant::VectorTypeSpecifier(x) => {
                let acc = f(&x.vector_type_keyword, acc);
                let acc = f(&x.vector_type_left_angle, acc);
                let acc = f(&x.vector_type_type, acc);
                let acc = f(&x.vector_type_trailing_comma, acc);
                let acc = f(&x.vector_type_right_angle, acc);
                acc
            },
            SyntaxVariant::KeysetTypeSpecifier(x) => {
                let acc = f(&x.keyset_type_keyword, acc);
                let acc = f(&x.keyset_type_left_angle, acc);
                let acc = f(&x.keyset_type_type, acc);
                let acc = f(&x.keyset_type_trailing_comma, acc);
                let acc = f(&x.keyset_type_right_angle, acc);
                acc
            },
            SyntaxVariant::TupleTypeExplicitSpecifier(x) => {
                let acc = f(&x.tuple_type_keyword, acc);
                let acc = f(&x.tuple_type_left_angle, acc);
                let acc = f(&x.tuple_type_types, acc);
                let acc = f(&x.tuple_type_right_angle, acc);
                acc
            },
            SyntaxVariant::VarrayTypeSpecifier(x) => {
                let acc = f(&x.varray_keyword, acc);
                let acc = f(&x.varray_left_angle, acc);
                let acc = f(&x.varray_type, acc);
                let acc = f(&x.varray_trailing_comma, acc);
                let acc = f(&x.varray_right_angle, acc);
                acc
            },
            SyntaxVariant::VectorArrayTypeSpecifier(x) => {
                let acc = f(&x.vector_array_keyword, acc);
                let acc = f(&x.vector_array_left_angle, acc);
                let acc = f(&x.vector_array_type, acc);
                let acc = f(&x.vector_array_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeParameter(x) => {
                let acc = f(&x.type_attribute_spec, acc);
                let acc = f(&x.type_reified, acc);
                let acc = f(&x.type_variance, acc);
                let acc = f(&x.type_name, acc);
                let acc = f(&x.type_constraints, acc);
                acc
            },
            SyntaxVariant::TypeConstraint(x) => {
                let acc = f(&x.constraint_keyword, acc);
                let acc = f(&x.constraint_type, acc);
                acc
            },
            SyntaxVariant::DarrayTypeSpecifier(x) => {
                let acc = f(&x.darray_keyword, acc);
                let acc = f(&x.darray_left_angle, acc);
                let acc = f(&x.darray_key, acc);
                let acc = f(&x.darray_comma, acc);
                let acc = f(&x.darray_value, acc);
                let acc = f(&x.darray_trailing_comma, acc);
                let acc = f(&x.darray_right_angle, acc);
                acc
            },
            SyntaxVariant::MapArrayTypeSpecifier(x) => {
                let acc = f(&x.map_array_keyword, acc);
                let acc = f(&x.map_array_left_angle, acc);
                let acc = f(&x.map_array_key, acc);
                let acc = f(&x.map_array_comma, acc);
                let acc = f(&x.map_array_value, acc);
                let acc = f(&x.map_array_right_angle, acc);
                acc
            },
            SyntaxVariant::DictionaryTypeSpecifier(x) => {
                let acc = f(&x.dictionary_type_keyword, acc);
                let acc = f(&x.dictionary_type_left_angle, acc);
                let acc = f(&x.dictionary_type_members, acc);
                let acc = f(&x.dictionary_type_right_angle, acc);
                acc
            },
            SyntaxVariant::ClosureTypeSpecifier(x) => {
                let acc = f(&x.closure_outer_left_paren, acc);
                let acc = f(&x.closure_coroutine, acc);
                let acc = f(&x.closure_function_keyword, acc);
                let acc = f(&x.closure_inner_left_paren, acc);
                let acc = f(&x.closure_parameter_list, acc);
                let acc = f(&x.closure_inner_right_paren, acc);
                let acc = f(&x.closure_colon, acc);
                let acc = f(&x.closure_return_type, acc);
                let acc = f(&x.closure_outer_right_paren, acc);
                acc
            },
            SyntaxVariant::ClosureParameterTypeSpecifier(x) => {
                let acc = f(&x.closure_parameter_call_convention, acc);
                let acc = f(&x.closure_parameter_type, acc);
                acc
            },
            SyntaxVariant::ClassnameTypeSpecifier(x) => {
                let acc = f(&x.classname_keyword, acc);
                let acc = f(&x.classname_left_angle, acc);
                let acc = f(&x.classname_type, acc);
                let acc = f(&x.classname_trailing_comma, acc);
                let acc = f(&x.classname_right_angle, acc);
                acc
            },
            SyntaxVariant::FieldSpecifier(x) => {
                let acc = f(&x.field_question, acc);
                let acc = f(&x.field_name, acc);
                let acc = f(&x.field_arrow, acc);
                let acc = f(&x.field_type, acc);
                acc
            },
            SyntaxVariant::FieldInitializer(x) => {
                let acc = f(&x.field_initializer_name, acc);
                let acc = f(&x.field_initializer_arrow, acc);
                let acc = f(&x.field_initializer_value, acc);
                acc
            },
            SyntaxVariant::ShapeTypeSpecifier(x) => {
                let acc = f(&x.shape_type_keyword, acc);
                let acc = f(&x.shape_type_left_paren, acc);
                let acc = f(&x.shape_type_fields, acc);
                let acc = f(&x.shape_type_ellipsis, acc);
                let acc = f(&x.shape_type_right_paren, acc);
                acc
            },
            SyntaxVariant::ShapeExpression(x) => {
                let acc = f(&x.shape_expression_keyword, acc);
                let acc = f(&x.shape_expression_left_paren, acc);
                let acc = f(&x.shape_expression_fields, acc);
                let acc = f(&x.shape_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::TupleExpression(x) => {
                let acc = f(&x.tuple_expression_keyword, acc);
                let acc = f(&x.tuple_expression_left_paren, acc);
                let acc = f(&x.tuple_expression_items, acc);
                let acc = f(&x.tuple_expression_right_paren, acc);
                acc
            },
            SyntaxVariant::GenericTypeSpecifier(x) => {
                let acc = f(&x.generic_class_type, acc);
                let acc = f(&x.generic_argument_list, acc);
                acc
            },
            SyntaxVariant::NullableTypeSpecifier(x) => {
                let acc = f(&x.nullable_question, acc);
                let acc = f(&x.nullable_type, acc);
                acc
            },
            SyntaxVariant::LikeTypeSpecifier(x) => {
                let acc = f(&x.like_tilde, acc);
                let acc = f(&x.like_type, acc);
                acc
            },
            SyntaxVariant::SoftTypeSpecifier(x) => {
                let acc = f(&x.soft_at, acc);
                let acc = f(&x.soft_type, acc);
                acc
            },
            SyntaxVariant::ReifiedTypeArgument(x) => {
                let acc = f(&x.reified_type_argument_reified, acc);
                let acc = f(&x.reified_type_argument_type, acc);
                acc
            },
            SyntaxVariant::TypeArguments(x) => {
                let acc = f(&x.type_arguments_left_angle, acc);
                let acc = f(&x.type_arguments_types, acc);
                let acc = f(&x.type_arguments_right_angle, acc);
                acc
            },
            SyntaxVariant::TypeParameters(x) => {
                let acc = f(&x.type_parameters_left_angle, acc);
                let acc = f(&x.type_parameters_parameters, acc);
                let acc = f(&x.type_parameters_right_angle, acc);
                acc
            },
            SyntaxVariant::TupleTypeSpecifier(x) => {
                let acc = f(&x.tuple_left_paren, acc);
                let acc = f(&x.tuple_types, acc);
                let acc = f(&x.tuple_right_paren, acc);
                acc
            },
            SyntaxVariant::ErrorSyntax(x) => {
                let acc = f(&x.error_error, acc);
                acc
            },
            SyntaxVariant::ListItem(x) => {
                let acc = f(&x.list_item, acc);
                let acc = f(&x.list_separator, acc);
                acc
            },
            SyntaxVariant::PocketAtomExpression(x) => {
                let acc = f(&x.pocket_atom_glyph, acc);
                let acc = f(&x.pocket_atom_expression, acc);
                acc
            },
            SyntaxVariant::PocketIdentifierExpression(x) => {
                let acc = f(&x.pocket_identifier_qualifier, acc);
                let acc = f(&x.pocket_identifier_pu_operator, acc);
                let acc = f(&x.pocket_identifier_field, acc);
                let acc = f(&x.pocket_identifier_operator, acc);
                let acc = f(&x.pocket_identifier_name, acc);
                acc
            },
            SyntaxVariant::PocketAtomMappingDeclaration(x) => {
                let acc = f(&x.pocket_atom_mapping_glyph, acc);
                let acc = f(&x.pocket_atom_mapping_name, acc);
                let acc = f(&x.pocket_atom_mapping_left_paren, acc);
                let acc = f(&x.pocket_atom_mapping_mappings, acc);
                let acc = f(&x.pocket_atom_mapping_right_paren, acc);
                let acc = f(&x.pocket_atom_mapping_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketEnumDeclaration(x) => {
                let acc = f(&x.pocket_enum_modifiers, acc);
                let acc = f(&x.pocket_enum_enum, acc);
                let acc = f(&x.pocket_enum_name, acc);
                let acc = f(&x.pocket_enum_left_brace, acc);
                let acc = f(&x.pocket_enum_fields, acc);
                let acc = f(&x.pocket_enum_right_brace, acc);
                acc
            },
            SyntaxVariant::PocketFieldTypeExprDeclaration(x) => {
                let acc = f(&x.pocket_field_type_expr_case, acc);
                let acc = f(&x.pocket_field_type_expr_type, acc);
                let acc = f(&x.pocket_field_type_expr_name, acc);
                let acc = f(&x.pocket_field_type_expr_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketFieldTypeDeclaration(x) => {
                let acc = f(&x.pocket_field_type_case, acc);
                let acc = f(&x.pocket_field_type_type, acc);
                let acc = f(&x.pocket_field_type_name, acc);
                let acc = f(&x.pocket_field_type_semicolon, acc);
                acc
            },
            SyntaxVariant::PocketMappingIdDeclaration(x) => {
                let acc = f(&x.pocket_mapping_id_name, acc);
                let acc = f(&x.pocket_mapping_id_initializer, acc);
                acc
            },
            SyntaxVariant::PocketMappingTypeDeclaration(x) => {
                let acc = f(&x.pocket_mapping_type_keyword, acc);
                let acc = f(&x.pocket_mapping_type_name, acc);
                let acc = f(&x.pocket_mapping_type_equal, acc);
                let acc = f(&x.pocket_mapping_type_type, acc);
                acc
            },

        }
    }

    fn kind(&self) -> SyntaxKind  {
        match self.syntax {
            SyntaxVariant::Missing => SyntaxKind::Missing,
            SyntaxVariant::Token (_) => SyntaxKind::Token,
            SyntaxVariant::SyntaxList (_) => SyntaxKind::SyntaxList,
            SyntaxVariant::EndOfFile {..} => SyntaxKind::EndOfFile,
            SyntaxVariant::Script {..} => SyntaxKind::Script,
            SyntaxVariant::QualifiedName {..} => SyntaxKind::QualifiedName,
            SyntaxVariant::SimpleTypeSpecifier {..} => SyntaxKind::SimpleTypeSpecifier,
            SyntaxVariant::LiteralExpression {..} => SyntaxKind::LiteralExpression,
            SyntaxVariant::PrefixedStringExpression {..} => SyntaxKind::PrefixedStringExpression,
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
            SyntaxVariant::NamespaceBody {..} => SyntaxKind::NamespaceBody,
            SyntaxVariant::NamespaceEmptyBody {..} => SyntaxKind::NamespaceEmptyBody,
            SyntaxVariant::NamespaceUseDeclaration {..} => SyntaxKind::NamespaceUseDeclaration,
            SyntaxVariant::NamespaceGroupUseDeclaration {..} => SyntaxKind::NamespaceGroupUseDeclaration,
            SyntaxVariant::NamespaceUseClause {..} => SyntaxKind::NamespaceUseClause,
            SyntaxVariant::FunctionDeclaration {..} => SyntaxKind::FunctionDeclaration,
            SyntaxVariant::FunctionDeclarationHeader {..} => SyntaxKind::FunctionDeclarationHeader,
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
            SyntaxVariant::AttributeSpecification {..} => SyntaxKind::AttributeSpecification,
            SyntaxVariant::InclusionExpression {..} => SyntaxKind::InclusionExpression,
            SyntaxVariant::InclusionDirective {..} => SyntaxKind::InclusionDirective,
            SyntaxVariant::CompoundStatement {..} => SyntaxKind::CompoundStatement,
            SyntaxVariant::AlternateLoopStatement {..} => SyntaxKind::AlternateLoopStatement,
            SyntaxVariant::ExpressionStatement {..} => SyntaxKind::ExpressionStatement,
            SyntaxVariant::MarkupSection {..} => SyntaxKind::MarkupSection,
            SyntaxVariant::MarkupSuffix {..} => SyntaxKind::MarkupSuffix,
            SyntaxVariant::UnsetStatement {..} => SyntaxKind::UnsetStatement,
            SyntaxVariant::LetStatement {..} => SyntaxKind::LetStatement,
            SyntaxVariant::UsingStatementBlockScoped {..} => SyntaxKind::UsingStatementBlockScoped,
            SyntaxVariant::UsingStatementFunctionScoped {..} => SyntaxKind::UsingStatementFunctionScoped,
            SyntaxVariant::DeclareDirectiveStatement {..} => SyntaxKind::DeclareDirectiveStatement,
            SyntaxVariant::DeclareBlockStatement {..} => SyntaxKind::DeclareBlockStatement,
            SyntaxVariant::WhileStatement {..} => SyntaxKind::WhileStatement,
            SyntaxVariant::IfStatement {..} => SyntaxKind::IfStatement,
            SyntaxVariant::ElseifClause {..} => SyntaxKind::ElseifClause,
            SyntaxVariant::ElseClause {..} => SyntaxKind::ElseClause,
            SyntaxVariant::AlternateIfStatement {..} => SyntaxKind::AlternateIfStatement,
            SyntaxVariant::AlternateElseifClause {..} => SyntaxKind::AlternateElseifClause,
            SyntaxVariant::AlternateElseClause {..} => SyntaxKind::AlternateElseClause,
            SyntaxVariant::TryStatement {..} => SyntaxKind::TryStatement,
            SyntaxVariant::CatchClause {..} => SyntaxKind::CatchClause,
            SyntaxVariant::FinallyClause {..} => SyntaxKind::FinallyClause,
            SyntaxVariant::DoStatement {..} => SyntaxKind::DoStatement,
            SyntaxVariant::ForStatement {..} => SyntaxKind::ForStatement,
            SyntaxVariant::ForeachStatement {..} => SyntaxKind::ForeachStatement,
            SyntaxVariant::SwitchStatement {..} => SyntaxKind::SwitchStatement,
            SyntaxVariant::AlternateSwitchStatement {..} => SyntaxKind::AlternateSwitchStatement,
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
            SyntaxVariant::Php7AnonymousFunction {..} => SyntaxKind::Php7AnonymousFunction,
            SyntaxVariant::AnonymousFunctionUseClause {..} => SyntaxKind::AnonymousFunctionUseClause,
            SyntaxVariant::LambdaExpression {..} => SyntaxKind::LambdaExpression,
            SyntaxVariant::LambdaSignature {..} => SyntaxKind::LambdaSignature,
            SyntaxVariant::CastExpression {..} => SyntaxKind::CastExpression,
            SyntaxVariant::ScopeResolutionExpression {..} => SyntaxKind::ScopeResolutionExpression,
            SyntaxVariant::MemberSelectionExpression {..} => SyntaxKind::MemberSelectionExpression,
            SyntaxVariant::SafeMemberSelectionExpression {..} => SyntaxKind::SafeMemberSelectionExpression,
            SyntaxVariant::EmbeddedMemberSelectionExpression {..} => SyntaxKind::EmbeddedMemberSelectionExpression,
            SyntaxVariant::YieldExpression {..} => SyntaxKind::YieldExpression,
            SyntaxVariant::YieldFromExpression {..} => SyntaxKind::YieldFromExpression,
            SyntaxVariant::PrefixUnaryExpression {..} => SyntaxKind::PrefixUnaryExpression,
            SyntaxVariant::PostfixUnaryExpression {..} => SyntaxKind::PostfixUnaryExpression,
            SyntaxVariant::BinaryExpression {..} => SyntaxKind::BinaryExpression,
            SyntaxVariant::InstanceofExpression {..} => SyntaxKind::InstanceofExpression,
            SyntaxVariant::IsExpression {..} => SyntaxKind::IsExpression,
            SyntaxVariant::AsExpression {..} => SyntaxKind::AsExpression,
            SyntaxVariant::NullableAsExpression {..} => SyntaxKind::NullableAsExpression,
            SyntaxVariant::ConditionalExpression {..} => SyntaxKind::ConditionalExpression,
            SyntaxVariant::EvalExpression {..} => SyntaxKind::EvalExpression,
            SyntaxVariant::EmptyExpression {..} => SyntaxKind::EmptyExpression,
            SyntaxVariant::DefineExpression {..} => SyntaxKind::DefineExpression,
            SyntaxVariant::HaltCompilerExpression {..} => SyntaxKind::HaltCompilerExpression,
            SyntaxVariant::IssetExpression {..} => SyntaxKind::IssetExpression,
            SyntaxVariant::FunctionCallExpression {..} => SyntaxKind::FunctionCallExpression,
            SyntaxVariant::ParenthesizedExpression {..} => SyntaxKind::ParenthesizedExpression,
            SyntaxVariant::BracedExpression {..} => SyntaxKind::BracedExpression,
            SyntaxVariant::EmbeddedBracedExpression {..} => SyntaxKind::EmbeddedBracedExpression,
            SyntaxVariant::ListExpression {..} => SyntaxKind::ListExpression,
            SyntaxVariant::CollectionLiteralExpression {..} => SyntaxKind::CollectionLiteralExpression,
            SyntaxVariant::ObjectCreationExpression {..} => SyntaxKind::ObjectCreationExpression,
            SyntaxVariant::ConstructorCall {..} => SyntaxKind::ConstructorCall,
            SyntaxVariant::RecordCreationExpression {..} => SyntaxKind::RecordCreationExpression,
            SyntaxVariant::ArrayCreationExpression {..} => SyntaxKind::ArrayCreationExpression,
            SyntaxVariant::ArrayIntrinsicExpression {..} => SyntaxKind::ArrayIntrinsicExpression,
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
            SyntaxVariant::ReifiedTypeArgument {..} => SyntaxKind::ReifiedTypeArgument,
            SyntaxVariant::TypeArguments {..} => SyntaxKind::TypeArguments,
            SyntaxVariant::TypeParameters {..} => SyntaxKind::TypeParameters,
            SyntaxVariant::TupleTypeSpecifier {..} => SyntaxKind::TupleTypeSpecifier,
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
    pub record_keyword: Syntax<T, V>,
    pub record_name: Syntax<T, V>,
    pub record_left_brace: Syntax<T, V>,
    pub record_fields: Syntax<T, V>,
    pub record_right_brace: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct RecordFieldChildren<T, V> {
    pub record_field_name: Syntax<T, V>,
    pub record_field_colon: Syntax<T, V>,
    pub record_field_type: Syntax<T, V>,
    pub record_field_init: Syntax<T, V>,
    pub record_field_comma: Syntax<T, V>,
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
    pub namespace_keyword: Syntax<T, V>,
    pub namespace_name: Syntax<T, V>,
    pub namespace_body: Syntax<T, V>,
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
    pub function_colon: Syntax<T, V>,
    pub function_type: Syntax<T, V>,
    pub function_where_clause: Syntax<T, V>,
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
    pub classish_keyword: Syntax<T, V>,
    pub classish_name: Syntax<T, V>,
    pub classish_type_parameters: Syntax<T, V>,
    pub classish_extends_keyword: Syntax<T, V>,
    pub classish_extends_list: Syntax<T, V>,
    pub classish_implements_keyword: Syntax<T, V>,
    pub classish_implements_list: Syntax<T, V>,
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
    pub const_visibility: Syntax<T, V>,
    pub const_abstract: Syntax<T, V>,
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
    pub type_const_abstract: Syntax<T, V>,
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
pub struct AttributeSpecificationChildren<T, V> {
    pub attribute_specification_left_double_angle: Syntax<T, V>,
    pub attribute_specification_attributes: Syntax<T, V>,
    pub attribute_specification_right_double_angle: Syntax<T, V>,
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
pub struct AlternateLoopStatementChildren<T, V> {
    pub alternate_loop_opening_colon: Syntax<T, V>,
    pub alternate_loop_statements: Syntax<T, V>,
    pub alternate_loop_closing_keyword: Syntax<T, V>,
    pub alternate_loop_closing_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ExpressionStatementChildren<T, V> {
    pub expression_statement_expression: Syntax<T, V>,
    pub expression_statement_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct MarkupSectionChildren<T, V> {
    pub markup_prefix: Syntax<T, V>,
    pub markup_text: Syntax<T, V>,
    pub markup_suffix: Syntax<T, V>,
    pub markup_expression: Syntax<T, V>,
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
pub struct LetStatementChildren<T, V> {
    pub let_statement_keyword: Syntax<T, V>,
    pub let_statement_name: Syntax<T, V>,
    pub let_statement_colon: Syntax<T, V>,
    pub let_statement_type: Syntax<T, V>,
    pub let_statement_initializer: Syntax<T, V>,
    pub let_statement_semicolon: Syntax<T, V>,
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
pub struct DeclareDirectiveStatementChildren<T, V> {
    pub declare_directive_keyword: Syntax<T, V>,
    pub declare_directive_left_paren: Syntax<T, V>,
    pub declare_directive_expression: Syntax<T, V>,
    pub declare_directive_right_paren: Syntax<T, V>,
    pub declare_directive_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DeclareBlockStatementChildren<T, V> {
    pub declare_block_keyword: Syntax<T, V>,
    pub declare_block_left_paren: Syntax<T, V>,
    pub declare_block_expression: Syntax<T, V>,
    pub declare_block_right_paren: Syntax<T, V>,
    pub declare_block_body: Syntax<T, V>,
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
pub struct AlternateIfStatementChildren<T, V> {
    pub alternate_if_keyword: Syntax<T, V>,
    pub alternate_if_left_paren: Syntax<T, V>,
    pub alternate_if_condition: Syntax<T, V>,
    pub alternate_if_right_paren: Syntax<T, V>,
    pub alternate_if_colon: Syntax<T, V>,
    pub alternate_if_statement: Syntax<T, V>,
    pub alternate_if_elseif_clauses: Syntax<T, V>,
    pub alternate_if_else_clause: Syntax<T, V>,
    pub alternate_if_endif_keyword: Syntax<T, V>,
    pub alternate_if_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AlternateElseifClauseChildren<T, V> {
    pub alternate_elseif_keyword: Syntax<T, V>,
    pub alternate_elseif_left_paren: Syntax<T, V>,
    pub alternate_elseif_condition: Syntax<T, V>,
    pub alternate_elseif_right_paren: Syntax<T, V>,
    pub alternate_elseif_colon: Syntax<T, V>,
    pub alternate_elseif_statement: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct AlternateElseClauseChildren<T, V> {
    pub alternate_else_keyword: Syntax<T, V>,
    pub alternate_else_colon: Syntax<T, V>,
    pub alternate_else_statement: Syntax<T, V>,
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
pub struct AlternateSwitchStatementChildren<T, V> {
    pub alternate_switch_keyword: Syntax<T, V>,
    pub alternate_switch_left_paren: Syntax<T, V>,
    pub alternate_switch_expression: Syntax<T, V>,
    pub alternate_switch_right_paren: Syntax<T, V>,
    pub alternate_switch_opening_colon: Syntax<T, V>,
    pub alternate_switch_sections: Syntax<T, V>,
    pub alternate_switch_closing_endswitch: Syntax<T, V>,
    pub alternate_switch_closing_semicolon: Syntax<T, V>,
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
    pub break_level: Syntax<T, V>,
    pub break_semicolon: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ContinueStatementChildren<T, V> {
    pub continue_keyword: Syntax<T, V>,
    pub continue_level: Syntax<T, V>,
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
    pub anonymous_coroutine_keyword: Syntax<T, V>,
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
pub struct Php7AnonymousFunctionChildren<T, V> {
    pub php7_anonymous_attribute_spec: Syntax<T, V>,
    pub php7_anonymous_static_keyword: Syntax<T, V>,
    pub php7_anonymous_async_keyword: Syntax<T, V>,
    pub php7_anonymous_coroutine_keyword: Syntax<T, V>,
    pub php7_anonymous_function_keyword: Syntax<T, V>,
    pub php7_anonymous_left_paren: Syntax<T, V>,
    pub php7_anonymous_parameters: Syntax<T, V>,
    pub php7_anonymous_right_paren: Syntax<T, V>,
    pub php7_anonymous_use: Syntax<T, V>,
    pub php7_anonymous_colon: Syntax<T, V>,
    pub php7_anonymous_type: Syntax<T, V>,
    pub php7_anonymous_body: Syntax<T, V>,
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
    pub lambda_coroutine: Syntax<T, V>,
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
pub struct YieldFromExpressionChildren<T, V> {
    pub yield_from_yield_keyword: Syntax<T, V>,
    pub yield_from_from_keyword: Syntax<T, V>,
    pub yield_from_operand: Syntax<T, V>,
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
pub struct InstanceofExpressionChildren<T, V> {
    pub instanceof_left_operand: Syntax<T, V>,
    pub instanceof_operator: Syntax<T, V>,
    pub instanceof_right_operand: Syntax<T, V>,
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
pub struct EmptyExpressionChildren<T, V> {
    pub empty_keyword: Syntax<T, V>,
    pub empty_left_paren: Syntax<T, V>,
    pub empty_argument: Syntax<T, V>,
    pub empty_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct DefineExpressionChildren<T, V> {
    pub define_keyword: Syntax<T, V>,
    pub define_left_paren: Syntax<T, V>,
    pub define_argument_list: Syntax<T, V>,
    pub define_right_paren: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct HaltCompilerExpressionChildren<T, V> {
    pub halt_compiler_keyword: Syntax<T, V>,
    pub halt_compiler_left_paren: Syntax<T, V>,
    pub halt_compiler_argument_list: Syntax<T, V>,
    pub halt_compiler_right_paren: Syntax<T, V>,
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
pub struct ArrayCreationExpressionChildren<T, V> {
    pub array_creation_left_bracket: Syntax<T, V>,
    pub array_creation_members: Syntax<T, V>,
    pub array_creation_right_bracket: Syntax<T, V>,
}

#[derive(Debug, Clone)]
pub struct ArrayIntrinsicExpressionChildren<T, V> {
    pub array_intrinsic_keyword: Syntax<T, V>,
    pub array_intrinsic_left_paren: Syntax<T, V>,
    pub array_intrinsic_members: Syntax<T, V>,
    pub array_intrinsic_right_paren: Syntax<T, V>,
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
    pub awaitable_coroutine: Syntax<T, V>,
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
    pub xhp_enum_optional: Syntax<T, V>,
    pub xhp_enum_keyword: Syntax<T, V>,
    pub xhp_enum_left_brace: Syntax<T, V>,
    pub xhp_enum_values: Syntax<T, V>,
    pub xhp_enum_right_brace: Syntax<T, V>,
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
    pub closure_coroutine: Syntax<T, V>,
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
    pub pocket_field_type_name: Syntax<T, V>,
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
    SyntaxList(Box<Vec<Syntax<T, V>>>),
    EndOfFile(Box<EndOfFileChildren<T, V>>),
    Script(Box<ScriptChildren<T, V>>),
    QualifiedName(Box<QualifiedNameChildren<T, V>>),
    SimpleTypeSpecifier(Box<SimpleTypeSpecifierChildren<T, V>>),
    LiteralExpression(Box<LiteralExpressionChildren<T, V>>),
    PrefixedStringExpression(Box<PrefixedStringExpressionChildren<T, V>>),
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
    NamespaceBody(Box<NamespaceBodyChildren<T, V>>),
    NamespaceEmptyBody(Box<NamespaceEmptyBodyChildren<T, V>>),
    NamespaceUseDeclaration(Box<NamespaceUseDeclarationChildren<T, V>>),
    NamespaceGroupUseDeclaration(Box<NamespaceGroupUseDeclarationChildren<T, V>>),
    NamespaceUseClause(Box<NamespaceUseClauseChildren<T, V>>),
    FunctionDeclaration(Box<FunctionDeclarationChildren<T, V>>),
    FunctionDeclarationHeader(Box<FunctionDeclarationHeaderChildren<T, V>>),
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
    AttributeSpecification(Box<AttributeSpecificationChildren<T, V>>),
    InclusionExpression(Box<InclusionExpressionChildren<T, V>>),
    InclusionDirective(Box<InclusionDirectiveChildren<T, V>>),
    CompoundStatement(Box<CompoundStatementChildren<T, V>>),
    AlternateLoopStatement(Box<AlternateLoopStatementChildren<T, V>>),
    ExpressionStatement(Box<ExpressionStatementChildren<T, V>>),
    MarkupSection(Box<MarkupSectionChildren<T, V>>),
    MarkupSuffix(Box<MarkupSuffixChildren<T, V>>),
    UnsetStatement(Box<UnsetStatementChildren<T, V>>),
    LetStatement(Box<LetStatementChildren<T, V>>),
    UsingStatementBlockScoped(Box<UsingStatementBlockScopedChildren<T, V>>),
    UsingStatementFunctionScoped(Box<UsingStatementFunctionScopedChildren<T, V>>),
    DeclareDirectiveStatement(Box<DeclareDirectiveStatementChildren<T, V>>),
    DeclareBlockStatement(Box<DeclareBlockStatementChildren<T, V>>),
    WhileStatement(Box<WhileStatementChildren<T, V>>),
    IfStatement(Box<IfStatementChildren<T, V>>),
    ElseifClause(Box<ElseifClauseChildren<T, V>>),
    ElseClause(Box<ElseClauseChildren<T, V>>),
    AlternateIfStatement(Box<AlternateIfStatementChildren<T, V>>),
    AlternateElseifClause(Box<AlternateElseifClauseChildren<T, V>>),
    AlternateElseClause(Box<AlternateElseClauseChildren<T, V>>),
    TryStatement(Box<TryStatementChildren<T, V>>),
    CatchClause(Box<CatchClauseChildren<T, V>>),
    FinallyClause(Box<FinallyClauseChildren<T, V>>),
    DoStatement(Box<DoStatementChildren<T, V>>),
    ForStatement(Box<ForStatementChildren<T, V>>),
    ForeachStatement(Box<ForeachStatementChildren<T, V>>),
    SwitchStatement(Box<SwitchStatementChildren<T, V>>),
    AlternateSwitchStatement(Box<AlternateSwitchStatementChildren<T, V>>),
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
    Php7AnonymousFunction(Box<Php7AnonymousFunctionChildren<T, V>>),
    AnonymousFunctionUseClause(Box<AnonymousFunctionUseClauseChildren<T, V>>),
    LambdaExpression(Box<LambdaExpressionChildren<T, V>>),
    LambdaSignature(Box<LambdaSignatureChildren<T, V>>),
    CastExpression(Box<CastExpressionChildren<T, V>>),
    ScopeResolutionExpression(Box<ScopeResolutionExpressionChildren<T, V>>),
    MemberSelectionExpression(Box<MemberSelectionExpressionChildren<T, V>>),
    SafeMemberSelectionExpression(Box<SafeMemberSelectionExpressionChildren<T, V>>),
    EmbeddedMemberSelectionExpression(Box<EmbeddedMemberSelectionExpressionChildren<T, V>>),
    YieldExpression(Box<YieldExpressionChildren<T, V>>),
    YieldFromExpression(Box<YieldFromExpressionChildren<T, V>>),
    PrefixUnaryExpression(Box<PrefixUnaryExpressionChildren<T, V>>),
    PostfixUnaryExpression(Box<PostfixUnaryExpressionChildren<T, V>>),
    BinaryExpression(Box<BinaryExpressionChildren<T, V>>),
    InstanceofExpression(Box<InstanceofExpressionChildren<T, V>>),
    IsExpression(Box<IsExpressionChildren<T, V>>),
    AsExpression(Box<AsExpressionChildren<T, V>>),
    NullableAsExpression(Box<NullableAsExpressionChildren<T, V>>),
    ConditionalExpression(Box<ConditionalExpressionChildren<T, V>>),
    EvalExpression(Box<EvalExpressionChildren<T, V>>),
    EmptyExpression(Box<EmptyExpressionChildren<T, V>>),
    DefineExpression(Box<DefineExpressionChildren<T, V>>),
    HaltCompilerExpression(Box<HaltCompilerExpressionChildren<T, V>>),
    IssetExpression(Box<IssetExpressionChildren<T, V>>),
    FunctionCallExpression(Box<FunctionCallExpressionChildren<T, V>>),
    ParenthesizedExpression(Box<ParenthesizedExpressionChildren<T, V>>),
    BracedExpression(Box<BracedExpressionChildren<T, V>>),
    EmbeddedBracedExpression(Box<EmbeddedBracedExpressionChildren<T, V>>),
    ListExpression(Box<ListExpressionChildren<T, V>>),
    CollectionLiteralExpression(Box<CollectionLiteralExpressionChildren<T, V>>),
    ObjectCreationExpression(Box<ObjectCreationExpressionChildren<T, V>>),
    ConstructorCall(Box<ConstructorCallChildren<T, V>>),
    RecordCreationExpression(Box<RecordCreationExpressionChildren<T, V>>),
    ArrayCreationExpression(Box<ArrayCreationExpressionChildren<T, V>>),
    ArrayIntrinsicExpression(Box<ArrayIntrinsicExpressionChildren<T, V>>),
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
    ReifiedTypeArgument(Box<ReifiedTypeArgumentChildren<T, V>>),
    TypeArguments(Box<TypeArgumentsChildren<T, V>>),
    TypeParameters(Box<TypeParametersChildren<T, V>>),
    TupleTypeSpecifier(Box<TupleTypeSpecifierChildren<T, V>>),
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
