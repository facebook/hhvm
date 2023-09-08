(*
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
 * With these factory methods, nodes can be built up from their child nodes. A
 * factory method must not just know all the children and the kind of node it is
 * constructing; it also must know how to construct the value that this node is
 * going to be tagged with. For that reason, an optional functor is provided.
 * This functor requires that methods be provided to construct the values
 * associated with a token or with any arbitrary node, given its children. If
 * this functor is used then the resulting module contains factory methods.
 *
 * This module also provides some useful helper functions, like an iterator,
 * a rewriting visitor, and so on.
 *)

open Hh_prelude
open Full_fidelity_syntax_type
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module Operator = Full_fidelity_operator

[@@@warning "-27"] (* unused variable *)

module WithToken (Token : TokenType) = struct
  module WithSyntaxValue (SyntaxValue : SyntaxValueType) = struct
    include MakeSyntaxType (Token) (SyntaxValue)

    let make syntax value = { syntax; value }

    let syntax node = node.syntax

    let value node = node.value

    let syntax_node_to_list node =
      match syntax node with
      | SyntaxList x -> x
      | Missing -> []
      | _ -> [node]

    let to_kind syntax =
      match syntax with
      | Missing -> SyntaxKind.Missing
      | Token t -> SyntaxKind.Token (Token.kind t)
      | SyntaxList _ -> SyntaxKind.SyntaxList
      | EndOfFile _ -> SyntaxKind.EndOfFile
      | Script _ -> SyntaxKind.Script
      | QualifiedName _ -> SyntaxKind.QualifiedName
      | ModuleName _ -> SyntaxKind.ModuleName
      | SimpleTypeSpecifier _ -> SyntaxKind.SimpleTypeSpecifier
      | LiteralExpression _ -> SyntaxKind.LiteralExpression
      | PrefixedStringExpression _ -> SyntaxKind.PrefixedStringExpression
      | PrefixedCodeExpression _ -> SyntaxKind.PrefixedCodeExpression
      | VariableExpression _ -> SyntaxKind.VariableExpression
      | PipeVariableExpression _ -> SyntaxKind.PipeVariableExpression
      | FileAttributeSpecification _ -> SyntaxKind.FileAttributeSpecification
      | EnumDeclaration _ -> SyntaxKind.EnumDeclaration
      | EnumUse _ -> SyntaxKind.EnumUse
      | Enumerator _ -> SyntaxKind.Enumerator
      | EnumClassDeclaration _ -> SyntaxKind.EnumClassDeclaration
      | EnumClassEnumerator _ -> SyntaxKind.EnumClassEnumerator
      | AliasDeclaration _ -> SyntaxKind.AliasDeclaration
      | ContextAliasDeclaration _ -> SyntaxKind.ContextAliasDeclaration
      | CaseTypeDeclaration _ -> SyntaxKind.CaseTypeDeclaration
      | CaseTypeVariant _ -> SyntaxKind.CaseTypeVariant
      | PropertyDeclaration _ -> SyntaxKind.PropertyDeclaration
      | PropertyDeclarator _ -> SyntaxKind.PropertyDeclarator
      | NamespaceDeclaration _ -> SyntaxKind.NamespaceDeclaration
      | NamespaceDeclarationHeader _ -> SyntaxKind.NamespaceDeclarationHeader
      | NamespaceBody _ -> SyntaxKind.NamespaceBody
      | NamespaceEmptyBody _ -> SyntaxKind.NamespaceEmptyBody
      | NamespaceUseDeclaration _ -> SyntaxKind.NamespaceUseDeclaration
      | NamespaceGroupUseDeclaration _ ->
        SyntaxKind.NamespaceGroupUseDeclaration
      | NamespaceUseClause _ -> SyntaxKind.NamespaceUseClause
      | FunctionDeclaration _ -> SyntaxKind.FunctionDeclaration
      | FunctionDeclarationHeader _ -> SyntaxKind.FunctionDeclarationHeader
      | Contexts _ -> SyntaxKind.Contexts
      | WhereClause _ -> SyntaxKind.WhereClause
      | WhereConstraint _ -> SyntaxKind.WhereConstraint
      | MethodishDeclaration _ -> SyntaxKind.MethodishDeclaration
      | MethodishTraitResolution _ -> SyntaxKind.MethodishTraitResolution
      | ClassishDeclaration _ -> SyntaxKind.ClassishDeclaration
      | ClassishBody _ -> SyntaxKind.ClassishBody
      | TraitUse _ -> SyntaxKind.TraitUse
      | RequireClause _ -> SyntaxKind.RequireClause
      | ConstDeclaration _ -> SyntaxKind.ConstDeclaration
      | ConstantDeclarator _ -> SyntaxKind.ConstantDeclarator
      | TypeConstDeclaration _ -> SyntaxKind.TypeConstDeclaration
      | ContextConstDeclaration _ -> SyntaxKind.ContextConstDeclaration
      | DecoratedExpression _ -> SyntaxKind.DecoratedExpression
      | ParameterDeclaration _ -> SyntaxKind.ParameterDeclaration
      | VariadicParameter _ -> SyntaxKind.VariadicParameter
      | OldAttributeSpecification _ -> SyntaxKind.OldAttributeSpecification
      | AttributeSpecification _ -> SyntaxKind.AttributeSpecification
      | Attribute _ -> SyntaxKind.Attribute
      | InclusionExpression _ -> SyntaxKind.InclusionExpression
      | InclusionDirective _ -> SyntaxKind.InclusionDirective
      | CompoundStatement _ -> SyntaxKind.CompoundStatement
      | ExpressionStatement _ -> SyntaxKind.ExpressionStatement
      | MarkupSection _ -> SyntaxKind.MarkupSection
      | MarkupSuffix _ -> SyntaxKind.MarkupSuffix
      | UnsetStatement _ -> SyntaxKind.UnsetStatement
      | DeclareLocalStatement _ -> SyntaxKind.DeclareLocalStatement
      | UsingStatementBlockScoped _ -> SyntaxKind.UsingStatementBlockScoped
      | UsingStatementFunctionScoped _ ->
        SyntaxKind.UsingStatementFunctionScoped
      | WhileStatement _ -> SyntaxKind.WhileStatement
      | IfStatement _ -> SyntaxKind.IfStatement
      | ElseClause _ -> SyntaxKind.ElseClause
      | TryStatement _ -> SyntaxKind.TryStatement
      | CatchClause _ -> SyntaxKind.CatchClause
      | FinallyClause _ -> SyntaxKind.FinallyClause
      | DoStatement _ -> SyntaxKind.DoStatement
      | ForStatement _ -> SyntaxKind.ForStatement
      | ForeachStatement _ -> SyntaxKind.ForeachStatement
      | SwitchStatement _ -> SyntaxKind.SwitchStatement
      | SwitchSection _ -> SyntaxKind.SwitchSection
      | SwitchFallthrough _ -> SyntaxKind.SwitchFallthrough
      | CaseLabel _ -> SyntaxKind.CaseLabel
      | DefaultLabel _ -> SyntaxKind.DefaultLabel
      | MatchStatement _ -> SyntaxKind.MatchStatement
      | MatchStatementArm _ -> SyntaxKind.MatchStatementArm
      | ReturnStatement _ -> SyntaxKind.ReturnStatement
      | YieldBreakStatement _ -> SyntaxKind.YieldBreakStatement
      | ThrowStatement _ -> SyntaxKind.ThrowStatement
      | BreakStatement _ -> SyntaxKind.BreakStatement
      | ContinueStatement _ -> SyntaxKind.ContinueStatement
      | EchoStatement _ -> SyntaxKind.EchoStatement
      | ConcurrentStatement _ -> SyntaxKind.ConcurrentStatement
      | SimpleInitializer _ -> SyntaxKind.SimpleInitializer
      | AnonymousClass _ -> SyntaxKind.AnonymousClass
      | AnonymousFunction _ -> SyntaxKind.AnonymousFunction
      | AnonymousFunctionUseClause _ -> SyntaxKind.AnonymousFunctionUseClause
      | VariablePattern _ -> SyntaxKind.VariablePattern
      | ConstructorPattern _ -> SyntaxKind.ConstructorPattern
      | RefinementPattern _ -> SyntaxKind.RefinementPattern
      | LambdaExpression _ -> SyntaxKind.LambdaExpression
      | LambdaSignature _ -> SyntaxKind.LambdaSignature
      | CastExpression _ -> SyntaxKind.CastExpression
      | ScopeResolutionExpression _ -> SyntaxKind.ScopeResolutionExpression
      | MemberSelectionExpression _ -> SyntaxKind.MemberSelectionExpression
      | SafeMemberSelectionExpression _ ->
        SyntaxKind.SafeMemberSelectionExpression
      | EmbeddedMemberSelectionExpression _ ->
        SyntaxKind.EmbeddedMemberSelectionExpression
      | YieldExpression _ -> SyntaxKind.YieldExpression
      | PrefixUnaryExpression _ -> SyntaxKind.PrefixUnaryExpression
      | PostfixUnaryExpression _ -> SyntaxKind.PostfixUnaryExpression
      | BinaryExpression _ -> SyntaxKind.BinaryExpression
      | IsExpression _ -> SyntaxKind.IsExpression
      | AsExpression _ -> SyntaxKind.AsExpression
      | NullableAsExpression _ -> SyntaxKind.NullableAsExpression
      | UpcastExpression _ -> SyntaxKind.UpcastExpression
      | ConditionalExpression _ -> SyntaxKind.ConditionalExpression
      | EvalExpression _ -> SyntaxKind.EvalExpression
      | IssetExpression _ -> SyntaxKind.IssetExpression
      | NameofExpression _ -> SyntaxKind.NameofExpression
      | FunctionCallExpression _ -> SyntaxKind.FunctionCallExpression
      | FunctionPointerExpression _ -> SyntaxKind.FunctionPointerExpression
      | ParenthesizedExpression _ -> SyntaxKind.ParenthesizedExpression
      | BracedExpression _ -> SyntaxKind.BracedExpression
      | ETSpliceExpression _ -> SyntaxKind.ETSpliceExpression
      | EmbeddedBracedExpression _ -> SyntaxKind.EmbeddedBracedExpression
      | ListExpression _ -> SyntaxKind.ListExpression
      | CollectionLiteralExpression _ -> SyntaxKind.CollectionLiteralExpression
      | ObjectCreationExpression _ -> SyntaxKind.ObjectCreationExpression
      | ConstructorCall _ -> SyntaxKind.ConstructorCall
      | DarrayIntrinsicExpression _ -> SyntaxKind.DarrayIntrinsicExpression
      | DictionaryIntrinsicExpression _ ->
        SyntaxKind.DictionaryIntrinsicExpression
      | KeysetIntrinsicExpression _ -> SyntaxKind.KeysetIntrinsicExpression
      | VarrayIntrinsicExpression _ -> SyntaxKind.VarrayIntrinsicExpression
      | VectorIntrinsicExpression _ -> SyntaxKind.VectorIntrinsicExpression
      | ElementInitializer _ -> SyntaxKind.ElementInitializer
      | SubscriptExpression _ -> SyntaxKind.SubscriptExpression
      | EmbeddedSubscriptExpression _ -> SyntaxKind.EmbeddedSubscriptExpression
      | AwaitableCreationExpression _ -> SyntaxKind.AwaitableCreationExpression
      | XHPChildrenDeclaration _ -> SyntaxKind.XHPChildrenDeclaration
      | XHPChildrenParenthesizedList _ ->
        SyntaxKind.XHPChildrenParenthesizedList
      | XHPCategoryDeclaration _ -> SyntaxKind.XHPCategoryDeclaration
      | XHPEnumType _ -> SyntaxKind.XHPEnumType
      | XHPLateinit _ -> SyntaxKind.XHPLateinit
      | XHPRequired _ -> SyntaxKind.XHPRequired
      | XHPClassAttributeDeclaration _ ->
        SyntaxKind.XHPClassAttributeDeclaration
      | XHPClassAttribute _ -> SyntaxKind.XHPClassAttribute
      | XHPSimpleClassAttribute _ -> SyntaxKind.XHPSimpleClassAttribute
      | XHPSimpleAttribute _ -> SyntaxKind.XHPSimpleAttribute
      | XHPSpreadAttribute _ -> SyntaxKind.XHPSpreadAttribute
      | XHPOpen _ -> SyntaxKind.XHPOpen
      | XHPExpression _ -> SyntaxKind.XHPExpression
      | XHPClose _ -> SyntaxKind.XHPClose
      | TypeConstant _ -> SyntaxKind.TypeConstant
      | VectorTypeSpecifier _ -> SyntaxKind.VectorTypeSpecifier
      | KeysetTypeSpecifier _ -> SyntaxKind.KeysetTypeSpecifier
      | TupleTypeExplicitSpecifier _ -> SyntaxKind.TupleTypeExplicitSpecifier
      | VarrayTypeSpecifier _ -> SyntaxKind.VarrayTypeSpecifier
      | FunctionCtxTypeSpecifier _ -> SyntaxKind.FunctionCtxTypeSpecifier
      | TypeParameter _ -> SyntaxKind.TypeParameter
      | TypeConstraint _ -> SyntaxKind.TypeConstraint
      | ContextConstraint _ -> SyntaxKind.ContextConstraint
      | DarrayTypeSpecifier _ -> SyntaxKind.DarrayTypeSpecifier
      | DictionaryTypeSpecifier _ -> SyntaxKind.DictionaryTypeSpecifier
      | ClosureTypeSpecifier _ -> SyntaxKind.ClosureTypeSpecifier
      | ClosureParameterTypeSpecifier _ ->
        SyntaxKind.ClosureParameterTypeSpecifier
      | TypeRefinement _ -> SyntaxKind.TypeRefinement
      | TypeInRefinement _ -> SyntaxKind.TypeInRefinement
      | CtxInRefinement _ -> SyntaxKind.CtxInRefinement
      | ClassnameTypeSpecifier _ -> SyntaxKind.ClassnameTypeSpecifier
      | FieldSpecifier _ -> SyntaxKind.FieldSpecifier
      | FieldInitializer _ -> SyntaxKind.FieldInitializer
      | ShapeTypeSpecifier _ -> SyntaxKind.ShapeTypeSpecifier
      | ShapeExpression _ -> SyntaxKind.ShapeExpression
      | TupleExpression _ -> SyntaxKind.TupleExpression
      | GenericTypeSpecifier _ -> SyntaxKind.GenericTypeSpecifier
      | NullableTypeSpecifier _ -> SyntaxKind.NullableTypeSpecifier
      | LikeTypeSpecifier _ -> SyntaxKind.LikeTypeSpecifier
      | SoftTypeSpecifier _ -> SyntaxKind.SoftTypeSpecifier
      | AttributizedSpecifier _ -> SyntaxKind.AttributizedSpecifier
      | ReifiedTypeArgument _ -> SyntaxKind.ReifiedTypeArgument
      | TypeArguments _ -> SyntaxKind.TypeArguments
      | TypeParameters _ -> SyntaxKind.TypeParameters
      | TupleTypeSpecifier _ -> SyntaxKind.TupleTypeSpecifier
      | UnionTypeSpecifier _ -> SyntaxKind.UnionTypeSpecifier
      | IntersectionTypeSpecifier _ -> SyntaxKind.IntersectionTypeSpecifier
      | ErrorSyntax _ -> SyntaxKind.ErrorSyntax
      | ListItem _ -> SyntaxKind.ListItem
      | EnumClassLabelExpression _ -> SyntaxKind.EnumClassLabelExpression
      | ModuleDeclaration _ -> SyntaxKind.ModuleDeclaration
      | ModuleExports _ -> SyntaxKind.ModuleExports
      | ModuleImports _ -> SyntaxKind.ModuleImports
      | ModuleMembershipDeclaration _ -> SyntaxKind.ModuleMembershipDeclaration
      | PackageExpression _ -> SyntaxKind.PackageExpression

    let kind node = to_kind (syntax node)

    let has_kind syntax_kind node = SyntaxKind.equal (kind node) syntax_kind

    let is_missing node =
      match kind node with
      | SyntaxKind.Missing -> true
      | _ -> false

    let is_list node =
      match kind node with
      | SyntaxKind.SyntaxList -> true
      | _ -> false

    let is_end_of_file = has_kind SyntaxKind.EndOfFile

    let is_script = has_kind SyntaxKind.Script

    let is_qualified_name = has_kind SyntaxKind.QualifiedName

    let is_module_name = has_kind SyntaxKind.ModuleName

    let is_simple_type_specifier = has_kind SyntaxKind.SimpleTypeSpecifier

    let is_literal_expression = has_kind SyntaxKind.LiteralExpression

    let is_prefixed_string_expression =
      has_kind SyntaxKind.PrefixedStringExpression

    let is_prefixed_code_expression = has_kind SyntaxKind.PrefixedCodeExpression

    let is_variable_expression = has_kind SyntaxKind.VariableExpression

    let is_pipe_variable_expression = has_kind SyntaxKind.PipeVariableExpression

    let is_file_attribute_specification =
      has_kind SyntaxKind.FileAttributeSpecification

    let is_enum_declaration = has_kind SyntaxKind.EnumDeclaration

    let is_enum_use = has_kind SyntaxKind.EnumUse

    let is_enumerator = has_kind SyntaxKind.Enumerator

    let is_enum_class_declaration = has_kind SyntaxKind.EnumClassDeclaration

    let is_enum_class_enumerator = has_kind SyntaxKind.EnumClassEnumerator

    let is_alias_declaration = has_kind SyntaxKind.AliasDeclaration

    let is_context_alias_declaration =
      has_kind SyntaxKind.ContextAliasDeclaration

    let is_case_type_declaration = has_kind SyntaxKind.CaseTypeDeclaration

    let is_case_type_variant = has_kind SyntaxKind.CaseTypeVariant

    let is_property_declaration = has_kind SyntaxKind.PropertyDeclaration

    let is_property_declarator = has_kind SyntaxKind.PropertyDeclarator

    let is_namespace_declaration = has_kind SyntaxKind.NamespaceDeclaration

    let is_namespace_declaration_header =
      has_kind SyntaxKind.NamespaceDeclarationHeader

    let is_namespace_body = has_kind SyntaxKind.NamespaceBody

    let is_namespace_empty_body = has_kind SyntaxKind.NamespaceEmptyBody

    let is_namespace_use_declaration =
      has_kind SyntaxKind.NamespaceUseDeclaration

    let is_namespace_group_use_declaration =
      has_kind SyntaxKind.NamespaceGroupUseDeclaration

    let is_namespace_use_clause = has_kind SyntaxKind.NamespaceUseClause

    let is_function_declaration = has_kind SyntaxKind.FunctionDeclaration

    let is_function_declaration_header =
      has_kind SyntaxKind.FunctionDeclarationHeader

    let is_contexts = has_kind SyntaxKind.Contexts

    let is_where_clause = has_kind SyntaxKind.WhereClause

    let is_where_constraint = has_kind SyntaxKind.WhereConstraint

    let is_methodish_declaration = has_kind SyntaxKind.MethodishDeclaration

    let is_methodish_trait_resolution =
      has_kind SyntaxKind.MethodishTraitResolution

    let is_classish_declaration = has_kind SyntaxKind.ClassishDeclaration

    let is_classish_body = has_kind SyntaxKind.ClassishBody

    let is_trait_use = has_kind SyntaxKind.TraitUse

    let is_require_clause = has_kind SyntaxKind.RequireClause

    let is_const_declaration = has_kind SyntaxKind.ConstDeclaration

    let is_constant_declarator = has_kind SyntaxKind.ConstantDeclarator

    let is_type_const_declaration = has_kind SyntaxKind.TypeConstDeclaration

    let is_context_const_declaration =
      has_kind SyntaxKind.ContextConstDeclaration

    let is_decorated_expression = has_kind SyntaxKind.DecoratedExpression

    let is_parameter_declaration = has_kind SyntaxKind.ParameterDeclaration

    let is_variadic_parameter = has_kind SyntaxKind.VariadicParameter

    let is_old_attribute_specification =
      has_kind SyntaxKind.OldAttributeSpecification

    let is_attribute_specification = has_kind SyntaxKind.AttributeSpecification

    let is_attribute = has_kind SyntaxKind.Attribute

    let is_inclusion_expression = has_kind SyntaxKind.InclusionExpression

    let is_inclusion_directive = has_kind SyntaxKind.InclusionDirective

    let is_compound_statement = has_kind SyntaxKind.CompoundStatement

    let is_expression_statement = has_kind SyntaxKind.ExpressionStatement

    let is_markup_section = has_kind SyntaxKind.MarkupSection

    let is_markup_suffix = has_kind SyntaxKind.MarkupSuffix

    let is_unset_statement = has_kind SyntaxKind.UnsetStatement

    let is_declare_local_statement = has_kind SyntaxKind.DeclareLocalStatement

    let is_using_statement_block_scoped =
      has_kind SyntaxKind.UsingStatementBlockScoped

    let is_using_statement_function_scoped =
      has_kind SyntaxKind.UsingStatementFunctionScoped

    let is_while_statement = has_kind SyntaxKind.WhileStatement

    let is_if_statement = has_kind SyntaxKind.IfStatement

    let is_else_clause = has_kind SyntaxKind.ElseClause

    let is_try_statement = has_kind SyntaxKind.TryStatement

    let is_catch_clause = has_kind SyntaxKind.CatchClause

    let is_finally_clause = has_kind SyntaxKind.FinallyClause

    let is_do_statement = has_kind SyntaxKind.DoStatement

    let is_for_statement = has_kind SyntaxKind.ForStatement

    let is_foreach_statement = has_kind SyntaxKind.ForeachStatement

    let is_switch_statement = has_kind SyntaxKind.SwitchStatement

    let is_switch_section = has_kind SyntaxKind.SwitchSection

    let is_switch_fallthrough = has_kind SyntaxKind.SwitchFallthrough

    let is_case_label = has_kind SyntaxKind.CaseLabel

    let is_default_label = has_kind SyntaxKind.DefaultLabel

    let is_match_statement = has_kind SyntaxKind.MatchStatement

    let is_match_statement_arm = has_kind SyntaxKind.MatchStatementArm

    let is_return_statement = has_kind SyntaxKind.ReturnStatement

    let is_yield_break_statement = has_kind SyntaxKind.YieldBreakStatement

    let is_throw_statement = has_kind SyntaxKind.ThrowStatement

    let is_break_statement = has_kind SyntaxKind.BreakStatement

    let is_continue_statement = has_kind SyntaxKind.ContinueStatement

    let is_echo_statement = has_kind SyntaxKind.EchoStatement

    let is_concurrent_statement = has_kind SyntaxKind.ConcurrentStatement

    let is_simple_initializer = has_kind SyntaxKind.SimpleInitializer

    let is_anonymous_class = has_kind SyntaxKind.AnonymousClass

    let is_anonymous_function = has_kind SyntaxKind.AnonymousFunction

    let is_anonymous_function_use_clause =
      has_kind SyntaxKind.AnonymousFunctionUseClause

    let is_variable_pattern = has_kind SyntaxKind.VariablePattern

    let is_constructor_pattern = has_kind SyntaxKind.ConstructorPattern

    let is_refinement_pattern = has_kind SyntaxKind.RefinementPattern

    let is_lambda_expression = has_kind SyntaxKind.LambdaExpression

    let is_lambda_signature = has_kind SyntaxKind.LambdaSignature

    let is_cast_expression = has_kind SyntaxKind.CastExpression

    let is_scope_resolution_expression =
      has_kind SyntaxKind.ScopeResolutionExpression

    let is_member_selection_expression =
      has_kind SyntaxKind.MemberSelectionExpression

    let is_safe_member_selection_expression =
      has_kind SyntaxKind.SafeMemberSelectionExpression

    let is_embedded_member_selection_expression =
      has_kind SyntaxKind.EmbeddedMemberSelectionExpression

    let is_yield_expression = has_kind SyntaxKind.YieldExpression

    let is_prefix_unary_expression = has_kind SyntaxKind.PrefixUnaryExpression

    let is_postfix_unary_expression = has_kind SyntaxKind.PostfixUnaryExpression

    let is_binary_expression = has_kind SyntaxKind.BinaryExpression

    let is_is_expression = has_kind SyntaxKind.IsExpression

    let is_as_expression = has_kind SyntaxKind.AsExpression

    let is_nullable_as_expression = has_kind SyntaxKind.NullableAsExpression

    let is_upcast_expression = has_kind SyntaxKind.UpcastExpression

    let is_conditional_expression = has_kind SyntaxKind.ConditionalExpression

    let is_eval_expression = has_kind SyntaxKind.EvalExpression

    let is_isset_expression = has_kind SyntaxKind.IssetExpression

    let is_nameof_expression = has_kind SyntaxKind.NameofExpression

    let is_function_call_expression = has_kind SyntaxKind.FunctionCallExpression

    let is_function_pointer_expression =
      has_kind SyntaxKind.FunctionPointerExpression

    let is_parenthesized_expression =
      has_kind SyntaxKind.ParenthesizedExpression

    let is_braced_expression = has_kind SyntaxKind.BracedExpression

    let is_et_splice_expression = has_kind SyntaxKind.ETSpliceExpression

    let is_embedded_braced_expression =
      has_kind SyntaxKind.EmbeddedBracedExpression

    let is_list_expression = has_kind SyntaxKind.ListExpression

    let is_collection_literal_expression =
      has_kind SyntaxKind.CollectionLiteralExpression

    let is_object_creation_expression =
      has_kind SyntaxKind.ObjectCreationExpression

    let is_constructor_call = has_kind SyntaxKind.ConstructorCall

    let is_darray_intrinsic_expression =
      has_kind SyntaxKind.DarrayIntrinsicExpression

    let is_dictionary_intrinsic_expression =
      has_kind SyntaxKind.DictionaryIntrinsicExpression

    let is_keyset_intrinsic_expression =
      has_kind SyntaxKind.KeysetIntrinsicExpression

    let is_varray_intrinsic_expression =
      has_kind SyntaxKind.VarrayIntrinsicExpression

    let is_vector_intrinsic_expression =
      has_kind SyntaxKind.VectorIntrinsicExpression

    let is_element_initializer = has_kind SyntaxKind.ElementInitializer

    let is_subscript_expression = has_kind SyntaxKind.SubscriptExpression

    let is_embedded_subscript_expression =
      has_kind SyntaxKind.EmbeddedSubscriptExpression

    let is_awaitable_creation_expression =
      has_kind SyntaxKind.AwaitableCreationExpression

    let is_xhp_children_declaration = has_kind SyntaxKind.XHPChildrenDeclaration

    let is_xhp_children_parenthesized_list =
      has_kind SyntaxKind.XHPChildrenParenthesizedList

    let is_xhp_category_declaration = has_kind SyntaxKind.XHPCategoryDeclaration

    let is_xhp_enum_type = has_kind SyntaxKind.XHPEnumType

    let is_xhp_lateinit = has_kind SyntaxKind.XHPLateinit

    let is_xhp_required = has_kind SyntaxKind.XHPRequired

    let is_xhp_class_attribute_declaration =
      has_kind SyntaxKind.XHPClassAttributeDeclaration

    let is_xhp_class_attribute = has_kind SyntaxKind.XHPClassAttribute

    let is_xhp_simple_class_attribute =
      has_kind SyntaxKind.XHPSimpleClassAttribute

    let is_xhp_simple_attribute = has_kind SyntaxKind.XHPSimpleAttribute

    let is_xhp_spread_attribute = has_kind SyntaxKind.XHPSpreadAttribute

    let is_xhp_open = has_kind SyntaxKind.XHPOpen

    let is_xhp_expression = has_kind SyntaxKind.XHPExpression

    let is_xhp_close = has_kind SyntaxKind.XHPClose

    let is_type_constant = has_kind SyntaxKind.TypeConstant

    let is_vector_type_specifier = has_kind SyntaxKind.VectorTypeSpecifier

    let is_keyset_type_specifier = has_kind SyntaxKind.KeysetTypeSpecifier

    let is_tuple_type_explicit_specifier =
      has_kind SyntaxKind.TupleTypeExplicitSpecifier

    let is_varray_type_specifier = has_kind SyntaxKind.VarrayTypeSpecifier

    let is_function_ctx_type_specifier =
      has_kind SyntaxKind.FunctionCtxTypeSpecifier

    let is_type_parameter = has_kind SyntaxKind.TypeParameter

    let is_type_constraint = has_kind SyntaxKind.TypeConstraint

    let is_context_constraint = has_kind SyntaxKind.ContextConstraint

    let is_darray_type_specifier = has_kind SyntaxKind.DarrayTypeSpecifier

    let is_dictionary_type_specifier =
      has_kind SyntaxKind.DictionaryTypeSpecifier

    let is_closure_type_specifier = has_kind SyntaxKind.ClosureTypeSpecifier

    let is_closure_parameter_type_specifier =
      has_kind SyntaxKind.ClosureParameterTypeSpecifier

    let is_type_refinement = has_kind SyntaxKind.TypeRefinement

    let is_type_in_refinement = has_kind SyntaxKind.TypeInRefinement

    let is_ctx_in_refinement = has_kind SyntaxKind.CtxInRefinement

    let is_classname_type_specifier = has_kind SyntaxKind.ClassnameTypeSpecifier

    let is_field_specifier = has_kind SyntaxKind.FieldSpecifier

    let is_field_initializer = has_kind SyntaxKind.FieldInitializer

    let is_shape_type_specifier = has_kind SyntaxKind.ShapeTypeSpecifier

    let is_shape_expression = has_kind SyntaxKind.ShapeExpression

    let is_tuple_expression = has_kind SyntaxKind.TupleExpression

    let is_generic_type_specifier = has_kind SyntaxKind.GenericTypeSpecifier

    let is_nullable_type_specifier = has_kind SyntaxKind.NullableTypeSpecifier

    let is_like_type_specifier = has_kind SyntaxKind.LikeTypeSpecifier

    let is_soft_type_specifier = has_kind SyntaxKind.SoftTypeSpecifier

    let is_attributized_specifier = has_kind SyntaxKind.AttributizedSpecifier

    let is_reified_type_argument = has_kind SyntaxKind.ReifiedTypeArgument

    let is_type_arguments = has_kind SyntaxKind.TypeArguments

    let is_type_parameters = has_kind SyntaxKind.TypeParameters

    let is_tuple_type_specifier = has_kind SyntaxKind.TupleTypeSpecifier

    let is_union_type_specifier = has_kind SyntaxKind.UnionTypeSpecifier

    let is_intersection_type_specifier =
      has_kind SyntaxKind.IntersectionTypeSpecifier

    let is_error = has_kind SyntaxKind.ErrorSyntax

    let is_list_item = has_kind SyntaxKind.ListItem

    let is_enum_class_label_expression =
      has_kind SyntaxKind.EnumClassLabelExpression

    let is_module_declaration = has_kind SyntaxKind.ModuleDeclaration

    let is_module_exports = has_kind SyntaxKind.ModuleExports

    let is_module_imports = has_kind SyntaxKind.ModuleImports

    let is_module_membership_declaration =
      has_kind SyntaxKind.ModuleMembershipDeclaration

    let is_package_expression = has_kind SyntaxKind.PackageExpression

    let is_loop_statement node =
      is_for_statement node
      || is_foreach_statement node
      || is_while_statement node
      || is_do_statement node

    let is_separable_prefix node =
      match syntax node with
      | Token t -> begin
        TokenKind.(
          match Token.kind t with
          | PlusPlus
          | MinusMinus ->
            false
          | _ -> true)
      end
      | _ -> true

    let is_specific_token kind node =
      match syntax node with
      | Token t -> TokenKind.equal (Token.kind t) kind
      | _ -> false

    let is_namespace_prefix node =
      match syntax node with
      | QualifiedName e -> begin
        match List.last (syntax_node_to_list e.qualified_name_parts) with
        | None -> false
        | Some p -> begin
          match syntax p with
          | ListItem p -> not (is_missing p.list_separator)
          | _ -> false
        end
      end
      | _ -> false

    let has_leading_trivia kind token =
      List.exists (Token.leading token) ~f:(fun trivia ->
          Full_fidelity_trivia_kind.equal (Token.Trivia.kind trivia) kind)

    let is_external e = is_specific_token TokenKind.Semicolon e || is_missing e

    let is_name = is_specific_token TokenKind.Name

    let is_construct = is_specific_token TokenKind.Construct

    let is_static = is_specific_token TokenKind.Static

    let is_private = is_specific_token TokenKind.Private

    let is_public = is_specific_token TokenKind.Public

    let is_protected = is_specific_token TokenKind.Protected

    let is_abstract = is_specific_token TokenKind.Abstract

    let is_final = is_specific_token TokenKind.Final

    let is_async = is_specific_token TokenKind.Async

    let is_void = is_specific_token TokenKind.Void

    let is_left_brace = is_specific_token TokenKind.LeftBrace

    let is_ellipsis = is_specific_token TokenKind.DotDotDot

    let is_comma = is_specific_token TokenKind.Comma

    let is_ampersand = is_specific_token TokenKind.Ampersand

    let is_inout = is_specific_token TokenKind.Inout

    let syntax_list_fold ~init ~f node =
      match syntax node with
      | SyntaxList sl ->
        List.fold_left
          ~init
          ~f:(fun init li ->
            match syntax li with
            | ListItem { list_item; _ } -> f init list_item
            | Missing -> init
            | _ -> f init li)
          sl
      | Missing -> init
      | _ -> f init node

    let fold_over_children f acc syntax =
      match syntax with
      | Missing -> acc
      | Token _ -> acc
      | SyntaxList items -> List.fold_left ~f ~init:acc items
      | EndOfFile { end_of_file_token } ->
        let acc = f acc end_of_file_token in
        acc
      | Script { script_declarations } ->
        let acc = f acc script_declarations in
        acc
      | QualifiedName { qualified_name_parts } ->
        let acc = f acc qualified_name_parts in
        acc
      | ModuleName { module_name_parts } ->
        let acc = f acc module_name_parts in
        acc
      | SimpleTypeSpecifier { simple_type_specifier } ->
        let acc = f acc simple_type_specifier in
        acc
      | LiteralExpression { literal_expression } ->
        let acc = f acc literal_expression in
        acc
      | PrefixedStringExpression { prefixed_string_name; prefixed_string_str }
        ->
        let acc = f acc prefixed_string_name in
        let acc = f acc prefixed_string_str in
        acc
      | PrefixedCodeExpression
          {
            prefixed_code_prefix;
            prefixed_code_left_backtick;
            prefixed_code_body;
            prefixed_code_right_backtick;
          } ->
        let acc = f acc prefixed_code_prefix in
        let acc = f acc prefixed_code_left_backtick in
        let acc = f acc prefixed_code_body in
        let acc = f acc prefixed_code_right_backtick in
        acc
      | VariableExpression { variable_expression } ->
        let acc = f acc variable_expression in
        acc
      | PipeVariableExpression { pipe_variable_expression } ->
        let acc = f acc pipe_variable_expression in
        acc
      | FileAttributeSpecification
          {
            file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword;
            file_attribute_specification_colon;
            file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle;
          } ->
        let acc = f acc file_attribute_specification_left_double_angle in
        let acc = f acc file_attribute_specification_keyword in
        let acc = f acc file_attribute_specification_colon in
        let acc = f acc file_attribute_specification_attributes in
        let acc = f acc file_attribute_specification_right_double_angle in
        acc
      | EnumDeclaration
          {
            enum_attribute_spec;
            enum_modifiers;
            enum_keyword;
            enum_name;
            enum_colon;
            enum_base;
            enum_type;
            enum_left_brace;
            enum_use_clauses;
            enum_enumerators;
            enum_right_brace;
          } ->
        let acc = f acc enum_attribute_spec in
        let acc = f acc enum_modifiers in
        let acc = f acc enum_keyword in
        let acc = f acc enum_name in
        let acc = f acc enum_colon in
        let acc = f acc enum_base in
        let acc = f acc enum_type in
        let acc = f acc enum_left_brace in
        let acc = f acc enum_use_clauses in
        let acc = f acc enum_enumerators in
        let acc = f acc enum_right_brace in
        acc
      | EnumUse { enum_use_keyword; enum_use_names; enum_use_semicolon } ->
        let acc = f acc enum_use_keyword in
        let acc = f acc enum_use_names in
        let acc = f acc enum_use_semicolon in
        acc
      | Enumerator
          {
            enumerator_name;
            enumerator_equal;
            enumerator_value;
            enumerator_semicolon;
          } ->
        let acc = f acc enumerator_name in
        let acc = f acc enumerator_equal in
        let acc = f acc enumerator_value in
        let acc = f acc enumerator_semicolon in
        acc
      | EnumClassDeclaration
          {
            enum_class_attribute_spec;
            enum_class_modifiers;
            enum_class_enum_keyword;
            enum_class_class_keyword;
            enum_class_name;
            enum_class_colon;
            enum_class_base;
            enum_class_extends;
            enum_class_extends_list;
            enum_class_left_brace;
            enum_class_elements;
            enum_class_right_brace;
          } ->
        let acc = f acc enum_class_attribute_spec in
        let acc = f acc enum_class_modifiers in
        let acc = f acc enum_class_enum_keyword in
        let acc = f acc enum_class_class_keyword in
        let acc = f acc enum_class_name in
        let acc = f acc enum_class_colon in
        let acc = f acc enum_class_base in
        let acc = f acc enum_class_extends in
        let acc = f acc enum_class_extends_list in
        let acc = f acc enum_class_left_brace in
        let acc = f acc enum_class_elements in
        let acc = f acc enum_class_right_brace in
        acc
      | EnumClassEnumerator
          {
            enum_class_enumerator_modifiers;
            enum_class_enumerator_type;
            enum_class_enumerator_name;
            enum_class_enumerator_initializer;
            enum_class_enumerator_semicolon;
          } ->
        let acc = f acc enum_class_enumerator_modifiers in
        let acc = f acc enum_class_enumerator_type in
        let acc = f acc enum_class_enumerator_name in
        let acc = f acc enum_class_enumerator_initializer in
        let acc = f acc enum_class_enumerator_semicolon in
        acc
      | AliasDeclaration
          {
            alias_attribute_spec;
            alias_modifiers;
            alias_module_kw_opt;
            alias_keyword;
            alias_name;
            alias_generic_parameter;
            alias_constraint;
            alias_equal;
            alias_type;
            alias_semicolon;
          } ->
        let acc = f acc alias_attribute_spec in
        let acc = f acc alias_modifiers in
        let acc = f acc alias_module_kw_opt in
        let acc = f acc alias_keyword in
        let acc = f acc alias_name in
        let acc = f acc alias_generic_parameter in
        let acc = f acc alias_constraint in
        let acc = f acc alias_equal in
        let acc = f acc alias_type in
        let acc = f acc alias_semicolon in
        acc
      | ContextAliasDeclaration
          {
            ctx_alias_attribute_spec;
            ctx_alias_keyword;
            ctx_alias_name;
            ctx_alias_generic_parameter;
            ctx_alias_as_constraint;
            ctx_alias_equal;
            ctx_alias_context;
            ctx_alias_semicolon;
          } ->
        let acc = f acc ctx_alias_attribute_spec in
        let acc = f acc ctx_alias_keyword in
        let acc = f acc ctx_alias_name in
        let acc = f acc ctx_alias_generic_parameter in
        let acc = f acc ctx_alias_as_constraint in
        let acc = f acc ctx_alias_equal in
        let acc = f acc ctx_alias_context in
        let acc = f acc ctx_alias_semicolon in
        acc
      | CaseTypeDeclaration
          {
            case_type_attribute_spec;
            case_type_modifiers;
            case_type_case_keyword;
            case_type_type_keyword;
            case_type_name;
            case_type_generic_parameter;
            case_type_as;
            case_type_bounds;
            case_type_equal;
            case_type_variants;
            case_type_semicolon;
          } ->
        let acc = f acc case_type_attribute_spec in
        let acc = f acc case_type_modifiers in
        let acc = f acc case_type_case_keyword in
        let acc = f acc case_type_type_keyword in
        let acc = f acc case_type_name in
        let acc = f acc case_type_generic_parameter in
        let acc = f acc case_type_as in
        let acc = f acc case_type_bounds in
        let acc = f acc case_type_equal in
        let acc = f acc case_type_variants in
        let acc = f acc case_type_semicolon in
        acc
      | CaseTypeVariant { case_type_variant_bar; case_type_variant_type } ->
        let acc = f acc case_type_variant_bar in
        let acc = f acc case_type_variant_type in
        acc
      | PropertyDeclaration
          {
            property_attribute_spec;
            property_modifiers;
            property_type;
            property_declarators;
            property_semicolon;
          } ->
        let acc = f acc property_attribute_spec in
        let acc = f acc property_modifiers in
        let acc = f acc property_type in
        let acc = f acc property_declarators in
        let acc = f acc property_semicolon in
        acc
      | PropertyDeclarator { property_name; property_initializer } ->
        let acc = f acc property_name in
        let acc = f acc property_initializer in
        acc
      | NamespaceDeclaration { namespace_header; namespace_body } ->
        let acc = f acc namespace_header in
        let acc = f acc namespace_body in
        acc
      | NamespaceDeclarationHeader { namespace_keyword; namespace_name } ->
        let acc = f acc namespace_keyword in
        let acc = f acc namespace_name in
        acc
      | NamespaceBody
          {
            namespace_left_brace;
            namespace_declarations;
            namespace_right_brace;
          } ->
        let acc = f acc namespace_left_brace in
        let acc = f acc namespace_declarations in
        let acc = f acc namespace_right_brace in
        acc
      | NamespaceEmptyBody { namespace_semicolon } ->
        let acc = f acc namespace_semicolon in
        acc
      | NamespaceUseDeclaration
          {
            namespace_use_keyword;
            namespace_use_kind;
            namespace_use_clauses;
            namespace_use_semicolon;
          } ->
        let acc = f acc namespace_use_keyword in
        let acc = f acc namespace_use_kind in
        let acc = f acc namespace_use_clauses in
        let acc = f acc namespace_use_semicolon in
        acc
      | NamespaceGroupUseDeclaration
          {
            namespace_group_use_keyword;
            namespace_group_use_kind;
            namespace_group_use_prefix;
            namespace_group_use_left_brace;
            namespace_group_use_clauses;
            namespace_group_use_right_brace;
            namespace_group_use_semicolon;
          } ->
        let acc = f acc namespace_group_use_keyword in
        let acc = f acc namespace_group_use_kind in
        let acc = f acc namespace_group_use_prefix in
        let acc = f acc namespace_group_use_left_brace in
        let acc = f acc namespace_group_use_clauses in
        let acc = f acc namespace_group_use_right_brace in
        let acc = f acc namespace_group_use_semicolon in
        acc
      | NamespaceUseClause
          {
            namespace_use_clause_kind;
            namespace_use_name;
            namespace_use_as;
            namespace_use_alias;
          } ->
        let acc = f acc namespace_use_clause_kind in
        let acc = f acc namespace_use_name in
        let acc = f acc namespace_use_as in
        let acc = f acc namespace_use_alias in
        acc
      | FunctionDeclaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          } ->
        let acc = f acc function_attribute_spec in
        let acc = f acc function_declaration_header in
        let acc = f acc function_body in
        acc
      | FunctionDeclarationHeader
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          } ->
        let acc = f acc function_modifiers in
        let acc = f acc function_keyword in
        let acc = f acc function_name in
        let acc = f acc function_type_parameter_list in
        let acc = f acc function_left_paren in
        let acc = f acc function_parameter_list in
        let acc = f acc function_right_paren in
        let acc = f acc function_contexts in
        let acc = f acc function_colon in
        let acc = f acc function_readonly_return in
        let acc = f acc function_type in
        let acc = f acc function_where_clause in
        acc
      | Contexts
          { contexts_left_bracket; contexts_types; contexts_right_bracket } ->
        let acc = f acc contexts_left_bracket in
        let acc = f acc contexts_types in
        let acc = f acc contexts_right_bracket in
        acc
      | WhereClause { where_clause_keyword; where_clause_constraints } ->
        let acc = f acc where_clause_keyword in
        let acc = f acc where_clause_constraints in
        acc
      | WhereConstraint
          {
            where_constraint_left_type;
            where_constraint_operator;
            where_constraint_right_type;
          } ->
        let acc = f acc where_constraint_left_type in
        let acc = f acc where_constraint_operator in
        let acc = f acc where_constraint_right_type in
        acc
      | MethodishDeclaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          } ->
        let acc = f acc methodish_attribute in
        let acc = f acc methodish_function_decl_header in
        let acc = f acc methodish_function_body in
        let acc = f acc methodish_semicolon in
        acc
      | MethodishTraitResolution
          {
            methodish_trait_attribute;
            methodish_trait_function_decl_header;
            methodish_trait_equal;
            methodish_trait_name;
            methodish_trait_semicolon;
          } ->
        let acc = f acc methodish_trait_attribute in
        let acc = f acc methodish_trait_function_decl_header in
        let acc = f acc methodish_trait_equal in
        let acc = f acc methodish_trait_name in
        let acc = f acc methodish_trait_semicolon in
        acc
      | ClassishDeclaration
          {
            classish_attribute;
            classish_modifiers;
            classish_xhp;
            classish_keyword;
            classish_name;
            classish_type_parameters;
            classish_extends_keyword;
            classish_extends_list;
            classish_implements_keyword;
            classish_implements_list;
            classish_where_clause;
            classish_body;
          } ->
        let acc = f acc classish_attribute in
        let acc = f acc classish_modifiers in
        let acc = f acc classish_xhp in
        let acc = f acc classish_keyword in
        let acc = f acc classish_name in
        let acc = f acc classish_type_parameters in
        let acc = f acc classish_extends_keyword in
        let acc = f acc classish_extends_list in
        let acc = f acc classish_implements_keyword in
        let acc = f acc classish_implements_list in
        let acc = f acc classish_where_clause in
        let acc = f acc classish_body in
        acc
      | ClassishBody
          {
            classish_body_left_brace;
            classish_body_elements;
            classish_body_right_brace;
          } ->
        let acc = f acc classish_body_left_brace in
        let acc = f acc classish_body_elements in
        let acc = f acc classish_body_right_brace in
        acc
      | TraitUse { trait_use_keyword; trait_use_names; trait_use_semicolon } ->
        let acc = f acc trait_use_keyword in
        let acc = f acc trait_use_names in
        let acc = f acc trait_use_semicolon in
        acc
      | RequireClause
          { require_keyword; require_kind; require_name; require_semicolon } ->
        let acc = f acc require_keyword in
        let acc = f acc require_kind in
        let acc = f acc require_name in
        let acc = f acc require_semicolon in
        acc
      | ConstDeclaration
          {
            const_attribute_spec;
            const_modifiers;
            const_keyword;
            const_type_specifier;
            const_declarators;
            const_semicolon;
          } ->
        let acc = f acc const_attribute_spec in
        let acc = f acc const_modifiers in
        let acc = f acc const_keyword in
        let acc = f acc const_type_specifier in
        let acc = f acc const_declarators in
        let acc = f acc const_semicolon in
        acc
      | ConstantDeclarator
          { constant_declarator_name; constant_declarator_initializer } ->
        let acc = f acc constant_declarator_name in
        let acc = f acc constant_declarator_initializer in
        acc
      | TypeConstDeclaration
          {
            type_const_attribute_spec;
            type_const_modifiers;
            type_const_keyword;
            type_const_type_keyword;
            type_const_name;
            type_const_type_parameters;
            type_const_type_constraints;
            type_const_equal;
            type_const_type_specifier;
            type_const_semicolon;
          } ->
        let acc = f acc type_const_attribute_spec in
        let acc = f acc type_const_modifiers in
        let acc = f acc type_const_keyword in
        let acc = f acc type_const_type_keyword in
        let acc = f acc type_const_name in
        let acc = f acc type_const_type_parameters in
        let acc = f acc type_const_type_constraints in
        let acc = f acc type_const_equal in
        let acc = f acc type_const_type_specifier in
        let acc = f acc type_const_semicolon in
        acc
      | ContextConstDeclaration
          {
            context_const_modifiers;
            context_const_const_keyword;
            context_const_ctx_keyword;
            context_const_name;
            context_const_type_parameters;
            context_const_constraint;
            context_const_equal;
            context_const_ctx_list;
            context_const_semicolon;
          } ->
        let acc = f acc context_const_modifiers in
        let acc = f acc context_const_const_keyword in
        let acc = f acc context_const_ctx_keyword in
        let acc = f acc context_const_name in
        let acc = f acc context_const_type_parameters in
        let acc = f acc context_const_constraint in
        let acc = f acc context_const_equal in
        let acc = f acc context_const_ctx_list in
        let acc = f acc context_const_semicolon in
        acc
      | DecoratedExpression
          { decorated_expression_decorator; decorated_expression_expression } ->
        let acc = f acc decorated_expression_decorator in
        let acc = f acc decorated_expression_expression in
        acc
      | ParameterDeclaration
          {
            parameter_attribute;
            parameter_visibility;
            parameter_call_convention;
            parameter_readonly;
            parameter_type;
            parameter_name;
            parameter_default_value;
          } ->
        let acc = f acc parameter_attribute in
        let acc = f acc parameter_visibility in
        let acc = f acc parameter_call_convention in
        let acc = f acc parameter_readonly in
        let acc = f acc parameter_type in
        let acc = f acc parameter_name in
        let acc = f acc parameter_default_value in
        acc
      | VariadicParameter
          {
            variadic_parameter_call_convention;
            variadic_parameter_type;
            variadic_parameter_ellipsis;
          } ->
        let acc = f acc variadic_parameter_call_convention in
        let acc = f acc variadic_parameter_type in
        let acc = f acc variadic_parameter_ellipsis in
        acc
      | OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle;
          } ->
        let acc = f acc old_attribute_specification_left_double_angle in
        let acc = f acc old_attribute_specification_attributes in
        let acc = f acc old_attribute_specification_right_double_angle in
        acc
      | AttributeSpecification { attribute_specification_attributes } ->
        let acc = f acc attribute_specification_attributes in
        acc
      | Attribute { attribute_at; attribute_attribute_name } ->
        let acc = f acc attribute_at in
        let acc = f acc attribute_attribute_name in
        acc
      | InclusionExpression { inclusion_require; inclusion_filename } ->
        let acc = f acc inclusion_require in
        let acc = f acc inclusion_filename in
        acc
      | InclusionDirective { inclusion_expression; inclusion_semicolon } ->
        let acc = f acc inclusion_expression in
        let acc = f acc inclusion_semicolon in
        acc
      | CompoundStatement
          { compound_left_brace; compound_statements; compound_right_brace } ->
        let acc = f acc compound_left_brace in
        let acc = f acc compound_statements in
        let acc = f acc compound_right_brace in
        acc
      | ExpressionStatement
          { expression_statement_expression; expression_statement_semicolon } ->
        let acc = f acc expression_statement_expression in
        let acc = f acc expression_statement_semicolon in
        acc
      | MarkupSection { markup_hashbang; markup_suffix } ->
        let acc = f acc markup_hashbang in
        let acc = f acc markup_suffix in
        acc
      | MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name } ->
        let acc = f acc markup_suffix_less_than_question in
        let acc = f acc markup_suffix_name in
        acc
      | UnsetStatement
          {
            unset_keyword;
            unset_left_paren;
            unset_variables;
            unset_right_paren;
            unset_semicolon;
          } ->
        let acc = f acc unset_keyword in
        let acc = f acc unset_left_paren in
        let acc = f acc unset_variables in
        let acc = f acc unset_right_paren in
        let acc = f acc unset_semicolon in
        acc
      | DeclareLocalStatement
          {
            declare_local_keyword;
            declare_local_variable;
            declare_local_colon;
            declare_local_type;
            declare_local_initializer;
            declare_local_semicolon;
          } ->
        let acc = f acc declare_local_keyword in
        let acc = f acc declare_local_variable in
        let acc = f acc declare_local_colon in
        let acc = f acc declare_local_type in
        let acc = f acc declare_local_initializer in
        let acc = f acc declare_local_semicolon in
        acc
      | UsingStatementBlockScoped
          {
            using_block_await_keyword;
            using_block_using_keyword;
            using_block_left_paren;
            using_block_expressions;
            using_block_right_paren;
            using_block_body;
          } ->
        let acc = f acc using_block_await_keyword in
        let acc = f acc using_block_using_keyword in
        let acc = f acc using_block_left_paren in
        let acc = f acc using_block_expressions in
        let acc = f acc using_block_right_paren in
        let acc = f acc using_block_body in
        acc
      | UsingStatementFunctionScoped
          {
            using_function_await_keyword;
            using_function_using_keyword;
            using_function_expression;
            using_function_semicolon;
          } ->
        let acc = f acc using_function_await_keyword in
        let acc = f acc using_function_using_keyword in
        let acc = f acc using_function_expression in
        let acc = f acc using_function_semicolon in
        acc
      | WhileStatement
          {
            while_keyword;
            while_left_paren;
            while_condition;
            while_right_paren;
            while_body;
          } ->
        let acc = f acc while_keyword in
        let acc = f acc while_left_paren in
        let acc = f acc while_condition in
        let acc = f acc while_right_paren in
        let acc = f acc while_body in
        acc
      | IfStatement
          {
            if_keyword;
            if_left_paren;
            if_condition;
            if_right_paren;
            if_statement;
            if_else_clause;
          } ->
        let acc = f acc if_keyword in
        let acc = f acc if_left_paren in
        let acc = f acc if_condition in
        let acc = f acc if_right_paren in
        let acc = f acc if_statement in
        let acc = f acc if_else_clause in
        acc
      | ElseClause { else_keyword; else_statement } ->
        let acc = f acc else_keyword in
        let acc = f acc else_statement in
        acc
      | TryStatement
          {
            try_keyword;
            try_compound_statement;
            try_catch_clauses;
            try_finally_clause;
          } ->
        let acc = f acc try_keyword in
        let acc = f acc try_compound_statement in
        let acc = f acc try_catch_clauses in
        let acc = f acc try_finally_clause in
        acc
      | CatchClause
          {
            catch_keyword;
            catch_left_paren;
            catch_type;
            catch_variable;
            catch_right_paren;
            catch_body;
          } ->
        let acc = f acc catch_keyword in
        let acc = f acc catch_left_paren in
        let acc = f acc catch_type in
        let acc = f acc catch_variable in
        let acc = f acc catch_right_paren in
        let acc = f acc catch_body in
        acc
      | FinallyClause { finally_keyword; finally_body } ->
        let acc = f acc finally_keyword in
        let acc = f acc finally_body in
        acc
      | DoStatement
          {
            do_keyword;
            do_body;
            do_while_keyword;
            do_left_paren;
            do_condition;
            do_right_paren;
            do_semicolon;
          } ->
        let acc = f acc do_keyword in
        let acc = f acc do_body in
        let acc = f acc do_while_keyword in
        let acc = f acc do_left_paren in
        let acc = f acc do_condition in
        let acc = f acc do_right_paren in
        let acc = f acc do_semicolon in
        acc
      | ForStatement
          {
            for_keyword;
            for_left_paren;
            for_initializer;
            for_first_semicolon;
            for_control;
            for_second_semicolon;
            for_end_of_loop;
            for_right_paren;
            for_body;
          } ->
        let acc = f acc for_keyword in
        let acc = f acc for_left_paren in
        let acc = f acc for_initializer in
        let acc = f acc for_first_semicolon in
        let acc = f acc for_control in
        let acc = f acc for_second_semicolon in
        let acc = f acc for_end_of_loop in
        let acc = f acc for_right_paren in
        let acc = f acc for_body in
        acc
      | ForeachStatement
          {
            foreach_keyword;
            foreach_left_paren;
            foreach_collection;
            foreach_await_keyword;
            foreach_as;
            foreach_key;
            foreach_arrow;
            foreach_value;
            foreach_right_paren;
            foreach_body;
          } ->
        let acc = f acc foreach_keyword in
        let acc = f acc foreach_left_paren in
        let acc = f acc foreach_collection in
        let acc = f acc foreach_await_keyword in
        let acc = f acc foreach_as in
        let acc = f acc foreach_key in
        let acc = f acc foreach_arrow in
        let acc = f acc foreach_value in
        let acc = f acc foreach_right_paren in
        let acc = f acc foreach_body in
        acc
      | SwitchStatement
          {
            switch_keyword;
            switch_left_paren;
            switch_expression;
            switch_right_paren;
            switch_left_brace;
            switch_sections;
            switch_right_brace;
          } ->
        let acc = f acc switch_keyword in
        let acc = f acc switch_left_paren in
        let acc = f acc switch_expression in
        let acc = f acc switch_right_paren in
        let acc = f acc switch_left_brace in
        let acc = f acc switch_sections in
        let acc = f acc switch_right_brace in
        acc
      | SwitchSection
          {
            switch_section_labels;
            switch_section_statements;
            switch_section_fallthrough;
          } ->
        let acc = f acc switch_section_labels in
        let acc = f acc switch_section_statements in
        let acc = f acc switch_section_fallthrough in
        acc
      | SwitchFallthrough { fallthrough_keyword; fallthrough_semicolon } ->
        let acc = f acc fallthrough_keyword in
        let acc = f acc fallthrough_semicolon in
        acc
      | CaseLabel { case_keyword; case_expression; case_colon } ->
        let acc = f acc case_keyword in
        let acc = f acc case_expression in
        let acc = f acc case_colon in
        acc
      | DefaultLabel { default_keyword; default_colon } ->
        let acc = f acc default_keyword in
        let acc = f acc default_colon in
        acc
      | MatchStatement
          {
            match_statement_keyword;
            match_statement_left_paren;
            match_statement_expression;
            match_statement_right_paren;
            match_statement_left_brace;
            match_statement_arms;
            match_statement_right_brace;
          } ->
        let acc = f acc match_statement_keyword in
        let acc = f acc match_statement_left_paren in
        let acc = f acc match_statement_expression in
        let acc = f acc match_statement_right_paren in
        let acc = f acc match_statement_left_brace in
        let acc = f acc match_statement_arms in
        let acc = f acc match_statement_right_brace in
        acc
      | MatchStatementArm
          {
            match_statement_arm_pattern;
            match_statement_arm_arrow;
            match_statement_arm_body;
          } ->
        let acc = f acc match_statement_arm_pattern in
        let acc = f acc match_statement_arm_arrow in
        let acc = f acc match_statement_arm_body in
        acc
      | ReturnStatement { return_keyword; return_expression; return_semicolon }
        ->
        let acc = f acc return_keyword in
        let acc = f acc return_expression in
        let acc = f acc return_semicolon in
        acc
      | YieldBreakStatement
          { yield_break_keyword; yield_break_break; yield_break_semicolon } ->
        let acc = f acc yield_break_keyword in
        let acc = f acc yield_break_break in
        let acc = f acc yield_break_semicolon in
        acc
      | ThrowStatement { throw_keyword; throw_expression; throw_semicolon } ->
        let acc = f acc throw_keyword in
        let acc = f acc throw_expression in
        let acc = f acc throw_semicolon in
        acc
      | BreakStatement { break_keyword; break_semicolon } ->
        let acc = f acc break_keyword in
        let acc = f acc break_semicolon in
        acc
      | ContinueStatement { continue_keyword; continue_semicolon } ->
        let acc = f acc continue_keyword in
        let acc = f acc continue_semicolon in
        acc
      | EchoStatement { echo_keyword; echo_expressions; echo_semicolon } ->
        let acc = f acc echo_keyword in
        let acc = f acc echo_expressions in
        let acc = f acc echo_semicolon in
        acc
      | ConcurrentStatement { concurrent_keyword; concurrent_statement } ->
        let acc = f acc concurrent_keyword in
        let acc = f acc concurrent_statement in
        acc
      | SimpleInitializer { simple_initializer_equal; simple_initializer_value }
        ->
        let acc = f acc simple_initializer_equal in
        let acc = f acc simple_initializer_value in
        acc
      | AnonymousClass
          {
            anonymous_class_class_keyword;
            anonymous_class_left_paren;
            anonymous_class_argument_list;
            anonymous_class_right_paren;
            anonymous_class_extends_keyword;
            anonymous_class_extends_list;
            anonymous_class_implements_keyword;
            anonymous_class_implements_list;
            anonymous_class_body;
          } ->
        let acc = f acc anonymous_class_class_keyword in
        let acc = f acc anonymous_class_left_paren in
        let acc = f acc anonymous_class_argument_list in
        let acc = f acc anonymous_class_right_paren in
        let acc = f acc anonymous_class_extends_keyword in
        let acc = f acc anonymous_class_extends_list in
        let acc = f acc anonymous_class_implements_keyword in
        let acc = f acc anonymous_class_implements_list in
        let acc = f acc anonymous_class_body in
        acc
      | AnonymousFunction
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          } ->
        let acc = f acc anonymous_attribute_spec in
        let acc = f acc anonymous_async_keyword in
        let acc = f acc anonymous_function_keyword in
        let acc = f acc anonymous_left_paren in
        let acc = f acc anonymous_parameters in
        let acc = f acc anonymous_right_paren in
        let acc = f acc anonymous_ctx_list in
        let acc = f acc anonymous_colon in
        let acc = f acc anonymous_readonly_return in
        let acc = f acc anonymous_type in
        let acc = f acc anonymous_use in
        let acc = f acc anonymous_body in
        acc
      | AnonymousFunctionUseClause
          {
            anonymous_use_keyword;
            anonymous_use_left_paren;
            anonymous_use_variables;
            anonymous_use_right_paren;
          } ->
        let acc = f acc anonymous_use_keyword in
        let acc = f acc anonymous_use_left_paren in
        let acc = f acc anonymous_use_variables in
        let acc = f acc anonymous_use_right_paren in
        acc
      | VariablePattern { variable_pattern_variable } ->
        let acc = f acc variable_pattern_variable in
        acc
      | ConstructorPattern
          {
            constructor_pattern_constructor;
            constructor_pattern_left_paren;
            constructor_pattern_members;
            constructor_pattern_right_paren;
          } ->
        let acc = f acc constructor_pattern_constructor in
        let acc = f acc constructor_pattern_left_paren in
        let acc = f acc constructor_pattern_members in
        let acc = f acc constructor_pattern_right_paren in
        acc
      | RefinementPattern
          {
            refinement_pattern_variable;
            refinement_pattern_colon;
            refinement_pattern_specifier;
          } ->
        let acc = f acc refinement_pattern_variable in
        let acc = f acc refinement_pattern_colon in
        let acc = f acc refinement_pattern_specifier in
        acc
      | LambdaExpression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          } ->
        let acc = f acc lambda_attribute_spec in
        let acc = f acc lambda_async in
        let acc = f acc lambda_signature in
        let acc = f acc lambda_arrow in
        let acc = f acc lambda_body in
        acc
      | LambdaSignature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          } ->
        let acc = f acc lambda_left_paren in
        let acc = f acc lambda_parameters in
        let acc = f acc lambda_right_paren in
        let acc = f acc lambda_contexts in
        let acc = f acc lambda_colon in
        let acc = f acc lambda_readonly_return in
        let acc = f acc lambda_type in
        acc
      | CastExpression
          { cast_left_paren; cast_type; cast_right_paren; cast_operand } ->
        let acc = f acc cast_left_paren in
        let acc = f acc cast_type in
        let acc = f acc cast_right_paren in
        let acc = f acc cast_operand in
        acc
      | ScopeResolutionExpression
          {
            scope_resolution_qualifier;
            scope_resolution_operator;
            scope_resolution_name;
          } ->
        let acc = f acc scope_resolution_qualifier in
        let acc = f acc scope_resolution_operator in
        let acc = f acc scope_resolution_name in
        acc
      | MemberSelectionExpression
          { member_object; member_operator; member_name } ->
        let acc = f acc member_object in
        let acc = f acc member_operator in
        let acc = f acc member_name in
        acc
      | SafeMemberSelectionExpression
          { safe_member_object; safe_member_operator; safe_member_name } ->
        let acc = f acc safe_member_object in
        let acc = f acc safe_member_operator in
        let acc = f acc safe_member_name in
        acc
      | EmbeddedMemberSelectionExpression
          {
            embedded_member_object;
            embedded_member_operator;
            embedded_member_name;
          } ->
        let acc = f acc embedded_member_object in
        let acc = f acc embedded_member_operator in
        let acc = f acc embedded_member_name in
        acc
      | YieldExpression { yield_keyword; yield_operand } ->
        let acc = f acc yield_keyword in
        let acc = f acc yield_operand in
        acc
      | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand } ->
        let acc = f acc prefix_unary_operator in
        let acc = f acc prefix_unary_operand in
        acc
      | PostfixUnaryExpression { postfix_unary_operand; postfix_unary_operator }
        ->
        let acc = f acc postfix_unary_operand in
        let acc = f acc postfix_unary_operator in
        acc
      | BinaryExpression
          { binary_left_operand; binary_operator; binary_right_operand } ->
        let acc = f acc binary_left_operand in
        let acc = f acc binary_operator in
        let acc = f acc binary_right_operand in
        acc
      | IsExpression { is_left_operand; is_operator; is_right_operand } ->
        let acc = f acc is_left_operand in
        let acc = f acc is_operator in
        let acc = f acc is_right_operand in
        acc
      | AsExpression { as_left_operand; as_operator; as_right_operand } ->
        let acc = f acc as_left_operand in
        let acc = f acc as_operator in
        let acc = f acc as_right_operand in
        acc
      | NullableAsExpression
          {
            nullable_as_left_operand;
            nullable_as_operator;
            nullable_as_right_operand;
          } ->
        let acc = f acc nullable_as_left_operand in
        let acc = f acc nullable_as_operator in
        let acc = f acc nullable_as_right_operand in
        acc
      | UpcastExpression
          { upcast_left_operand; upcast_operator; upcast_right_operand } ->
        let acc = f acc upcast_left_operand in
        let acc = f acc upcast_operator in
        let acc = f acc upcast_right_operand in
        acc
      | ConditionalExpression
          {
            conditional_test;
            conditional_question;
            conditional_consequence;
            conditional_colon;
            conditional_alternative;
          } ->
        let acc = f acc conditional_test in
        let acc = f acc conditional_question in
        let acc = f acc conditional_consequence in
        let acc = f acc conditional_colon in
        let acc = f acc conditional_alternative in
        acc
      | EvalExpression
          { eval_keyword; eval_left_paren; eval_argument; eval_right_paren } ->
        let acc = f acc eval_keyword in
        let acc = f acc eval_left_paren in
        let acc = f acc eval_argument in
        let acc = f acc eval_right_paren in
        acc
      | IssetExpression
          {
            isset_keyword;
            isset_left_paren;
            isset_argument_list;
            isset_right_paren;
          } ->
        let acc = f acc isset_keyword in
        let acc = f acc isset_left_paren in
        let acc = f acc isset_argument_list in
        let acc = f acc isset_right_paren in
        acc
      | NameofExpression { nameof_keyword; nameof_target } ->
        let acc = f acc nameof_keyword in
        let acc = f acc nameof_target in
        acc
      | FunctionCallExpression
          {
            function_call_receiver;
            function_call_type_args;
            function_call_left_paren;
            function_call_argument_list;
            function_call_right_paren;
          } ->
        let acc = f acc function_call_receiver in
        let acc = f acc function_call_type_args in
        let acc = f acc function_call_left_paren in
        let acc = f acc function_call_argument_list in
        let acc = f acc function_call_right_paren in
        acc
      | FunctionPointerExpression
          { function_pointer_receiver; function_pointer_type_args } ->
        let acc = f acc function_pointer_receiver in
        let acc = f acc function_pointer_type_args in
        acc
      | ParenthesizedExpression
          {
            parenthesized_expression_left_paren;
            parenthesized_expression_expression;
            parenthesized_expression_right_paren;
          } ->
        let acc = f acc parenthesized_expression_left_paren in
        let acc = f acc parenthesized_expression_expression in
        let acc = f acc parenthesized_expression_right_paren in
        acc
      | BracedExpression
          {
            braced_expression_left_brace;
            braced_expression_expression;
            braced_expression_right_brace;
          } ->
        let acc = f acc braced_expression_left_brace in
        let acc = f acc braced_expression_expression in
        let acc = f acc braced_expression_right_brace in
        acc
      | ETSpliceExpression
          {
            et_splice_expression_dollar;
            et_splice_expression_left_brace;
            et_splice_expression_expression;
            et_splice_expression_right_brace;
          } ->
        let acc = f acc et_splice_expression_dollar in
        let acc = f acc et_splice_expression_left_brace in
        let acc = f acc et_splice_expression_expression in
        let acc = f acc et_splice_expression_right_brace in
        acc
      | EmbeddedBracedExpression
          {
            embedded_braced_expression_left_brace;
            embedded_braced_expression_expression;
            embedded_braced_expression_right_brace;
          } ->
        let acc = f acc embedded_braced_expression_left_brace in
        let acc = f acc embedded_braced_expression_expression in
        let acc = f acc embedded_braced_expression_right_brace in
        acc
      | ListExpression
          { list_keyword; list_left_paren; list_members; list_right_paren } ->
        let acc = f acc list_keyword in
        let acc = f acc list_left_paren in
        let acc = f acc list_members in
        let acc = f acc list_right_paren in
        acc
      | CollectionLiteralExpression
          {
            collection_literal_name;
            collection_literal_left_brace;
            collection_literal_initializers;
            collection_literal_right_brace;
          } ->
        let acc = f acc collection_literal_name in
        let acc = f acc collection_literal_left_brace in
        let acc = f acc collection_literal_initializers in
        let acc = f acc collection_literal_right_brace in
        acc
      | ObjectCreationExpression
          { object_creation_new_keyword; object_creation_object } ->
        let acc = f acc object_creation_new_keyword in
        let acc = f acc object_creation_object in
        acc
      | ConstructorCall
          {
            constructor_call_type;
            constructor_call_left_paren;
            constructor_call_argument_list;
            constructor_call_right_paren;
          } ->
        let acc = f acc constructor_call_type in
        let acc = f acc constructor_call_left_paren in
        let acc = f acc constructor_call_argument_list in
        let acc = f acc constructor_call_right_paren in
        acc
      | DarrayIntrinsicExpression
          {
            darray_intrinsic_keyword;
            darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket;
            darray_intrinsic_members;
            darray_intrinsic_right_bracket;
          } ->
        let acc = f acc darray_intrinsic_keyword in
        let acc = f acc darray_intrinsic_explicit_type in
        let acc = f acc darray_intrinsic_left_bracket in
        let acc = f acc darray_intrinsic_members in
        let acc = f acc darray_intrinsic_right_bracket in
        acc
      | DictionaryIntrinsicExpression
          {
            dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket;
          } ->
        let acc = f acc dictionary_intrinsic_keyword in
        let acc = f acc dictionary_intrinsic_explicit_type in
        let acc = f acc dictionary_intrinsic_left_bracket in
        let acc = f acc dictionary_intrinsic_members in
        let acc = f acc dictionary_intrinsic_right_bracket in
        acc
      | KeysetIntrinsicExpression
          {
            keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket;
            keyset_intrinsic_members;
            keyset_intrinsic_right_bracket;
          } ->
        let acc = f acc keyset_intrinsic_keyword in
        let acc = f acc keyset_intrinsic_explicit_type in
        let acc = f acc keyset_intrinsic_left_bracket in
        let acc = f acc keyset_intrinsic_members in
        let acc = f acc keyset_intrinsic_right_bracket in
        acc
      | VarrayIntrinsicExpression
          {
            varray_intrinsic_keyword;
            varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket;
            varray_intrinsic_members;
            varray_intrinsic_right_bracket;
          } ->
        let acc = f acc varray_intrinsic_keyword in
        let acc = f acc varray_intrinsic_explicit_type in
        let acc = f acc varray_intrinsic_left_bracket in
        let acc = f acc varray_intrinsic_members in
        let acc = f acc varray_intrinsic_right_bracket in
        acc
      | VectorIntrinsicExpression
          {
            vector_intrinsic_keyword;
            vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket;
            vector_intrinsic_members;
            vector_intrinsic_right_bracket;
          } ->
        let acc = f acc vector_intrinsic_keyword in
        let acc = f acc vector_intrinsic_explicit_type in
        let acc = f acc vector_intrinsic_left_bracket in
        let acc = f acc vector_intrinsic_members in
        let acc = f acc vector_intrinsic_right_bracket in
        acc
      | ElementInitializer { element_key; element_arrow; element_value } ->
        let acc = f acc element_key in
        let acc = f acc element_arrow in
        let acc = f acc element_value in
        acc
      | SubscriptExpression
          {
            subscript_receiver;
            subscript_left_bracket;
            subscript_index;
            subscript_right_bracket;
          } ->
        let acc = f acc subscript_receiver in
        let acc = f acc subscript_left_bracket in
        let acc = f acc subscript_index in
        let acc = f acc subscript_right_bracket in
        acc
      | EmbeddedSubscriptExpression
          {
            embedded_subscript_receiver;
            embedded_subscript_left_bracket;
            embedded_subscript_index;
            embedded_subscript_right_bracket;
          } ->
        let acc = f acc embedded_subscript_receiver in
        let acc = f acc embedded_subscript_left_bracket in
        let acc = f acc embedded_subscript_index in
        let acc = f acc embedded_subscript_right_bracket in
        acc
      | AwaitableCreationExpression
          {
            awaitable_attribute_spec;
            awaitable_async;
            awaitable_compound_statement;
          } ->
        let acc = f acc awaitable_attribute_spec in
        let acc = f acc awaitable_async in
        let acc = f acc awaitable_compound_statement in
        acc
      | XHPChildrenDeclaration
          {
            xhp_children_keyword;
            xhp_children_expression;
            xhp_children_semicolon;
          } ->
        let acc = f acc xhp_children_keyword in
        let acc = f acc xhp_children_expression in
        let acc = f acc xhp_children_semicolon in
        acc
      | XHPChildrenParenthesizedList
          {
            xhp_children_list_left_paren;
            xhp_children_list_xhp_children;
            xhp_children_list_right_paren;
          } ->
        let acc = f acc xhp_children_list_left_paren in
        let acc = f acc xhp_children_list_xhp_children in
        let acc = f acc xhp_children_list_right_paren in
        acc
      | XHPCategoryDeclaration
          {
            xhp_category_keyword;
            xhp_category_categories;
            xhp_category_semicolon;
          } ->
        let acc = f acc xhp_category_keyword in
        let acc = f acc xhp_category_categories in
        let acc = f acc xhp_category_semicolon in
        acc
      | XHPEnumType
          {
            xhp_enum_like;
            xhp_enum_keyword;
            xhp_enum_left_brace;
            xhp_enum_values;
            xhp_enum_right_brace;
          } ->
        let acc = f acc xhp_enum_like in
        let acc = f acc xhp_enum_keyword in
        let acc = f acc xhp_enum_left_brace in
        let acc = f acc xhp_enum_values in
        let acc = f acc xhp_enum_right_brace in
        acc
      | XHPLateinit { xhp_lateinit_at; xhp_lateinit_keyword } ->
        let acc = f acc xhp_lateinit_at in
        let acc = f acc xhp_lateinit_keyword in
        acc
      | XHPRequired { xhp_required_at; xhp_required_keyword } ->
        let acc = f acc xhp_required_at in
        let acc = f acc xhp_required_keyword in
        acc
      | XHPClassAttributeDeclaration
          {
            xhp_attribute_keyword;
            xhp_attribute_attributes;
            xhp_attribute_semicolon;
          } ->
        let acc = f acc xhp_attribute_keyword in
        let acc = f acc xhp_attribute_attributes in
        let acc = f acc xhp_attribute_semicolon in
        acc
      | XHPClassAttribute
          {
            xhp_attribute_decl_type;
            xhp_attribute_decl_name;
            xhp_attribute_decl_initializer;
            xhp_attribute_decl_required;
          } ->
        let acc = f acc xhp_attribute_decl_type in
        let acc = f acc xhp_attribute_decl_name in
        let acc = f acc xhp_attribute_decl_initializer in
        let acc = f acc xhp_attribute_decl_required in
        acc
      | XHPSimpleClassAttribute { xhp_simple_class_attribute_type } ->
        let acc = f acc xhp_simple_class_attribute_type in
        acc
      | XHPSimpleAttribute
          {
            xhp_simple_attribute_name;
            xhp_simple_attribute_equal;
            xhp_simple_attribute_expression;
          } ->
        let acc = f acc xhp_simple_attribute_name in
        let acc = f acc xhp_simple_attribute_equal in
        let acc = f acc xhp_simple_attribute_expression in
        acc
      | XHPSpreadAttribute
          {
            xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace;
          } ->
        let acc = f acc xhp_spread_attribute_left_brace in
        let acc = f acc xhp_spread_attribute_spread_operator in
        let acc = f acc xhp_spread_attribute_expression in
        let acc = f acc xhp_spread_attribute_right_brace in
        acc
      | XHPOpen
          {
            xhp_open_left_angle;
            xhp_open_name;
            xhp_open_attributes;
            xhp_open_right_angle;
          } ->
        let acc = f acc xhp_open_left_angle in
        let acc = f acc xhp_open_name in
        let acc = f acc xhp_open_attributes in
        let acc = f acc xhp_open_right_angle in
        acc
      | XHPExpression { xhp_open; xhp_body; xhp_close } ->
        let acc = f acc xhp_open in
        let acc = f acc xhp_body in
        let acc = f acc xhp_close in
        acc
      | XHPClose { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
        ->
        let acc = f acc xhp_close_left_angle in
        let acc = f acc xhp_close_name in
        let acc = f acc xhp_close_right_angle in
        acc
      | TypeConstant
          {
            type_constant_left_type;
            type_constant_separator;
            type_constant_right_type;
          } ->
        let acc = f acc type_constant_left_type in
        let acc = f acc type_constant_separator in
        let acc = f acc type_constant_right_type in
        acc
      | VectorTypeSpecifier
          {
            vector_type_keyword;
            vector_type_left_angle;
            vector_type_type;
            vector_type_trailing_comma;
            vector_type_right_angle;
          } ->
        let acc = f acc vector_type_keyword in
        let acc = f acc vector_type_left_angle in
        let acc = f acc vector_type_type in
        let acc = f acc vector_type_trailing_comma in
        let acc = f acc vector_type_right_angle in
        acc
      | KeysetTypeSpecifier
          {
            keyset_type_keyword;
            keyset_type_left_angle;
            keyset_type_type;
            keyset_type_trailing_comma;
            keyset_type_right_angle;
          } ->
        let acc = f acc keyset_type_keyword in
        let acc = f acc keyset_type_left_angle in
        let acc = f acc keyset_type_type in
        let acc = f acc keyset_type_trailing_comma in
        let acc = f acc keyset_type_right_angle in
        acc
      | TupleTypeExplicitSpecifier
          {
            tuple_type_keyword;
            tuple_type_left_angle;
            tuple_type_types;
            tuple_type_right_angle;
          } ->
        let acc = f acc tuple_type_keyword in
        let acc = f acc tuple_type_left_angle in
        let acc = f acc tuple_type_types in
        let acc = f acc tuple_type_right_angle in
        acc
      | VarrayTypeSpecifier
          {
            varray_keyword;
            varray_left_angle;
            varray_type;
            varray_trailing_comma;
            varray_right_angle;
          } ->
        let acc = f acc varray_keyword in
        let acc = f acc varray_left_angle in
        let acc = f acc varray_type in
        let acc = f acc varray_trailing_comma in
        let acc = f acc varray_right_angle in
        acc
      | FunctionCtxTypeSpecifier
          { function_ctx_type_keyword; function_ctx_type_variable } ->
        let acc = f acc function_ctx_type_keyword in
        let acc = f acc function_ctx_type_variable in
        acc
      | TypeParameter
          {
            type_attribute_spec;
            type_reified;
            type_variance;
            type_name;
            type_param_params;
            type_constraints;
          } ->
        let acc = f acc type_attribute_spec in
        let acc = f acc type_reified in
        let acc = f acc type_variance in
        let acc = f acc type_name in
        let acc = f acc type_param_params in
        let acc = f acc type_constraints in
        acc
      | TypeConstraint { constraint_keyword; constraint_type } ->
        let acc = f acc constraint_keyword in
        let acc = f acc constraint_type in
        acc
      | ContextConstraint { ctx_constraint_keyword; ctx_constraint_ctx_list } ->
        let acc = f acc ctx_constraint_keyword in
        let acc = f acc ctx_constraint_ctx_list in
        acc
      | DarrayTypeSpecifier
          {
            darray_keyword;
            darray_left_angle;
            darray_key;
            darray_comma;
            darray_value;
            darray_trailing_comma;
            darray_right_angle;
          } ->
        let acc = f acc darray_keyword in
        let acc = f acc darray_left_angle in
        let acc = f acc darray_key in
        let acc = f acc darray_comma in
        let acc = f acc darray_value in
        let acc = f acc darray_trailing_comma in
        let acc = f acc darray_right_angle in
        acc
      | DictionaryTypeSpecifier
          {
            dictionary_type_keyword;
            dictionary_type_left_angle;
            dictionary_type_members;
            dictionary_type_right_angle;
          } ->
        let acc = f acc dictionary_type_keyword in
        let acc = f acc dictionary_type_left_angle in
        let acc = f acc dictionary_type_members in
        let acc = f acc dictionary_type_right_angle in
        acc
      | ClosureTypeSpecifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          } ->
        let acc = f acc closure_outer_left_paren in
        let acc = f acc closure_readonly_keyword in
        let acc = f acc closure_function_keyword in
        let acc = f acc closure_inner_left_paren in
        let acc = f acc closure_parameter_list in
        let acc = f acc closure_inner_right_paren in
        let acc = f acc closure_contexts in
        let acc = f acc closure_colon in
        let acc = f acc closure_readonly_return in
        let acc = f acc closure_return_type in
        let acc = f acc closure_outer_right_paren in
        acc
      | ClosureParameterTypeSpecifier
          {
            closure_parameter_call_convention;
            closure_parameter_readonly;
            closure_parameter_type;
          } ->
        let acc = f acc closure_parameter_call_convention in
        let acc = f acc closure_parameter_readonly in
        let acc = f acc closure_parameter_type in
        acc
      | TypeRefinement
          {
            type_refinement_type;
            type_refinement_keyword;
            type_refinement_left_brace;
            type_refinement_members;
            type_refinement_right_brace;
          } ->
        let acc = f acc type_refinement_type in
        let acc = f acc type_refinement_keyword in
        let acc = f acc type_refinement_left_brace in
        let acc = f acc type_refinement_members in
        let acc = f acc type_refinement_right_brace in
        acc
      | TypeInRefinement
          {
            type_in_refinement_keyword;
            type_in_refinement_name;
            type_in_refinement_type_parameters;
            type_in_refinement_constraints;
            type_in_refinement_equal;
            type_in_refinement_type;
          } ->
        let acc = f acc type_in_refinement_keyword in
        let acc = f acc type_in_refinement_name in
        let acc = f acc type_in_refinement_type_parameters in
        let acc = f acc type_in_refinement_constraints in
        let acc = f acc type_in_refinement_equal in
        let acc = f acc type_in_refinement_type in
        acc
      | CtxInRefinement
          {
            ctx_in_refinement_keyword;
            ctx_in_refinement_name;
            ctx_in_refinement_type_parameters;
            ctx_in_refinement_constraints;
            ctx_in_refinement_equal;
            ctx_in_refinement_ctx_list;
          } ->
        let acc = f acc ctx_in_refinement_keyword in
        let acc = f acc ctx_in_refinement_name in
        let acc = f acc ctx_in_refinement_type_parameters in
        let acc = f acc ctx_in_refinement_constraints in
        let acc = f acc ctx_in_refinement_equal in
        let acc = f acc ctx_in_refinement_ctx_list in
        acc
      | ClassnameTypeSpecifier
          {
            classname_keyword;
            classname_left_angle;
            classname_type;
            classname_trailing_comma;
            classname_right_angle;
          } ->
        let acc = f acc classname_keyword in
        let acc = f acc classname_left_angle in
        let acc = f acc classname_type in
        let acc = f acc classname_trailing_comma in
        let acc = f acc classname_right_angle in
        acc
      | FieldSpecifier { field_question; field_name; field_arrow; field_type }
        ->
        let acc = f acc field_question in
        let acc = f acc field_name in
        let acc = f acc field_arrow in
        let acc = f acc field_type in
        acc
      | FieldInitializer
          {
            field_initializer_name;
            field_initializer_arrow;
            field_initializer_value;
          } ->
        let acc = f acc field_initializer_name in
        let acc = f acc field_initializer_arrow in
        let acc = f acc field_initializer_value in
        acc
      | ShapeTypeSpecifier
          {
            shape_type_keyword;
            shape_type_left_paren;
            shape_type_fields;
            shape_type_ellipsis;
            shape_type_right_paren;
          } ->
        let acc = f acc shape_type_keyword in
        let acc = f acc shape_type_left_paren in
        let acc = f acc shape_type_fields in
        let acc = f acc shape_type_ellipsis in
        let acc = f acc shape_type_right_paren in
        acc
      | ShapeExpression
          {
            shape_expression_keyword;
            shape_expression_left_paren;
            shape_expression_fields;
            shape_expression_right_paren;
          } ->
        let acc = f acc shape_expression_keyword in
        let acc = f acc shape_expression_left_paren in
        let acc = f acc shape_expression_fields in
        let acc = f acc shape_expression_right_paren in
        acc
      | TupleExpression
          {
            tuple_expression_keyword;
            tuple_expression_left_paren;
            tuple_expression_items;
            tuple_expression_right_paren;
          } ->
        let acc = f acc tuple_expression_keyword in
        let acc = f acc tuple_expression_left_paren in
        let acc = f acc tuple_expression_items in
        let acc = f acc tuple_expression_right_paren in
        acc
      | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
        let acc = f acc generic_class_type in
        let acc = f acc generic_argument_list in
        acc
      | NullableTypeSpecifier { nullable_question; nullable_type } ->
        let acc = f acc nullable_question in
        let acc = f acc nullable_type in
        acc
      | LikeTypeSpecifier { like_tilde; like_type } ->
        let acc = f acc like_tilde in
        let acc = f acc like_type in
        acc
      | SoftTypeSpecifier { soft_at; soft_type } ->
        let acc = f acc soft_at in
        let acc = f acc soft_type in
        acc
      | AttributizedSpecifier
          { attributized_specifier_attribute_spec; attributized_specifier_type }
        ->
        let acc = f acc attributized_specifier_attribute_spec in
        let acc = f acc attributized_specifier_type in
        acc
      | ReifiedTypeArgument
          { reified_type_argument_reified; reified_type_argument_type } ->
        let acc = f acc reified_type_argument_reified in
        let acc = f acc reified_type_argument_type in
        acc
      | TypeArguments
          {
            type_arguments_left_angle;
            type_arguments_types;
            type_arguments_right_angle;
          } ->
        let acc = f acc type_arguments_left_angle in
        let acc = f acc type_arguments_types in
        let acc = f acc type_arguments_right_angle in
        acc
      | TypeParameters
          {
            type_parameters_left_angle;
            type_parameters_parameters;
            type_parameters_right_angle;
          } ->
        let acc = f acc type_parameters_left_angle in
        let acc = f acc type_parameters_parameters in
        let acc = f acc type_parameters_right_angle in
        acc
      | TupleTypeSpecifier { tuple_left_paren; tuple_types; tuple_right_paren }
        ->
        let acc = f acc tuple_left_paren in
        let acc = f acc tuple_types in
        let acc = f acc tuple_right_paren in
        acc
      | UnionTypeSpecifier { union_left_paren; union_types; union_right_paren }
        ->
        let acc = f acc union_left_paren in
        let acc = f acc union_types in
        let acc = f acc union_right_paren in
        acc
      | IntersectionTypeSpecifier
          {
            intersection_left_paren;
            intersection_types;
            intersection_right_paren;
          } ->
        let acc = f acc intersection_left_paren in
        let acc = f acc intersection_types in
        let acc = f acc intersection_right_paren in
        acc
      | ErrorSyntax { error_error } ->
        let acc = f acc error_error in
        acc
      | ListItem { list_item; list_separator } ->
        let acc = f acc list_item in
        let acc = f acc list_separator in
        acc
      | EnumClassLabelExpression
          {
            enum_class_label_qualifier;
            enum_class_label_hash;
            enum_class_label_expression;
          } ->
        let acc = f acc enum_class_label_qualifier in
        let acc = f acc enum_class_label_hash in
        let acc = f acc enum_class_label_expression in
        acc
      | ModuleDeclaration
          {
            module_declaration_attribute_spec;
            module_declaration_new_keyword;
            module_declaration_module_keyword;
            module_declaration_name;
            module_declaration_left_brace;
            module_declaration_exports;
            module_declaration_imports;
            module_declaration_right_brace;
          } ->
        let acc = f acc module_declaration_attribute_spec in
        let acc = f acc module_declaration_new_keyword in
        let acc = f acc module_declaration_module_keyword in
        let acc = f acc module_declaration_name in
        let acc = f acc module_declaration_left_brace in
        let acc = f acc module_declaration_exports in
        let acc = f acc module_declaration_imports in
        let acc = f acc module_declaration_right_brace in
        acc
      | ModuleExports
          {
            module_exports_exports_keyword;
            module_exports_left_brace;
            module_exports_exports;
            module_exports_right_brace;
          } ->
        let acc = f acc module_exports_exports_keyword in
        let acc = f acc module_exports_left_brace in
        let acc = f acc module_exports_exports in
        let acc = f acc module_exports_right_brace in
        acc
      | ModuleImports
          {
            module_imports_imports_keyword;
            module_imports_left_brace;
            module_imports_imports;
            module_imports_right_brace;
          } ->
        let acc = f acc module_imports_imports_keyword in
        let acc = f acc module_imports_left_brace in
        let acc = f acc module_imports_imports in
        let acc = f acc module_imports_right_brace in
        acc
      | ModuleMembershipDeclaration
          {
            module_membership_declaration_module_keyword;
            module_membership_declaration_name;
            module_membership_declaration_semicolon;
          } ->
        let acc = f acc module_membership_declaration_module_keyword in
        let acc = f acc module_membership_declaration_name in
        let acc = f acc module_membership_declaration_semicolon in
        acc
      | PackageExpression
          { package_expression_keyword; package_expression_name } ->
        let acc = f acc package_expression_keyword in
        let acc = f acc package_expression_name in
        acc

    (* The order that the children are returned in should match the order
       that they appear in the source text *)
    let children_from_syntax s =
      match s with
      | Missing -> []
      | Token _ -> []
      | SyntaxList x -> x
      | EndOfFile { end_of_file_token } -> [end_of_file_token]
      | Script { script_declarations } -> [script_declarations]
      | QualifiedName { qualified_name_parts } -> [qualified_name_parts]
      | ModuleName { module_name_parts } -> [module_name_parts]
      | SimpleTypeSpecifier { simple_type_specifier } -> [simple_type_specifier]
      | LiteralExpression { literal_expression } -> [literal_expression]
      | PrefixedStringExpression { prefixed_string_name; prefixed_string_str }
        ->
        [prefixed_string_name; prefixed_string_str]
      | PrefixedCodeExpression
          {
            prefixed_code_prefix;
            prefixed_code_left_backtick;
            prefixed_code_body;
            prefixed_code_right_backtick;
          } ->
        [
          prefixed_code_prefix;
          prefixed_code_left_backtick;
          prefixed_code_body;
          prefixed_code_right_backtick;
        ]
      | VariableExpression { variable_expression } -> [variable_expression]
      | PipeVariableExpression { pipe_variable_expression } ->
        [pipe_variable_expression]
      | FileAttributeSpecification
          {
            file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword;
            file_attribute_specification_colon;
            file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle;
          } ->
        [
          file_attribute_specification_left_double_angle;
          file_attribute_specification_keyword;
          file_attribute_specification_colon;
          file_attribute_specification_attributes;
          file_attribute_specification_right_double_angle;
        ]
      | EnumDeclaration
          {
            enum_attribute_spec;
            enum_modifiers;
            enum_keyword;
            enum_name;
            enum_colon;
            enum_base;
            enum_type;
            enum_left_brace;
            enum_use_clauses;
            enum_enumerators;
            enum_right_brace;
          } ->
        [
          enum_attribute_spec;
          enum_modifiers;
          enum_keyword;
          enum_name;
          enum_colon;
          enum_base;
          enum_type;
          enum_left_brace;
          enum_use_clauses;
          enum_enumerators;
          enum_right_brace;
        ]
      | EnumUse { enum_use_keyword; enum_use_names; enum_use_semicolon } ->
        [enum_use_keyword; enum_use_names; enum_use_semicolon]
      | Enumerator
          {
            enumerator_name;
            enumerator_equal;
            enumerator_value;
            enumerator_semicolon;
          } ->
        [
          enumerator_name;
          enumerator_equal;
          enumerator_value;
          enumerator_semicolon;
        ]
      | EnumClassDeclaration
          {
            enum_class_attribute_spec;
            enum_class_modifiers;
            enum_class_enum_keyword;
            enum_class_class_keyword;
            enum_class_name;
            enum_class_colon;
            enum_class_base;
            enum_class_extends;
            enum_class_extends_list;
            enum_class_left_brace;
            enum_class_elements;
            enum_class_right_brace;
          } ->
        [
          enum_class_attribute_spec;
          enum_class_modifiers;
          enum_class_enum_keyword;
          enum_class_class_keyword;
          enum_class_name;
          enum_class_colon;
          enum_class_base;
          enum_class_extends;
          enum_class_extends_list;
          enum_class_left_brace;
          enum_class_elements;
          enum_class_right_brace;
        ]
      | EnumClassEnumerator
          {
            enum_class_enumerator_modifiers;
            enum_class_enumerator_type;
            enum_class_enumerator_name;
            enum_class_enumerator_initializer;
            enum_class_enumerator_semicolon;
          } ->
        [
          enum_class_enumerator_modifiers;
          enum_class_enumerator_type;
          enum_class_enumerator_name;
          enum_class_enumerator_initializer;
          enum_class_enumerator_semicolon;
        ]
      | AliasDeclaration
          {
            alias_attribute_spec;
            alias_modifiers;
            alias_module_kw_opt;
            alias_keyword;
            alias_name;
            alias_generic_parameter;
            alias_constraint;
            alias_equal;
            alias_type;
            alias_semicolon;
          } ->
        [
          alias_attribute_spec;
          alias_modifiers;
          alias_module_kw_opt;
          alias_keyword;
          alias_name;
          alias_generic_parameter;
          alias_constraint;
          alias_equal;
          alias_type;
          alias_semicolon;
        ]
      | ContextAliasDeclaration
          {
            ctx_alias_attribute_spec;
            ctx_alias_keyword;
            ctx_alias_name;
            ctx_alias_generic_parameter;
            ctx_alias_as_constraint;
            ctx_alias_equal;
            ctx_alias_context;
            ctx_alias_semicolon;
          } ->
        [
          ctx_alias_attribute_spec;
          ctx_alias_keyword;
          ctx_alias_name;
          ctx_alias_generic_parameter;
          ctx_alias_as_constraint;
          ctx_alias_equal;
          ctx_alias_context;
          ctx_alias_semicolon;
        ]
      | CaseTypeDeclaration
          {
            case_type_attribute_spec;
            case_type_modifiers;
            case_type_case_keyword;
            case_type_type_keyword;
            case_type_name;
            case_type_generic_parameter;
            case_type_as;
            case_type_bounds;
            case_type_equal;
            case_type_variants;
            case_type_semicolon;
          } ->
        [
          case_type_attribute_spec;
          case_type_modifiers;
          case_type_case_keyword;
          case_type_type_keyword;
          case_type_name;
          case_type_generic_parameter;
          case_type_as;
          case_type_bounds;
          case_type_equal;
          case_type_variants;
          case_type_semicolon;
        ]
      | CaseTypeVariant { case_type_variant_bar; case_type_variant_type } ->
        [case_type_variant_bar; case_type_variant_type]
      | PropertyDeclaration
          {
            property_attribute_spec;
            property_modifiers;
            property_type;
            property_declarators;
            property_semicolon;
          } ->
        [
          property_attribute_spec;
          property_modifiers;
          property_type;
          property_declarators;
          property_semicolon;
        ]
      | PropertyDeclarator { property_name; property_initializer } ->
        [property_name; property_initializer]
      | NamespaceDeclaration { namespace_header; namespace_body } ->
        [namespace_header; namespace_body]
      | NamespaceDeclarationHeader { namespace_keyword; namespace_name } ->
        [namespace_keyword; namespace_name]
      | NamespaceBody
          {
            namespace_left_brace;
            namespace_declarations;
            namespace_right_brace;
          } ->
        [namespace_left_brace; namespace_declarations; namespace_right_brace]
      | NamespaceEmptyBody { namespace_semicolon } -> [namespace_semicolon]
      | NamespaceUseDeclaration
          {
            namespace_use_keyword;
            namespace_use_kind;
            namespace_use_clauses;
            namespace_use_semicolon;
          } ->
        [
          namespace_use_keyword;
          namespace_use_kind;
          namespace_use_clauses;
          namespace_use_semicolon;
        ]
      | NamespaceGroupUseDeclaration
          {
            namespace_group_use_keyword;
            namespace_group_use_kind;
            namespace_group_use_prefix;
            namespace_group_use_left_brace;
            namespace_group_use_clauses;
            namespace_group_use_right_brace;
            namespace_group_use_semicolon;
          } ->
        [
          namespace_group_use_keyword;
          namespace_group_use_kind;
          namespace_group_use_prefix;
          namespace_group_use_left_brace;
          namespace_group_use_clauses;
          namespace_group_use_right_brace;
          namespace_group_use_semicolon;
        ]
      | NamespaceUseClause
          {
            namespace_use_clause_kind;
            namespace_use_name;
            namespace_use_as;
            namespace_use_alias;
          } ->
        [
          namespace_use_clause_kind;
          namespace_use_name;
          namespace_use_as;
          namespace_use_alias;
        ]
      | FunctionDeclaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          } ->
        [function_attribute_spec; function_declaration_header; function_body]
      | FunctionDeclarationHeader
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          } ->
        [
          function_modifiers;
          function_keyword;
          function_name;
          function_type_parameter_list;
          function_left_paren;
          function_parameter_list;
          function_right_paren;
          function_contexts;
          function_colon;
          function_readonly_return;
          function_type;
          function_where_clause;
        ]
      | Contexts
          { contexts_left_bracket; contexts_types; contexts_right_bracket } ->
        [contexts_left_bracket; contexts_types; contexts_right_bracket]
      | WhereClause { where_clause_keyword; where_clause_constraints } ->
        [where_clause_keyword; where_clause_constraints]
      | WhereConstraint
          {
            where_constraint_left_type;
            where_constraint_operator;
            where_constraint_right_type;
          } ->
        [
          where_constraint_left_type;
          where_constraint_operator;
          where_constraint_right_type;
        ]
      | MethodishDeclaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          } ->
        [
          methodish_attribute;
          methodish_function_decl_header;
          methodish_function_body;
          methodish_semicolon;
        ]
      | MethodishTraitResolution
          {
            methodish_trait_attribute;
            methodish_trait_function_decl_header;
            methodish_trait_equal;
            methodish_trait_name;
            methodish_trait_semicolon;
          } ->
        [
          methodish_trait_attribute;
          methodish_trait_function_decl_header;
          methodish_trait_equal;
          methodish_trait_name;
          methodish_trait_semicolon;
        ]
      | ClassishDeclaration
          {
            classish_attribute;
            classish_modifiers;
            classish_xhp;
            classish_keyword;
            classish_name;
            classish_type_parameters;
            classish_extends_keyword;
            classish_extends_list;
            classish_implements_keyword;
            classish_implements_list;
            classish_where_clause;
            classish_body;
          } ->
        [
          classish_attribute;
          classish_modifiers;
          classish_xhp;
          classish_keyword;
          classish_name;
          classish_type_parameters;
          classish_extends_keyword;
          classish_extends_list;
          classish_implements_keyword;
          classish_implements_list;
          classish_where_clause;
          classish_body;
        ]
      | ClassishBody
          {
            classish_body_left_brace;
            classish_body_elements;
            classish_body_right_brace;
          } ->
        [
          classish_body_left_brace;
          classish_body_elements;
          classish_body_right_brace;
        ]
      | TraitUse { trait_use_keyword; trait_use_names; trait_use_semicolon } ->
        [trait_use_keyword; trait_use_names; trait_use_semicolon]
      | RequireClause
          { require_keyword; require_kind; require_name; require_semicolon } ->
        [require_keyword; require_kind; require_name; require_semicolon]
      | ConstDeclaration
          {
            const_attribute_spec;
            const_modifiers;
            const_keyword;
            const_type_specifier;
            const_declarators;
            const_semicolon;
          } ->
        [
          const_attribute_spec;
          const_modifiers;
          const_keyword;
          const_type_specifier;
          const_declarators;
          const_semicolon;
        ]
      | ConstantDeclarator
          { constant_declarator_name; constant_declarator_initializer } ->
        [constant_declarator_name; constant_declarator_initializer]
      | TypeConstDeclaration
          {
            type_const_attribute_spec;
            type_const_modifiers;
            type_const_keyword;
            type_const_type_keyword;
            type_const_name;
            type_const_type_parameters;
            type_const_type_constraints;
            type_const_equal;
            type_const_type_specifier;
            type_const_semicolon;
          } ->
        [
          type_const_attribute_spec;
          type_const_modifiers;
          type_const_keyword;
          type_const_type_keyword;
          type_const_name;
          type_const_type_parameters;
          type_const_type_constraints;
          type_const_equal;
          type_const_type_specifier;
          type_const_semicolon;
        ]
      | ContextConstDeclaration
          {
            context_const_modifiers;
            context_const_const_keyword;
            context_const_ctx_keyword;
            context_const_name;
            context_const_type_parameters;
            context_const_constraint;
            context_const_equal;
            context_const_ctx_list;
            context_const_semicolon;
          } ->
        [
          context_const_modifiers;
          context_const_const_keyword;
          context_const_ctx_keyword;
          context_const_name;
          context_const_type_parameters;
          context_const_constraint;
          context_const_equal;
          context_const_ctx_list;
          context_const_semicolon;
        ]
      | DecoratedExpression
          { decorated_expression_decorator; decorated_expression_expression } ->
        [decorated_expression_decorator; decorated_expression_expression]
      | ParameterDeclaration
          {
            parameter_attribute;
            parameter_visibility;
            parameter_call_convention;
            parameter_readonly;
            parameter_type;
            parameter_name;
            parameter_default_value;
          } ->
        [
          parameter_attribute;
          parameter_visibility;
          parameter_call_convention;
          parameter_readonly;
          parameter_type;
          parameter_name;
          parameter_default_value;
        ]
      | VariadicParameter
          {
            variadic_parameter_call_convention;
            variadic_parameter_type;
            variadic_parameter_ellipsis;
          } ->
        [
          variadic_parameter_call_convention;
          variadic_parameter_type;
          variadic_parameter_ellipsis;
        ]
      | OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle;
          } ->
        [
          old_attribute_specification_left_double_angle;
          old_attribute_specification_attributes;
          old_attribute_specification_right_double_angle;
        ]
      | AttributeSpecification { attribute_specification_attributes } ->
        [attribute_specification_attributes]
      | Attribute { attribute_at; attribute_attribute_name } ->
        [attribute_at; attribute_attribute_name]
      | InclusionExpression { inclusion_require; inclusion_filename } ->
        [inclusion_require; inclusion_filename]
      | InclusionDirective { inclusion_expression; inclusion_semicolon } ->
        [inclusion_expression; inclusion_semicolon]
      | CompoundStatement
          { compound_left_brace; compound_statements; compound_right_brace } ->
        [compound_left_brace; compound_statements; compound_right_brace]
      | ExpressionStatement
          { expression_statement_expression; expression_statement_semicolon } ->
        [expression_statement_expression; expression_statement_semicolon]
      | MarkupSection { markup_hashbang; markup_suffix } ->
        [markup_hashbang; markup_suffix]
      | MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name } ->
        [markup_suffix_less_than_question; markup_suffix_name]
      | UnsetStatement
          {
            unset_keyword;
            unset_left_paren;
            unset_variables;
            unset_right_paren;
            unset_semicolon;
          } ->
        [
          unset_keyword;
          unset_left_paren;
          unset_variables;
          unset_right_paren;
          unset_semicolon;
        ]
      | DeclareLocalStatement
          {
            declare_local_keyword;
            declare_local_variable;
            declare_local_colon;
            declare_local_type;
            declare_local_initializer;
            declare_local_semicolon;
          } ->
        [
          declare_local_keyword;
          declare_local_variable;
          declare_local_colon;
          declare_local_type;
          declare_local_initializer;
          declare_local_semicolon;
        ]
      | UsingStatementBlockScoped
          {
            using_block_await_keyword;
            using_block_using_keyword;
            using_block_left_paren;
            using_block_expressions;
            using_block_right_paren;
            using_block_body;
          } ->
        [
          using_block_await_keyword;
          using_block_using_keyword;
          using_block_left_paren;
          using_block_expressions;
          using_block_right_paren;
          using_block_body;
        ]
      | UsingStatementFunctionScoped
          {
            using_function_await_keyword;
            using_function_using_keyword;
            using_function_expression;
            using_function_semicolon;
          } ->
        [
          using_function_await_keyword;
          using_function_using_keyword;
          using_function_expression;
          using_function_semicolon;
        ]
      | WhileStatement
          {
            while_keyword;
            while_left_paren;
            while_condition;
            while_right_paren;
            while_body;
          } ->
        [
          while_keyword;
          while_left_paren;
          while_condition;
          while_right_paren;
          while_body;
        ]
      | IfStatement
          {
            if_keyword;
            if_left_paren;
            if_condition;
            if_right_paren;
            if_statement;
            if_else_clause;
          } ->
        [
          if_keyword;
          if_left_paren;
          if_condition;
          if_right_paren;
          if_statement;
          if_else_clause;
        ]
      | ElseClause { else_keyword; else_statement } ->
        [else_keyword; else_statement]
      | TryStatement
          {
            try_keyword;
            try_compound_statement;
            try_catch_clauses;
            try_finally_clause;
          } ->
        [
          try_keyword;
          try_compound_statement;
          try_catch_clauses;
          try_finally_clause;
        ]
      | CatchClause
          {
            catch_keyword;
            catch_left_paren;
            catch_type;
            catch_variable;
            catch_right_paren;
            catch_body;
          } ->
        [
          catch_keyword;
          catch_left_paren;
          catch_type;
          catch_variable;
          catch_right_paren;
          catch_body;
        ]
      | FinallyClause { finally_keyword; finally_body } ->
        [finally_keyword; finally_body]
      | DoStatement
          {
            do_keyword;
            do_body;
            do_while_keyword;
            do_left_paren;
            do_condition;
            do_right_paren;
            do_semicolon;
          } ->
        [
          do_keyword;
          do_body;
          do_while_keyword;
          do_left_paren;
          do_condition;
          do_right_paren;
          do_semicolon;
        ]
      | ForStatement
          {
            for_keyword;
            for_left_paren;
            for_initializer;
            for_first_semicolon;
            for_control;
            for_second_semicolon;
            for_end_of_loop;
            for_right_paren;
            for_body;
          } ->
        [
          for_keyword;
          for_left_paren;
          for_initializer;
          for_first_semicolon;
          for_control;
          for_second_semicolon;
          for_end_of_loop;
          for_right_paren;
          for_body;
        ]
      | ForeachStatement
          {
            foreach_keyword;
            foreach_left_paren;
            foreach_collection;
            foreach_await_keyword;
            foreach_as;
            foreach_key;
            foreach_arrow;
            foreach_value;
            foreach_right_paren;
            foreach_body;
          } ->
        [
          foreach_keyword;
          foreach_left_paren;
          foreach_collection;
          foreach_await_keyword;
          foreach_as;
          foreach_key;
          foreach_arrow;
          foreach_value;
          foreach_right_paren;
          foreach_body;
        ]
      | SwitchStatement
          {
            switch_keyword;
            switch_left_paren;
            switch_expression;
            switch_right_paren;
            switch_left_brace;
            switch_sections;
            switch_right_brace;
          } ->
        [
          switch_keyword;
          switch_left_paren;
          switch_expression;
          switch_right_paren;
          switch_left_brace;
          switch_sections;
          switch_right_brace;
        ]
      | SwitchSection
          {
            switch_section_labels;
            switch_section_statements;
            switch_section_fallthrough;
          } ->
        [
          switch_section_labels;
          switch_section_statements;
          switch_section_fallthrough;
        ]
      | SwitchFallthrough { fallthrough_keyword; fallthrough_semicolon } ->
        [fallthrough_keyword; fallthrough_semicolon]
      | CaseLabel { case_keyword; case_expression; case_colon } ->
        [case_keyword; case_expression; case_colon]
      | DefaultLabel { default_keyword; default_colon } ->
        [default_keyword; default_colon]
      | MatchStatement
          {
            match_statement_keyword;
            match_statement_left_paren;
            match_statement_expression;
            match_statement_right_paren;
            match_statement_left_brace;
            match_statement_arms;
            match_statement_right_brace;
          } ->
        [
          match_statement_keyword;
          match_statement_left_paren;
          match_statement_expression;
          match_statement_right_paren;
          match_statement_left_brace;
          match_statement_arms;
          match_statement_right_brace;
        ]
      | MatchStatementArm
          {
            match_statement_arm_pattern;
            match_statement_arm_arrow;
            match_statement_arm_body;
          } ->
        [
          match_statement_arm_pattern;
          match_statement_arm_arrow;
          match_statement_arm_body;
        ]
      | ReturnStatement { return_keyword; return_expression; return_semicolon }
        ->
        [return_keyword; return_expression; return_semicolon]
      | YieldBreakStatement
          { yield_break_keyword; yield_break_break; yield_break_semicolon } ->
        [yield_break_keyword; yield_break_break; yield_break_semicolon]
      | ThrowStatement { throw_keyword; throw_expression; throw_semicolon } ->
        [throw_keyword; throw_expression; throw_semicolon]
      | BreakStatement { break_keyword; break_semicolon } ->
        [break_keyword; break_semicolon]
      | ContinueStatement { continue_keyword; continue_semicolon } ->
        [continue_keyword; continue_semicolon]
      | EchoStatement { echo_keyword; echo_expressions; echo_semicolon } ->
        [echo_keyword; echo_expressions; echo_semicolon]
      | ConcurrentStatement { concurrent_keyword; concurrent_statement } ->
        [concurrent_keyword; concurrent_statement]
      | SimpleInitializer { simple_initializer_equal; simple_initializer_value }
        ->
        [simple_initializer_equal; simple_initializer_value]
      | AnonymousClass
          {
            anonymous_class_class_keyword;
            anonymous_class_left_paren;
            anonymous_class_argument_list;
            anonymous_class_right_paren;
            anonymous_class_extends_keyword;
            anonymous_class_extends_list;
            anonymous_class_implements_keyword;
            anonymous_class_implements_list;
            anonymous_class_body;
          } ->
        [
          anonymous_class_class_keyword;
          anonymous_class_left_paren;
          anonymous_class_argument_list;
          anonymous_class_right_paren;
          anonymous_class_extends_keyword;
          anonymous_class_extends_list;
          anonymous_class_implements_keyword;
          anonymous_class_implements_list;
          anonymous_class_body;
        ]
      | AnonymousFunction
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          } ->
        [
          anonymous_attribute_spec;
          anonymous_async_keyword;
          anonymous_function_keyword;
          anonymous_left_paren;
          anonymous_parameters;
          anonymous_right_paren;
          anonymous_ctx_list;
          anonymous_colon;
          anonymous_readonly_return;
          anonymous_type;
          anonymous_use;
          anonymous_body;
        ]
      | AnonymousFunctionUseClause
          {
            anonymous_use_keyword;
            anonymous_use_left_paren;
            anonymous_use_variables;
            anonymous_use_right_paren;
          } ->
        [
          anonymous_use_keyword;
          anonymous_use_left_paren;
          anonymous_use_variables;
          anonymous_use_right_paren;
        ]
      | VariablePattern { variable_pattern_variable } ->
        [variable_pattern_variable]
      | ConstructorPattern
          {
            constructor_pattern_constructor;
            constructor_pattern_left_paren;
            constructor_pattern_members;
            constructor_pattern_right_paren;
          } ->
        [
          constructor_pattern_constructor;
          constructor_pattern_left_paren;
          constructor_pattern_members;
          constructor_pattern_right_paren;
        ]
      | RefinementPattern
          {
            refinement_pattern_variable;
            refinement_pattern_colon;
            refinement_pattern_specifier;
          } ->
        [
          refinement_pattern_variable;
          refinement_pattern_colon;
          refinement_pattern_specifier;
        ]
      | LambdaExpression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          } ->
        [
          lambda_attribute_spec;
          lambda_async;
          lambda_signature;
          lambda_arrow;
          lambda_body;
        ]
      | LambdaSignature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          } ->
        [
          lambda_left_paren;
          lambda_parameters;
          lambda_right_paren;
          lambda_contexts;
          lambda_colon;
          lambda_readonly_return;
          lambda_type;
        ]
      | CastExpression
          { cast_left_paren; cast_type; cast_right_paren; cast_operand } ->
        [cast_left_paren; cast_type; cast_right_paren; cast_operand]
      | ScopeResolutionExpression
          {
            scope_resolution_qualifier;
            scope_resolution_operator;
            scope_resolution_name;
          } ->
        [
          scope_resolution_qualifier;
          scope_resolution_operator;
          scope_resolution_name;
        ]
      | MemberSelectionExpression
          { member_object; member_operator; member_name } ->
        [member_object; member_operator; member_name]
      | SafeMemberSelectionExpression
          { safe_member_object; safe_member_operator; safe_member_name } ->
        [safe_member_object; safe_member_operator; safe_member_name]
      | EmbeddedMemberSelectionExpression
          {
            embedded_member_object;
            embedded_member_operator;
            embedded_member_name;
          } ->
        [embedded_member_object; embedded_member_operator; embedded_member_name]
      | YieldExpression { yield_keyword; yield_operand } ->
        [yield_keyword; yield_operand]
      | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand } ->
        [prefix_unary_operator; prefix_unary_operand]
      | PostfixUnaryExpression { postfix_unary_operand; postfix_unary_operator }
        ->
        [postfix_unary_operand; postfix_unary_operator]
      | BinaryExpression
          { binary_left_operand; binary_operator; binary_right_operand } ->
        [binary_left_operand; binary_operator; binary_right_operand]
      | IsExpression { is_left_operand; is_operator; is_right_operand } ->
        [is_left_operand; is_operator; is_right_operand]
      | AsExpression { as_left_operand; as_operator; as_right_operand } ->
        [as_left_operand; as_operator; as_right_operand]
      | NullableAsExpression
          {
            nullable_as_left_operand;
            nullable_as_operator;
            nullable_as_right_operand;
          } ->
        [
          nullable_as_left_operand;
          nullable_as_operator;
          nullable_as_right_operand;
        ]
      | UpcastExpression
          { upcast_left_operand; upcast_operator; upcast_right_operand } ->
        [upcast_left_operand; upcast_operator; upcast_right_operand]
      | ConditionalExpression
          {
            conditional_test;
            conditional_question;
            conditional_consequence;
            conditional_colon;
            conditional_alternative;
          } ->
        [
          conditional_test;
          conditional_question;
          conditional_consequence;
          conditional_colon;
          conditional_alternative;
        ]
      | EvalExpression
          { eval_keyword; eval_left_paren; eval_argument; eval_right_paren } ->
        [eval_keyword; eval_left_paren; eval_argument; eval_right_paren]
      | IssetExpression
          {
            isset_keyword;
            isset_left_paren;
            isset_argument_list;
            isset_right_paren;
          } ->
        [
          isset_keyword; isset_left_paren; isset_argument_list; isset_right_paren;
        ]
      | NameofExpression { nameof_keyword; nameof_target } ->
        [nameof_keyword; nameof_target]
      | FunctionCallExpression
          {
            function_call_receiver;
            function_call_type_args;
            function_call_left_paren;
            function_call_argument_list;
            function_call_right_paren;
          } ->
        [
          function_call_receiver;
          function_call_type_args;
          function_call_left_paren;
          function_call_argument_list;
          function_call_right_paren;
        ]
      | FunctionPointerExpression
          { function_pointer_receiver; function_pointer_type_args } ->
        [function_pointer_receiver; function_pointer_type_args]
      | ParenthesizedExpression
          {
            parenthesized_expression_left_paren;
            parenthesized_expression_expression;
            parenthesized_expression_right_paren;
          } ->
        [
          parenthesized_expression_left_paren;
          parenthesized_expression_expression;
          parenthesized_expression_right_paren;
        ]
      | BracedExpression
          {
            braced_expression_left_brace;
            braced_expression_expression;
            braced_expression_right_brace;
          } ->
        [
          braced_expression_left_brace;
          braced_expression_expression;
          braced_expression_right_brace;
        ]
      | ETSpliceExpression
          {
            et_splice_expression_dollar;
            et_splice_expression_left_brace;
            et_splice_expression_expression;
            et_splice_expression_right_brace;
          } ->
        [
          et_splice_expression_dollar;
          et_splice_expression_left_brace;
          et_splice_expression_expression;
          et_splice_expression_right_brace;
        ]
      | EmbeddedBracedExpression
          {
            embedded_braced_expression_left_brace;
            embedded_braced_expression_expression;
            embedded_braced_expression_right_brace;
          } ->
        [
          embedded_braced_expression_left_brace;
          embedded_braced_expression_expression;
          embedded_braced_expression_right_brace;
        ]
      | ListExpression
          { list_keyword; list_left_paren; list_members; list_right_paren } ->
        [list_keyword; list_left_paren; list_members; list_right_paren]
      | CollectionLiteralExpression
          {
            collection_literal_name;
            collection_literal_left_brace;
            collection_literal_initializers;
            collection_literal_right_brace;
          } ->
        [
          collection_literal_name;
          collection_literal_left_brace;
          collection_literal_initializers;
          collection_literal_right_brace;
        ]
      | ObjectCreationExpression
          { object_creation_new_keyword; object_creation_object } ->
        [object_creation_new_keyword; object_creation_object]
      | ConstructorCall
          {
            constructor_call_type;
            constructor_call_left_paren;
            constructor_call_argument_list;
            constructor_call_right_paren;
          } ->
        [
          constructor_call_type;
          constructor_call_left_paren;
          constructor_call_argument_list;
          constructor_call_right_paren;
        ]
      | DarrayIntrinsicExpression
          {
            darray_intrinsic_keyword;
            darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket;
            darray_intrinsic_members;
            darray_intrinsic_right_bracket;
          } ->
        [
          darray_intrinsic_keyword;
          darray_intrinsic_explicit_type;
          darray_intrinsic_left_bracket;
          darray_intrinsic_members;
          darray_intrinsic_right_bracket;
        ]
      | DictionaryIntrinsicExpression
          {
            dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket;
          } ->
        [
          dictionary_intrinsic_keyword;
          dictionary_intrinsic_explicit_type;
          dictionary_intrinsic_left_bracket;
          dictionary_intrinsic_members;
          dictionary_intrinsic_right_bracket;
        ]
      | KeysetIntrinsicExpression
          {
            keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket;
            keyset_intrinsic_members;
            keyset_intrinsic_right_bracket;
          } ->
        [
          keyset_intrinsic_keyword;
          keyset_intrinsic_explicit_type;
          keyset_intrinsic_left_bracket;
          keyset_intrinsic_members;
          keyset_intrinsic_right_bracket;
        ]
      | VarrayIntrinsicExpression
          {
            varray_intrinsic_keyword;
            varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket;
            varray_intrinsic_members;
            varray_intrinsic_right_bracket;
          } ->
        [
          varray_intrinsic_keyword;
          varray_intrinsic_explicit_type;
          varray_intrinsic_left_bracket;
          varray_intrinsic_members;
          varray_intrinsic_right_bracket;
        ]
      | VectorIntrinsicExpression
          {
            vector_intrinsic_keyword;
            vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket;
            vector_intrinsic_members;
            vector_intrinsic_right_bracket;
          } ->
        [
          vector_intrinsic_keyword;
          vector_intrinsic_explicit_type;
          vector_intrinsic_left_bracket;
          vector_intrinsic_members;
          vector_intrinsic_right_bracket;
        ]
      | ElementInitializer { element_key; element_arrow; element_value } ->
        [element_key; element_arrow; element_value]
      | SubscriptExpression
          {
            subscript_receiver;
            subscript_left_bracket;
            subscript_index;
            subscript_right_bracket;
          } ->
        [
          subscript_receiver;
          subscript_left_bracket;
          subscript_index;
          subscript_right_bracket;
        ]
      | EmbeddedSubscriptExpression
          {
            embedded_subscript_receiver;
            embedded_subscript_left_bracket;
            embedded_subscript_index;
            embedded_subscript_right_bracket;
          } ->
        [
          embedded_subscript_receiver;
          embedded_subscript_left_bracket;
          embedded_subscript_index;
          embedded_subscript_right_bracket;
        ]
      | AwaitableCreationExpression
          {
            awaitable_attribute_spec;
            awaitable_async;
            awaitable_compound_statement;
          } ->
        [
          awaitable_attribute_spec; awaitable_async; awaitable_compound_statement;
        ]
      | XHPChildrenDeclaration
          {
            xhp_children_keyword;
            xhp_children_expression;
            xhp_children_semicolon;
          } ->
        [xhp_children_keyword; xhp_children_expression; xhp_children_semicolon]
      | XHPChildrenParenthesizedList
          {
            xhp_children_list_left_paren;
            xhp_children_list_xhp_children;
            xhp_children_list_right_paren;
          } ->
        [
          xhp_children_list_left_paren;
          xhp_children_list_xhp_children;
          xhp_children_list_right_paren;
        ]
      | XHPCategoryDeclaration
          {
            xhp_category_keyword;
            xhp_category_categories;
            xhp_category_semicolon;
          } ->
        [xhp_category_keyword; xhp_category_categories; xhp_category_semicolon]
      | XHPEnumType
          {
            xhp_enum_like;
            xhp_enum_keyword;
            xhp_enum_left_brace;
            xhp_enum_values;
            xhp_enum_right_brace;
          } ->
        [
          xhp_enum_like;
          xhp_enum_keyword;
          xhp_enum_left_brace;
          xhp_enum_values;
          xhp_enum_right_brace;
        ]
      | XHPLateinit { xhp_lateinit_at; xhp_lateinit_keyword } ->
        [xhp_lateinit_at; xhp_lateinit_keyword]
      | XHPRequired { xhp_required_at; xhp_required_keyword } ->
        [xhp_required_at; xhp_required_keyword]
      | XHPClassAttributeDeclaration
          {
            xhp_attribute_keyword;
            xhp_attribute_attributes;
            xhp_attribute_semicolon;
          } ->
        [
          xhp_attribute_keyword;
          xhp_attribute_attributes;
          xhp_attribute_semicolon;
        ]
      | XHPClassAttribute
          {
            xhp_attribute_decl_type;
            xhp_attribute_decl_name;
            xhp_attribute_decl_initializer;
            xhp_attribute_decl_required;
          } ->
        [
          xhp_attribute_decl_type;
          xhp_attribute_decl_name;
          xhp_attribute_decl_initializer;
          xhp_attribute_decl_required;
        ]
      | XHPSimpleClassAttribute { xhp_simple_class_attribute_type } ->
        [xhp_simple_class_attribute_type]
      | XHPSimpleAttribute
          {
            xhp_simple_attribute_name;
            xhp_simple_attribute_equal;
            xhp_simple_attribute_expression;
          } ->
        [
          xhp_simple_attribute_name;
          xhp_simple_attribute_equal;
          xhp_simple_attribute_expression;
        ]
      | XHPSpreadAttribute
          {
            xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace;
          } ->
        [
          xhp_spread_attribute_left_brace;
          xhp_spread_attribute_spread_operator;
          xhp_spread_attribute_expression;
          xhp_spread_attribute_right_brace;
        ]
      | XHPOpen
          {
            xhp_open_left_angle;
            xhp_open_name;
            xhp_open_attributes;
            xhp_open_right_angle;
          } ->
        [
          xhp_open_left_angle;
          xhp_open_name;
          xhp_open_attributes;
          xhp_open_right_angle;
        ]
      | XHPExpression { xhp_open; xhp_body; xhp_close } ->
        [xhp_open; xhp_body; xhp_close]
      | XHPClose { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
        ->
        [xhp_close_left_angle; xhp_close_name; xhp_close_right_angle]
      | TypeConstant
          {
            type_constant_left_type;
            type_constant_separator;
            type_constant_right_type;
          } ->
        [
          type_constant_left_type;
          type_constant_separator;
          type_constant_right_type;
        ]
      | VectorTypeSpecifier
          {
            vector_type_keyword;
            vector_type_left_angle;
            vector_type_type;
            vector_type_trailing_comma;
            vector_type_right_angle;
          } ->
        [
          vector_type_keyword;
          vector_type_left_angle;
          vector_type_type;
          vector_type_trailing_comma;
          vector_type_right_angle;
        ]
      | KeysetTypeSpecifier
          {
            keyset_type_keyword;
            keyset_type_left_angle;
            keyset_type_type;
            keyset_type_trailing_comma;
            keyset_type_right_angle;
          } ->
        [
          keyset_type_keyword;
          keyset_type_left_angle;
          keyset_type_type;
          keyset_type_trailing_comma;
          keyset_type_right_angle;
        ]
      | TupleTypeExplicitSpecifier
          {
            tuple_type_keyword;
            tuple_type_left_angle;
            tuple_type_types;
            tuple_type_right_angle;
          } ->
        [
          tuple_type_keyword;
          tuple_type_left_angle;
          tuple_type_types;
          tuple_type_right_angle;
        ]
      | VarrayTypeSpecifier
          {
            varray_keyword;
            varray_left_angle;
            varray_type;
            varray_trailing_comma;
            varray_right_angle;
          } ->
        [
          varray_keyword;
          varray_left_angle;
          varray_type;
          varray_trailing_comma;
          varray_right_angle;
        ]
      | FunctionCtxTypeSpecifier
          { function_ctx_type_keyword; function_ctx_type_variable } ->
        [function_ctx_type_keyword; function_ctx_type_variable]
      | TypeParameter
          {
            type_attribute_spec;
            type_reified;
            type_variance;
            type_name;
            type_param_params;
            type_constraints;
          } ->
        [
          type_attribute_spec;
          type_reified;
          type_variance;
          type_name;
          type_param_params;
          type_constraints;
        ]
      | TypeConstraint { constraint_keyword; constraint_type } ->
        [constraint_keyword; constraint_type]
      | ContextConstraint { ctx_constraint_keyword; ctx_constraint_ctx_list } ->
        [ctx_constraint_keyword; ctx_constraint_ctx_list]
      | DarrayTypeSpecifier
          {
            darray_keyword;
            darray_left_angle;
            darray_key;
            darray_comma;
            darray_value;
            darray_trailing_comma;
            darray_right_angle;
          } ->
        [
          darray_keyword;
          darray_left_angle;
          darray_key;
          darray_comma;
          darray_value;
          darray_trailing_comma;
          darray_right_angle;
        ]
      | DictionaryTypeSpecifier
          {
            dictionary_type_keyword;
            dictionary_type_left_angle;
            dictionary_type_members;
            dictionary_type_right_angle;
          } ->
        [
          dictionary_type_keyword;
          dictionary_type_left_angle;
          dictionary_type_members;
          dictionary_type_right_angle;
        ]
      | ClosureTypeSpecifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          } ->
        [
          closure_outer_left_paren;
          closure_readonly_keyword;
          closure_function_keyword;
          closure_inner_left_paren;
          closure_parameter_list;
          closure_inner_right_paren;
          closure_contexts;
          closure_colon;
          closure_readonly_return;
          closure_return_type;
          closure_outer_right_paren;
        ]
      | ClosureParameterTypeSpecifier
          {
            closure_parameter_call_convention;
            closure_parameter_readonly;
            closure_parameter_type;
          } ->
        [
          closure_parameter_call_convention;
          closure_parameter_readonly;
          closure_parameter_type;
        ]
      | TypeRefinement
          {
            type_refinement_type;
            type_refinement_keyword;
            type_refinement_left_brace;
            type_refinement_members;
            type_refinement_right_brace;
          } ->
        [
          type_refinement_type;
          type_refinement_keyword;
          type_refinement_left_brace;
          type_refinement_members;
          type_refinement_right_brace;
        ]
      | TypeInRefinement
          {
            type_in_refinement_keyword;
            type_in_refinement_name;
            type_in_refinement_type_parameters;
            type_in_refinement_constraints;
            type_in_refinement_equal;
            type_in_refinement_type;
          } ->
        [
          type_in_refinement_keyword;
          type_in_refinement_name;
          type_in_refinement_type_parameters;
          type_in_refinement_constraints;
          type_in_refinement_equal;
          type_in_refinement_type;
        ]
      | CtxInRefinement
          {
            ctx_in_refinement_keyword;
            ctx_in_refinement_name;
            ctx_in_refinement_type_parameters;
            ctx_in_refinement_constraints;
            ctx_in_refinement_equal;
            ctx_in_refinement_ctx_list;
          } ->
        [
          ctx_in_refinement_keyword;
          ctx_in_refinement_name;
          ctx_in_refinement_type_parameters;
          ctx_in_refinement_constraints;
          ctx_in_refinement_equal;
          ctx_in_refinement_ctx_list;
        ]
      | ClassnameTypeSpecifier
          {
            classname_keyword;
            classname_left_angle;
            classname_type;
            classname_trailing_comma;
            classname_right_angle;
          } ->
        [
          classname_keyword;
          classname_left_angle;
          classname_type;
          classname_trailing_comma;
          classname_right_angle;
        ]
      | FieldSpecifier { field_question; field_name; field_arrow; field_type }
        ->
        [field_question; field_name; field_arrow; field_type]
      | FieldInitializer
          {
            field_initializer_name;
            field_initializer_arrow;
            field_initializer_value;
          } ->
        [
          field_initializer_name;
          field_initializer_arrow;
          field_initializer_value;
        ]
      | ShapeTypeSpecifier
          {
            shape_type_keyword;
            shape_type_left_paren;
            shape_type_fields;
            shape_type_ellipsis;
            shape_type_right_paren;
          } ->
        [
          shape_type_keyword;
          shape_type_left_paren;
          shape_type_fields;
          shape_type_ellipsis;
          shape_type_right_paren;
        ]
      | ShapeExpression
          {
            shape_expression_keyword;
            shape_expression_left_paren;
            shape_expression_fields;
            shape_expression_right_paren;
          } ->
        [
          shape_expression_keyword;
          shape_expression_left_paren;
          shape_expression_fields;
          shape_expression_right_paren;
        ]
      | TupleExpression
          {
            tuple_expression_keyword;
            tuple_expression_left_paren;
            tuple_expression_items;
            tuple_expression_right_paren;
          } ->
        [
          tuple_expression_keyword;
          tuple_expression_left_paren;
          tuple_expression_items;
          tuple_expression_right_paren;
        ]
      | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
        [generic_class_type; generic_argument_list]
      | NullableTypeSpecifier { nullable_question; nullable_type } ->
        [nullable_question; nullable_type]
      | LikeTypeSpecifier { like_tilde; like_type } -> [like_tilde; like_type]
      | SoftTypeSpecifier { soft_at; soft_type } -> [soft_at; soft_type]
      | AttributizedSpecifier
          { attributized_specifier_attribute_spec; attributized_specifier_type }
        ->
        [attributized_specifier_attribute_spec; attributized_specifier_type]
      | ReifiedTypeArgument
          { reified_type_argument_reified; reified_type_argument_type } ->
        [reified_type_argument_reified; reified_type_argument_type]
      | TypeArguments
          {
            type_arguments_left_angle;
            type_arguments_types;
            type_arguments_right_angle;
          } ->
        [
          type_arguments_left_angle;
          type_arguments_types;
          type_arguments_right_angle;
        ]
      | TypeParameters
          {
            type_parameters_left_angle;
            type_parameters_parameters;
            type_parameters_right_angle;
          } ->
        [
          type_parameters_left_angle;
          type_parameters_parameters;
          type_parameters_right_angle;
        ]
      | TupleTypeSpecifier { tuple_left_paren; tuple_types; tuple_right_paren }
        ->
        [tuple_left_paren; tuple_types; tuple_right_paren]
      | UnionTypeSpecifier { union_left_paren; union_types; union_right_paren }
        ->
        [union_left_paren; union_types; union_right_paren]
      | IntersectionTypeSpecifier
          {
            intersection_left_paren;
            intersection_types;
            intersection_right_paren;
          } ->
        [intersection_left_paren; intersection_types; intersection_right_paren]
      | ErrorSyntax { error_error } -> [error_error]
      | ListItem { list_item; list_separator } -> [list_item; list_separator]
      | EnumClassLabelExpression
          {
            enum_class_label_qualifier;
            enum_class_label_hash;
            enum_class_label_expression;
          } ->
        [
          enum_class_label_qualifier;
          enum_class_label_hash;
          enum_class_label_expression;
        ]
      | ModuleDeclaration
          {
            module_declaration_attribute_spec;
            module_declaration_new_keyword;
            module_declaration_module_keyword;
            module_declaration_name;
            module_declaration_left_brace;
            module_declaration_exports;
            module_declaration_imports;
            module_declaration_right_brace;
          } ->
        [
          module_declaration_attribute_spec;
          module_declaration_new_keyword;
          module_declaration_module_keyword;
          module_declaration_name;
          module_declaration_left_brace;
          module_declaration_exports;
          module_declaration_imports;
          module_declaration_right_brace;
        ]
      | ModuleExports
          {
            module_exports_exports_keyword;
            module_exports_left_brace;
            module_exports_exports;
            module_exports_right_brace;
          } ->
        [
          module_exports_exports_keyword;
          module_exports_left_brace;
          module_exports_exports;
          module_exports_right_brace;
        ]
      | ModuleImports
          {
            module_imports_imports_keyword;
            module_imports_left_brace;
            module_imports_imports;
            module_imports_right_brace;
          } ->
        [
          module_imports_imports_keyword;
          module_imports_left_brace;
          module_imports_imports;
          module_imports_right_brace;
        ]
      | ModuleMembershipDeclaration
          {
            module_membership_declaration_module_keyword;
            module_membership_declaration_name;
            module_membership_declaration_semicolon;
          } ->
        [
          module_membership_declaration_module_keyword;
          module_membership_declaration_name;
          module_membership_declaration_semicolon;
        ]
      | PackageExpression
          { package_expression_keyword; package_expression_name } ->
        [package_expression_keyword; package_expression_name]

    let children node = children_from_syntax node.syntax

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList _ -> []
      | EndOfFile { end_of_file_token } -> ["end_of_file_token"]
      | Script { script_declarations } -> ["script_declarations"]
      | QualifiedName { qualified_name_parts } -> ["qualified_name_parts"]
      | ModuleName { module_name_parts } -> ["module_name_parts"]
      | SimpleTypeSpecifier { simple_type_specifier } ->
        ["simple_type_specifier"]
      | LiteralExpression { literal_expression } -> ["literal_expression"]
      | PrefixedStringExpression { prefixed_string_name; prefixed_string_str }
        ->
        ["prefixed_string_name"; "prefixed_string_str"]
      | PrefixedCodeExpression
          {
            prefixed_code_prefix;
            prefixed_code_left_backtick;
            prefixed_code_body;
            prefixed_code_right_backtick;
          } ->
        [
          "prefixed_code_prefix";
          "prefixed_code_left_backtick";
          "prefixed_code_body";
          "prefixed_code_right_backtick";
        ]
      | VariableExpression { variable_expression } -> ["variable_expression"]
      | PipeVariableExpression { pipe_variable_expression } ->
        ["pipe_variable_expression"]
      | FileAttributeSpecification
          {
            file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword;
            file_attribute_specification_colon;
            file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle;
          } ->
        [
          "file_attribute_specification_left_double_angle";
          "file_attribute_specification_keyword";
          "file_attribute_specification_colon";
          "file_attribute_specification_attributes";
          "file_attribute_specification_right_double_angle";
        ]
      | EnumDeclaration
          {
            enum_attribute_spec;
            enum_modifiers;
            enum_keyword;
            enum_name;
            enum_colon;
            enum_base;
            enum_type;
            enum_left_brace;
            enum_use_clauses;
            enum_enumerators;
            enum_right_brace;
          } ->
        [
          "enum_attribute_spec";
          "enum_modifiers";
          "enum_keyword";
          "enum_name";
          "enum_colon";
          "enum_base";
          "enum_type";
          "enum_left_brace";
          "enum_use_clauses";
          "enum_enumerators";
          "enum_right_brace";
        ]
      | EnumUse { enum_use_keyword; enum_use_names; enum_use_semicolon } ->
        ["enum_use_keyword"; "enum_use_names"; "enum_use_semicolon"]
      | Enumerator
          {
            enumerator_name;
            enumerator_equal;
            enumerator_value;
            enumerator_semicolon;
          } ->
        [
          "enumerator_name";
          "enumerator_equal";
          "enumerator_value";
          "enumerator_semicolon";
        ]
      | EnumClassDeclaration
          {
            enum_class_attribute_spec;
            enum_class_modifiers;
            enum_class_enum_keyword;
            enum_class_class_keyword;
            enum_class_name;
            enum_class_colon;
            enum_class_base;
            enum_class_extends;
            enum_class_extends_list;
            enum_class_left_brace;
            enum_class_elements;
            enum_class_right_brace;
          } ->
        [
          "enum_class_attribute_spec";
          "enum_class_modifiers";
          "enum_class_enum_keyword";
          "enum_class_class_keyword";
          "enum_class_name";
          "enum_class_colon";
          "enum_class_base";
          "enum_class_extends";
          "enum_class_extends_list";
          "enum_class_left_brace";
          "enum_class_elements";
          "enum_class_right_brace";
        ]
      | EnumClassEnumerator
          {
            enum_class_enumerator_modifiers;
            enum_class_enumerator_type;
            enum_class_enumerator_name;
            enum_class_enumerator_initializer;
            enum_class_enumerator_semicolon;
          } ->
        [
          "enum_class_enumerator_modifiers";
          "enum_class_enumerator_type";
          "enum_class_enumerator_name";
          "enum_class_enumerator_initializer";
          "enum_class_enumerator_semicolon";
        ]
      | AliasDeclaration
          {
            alias_attribute_spec;
            alias_modifiers;
            alias_module_kw_opt;
            alias_keyword;
            alias_name;
            alias_generic_parameter;
            alias_constraint;
            alias_equal;
            alias_type;
            alias_semicolon;
          } ->
        [
          "alias_attribute_spec";
          "alias_modifiers";
          "alias_module_kw_opt";
          "alias_keyword";
          "alias_name";
          "alias_generic_parameter";
          "alias_constraint";
          "alias_equal";
          "alias_type";
          "alias_semicolon";
        ]
      | ContextAliasDeclaration
          {
            ctx_alias_attribute_spec;
            ctx_alias_keyword;
            ctx_alias_name;
            ctx_alias_generic_parameter;
            ctx_alias_as_constraint;
            ctx_alias_equal;
            ctx_alias_context;
            ctx_alias_semicolon;
          } ->
        [
          "ctx_alias_attribute_spec";
          "ctx_alias_keyword";
          "ctx_alias_name";
          "ctx_alias_generic_parameter";
          "ctx_alias_as_constraint";
          "ctx_alias_equal";
          "ctx_alias_context";
          "ctx_alias_semicolon";
        ]
      | CaseTypeDeclaration
          {
            case_type_attribute_spec;
            case_type_modifiers;
            case_type_case_keyword;
            case_type_type_keyword;
            case_type_name;
            case_type_generic_parameter;
            case_type_as;
            case_type_bounds;
            case_type_equal;
            case_type_variants;
            case_type_semicolon;
          } ->
        [
          "case_type_attribute_spec";
          "case_type_modifiers";
          "case_type_case_keyword";
          "case_type_type_keyword";
          "case_type_name";
          "case_type_generic_parameter";
          "case_type_as";
          "case_type_bounds";
          "case_type_equal";
          "case_type_variants";
          "case_type_semicolon";
        ]
      | CaseTypeVariant { case_type_variant_bar; case_type_variant_type } ->
        ["case_type_variant_bar"; "case_type_variant_type"]
      | PropertyDeclaration
          {
            property_attribute_spec;
            property_modifiers;
            property_type;
            property_declarators;
            property_semicolon;
          } ->
        [
          "property_attribute_spec";
          "property_modifiers";
          "property_type";
          "property_declarators";
          "property_semicolon";
        ]
      | PropertyDeclarator { property_name; property_initializer } ->
        ["property_name"; "property_initializer"]
      | NamespaceDeclaration { namespace_header; namespace_body } ->
        ["namespace_header"; "namespace_body"]
      | NamespaceDeclarationHeader { namespace_keyword; namespace_name } ->
        ["namespace_keyword"; "namespace_name"]
      | NamespaceBody
          {
            namespace_left_brace;
            namespace_declarations;
            namespace_right_brace;
          } ->
        [
          "namespace_left_brace";
          "namespace_declarations";
          "namespace_right_brace";
        ]
      | NamespaceEmptyBody { namespace_semicolon } -> ["namespace_semicolon"]
      | NamespaceUseDeclaration
          {
            namespace_use_keyword;
            namespace_use_kind;
            namespace_use_clauses;
            namespace_use_semicolon;
          } ->
        [
          "namespace_use_keyword";
          "namespace_use_kind";
          "namespace_use_clauses";
          "namespace_use_semicolon";
        ]
      | NamespaceGroupUseDeclaration
          {
            namespace_group_use_keyword;
            namespace_group_use_kind;
            namespace_group_use_prefix;
            namespace_group_use_left_brace;
            namespace_group_use_clauses;
            namespace_group_use_right_brace;
            namespace_group_use_semicolon;
          } ->
        [
          "namespace_group_use_keyword";
          "namespace_group_use_kind";
          "namespace_group_use_prefix";
          "namespace_group_use_left_brace";
          "namespace_group_use_clauses";
          "namespace_group_use_right_brace";
          "namespace_group_use_semicolon";
        ]
      | NamespaceUseClause
          {
            namespace_use_clause_kind;
            namespace_use_name;
            namespace_use_as;
            namespace_use_alias;
          } ->
        [
          "namespace_use_clause_kind";
          "namespace_use_name";
          "namespace_use_as";
          "namespace_use_alias";
        ]
      | FunctionDeclaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          } ->
        [
          "function_attribute_spec";
          "function_declaration_header";
          "function_body";
        ]
      | FunctionDeclarationHeader
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          } ->
        [
          "function_modifiers";
          "function_keyword";
          "function_name";
          "function_type_parameter_list";
          "function_left_paren";
          "function_parameter_list";
          "function_right_paren";
          "function_contexts";
          "function_colon";
          "function_readonly_return";
          "function_type";
          "function_where_clause";
        ]
      | Contexts
          { contexts_left_bracket; contexts_types; contexts_right_bracket } ->
        ["contexts_left_bracket"; "contexts_types"; "contexts_right_bracket"]
      | WhereClause { where_clause_keyword; where_clause_constraints } ->
        ["where_clause_keyword"; "where_clause_constraints"]
      | WhereConstraint
          {
            where_constraint_left_type;
            where_constraint_operator;
            where_constraint_right_type;
          } ->
        [
          "where_constraint_left_type";
          "where_constraint_operator";
          "where_constraint_right_type";
        ]
      | MethodishDeclaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          } ->
        [
          "methodish_attribute";
          "methodish_function_decl_header";
          "methodish_function_body";
          "methodish_semicolon";
        ]
      | MethodishTraitResolution
          {
            methodish_trait_attribute;
            methodish_trait_function_decl_header;
            methodish_trait_equal;
            methodish_trait_name;
            methodish_trait_semicolon;
          } ->
        [
          "methodish_trait_attribute";
          "methodish_trait_function_decl_header";
          "methodish_trait_equal";
          "methodish_trait_name";
          "methodish_trait_semicolon";
        ]
      | ClassishDeclaration
          {
            classish_attribute;
            classish_modifiers;
            classish_xhp;
            classish_keyword;
            classish_name;
            classish_type_parameters;
            classish_extends_keyword;
            classish_extends_list;
            classish_implements_keyword;
            classish_implements_list;
            classish_where_clause;
            classish_body;
          } ->
        [
          "classish_attribute";
          "classish_modifiers";
          "classish_xhp";
          "classish_keyword";
          "classish_name";
          "classish_type_parameters";
          "classish_extends_keyword";
          "classish_extends_list";
          "classish_implements_keyword";
          "classish_implements_list";
          "classish_where_clause";
          "classish_body";
        ]
      | ClassishBody
          {
            classish_body_left_brace;
            classish_body_elements;
            classish_body_right_brace;
          } ->
        [
          "classish_body_left_brace";
          "classish_body_elements";
          "classish_body_right_brace";
        ]
      | TraitUse { trait_use_keyword; trait_use_names; trait_use_semicolon } ->
        ["trait_use_keyword"; "trait_use_names"; "trait_use_semicolon"]
      | RequireClause
          { require_keyword; require_kind; require_name; require_semicolon } ->
        ["require_keyword"; "require_kind"; "require_name"; "require_semicolon"]
      | ConstDeclaration
          {
            const_attribute_spec;
            const_modifiers;
            const_keyword;
            const_type_specifier;
            const_declarators;
            const_semicolon;
          } ->
        [
          "const_attribute_spec";
          "const_modifiers";
          "const_keyword";
          "const_type_specifier";
          "const_declarators";
          "const_semicolon";
        ]
      | ConstantDeclarator
          { constant_declarator_name; constant_declarator_initializer } ->
        ["constant_declarator_name"; "constant_declarator_initializer"]
      | TypeConstDeclaration
          {
            type_const_attribute_spec;
            type_const_modifiers;
            type_const_keyword;
            type_const_type_keyword;
            type_const_name;
            type_const_type_parameters;
            type_const_type_constraints;
            type_const_equal;
            type_const_type_specifier;
            type_const_semicolon;
          } ->
        [
          "type_const_attribute_spec";
          "type_const_modifiers";
          "type_const_keyword";
          "type_const_type_keyword";
          "type_const_name";
          "type_const_type_parameters";
          "type_const_type_constraints";
          "type_const_equal";
          "type_const_type_specifier";
          "type_const_semicolon";
        ]
      | ContextConstDeclaration
          {
            context_const_modifiers;
            context_const_const_keyword;
            context_const_ctx_keyword;
            context_const_name;
            context_const_type_parameters;
            context_const_constraint;
            context_const_equal;
            context_const_ctx_list;
            context_const_semicolon;
          } ->
        [
          "context_const_modifiers";
          "context_const_const_keyword";
          "context_const_ctx_keyword";
          "context_const_name";
          "context_const_type_parameters";
          "context_const_constraint";
          "context_const_equal";
          "context_const_ctx_list";
          "context_const_semicolon";
        ]
      | DecoratedExpression
          { decorated_expression_decorator; decorated_expression_expression } ->
        ["decorated_expression_decorator"; "decorated_expression_expression"]
      | ParameterDeclaration
          {
            parameter_attribute;
            parameter_visibility;
            parameter_call_convention;
            parameter_readonly;
            parameter_type;
            parameter_name;
            parameter_default_value;
          } ->
        [
          "parameter_attribute";
          "parameter_visibility";
          "parameter_call_convention";
          "parameter_readonly";
          "parameter_type";
          "parameter_name";
          "parameter_default_value";
        ]
      | VariadicParameter
          {
            variadic_parameter_call_convention;
            variadic_parameter_type;
            variadic_parameter_ellipsis;
          } ->
        [
          "variadic_parameter_call_convention";
          "variadic_parameter_type";
          "variadic_parameter_ellipsis";
        ]
      | OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle;
          } ->
        [
          "old_attribute_specification_left_double_angle";
          "old_attribute_specification_attributes";
          "old_attribute_specification_right_double_angle";
        ]
      | AttributeSpecification { attribute_specification_attributes } ->
        ["attribute_specification_attributes"]
      | Attribute { attribute_at; attribute_attribute_name } ->
        ["attribute_at"; "attribute_attribute_name"]
      | InclusionExpression { inclusion_require; inclusion_filename } ->
        ["inclusion_require"; "inclusion_filename"]
      | InclusionDirective { inclusion_expression; inclusion_semicolon } ->
        ["inclusion_expression"; "inclusion_semicolon"]
      | CompoundStatement
          { compound_left_brace; compound_statements; compound_right_brace } ->
        ["compound_left_brace"; "compound_statements"; "compound_right_brace"]
      | ExpressionStatement
          { expression_statement_expression; expression_statement_semicolon } ->
        ["expression_statement_expression"; "expression_statement_semicolon"]
      | MarkupSection { markup_hashbang; markup_suffix } ->
        ["markup_hashbang"; "markup_suffix"]
      | MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name } ->
        ["markup_suffix_less_than_question"; "markup_suffix_name"]
      | UnsetStatement
          {
            unset_keyword;
            unset_left_paren;
            unset_variables;
            unset_right_paren;
            unset_semicolon;
          } ->
        [
          "unset_keyword";
          "unset_left_paren";
          "unset_variables";
          "unset_right_paren";
          "unset_semicolon";
        ]
      | DeclareLocalStatement
          {
            declare_local_keyword;
            declare_local_variable;
            declare_local_colon;
            declare_local_type;
            declare_local_initializer;
            declare_local_semicolon;
          } ->
        [
          "declare_local_keyword";
          "declare_local_variable";
          "declare_local_colon";
          "declare_local_type";
          "declare_local_initializer";
          "declare_local_semicolon";
        ]
      | UsingStatementBlockScoped
          {
            using_block_await_keyword;
            using_block_using_keyword;
            using_block_left_paren;
            using_block_expressions;
            using_block_right_paren;
            using_block_body;
          } ->
        [
          "using_block_await_keyword";
          "using_block_using_keyword";
          "using_block_left_paren";
          "using_block_expressions";
          "using_block_right_paren";
          "using_block_body";
        ]
      | UsingStatementFunctionScoped
          {
            using_function_await_keyword;
            using_function_using_keyword;
            using_function_expression;
            using_function_semicolon;
          } ->
        [
          "using_function_await_keyword";
          "using_function_using_keyword";
          "using_function_expression";
          "using_function_semicolon";
        ]
      | WhileStatement
          {
            while_keyword;
            while_left_paren;
            while_condition;
            while_right_paren;
            while_body;
          } ->
        [
          "while_keyword";
          "while_left_paren";
          "while_condition";
          "while_right_paren";
          "while_body";
        ]
      | IfStatement
          {
            if_keyword;
            if_left_paren;
            if_condition;
            if_right_paren;
            if_statement;
            if_else_clause;
          } ->
        [
          "if_keyword";
          "if_left_paren";
          "if_condition";
          "if_right_paren";
          "if_statement";
          "if_else_clause";
        ]
      | ElseClause { else_keyword; else_statement } ->
        ["else_keyword"; "else_statement"]
      | TryStatement
          {
            try_keyword;
            try_compound_statement;
            try_catch_clauses;
            try_finally_clause;
          } ->
        [
          "try_keyword";
          "try_compound_statement";
          "try_catch_clauses";
          "try_finally_clause";
        ]
      | CatchClause
          {
            catch_keyword;
            catch_left_paren;
            catch_type;
            catch_variable;
            catch_right_paren;
            catch_body;
          } ->
        [
          "catch_keyword";
          "catch_left_paren";
          "catch_type";
          "catch_variable";
          "catch_right_paren";
          "catch_body";
        ]
      | FinallyClause { finally_keyword; finally_body } ->
        ["finally_keyword"; "finally_body"]
      | DoStatement
          {
            do_keyword;
            do_body;
            do_while_keyword;
            do_left_paren;
            do_condition;
            do_right_paren;
            do_semicolon;
          } ->
        [
          "do_keyword";
          "do_body";
          "do_while_keyword";
          "do_left_paren";
          "do_condition";
          "do_right_paren";
          "do_semicolon";
        ]
      | ForStatement
          {
            for_keyword;
            for_left_paren;
            for_initializer;
            for_first_semicolon;
            for_control;
            for_second_semicolon;
            for_end_of_loop;
            for_right_paren;
            for_body;
          } ->
        [
          "for_keyword";
          "for_left_paren";
          "for_initializer";
          "for_first_semicolon";
          "for_control";
          "for_second_semicolon";
          "for_end_of_loop";
          "for_right_paren";
          "for_body";
        ]
      | ForeachStatement
          {
            foreach_keyword;
            foreach_left_paren;
            foreach_collection;
            foreach_await_keyword;
            foreach_as;
            foreach_key;
            foreach_arrow;
            foreach_value;
            foreach_right_paren;
            foreach_body;
          } ->
        [
          "foreach_keyword";
          "foreach_left_paren";
          "foreach_collection";
          "foreach_await_keyword";
          "foreach_as";
          "foreach_key";
          "foreach_arrow";
          "foreach_value";
          "foreach_right_paren";
          "foreach_body";
        ]
      | SwitchStatement
          {
            switch_keyword;
            switch_left_paren;
            switch_expression;
            switch_right_paren;
            switch_left_brace;
            switch_sections;
            switch_right_brace;
          } ->
        [
          "switch_keyword";
          "switch_left_paren";
          "switch_expression";
          "switch_right_paren";
          "switch_left_brace";
          "switch_sections";
          "switch_right_brace";
        ]
      | SwitchSection
          {
            switch_section_labels;
            switch_section_statements;
            switch_section_fallthrough;
          } ->
        [
          "switch_section_labels";
          "switch_section_statements";
          "switch_section_fallthrough";
        ]
      | SwitchFallthrough { fallthrough_keyword; fallthrough_semicolon } ->
        ["fallthrough_keyword"; "fallthrough_semicolon"]
      | CaseLabel { case_keyword; case_expression; case_colon } ->
        ["case_keyword"; "case_expression"; "case_colon"]
      | DefaultLabel { default_keyword; default_colon } ->
        ["default_keyword"; "default_colon"]
      | MatchStatement
          {
            match_statement_keyword;
            match_statement_left_paren;
            match_statement_expression;
            match_statement_right_paren;
            match_statement_left_brace;
            match_statement_arms;
            match_statement_right_brace;
          } ->
        [
          "match_statement_keyword";
          "match_statement_left_paren";
          "match_statement_expression";
          "match_statement_right_paren";
          "match_statement_left_brace";
          "match_statement_arms";
          "match_statement_right_brace";
        ]
      | MatchStatementArm
          {
            match_statement_arm_pattern;
            match_statement_arm_arrow;
            match_statement_arm_body;
          } ->
        [
          "match_statement_arm_pattern";
          "match_statement_arm_arrow";
          "match_statement_arm_body";
        ]
      | ReturnStatement { return_keyword; return_expression; return_semicolon }
        ->
        ["return_keyword"; "return_expression"; "return_semicolon"]
      | YieldBreakStatement
          { yield_break_keyword; yield_break_break; yield_break_semicolon } ->
        ["yield_break_keyword"; "yield_break_break"; "yield_break_semicolon"]
      | ThrowStatement { throw_keyword; throw_expression; throw_semicolon } ->
        ["throw_keyword"; "throw_expression"; "throw_semicolon"]
      | BreakStatement { break_keyword; break_semicolon } ->
        ["break_keyword"; "break_semicolon"]
      | ContinueStatement { continue_keyword; continue_semicolon } ->
        ["continue_keyword"; "continue_semicolon"]
      | EchoStatement { echo_keyword; echo_expressions; echo_semicolon } ->
        ["echo_keyword"; "echo_expressions"; "echo_semicolon"]
      | ConcurrentStatement { concurrent_keyword; concurrent_statement } ->
        ["concurrent_keyword"; "concurrent_statement"]
      | SimpleInitializer { simple_initializer_equal; simple_initializer_value }
        ->
        ["simple_initializer_equal"; "simple_initializer_value"]
      | AnonymousClass
          {
            anonymous_class_class_keyword;
            anonymous_class_left_paren;
            anonymous_class_argument_list;
            anonymous_class_right_paren;
            anonymous_class_extends_keyword;
            anonymous_class_extends_list;
            anonymous_class_implements_keyword;
            anonymous_class_implements_list;
            anonymous_class_body;
          } ->
        [
          "anonymous_class_class_keyword";
          "anonymous_class_left_paren";
          "anonymous_class_argument_list";
          "anonymous_class_right_paren";
          "anonymous_class_extends_keyword";
          "anonymous_class_extends_list";
          "anonymous_class_implements_keyword";
          "anonymous_class_implements_list";
          "anonymous_class_body";
        ]
      | AnonymousFunction
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          } ->
        [
          "anonymous_attribute_spec";
          "anonymous_async_keyword";
          "anonymous_function_keyword";
          "anonymous_left_paren";
          "anonymous_parameters";
          "anonymous_right_paren";
          "anonymous_ctx_list";
          "anonymous_colon";
          "anonymous_readonly_return";
          "anonymous_type";
          "anonymous_use";
          "anonymous_body";
        ]
      | AnonymousFunctionUseClause
          {
            anonymous_use_keyword;
            anonymous_use_left_paren;
            anonymous_use_variables;
            anonymous_use_right_paren;
          } ->
        [
          "anonymous_use_keyword";
          "anonymous_use_left_paren";
          "anonymous_use_variables";
          "anonymous_use_right_paren";
        ]
      | VariablePattern { variable_pattern_variable } ->
        ["variable_pattern_variable"]
      | ConstructorPattern
          {
            constructor_pattern_constructor;
            constructor_pattern_left_paren;
            constructor_pattern_members;
            constructor_pattern_right_paren;
          } ->
        [
          "constructor_pattern_constructor";
          "constructor_pattern_left_paren";
          "constructor_pattern_members";
          "constructor_pattern_right_paren";
        ]
      | RefinementPattern
          {
            refinement_pattern_variable;
            refinement_pattern_colon;
            refinement_pattern_specifier;
          } ->
        [
          "refinement_pattern_variable";
          "refinement_pattern_colon";
          "refinement_pattern_specifier";
        ]
      | LambdaExpression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          } ->
        [
          "lambda_attribute_spec";
          "lambda_async";
          "lambda_signature";
          "lambda_arrow";
          "lambda_body";
        ]
      | LambdaSignature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          } ->
        [
          "lambda_left_paren";
          "lambda_parameters";
          "lambda_right_paren";
          "lambda_contexts";
          "lambda_colon";
          "lambda_readonly_return";
          "lambda_type";
        ]
      | CastExpression
          { cast_left_paren; cast_type; cast_right_paren; cast_operand } ->
        ["cast_left_paren"; "cast_type"; "cast_right_paren"; "cast_operand"]
      | ScopeResolutionExpression
          {
            scope_resolution_qualifier;
            scope_resolution_operator;
            scope_resolution_name;
          } ->
        [
          "scope_resolution_qualifier";
          "scope_resolution_operator";
          "scope_resolution_name";
        ]
      | MemberSelectionExpression
          { member_object; member_operator; member_name } ->
        ["member_object"; "member_operator"; "member_name"]
      | SafeMemberSelectionExpression
          { safe_member_object; safe_member_operator; safe_member_name } ->
        ["safe_member_object"; "safe_member_operator"; "safe_member_name"]
      | EmbeddedMemberSelectionExpression
          {
            embedded_member_object;
            embedded_member_operator;
            embedded_member_name;
          } ->
        [
          "embedded_member_object";
          "embedded_member_operator";
          "embedded_member_name";
        ]
      | YieldExpression { yield_keyword; yield_operand } ->
        ["yield_keyword"; "yield_operand"]
      | PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand } ->
        ["prefix_unary_operator"; "prefix_unary_operand"]
      | PostfixUnaryExpression { postfix_unary_operand; postfix_unary_operator }
        ->
        ["postfix_unary_operand"; "postfix_unary_operator"]
      | BinaryExpression
          { binary_left_operand; binary_operator; binary_right_operand } ->
        ["binary_left_operand"; "binary_operator"; "binary_right_operand"]
      | IsExpression { is_left_operand; is_operator; is_right_operand } ->
        ["is_left_operand"; "is_operator"; "is_right_operand"]
      | AsExpression { as_left_operand; as_operator; as_right_operand } ->
        ["as_left_operand"; "as_operator"; "as_right_operand"]
      | NullableAsExpression
          {
            nullable_as_left_operand;
            nullable_as_operator;
            nullable_as_right_operand;
          } ->
        [
          "nullable_as_left_operand";
          "nullable_as_operator";
          "nullable_as_right_operand";
        ]
      | UpcastExpression
          { upcast_left_operand; upcast_operator; upcast_right_operand } ->
        ["upcast_left_operand"; "upcast_operator"; "upcast_right_operand"]
      | ConditionalExpression
          {
            conditional_test;
            conditional_question;
            conditional_consequence;
            conditional_colon;
            conditional_alternative;
          } ->
        [
          "conditional_test";
          "conditional_question";
          "conditional_consequence";
          "conditional_colon";
          "conditional_alternative";
        ]
      | EvalExpression
          { eval_keyword; eval_left_paren; eval_argument; eval_right_paren } ->
        ["eval_keyword"; "eval_left_paren"; "eval_argument"; "eval_right_paren"]
      | IssetExpression
          {
            isset_keyword;
            isset_left_paren;
            isset_argument_list;
            isset_right_paren;
          } ->
        [
          "isset_keyword";
          "isset_left_paren";
          "isset_argument_list";
          "isset_right_paren";
        ]
      | NameofExpression { nameof_keyword; nameof_target } ->
        ["nameof_keyword"; "nameof_target"]
      | FunctionCallExpression
          {
            function_call_receiver;
            function_call_type_args;
            function_call_left_paren;
            function_call_argument_list;
            function_call_right_paren;
          } ->
        [
          "function_call_receiver";
          "function_call_type_args";
          "function_call_left_paren";
          "function_call_argument_list";
          "function_call_right_paren";
        ]
      | FunctionPointerExpression
          { function_pointer_receiver; function_pointer_type_args } ->
        ["function_pointer_receiver"; "function_pointer_type_args"]
      | ParenthesizedExpression
          {
            parenthesized_expression_left_paren;
            parenthesized_expression_expression;
            parenthesized_expression_right_paren;
          } ->
        [
          "parenthesized_expression_left_paren";
          "parenthesized_expression_expression";
          "parenthesized_expression_right_paren";
        ]
      | BracedExpression
          {
            braced_expression_left_brace;
            braced_expression_expression;
            braced_expression_right_brace;
          } ->
        [
          "braced_expression_left_brace";
          "braced_expression_expression";
          "braced_expression_right_brace";
        ]
      | ETSpliceExpression
          {
            et_splice_expression_dollar;
            et_splice_expression_left_brace;
            et_splice_expression_expression;
            et_splice_expression_right_brace;
          } ->
        [
          "et_splice_expression_dollar";
          "et_splice_expression_left_brace";
          "et_splice_expression_expression";
          "et_splice_expression_right_brace";
        ]
      | EmbeddedBracedExpression
          {
            embedded_braced_expression_left_brace;
            embedded_braced_expression_expression;
            embedded_braced_expression_right_brace;
          } ->
        [
          "embedded_braced_expression_left_brace";
          "embedded_braced_expression_expression";
          "embedded_braced_expression_right_brace";
        ]
      | ListExpression
          { list_keyword; list_left_paren; list_members; list_right_paren } ->
        ["list_keyword"; "list_left_paren"; "list_members"; "list_right_paren"]
      | CollectionLiteralExpression
          {
            collection_literal_name;
            collection_literal_left_brace;
            collection_literal_initializers;
            collection_literal_right_brace;
          } ->
        [
          "collection_literal_name";
          "collection_literal_left_brace";
          "collection_literal_initializers";
          "collection_literal_right_brace";
        ]
      | ObjectCreationExpression
          { object_creation_new_keyword; object_creation_object } ->
        ["object_creation_new_keyword"; "object_creation_object"]
      | ConstructorCall
          {
            constructor_call_type;
            constructor_call_left_paren;
            constructor_call_argument_list;
            constructor_call_right_paren;
          } ->
        [
          "constructor_call_type";
          "constructor_call_left_paren";
          "constructor_call_argument_list";
          "constructor_call_right_paren";
        ]
      | DarrayIntrinsicExpression
          {
            darray_intrinsic_keyword;
            darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket;
            darray_intrinsic_members;
            darray_intrinsic_right_bracket;
          } ->
        [
          "darray_intrinsic_keyword";
          "darray_intrinsic_explicit_type";
          "darray_intrinsic_left_bracket";
          "darray_intrinsic_members";
          "darray_intrinsic_right_bracket";
        ]
      | DictionaryIntrinsicExpression
          {
            dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket;
          } ->
        [
          "dictionary_intrinsic_keyword";
          "dictionary_intrinsic_explicit_type";
          "dictionary_intrinsic_left_bracket";
          "dictionary_intrinsic_members";
          "dictionary_intrinsic_right_bracket";
        ]
      | KeysetIntrinsicExpression
          {
            keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket;
            keyset_intrinsic_members;
            keyset_intrinsic_right_bracket;
          } ->
        [
          "keyset_intrinsic_keyword";
          "keyset_intrinsic_explicit_type";
          "keyset_intrinsic_left_bracket";
          "keyset_intrinsic_members";
          "keyset_intrinsic_right_bracket";
        ]
      | VarrayIntrinsicExpression
          {
            varray_intrinsic_keyword;
            varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket;
            varray_intrinsic_members;
            varray_intrinsic_right_bracket;
          } ->
        [
          "varray_intrinsic_keyword";
          "varray_intrinsic_explicit_type";
          "varray_intrinsic_left_bracket";
          "varray_intrinsic_members";
          "varray_intrinsic_right_bracket";
        ]
      | VectorIntrinsicExpression
          {
            vector_intrinsic_keyword;
            vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket;
            vector_intrinsic_members;
            vector_intrinsic_right_bracket;
          } ->
        [
          "vector_intrinsic_keyword";
          "vector_intrinsic_explicit_type";
          "vector_intrinsic_left_bracket";
          "vector_intrinsic_members";
          "vector_intrinsic_right_bracket";
        ]
      | ElementInitializer { element_key; element_arrow; element_value } ->
        ["element_key"; "element_arrow"; "element_value"]
      | SubscriptExpression
          {
            subscript_receiver;
            subscript_left_bracket;
            subscript_index;
            subscript_right_bracket;
          } ->
        [
          "subscript_receiver";
          "subscript_left_bracket";
          "subscript_index";
          "subscript_right_bracket";
        ]
      | EmbeddedSubscriptExpression
          {
            embedded_subscript_receiver;
            embedded_subscript_left_bracket;
            embedded_subscript_index;
            embedded_subscript_right_bracket;
          } ->
        [
          "embedded_subscript_receiver";
          "embedded_subscript_left_bracket";
          "embedded_subscript_index";
          "embedded_subscript_right_bracket";
        ]
      | AwaitableCreationExpression
          {
            awaitable_attribute_spec;
            awaitable_async;
            awaitable_compound_statement;
          } ->
        [
          "awaitable_attribute_spec";
          "awaitable_async";
          "awaitable_compound_statement";
        ]
      | XHPChildrenDeclaration
          {
            xhp_children_keyword;
            xhp_children_expression;
            xhp_children_semicolon;
          } ->
        [
          "xhp_children_keyword";
          "xhp_children_expression";
          "xhp_children_semicolon";
        ]
      | XHPChildrenParenthesizedList
          {
            xhp_children_list_left_paren;
            xhp_children_list_xhp_children;
            xhp_children_list_right_paren;
          } ->
        [
          "xhp_children_list_left_paren";
          "xhp_children_list_xhp_children";
          "xhp_children_list_right_paren";
        ]
      | XHPCategoryDeclaration
          {
            xhp_category_keyword;
            xhp_category_categories;
            xhp_category_semicolon;
          } ->
        [
          "xhp_category_keyword";
          "xhp_category_categories";
          "xhp_category_semicolon";
        ]
      | XHPEnumType
          {
            xhp_enum_like;
            xhp_enum_keyword;
            xhp_enum_left_brace;
            xhp_enum_values;
            xhp_enum_right_brace;
          } ->
        [
          "xhp_enum_like";
          "xhp_enum_keyword";
          "xhp_enum_left_brace";
          "xhp_enum_values";
          "xhp_enum_right_brace";
        ]
      | XHPLateinit { xhp_lateinit_at; xhp_lateinit_keyword } ->
        ["xhp_lateinit_at"; "xhp_lateinit_keyword"]
      | XHPRequired { xhp_required_at; xhp_required_keyword } ->
        ["xhp_required_at"; "xhp_required_keyword"]
      | XHPClassAttributeDeclaration
          {
            xhp_attribute_keyword;
            xhp_attribute_attributes;
            xhp_attribute_semicolon;
          } ->
        [
          "xhp_attribute_keyword";
          "xhp_attribute_attributes";
          "xhp_attribute_semicolon";
        ]
      | XHPClassAttribute
          {
            xhp_attribute_decl_type;
            xhp_attribute_decl_name;
            xhp_attribute_decl_initializer;
            xhp_attribute_decl_required;
          } ->
        [
          "xhp_attribute_decl_type";
          "xhp_attribute_decl_name";
          "xhp_attribute_decl_initializer";
          "xhp_attribute_decl_required";
        ]
      | XHPSimpleClassAttribute { xhp_simple_class_attribute_type } ->
        ["xhp_simple_class_attribute_type"]
      | XHPSimpleAttribute
          {
            xhp_simple_attribute_name;
            xhp_simple_attribute_equal;
            xhp_simple_attribute_expression;
          } ->
        [
          "xhp_simple_attribute_name";
          "xhp_simple_attribute_equal";
          "xhp_simple_attribute_expression";
        ]
      | XHPSpreadAttribute
          {
            xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace;
          } ->
        [
          "xhp_spread_attribute_left_brace";
          "xhp_spread_attribute_spread_operator";
          "xhp_spread_attribute_expression";
          "xhp_spread_attribute_right_brace";
        ]
      | XHPOpen
          {
            xhp_open_left_angle;
            xhp_open_name;
            xhp_open_attributes;
            xhp_open_right_angle;
          } ->
        [
          "xhp_open_left_angle";
          "xhp_open_name";
          "xhp_open_attributes";
          "xhp_open_right_angle";
        ]
      | XHPExpression { xhp_open; xhp_body; xhp_close } ->
        ["xhp_open"; "xhp_body"; "xhp_close"]
      | XHPClose { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
        ->
        ["xhp_close_left_angle"; "xhp_close_name"; "xhp_close_right_angle"]
      | TypeConstant
          {
            type_constant_left_type;
            type_constant_separator;
            type_constant_right_type;
          } ->
        [
          "type_constant_left_type";
          "type_constant_separator";
          "type_constant_right_type";
        ]
      | VectorTypeSpecifier
          {
            vector_type_keyword;
            vector_type_left_angle;
            vector_type_type;
            vector_type_trailing_comma;
            vector_type_right_angle;
          } ->
        [
          "vector_type_keyword";
          "vector_type_left_angle";
          "vector_type_type";
          "vector_type_trailing_comma";
          "vector_type_right_angle";
        ]
      | KeysetTypeSpecifier
          {
            keyset_type_keyword;
            keyset_type_left_angle;
            keyset_type_type;
            keyset_type_trailing_comma;
            keyset_type_right_angle;
          } ->
        [
          "keyset_type_keyword";
          "keyset_type_left_angle";
          "keyset_type_type";
          "keyset_type_trailing_comma";
          "keyset_type_right_angle";
        ]
      | TupleTypeExplicitSpecifier
          {
            tuple_type_keyword;
            tuple_type_left_angle;
            tuple_type_types;
            tuple_type_right_angle;
          } ->
        [
          "tuple_type_keyword";
          "tuple_type_left_angle";
          "tuple_type_types";
          "tuple_type_right_angle";
        ]
      | VarrayTypeSpecifier
          {
            varray_keyword;
            varray_left_angle;
            varray_type;
            varray_trailing_comma;
            varray_right_angle;
          } ->
        [
          "varray_keyword";
          "varray_left_angle";
          "varray_type";
          "varray_trailing_comma";
          "varray_right_angle";
        ]
      | FunctionCtxTypeSpecifier
          { function_ctx_type_keyword; function_ctx_type_variable } ->
        ["function_ctx_type_keyword"; "function_ctx_type_variable"]
      | TypeParameter
          {
            type_attribute_spec;
            type_reified;
            type_variance;
            type_name;
            type_param_params;
            type_constraints;
          } ->
        [
          "type_attribute_spec";
          "type_reified";
          "type_variance";
          "type_name";
          "type_param_params";
          "type_constraints";
        ]
      | TypeConstraint { constraint_keyword; constraint_type } ->
        ["constraint_keyword"; "constraint_type"]
      | ContextConstraint { ctx_constraint_keyword; ctx_constraint_ctx_list } ->
        ["ctx_constraint_keyword"; "ctx_constraint_ctx_list"]
      | DarrayTypeSpecifier
          {
            darray_keyword;
            darray_left_angle;
            darray_key;
            darray_comma;
            darray_value;
            darray_trailing_comma;
            darray_right_angle;
          } ->
        [
          "darray_keyword";
          "darray_left_angle";
          "darray_key";
          "darray_comma";
          "darray_value";
          "darray_trailing_comma";
          "darray_right_angle";
        ]
      | DictionaryTypeSpecifier
          {
            dictionary_type_keyword;
            dictionary_type_left_angle;
            dictionary_type_members;
            dictionary_type_right_angle;
          } ->
        [
          "dictionary_type_keyword";
          "dictionary_type_left_angle";
          "dictionary_type_members";
          "dictionary_type_right_angle";
        ]
      | ClosureTypeSpecifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          } ->
        [
          "closure_outer_left_paren";
          "closure_readonly_keyword";
          "closure_function_keyword";
          "closure_inner_left_paren";
          "closure_parameter_list";
          "closure_inner_right_paren";
          "closure_contexts";
          "closure_colon";
          "closure_readonly_return";
          "closure_return_type";
          "closure_outer_right_paren";
        ]
      | ClosureParameterTypeSpecifier
          {
            closure_parameter_call_convention;
            closure_parameter_readonly;
            closure_parameter_type;
          } ->
        [
          "closure_parameter_call_convention";
          "closure_parameter_readonly";
          "closure_parameter_type";
        ]
      | TypeRefinement
          {
            type_refinement_type;
            type_refinement_keyword;
            type_refinement_left_brace;
            type_refinement_members;
            type_refinement_right_brace;
          } ->
        [
          "type_refinement_type";
          "type_refinement_keyword";
          "type_refinement_left_brace";
          "type_refinement_members";
          "type_refinement_right_brace";
        ]
      | TypeInRefinement
          {
            type_in_refinement_keyword;
            type_in_refinement_name;
            type_in_refinement_type_parameters;
            type_in_refinement_constraints;
            type_in_refinement_equal;
            type_in_refinement_type;
          } ->
        [
          "type_in_refinement_keyword";
          "type_in_refinement_name";
          "type_in_refinement_type_parameters";
          "type_in_refinement_constraints";
          "type_in_refinement_equal";
          "type_in_refinement_type";
        ]
      | CtxInRefinement
          {
            ctx_in_refinement_keyword;
            ctx_in_refinement_name;
            ctx_in_refinement_type_parameters;
            ctx_in_refinement_constraints;
            ctx_in_refinement_equal;
            ctx_in_refinement_ctx_list;
          } ->
        [
          "ctx_in_refinement_keyword";
          "ctx_in_refinement_name";
          "ctx_in_refinement_type_parameters";
          "ctx_in_refinement_constraints";
          "ctx_in_refinement_equal";
          "ctx_in_refinement_ctx_list";
        ]
      | ClassnameTypeSpecifier
          {
            classname_keyword;
            classname_left_angle;
            classname_type;
            classname_trailing_comma;
            classname_right_angle;
          } ->
        [
          "classname_keyword";
          "classname_left_angle";
          "classname_type";
          "classname_trailing_comma";
          "classname_right_angle";
        ]
      | FieldSpecifier { field_question; field_name; field_arrow; field_type }
        ->
        ["field_question"; "field_name"; "field_arrow"; "field_type"]
      | FieldInitializer
          {
            field_initializer_name;
            field_initializer_arrow;
            field_initializer_value;
          } ->
        [
          "field_initializer_name";
          "field_initializer_arrow";
          "field_initializer_value";
        ]
      | ShapeTypeSpecifier
          {
            shape_type_keyword;
            shape_type_left_paren;
            shape_type_fields;
            shape_type_ellipsis;
            shape_type_right_paren;
          } ->
        [
          "shape_type_keyword";
          "shape_type_left_paren";
          "shape_type_fields";
          "shape_type_ellipsis";
          "shape_type_right_paren";
        ]
      | ShapeExpression
          {
            shape_expression_keyword;
            shape_expression_left_paren;
            shape_expression_fields;
            shape_expression_right_paren;
          } ->
        [
          "shape_expression_keyword";
          "shape_expression_left_paren";
          "shape_expression_fields";
          "shape_expression_right_paren";
        ]
      | TupleExpression
          {
            tuple_expression_keyword;
            tuple_expression_left_paren;
            tuple_expression_items;
            tuple_expression_right_paren;
          } ->
        [
          "tuple_expression_keyword";
          "tuple_expression_left_paren";
          "tuple_expression_items";
          "tuple_expression_right_paren";
        ]
      | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
        ["generic_class_type"; "generic_argument_list"]
      | NullableTypeSpecifier { nullable_question; nullable_type } ->
        ["nullable_question"; "nullable_type"]
      | LikeTypeSpecifier { like_tilde; like_type } ->
        ["like_tilde"; "like_type"]
      | SoftTypeSpecifier { soft_at; soft_type } -> ["soft_at"; "soft_type"]
      | AttributizedSpecifier
          { attributized_specifier_attribute_spec; attributized_specifier_type }
        ->
        ["attributized_specifier_attribute_spec"; "attributized_specifier_type"]
      | ReifiedTypeArgument
          { reified_type_argument_reified; reified_type_argument_type } ->
        ["reified_type_argument_reified"; "reified_type_argument_type"]
      | TypeArguments
          {
            type_arguments_left_angle;
            type_arguments_types;
            type_arguments_right_angle;
          } ->
        [
          "type_arguments_left_angle";
          "type_arguments_types";
          "type_arguments_right_angle";
        ]
      | TypeParameters
          {
            type_parameters_left_angle;
            type_parameters_parameters;
            type_parameters_right_angle;
          } ->
        [
          "type_parameters_left_angle";
          "type_parameters_parameters";
          "type_parameters_right_angle";
        ]
      | TupleTypeSpecifier { tuple_left_paren; tuple_types; tuple_right_paren }
        ->
        ["tuple_left_paren"; "tuple_types"; "tuple_right_paren"]
      | UnionTypeSpecifier { union_left_paren; union_types; union_right_paren }
        ->
        ["union_left_paren"; "union_types"; "union_right_paren"]
      | IntersectionTypeSpecifier
          {
            intersection_left_paren;
            intersection_types;
            intersection_right_paren;
          } ->
        [
          "intersection_left_paren";
          "intersection_types";
          "intersection_right_paren";
        ]
      | ErrorSyntax { error_error } -> ["error_error"]
      | ListItem { list_item; list_separator } ->
        ["list_item"; "list_separator"]
      | EnumClassLabelExpression
          {
            enum_class_label_qualifier;
            enum_class_label_hash;
            enum_class_label_expression;
          } ->
        [
          "enum_class_label_qualifier";
          "enum_class_label_hash";
          "enum_class_label_expression";
        ]
      | ModuleDeclaration
          {
            module_declaration_attribute_spec;
            module_declaration_new_keyword;
            module_declaration_module_keyword;
            module_declaration_name;
            module_declaration_left_brace;
            module_declaration_exports;
            module_declaration_imports;
            module_declaration_right_brace;
          } ->
        [
          "module_declaration_attribute_spec";
          "module_declaration_new_keyword";
          "module_declaration_module_keyword";
          "module_declaration_name";
          "module_declaration_left_brace";
          "module_declaration_exports";
          "module_declaration_imports";
          "module_declaration_right_brace";
        ]
      | ModuleExports
          {
            module_exports_exports_keyword;
            module_exports_left_brace;
            module_exports_exports;
            module_exports_right_brace;
          } ->
        [
          "module_exports_exports_keyword";
          "module_exports_left_brace";
          "module_exports_exports";
          "module_exports_right_brace";
        ]
      | ModuleImports
          {
            module_imports_imports_keyword;
            module_imports_left_brace;
            module_imports_imports;
            module_imports_right_brace;
          } ->
        [
          "module_imports_imports_keyword";
          "module_imports_left_brace";
          "module_imports_imports";
          "module_imports_right_brace";
        ]
      | ModuleMembershipDeclaration
          {
            module_membership_declaration_module_keyword;
            module_membership_declaration_name;
            module_membership_declaration_semicolon;
          } ->
        [
          "module_membership_declaration_module_keyword";
          "module_membership_declaration_name";
          "module_membership_declaration_semicolon";
        ]
      | PackageExpression
          { package_expression_keyword; package_expression_name } ->
        ["package_expression_keyword"; "package_expression_name"]

    let rec to_json_ ?(with_value = false) ?(ignore_missing = false) node =
      let open Hh_json in
      let ch =
        match node.syntax with
        | Token t -> [("token", Token.to_json t)]
        | SyntaxList x ->
          [
            ( "elements",
              JSON_Array
                (List.filter_map ~f:(to_json_ ~with_value ~ignore_missing) x) );
          ]
        | _ ->
          let rec aux acc c n =
            match (c, n) with
            | ([], []) -> acc
            | (hc :: tc, hn :: tn) ->
              let result = (to_json_ ~with_value ~ignore_missing) hc in
              (match result with
              | Some r -> aux ((hn, r) :: acc) tc tn
              | None -> aux acc tc tn)
            | _ -> failwith "mismatch between children and names"
          in
          List.rev (aux [] (children node) (children_names node))
      in
      let k = ("kind", JSON_String (SyntaxKind.to_string (kind node))) in
      let v =
        if with_value then
          ("value", SyntaxValue.to_json node.value) :: ch
        else
          ch
      in
      if ignore_missing && List.is_empty ch then
        None
      else
        Some (JSON_Object (k :: v))

    let to_json ?(with_value = false) ?(ignore_missing = false) node =
      match to_json_ ~with_value ~ignore_missing node with
      | Some x -> x
      | None -> Hh_json.JSON_Object []

    let binary_operator_kind b =
      match syntax b with
      | Token token ->
        let kind = Token.kind token in
        if Operator.is_trailing_operator_token kind then
          Some (Operator.trailing_from_token kind)
        else
          None
      | _ -> None

    let get_token node =
      match syntax node with
      | Token token -> Some token
      | _ -> None

    let leading_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if Option.is_none token then
            let result = aux (children h) in
            if Option.is_none result then
              aux t
            else
              result
          else
            token
      in
      aux [node]

    let trailing_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if Option.is_none token then
            let result = aux (List.rev (children h)) in
            if Option.is_none result then
              aux t
            else
              result
          else
            token
      in
      aux [node]

    let syntax_from_children kind ts =
      match (kind, ts) with
      | (SyntaxKind.EndOfFile, [end_of_file_token]) ->
        EndOfFile { end_of_file_token }
      | (SyntaxKind.Script, [script_declarations]) ->
        Script { script_declarations }
      | (SyntaxKind.QualifiedName, [qualified_name_parts]) ->
        QualifiedName { qualified_name_parts }
      | (SyntaxKind.ModuleName, [module_name_parts]) ->
        ModuleName { module_name_parts }
      | (SyntaxKind.SimpleTypeSpecifier, [simple_type_specifier]) ->
        SimpleTypeSpecifier { simple_type_specifier }
      | (SyntaxKind.LiteralExpression, [literal_expression]) ->
        LiteralExpression { literal_expression }
      | ( SyntaxKind.PrefixedStringExpression,
          [prefixed_string_name; prefixed_string_str] ) ->
        PrefixedStringExpression { prefixed_string_name; prefixed_string_str }
      | ( SyntaxKind.PrefixedCodeExpression,
          [
            prefixed_code_prefix;
            prefixed_code_left_backtick;
            prefixed_code_body;
            prefixed_code_right_backtick;
          ] ) ->
        PrefixedCodeExpression
          {
            prefixed_code_prefix;
            prefixed_code_left_backtick;
            prefixed_code_body;
            prefixed_code_right_backtick;
          }
      | (SyntaxKind.VariableExpression, [variable_expression]) ->
        VariableExpression { variable_expression }
      | (SyntaxKind.PipeVariableExpression, [pipe_variable_expression]) ->
        PipeVariableExpression { pipe_variable_expression }
      | ( SyntaxKind.FileAttributeSpecification,
          [
            file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword;
            file_attribute_specification_colon;
            file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle;
          ] ) ->
        FileAttributeSpecification
          {
            file_attribute_specification_left_double_angle;
            file_attribute_specification_keyword;
            file_attribute_specification_colon;
            file_attribute_specification_attributes;
            file_attribute_specification_right_double_angle;
          }
      | ( SyntaxKind.EnumDeclaration,
          [
            enum_attribute_spec;
            enum_modifiers;
            enum_keyword;
            enum_name;
            enum_colon;
            enum_base;
            enum_type;
            enum_left_brace;
            enum_use_clauses;
            enum_enumerators;
            enum_right_brace;
          ] ) ->
        EnumDeclaration
          {
            enum_attribute_spec;
            enum_modifiers;
            enum_keyword;
            enum_name;
            enum_colon;
            enum_base;
            enum_type;
            enum_left_brace;
            enum_use_clauses;
            enum_enumerators;
            enum_right_brace;
          }
      | ( SyntaxKind.EnumUse,
          [enum_use_keyword; enum_use_names; enum_use_semicolon] ) ->
        EnumUse { enum_use_keyword; enum_use_names; enum_use_semicolon }
      | ( SyntaxKind.Enumerator,
          [
            enumerator_name;
            enumerator_equal;
            enumerator_value;
            enumerator_semicolon;
          ] ) ->
        Enumerator
          {
            enumerator_name;
            enumerator_equal;
            enumerator_value;
            enumerator_semicolon;
          }
      | ( SyntaxKind.EnumClassDeclaration,
          [
            enum_class_attribute_spec;
            enum_class_modifiers;
            enum_class_enum_keyword;
            enum_class_class_keyword;
            enum_class_name;
            enum_class_colon;
            enum_class_base;
            enum_class_extends;
            enum_class_extends_list;
            enum_class_left_brace;
            enum_class_elements;
            enum_class_right_brace;
          ] ) ->
        EnumClassDeclaration
          {
            enum_class_attribute_spec;
            enum_class_modifiers;
            enum_class_enum_keyword;
            enum_class_class_keyword;
            enum_class_name;
            enum_class_colon;
            enum_class_base;
            enum_class_extends;
            enum_class_extends_list;
            enum_class_left_brace;
            enum_class_elements;
            enum_class_right_brace;
          }
      | ( SyntaxKind.EnumClassEnumerator,
          [
            enum_class_enumerator_modifiers;
            enum_class_enumerator_type;
            enum_class_enumerator_name;
            enum_class_enumerator_initializer;
            enum_class_enumerator_semicolon;
          ] ) ->
        EnumClassEnumerator
          {
            enum_class_enumerator_modifiers;
            enum_class_enumerator_type;
            enum_class_enumerator_name;
            enum_class_enumerator_initializer;
            enum_class_enumerator_semicolon;
          }
      | ( SyntaxKind.AliasDeclaration,
          [
            alias_attribute_spec;
            alias_modifiers;
            alias_module_kw_opt;
            alias_keyword;
            alias_name;
            alias_generic_parameter;
            alias_constraint;
            alias_equal;
            alias_type;
            alias_semicolon;
          ] ) ->
        AliasDeclaration
          {
            alias_attribute_spec;
            alias_modifiers;
            alias_module_kw_opt;
            alias_keyword;
            alias_name;
            alias_generic_parameter;
            alias_constraint;
            alias_equal;
            alias_type;
            alias_semicolon;
          }
      | ( SyntaxKind.ContextAliasDeclaration,
          [
            ctx_alias_attribute_spec;
            ctx_alias_keyword;
            ctx_alias_name;
            ctx_alias_generic_parameter;
            ctx_alias_as_constraint;
            ctx_alias_equal;
            ctx_alias_context;
            ctx_alias_semicolon;
          ] ) ->
        ContextAliasDeclaration
          {
            ctx_alias_attribute_spec;
            ctx_alias_keyword;
            ctx_alias_name;
            ctx_alias_generic_parameter;
            ctx_alias_as_constraint;
            ctx_alias_equal;
            ctx_alias_context;
            ctx_alias_semicolon;
          }
      | ( SyntaxKind.CaseTypeDeclaration,
          [
            case_type_attribute_spec;
            case_type_modifiers;
            case_type_case_keyword;
            case_type_type_keyword;
            case_type_name;
            case_type_generic_parameter;
            case_type_as;
            case_type_bounds;
            case_type_equal;
            case_type_variants;
            case_type_semicolon;
          ] ) ->
        CaseTypeDeclaration
          {
            case_type_attribute_spec;
            case_type_modifiers;
            case_type_case_keyword;
            case_type_type_keyword;
            case_type_name;
            case_type_generic_parameter;
            case_type_as;
            case_type_bounds;
            case_type_equal;
            case_type_variants;
            case_type_semicolon;
          }
      | ( SyntaxKind.CaseTypeVariant,
          [case_type_variant_bar; case_type_variant_type] ) ->
        CaseTypeVariant { case_type_variant_bar; case_type_variant_type }
      | ( SyntaxKind.PropertyDeclaration,
          [
            property_attribute_spec;
            property_modifiers;
            property_type;
            property_declarators;
            property_semicolon;
          ] ) ->
        PropertyDeclaration
          {
            property_attribute_spec;
            property_modifiers;
            property_type;
            property_declarators;
            property_semicolon;
          }
      | (SyntaxKind.PropertyDeclarator, [property_name; property_initializer])
        ->
        PropertyDeclarator { property_name; property_initializer }
      | (SyntaxKind.NamespaceDeclaration, [namespace_header; namespace_body]) ->
        NamespaceDeclaration { namespace_header; namespace_body }
      | ( SyntaxKind.NamespaceDeclarationHeader,
          [namespace_keyword; namespace_name] ) ->
        NamespaceDeclarationHeader { namespace_keyword; namespace_name }
      | ( SyntaxKind.NamespaceBody,
          [namespace_left_brace; namespace_declarations; namespace_right_brace]
        ) ->
        NamespaceBody
          {
            namespace_left_brace;
            namespace_declarations;
            namespace_right_brace;
          }
      | (SyntaxKind.NamespaceEmptyBody, [namespace_semicolon]) ->
        NamespaceEmptyBody { namespace_semicolon }
      | ( SyntaxKind.NamespaceUseDeclaration,
          [
            namespace_use_keyword;
            namespace_use_kind;
            namespace_use_clauses;
            namespace_use_semicolon;
          ] ) ->
        NamespaceUseDeclaration
          {
            namespace_use_keyword;
            namespace_use_kind;
            namespace_use_clauses;
            namespace_use_semicolon;
          }
      | ( SyntaxKind.NamespaceGroupUseDeclaration,
          [
            namespace_group_use_keyword;
            namespace_group_use_kind;
            namespace_group_use_prefix;
            namespace_group_use_left_brace;
            namespace_group_use_clauses;
            namespace_group_use_right_brace;
            namespace_group_use_semicolon;
          ] ) ->
        NamespaceGroupUseDeclaration
          {
            namespace_group_use_keyword;
            namespace_group_use_kind;
            namespace_group_use_prefix;
            namespace_group_use_left_brace;
            namespace_group_use_clauses;
            namespace_group_use_right_brace;
            namespace_group_use_semicolon;
          }
      | ( SyntaxKind.NamespaceUseClause,
          [
            namespace_use_clause_kind;
            namespace_use_name;
            namespace_use_as;
            namespace_use_alias;
          ] ) ->
        NamespaceUseClause
          {
            namespace_use_clause_kind;
            namespace_use_name;
            namespace_use_as;
            namespace_use_alias;
          }
      | ( SyntaxKind.FunctionDeclaration,
          [function_attribute_spec; function_declaration_header; function_body]
        ) ->
        FunctionDeclaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          }
      | ( SyntaxKind.FunctionDeclarationHeader,
          [
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          ] ) ->
        FunctionDeclarationHeader
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          }
      | ( SyntaxKind.Contexts,
          [contexts_left_bracket; contexts_types; contexts_right_bracket] ) ->
        Contexts
          { contexts_left_bracket; contexts_types; contexts_right_bracket }
      | ( SyntaxKind.WhereClause,
          [where_clause_keyword; where_clause_constraints] ) ->
        WhereClause { where_clause_keyword; where_clause_constraints }
      | ( SyntaxKind.WhereConstraint,
          [
            where_constraint_left_type;
            where_constraint_operator;
            where_constraint_right_type;
          ] ) ->
        WhereConstraint
          {
            where_constraint_left_type;
            where_constraint_operator;
            where_constraint_right_type;
          }
      | ( SyntaxKind.MethodishDeclaration,
          [
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          ] ) ->
        MethodishDeclaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          }
      | ( SyntaxKind.MethodishTraitResolution,
          [
            methodish_trait_attribute;
            methodish_trait_function_decl_header;
            methodish_trait_equal;
            methodish_trait_name;
            methodish_trait_semicolon;
          ] ) ->
        MethodishTraitResolution
          {
            methodish_trait_attribute;
            methodish_trait_function_decl_header;
            methodish_trait_equal;
            methodish_trait_name;
            methodish_trait_semicolon;
          }
      | ( SyntaxKind.ClassishDeclaration,
          [
            classish_attribute;
            classish_modifiers;
            classish_xhp;
            classish_keyword;
            classish_name;
            classish_type_parameters;
            classish_extends_keyword;
            classish_extends_list;
            classish_implements_keyword;
            classish_implements_list;
            classish_where_clause;
            classish_body;
          ] ) ->
        ClassishDeclaration
          {
            classish_attribute;
            classish_modifiers;
            classish_xhp;
            classish_keyword;
            classish_name;
            classish_type_parameters;
            classish_extends_keyword;
            classish_extends_list;
            classish_implements_keyword;
            classish_implements_list;
            classish_where_clause;
            classish_body;
          }
      | ( SyntaxKind.ClassishBody,
          [
            classish_body_left_brace;
            classish_body_elements;
            classish_body_right_brace;
          ] ) ->
        ClassishBody
          {
            classish_body_left_brace;
            classish_body_elements;
            classish_body_right_brace;
          }
      | ( SyntaxKind.TraitUse,
          [trait_use_keyword; trait_use_names; trait_use_semicolon] ) ->
        TraitUse { trait_use_keyword; trait_use_names; trait_use_semicolon }
      | ( SyntaxKind.RequireClause,
          [require_keyword; require_kind; require_name; require_semicolon] ) ->
        RequireClause
          { require_keyword; require_kind; require_name; require_semicolon }
      | ( SyntaxKind.ConstDeclaration,
          [
            const_attribute_spec;
            const_modifiers;
            const_keyword;
            const_type_specifier;
            const_declarators;
            const_semicolon;
          ] ) ->
        ConstDeclaration
          {
            const_attribute_spec;
            const_modifiers;
            const_keyword;
            const_type_specifier;
            const_declarators;
            const_semicolon;
          }
      | ( SyntaxKind.ConstantDeclarator,
          [constant_declarator_name; constant_declarator_initializer] ) ->
        ConstantDeclarator
          { constant_declarator_name; constant_declarator_initializer }
      | ( SyntaxKind.TypeConstDeclaration,
          [
            type_const_attribute_spec;
            type_const_modifiers;
            type_const_keyword;
            type_const_type_keyword;
            type_const_name;
            type_const_type_parameters;
            type_const_type_constraints;
            type_const_equal;
            type_const_type_specifier;
            type_const_semicolon;
          ] ) ->
        TypeConstDeclaration
          {
            type_const_attribute_spec;
            type_const_modifiers;
            type_const_keyword;
            type_const_type_keyword;
            type_const_name;
            type_const_type_parameters;
            type_const_type_constraints;
            type_const_equal;
            type_const_type_specifier;
            type_const_semicolon;
          }
      | ( SyntaxKind.ContextConstDeclaration,
          [
            context_const_modifiers;
            context_const_const_keyword;
            context_const_ctx_keyword;
            context_const_name;
            context_const_type_parameters;
            context_const_constraint;
            context_const_equal;
            context_const_ctx_list;
            context_const_semicolon;
          ] ) ->
        ContextConstDeclaration
          {
            context_const_modifiers;
            context_const_const_keyword;
            context_const_ctx_keyword;
            context_const_name;
            context_const_type_parameters;
            context_const_constraint;
            context_const_equal;
            context_const_ctx_list;
            context_const_semicolon;
          }
      | ( SyntaxKind.DecoratedExpression,
          [decorated_expression_decorator; decorated_expression_expression] ) ->
        DecoratedExpression
          { decorated_expression_decorator; decorated_expression_expression }
      | ( SyntaxKind.ParameterDeclaration,
          [
            parameter_attribute;
            parameter_visibility;
            parameter_call_convention;
            parameter_readonly;
            parameter_type;
            parameter_name;
            parameter_default_value;
          ] ) ->
        ParameterDeclaration
          {
            parameter_attribute;
            parameter_visibility;
            parameter_call_convention;
            parameter_readonly;
            parameter_type;
            parameter_name;
            parameter_default_value;
          }
      | ( SyntaxKind.VariadicParameter,
          [
            variadic_parameter_call_convention;
            variadic_parameter_type;
            variadic_parameter_ellipsis;
          ] ) ->
        VariadicParameter
          {
            variadic_parameter_call_convention;
            variadic_parameter_type;
            variadic_parameter_ellipsis;
          }
      | ( SyntaxKind.OldAttributeSpecification,
          [
            old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle;
          ] ) ->
        OldAttributeSpecification
          {
            old_attribute_specification_left_double_angle;
            old_attribute_specification_attributes;
            old_attribute_specification_right_double_angle;
          }
      | (SyntaxKind.AttributeSpecification, [attribute_specification_attributes])
        ->
        AttributeSpecification { attribute_specification_attributes }
      | (SyntaxKind.Attribute, [attribute_at; attribute_attribute_name]) ->
        Attribute { attribute_at; attribute_attribute_name }
      | (SyntaxKind.InclusionExpression, [inclusion_require; inclusion_filename])
        ->
        InclusionExpression { inclusion_require; inclusion_filename }
      | ( SyntaxKind.InclusionDirective,
          [inclusion_expression; inclusion_semicolon] ) ->
        InclusionDirective { inclusion_expression; inclusion_semicolon }
      | ( SyntaxKind.CompoundStatement,
          [compound_left_brace; compound_statements; compound_right_brace] ) ->
        CompoundStatement
          { compound_left_brace; compound_statements; compound_right_brace }
      | ( SyntaxKind.ExpressionStatement,
          [expression_statement_expression; expression_statement_semicolon] ) ->
        ExpressionStatement
          { expression_statement_expression; expression_statement_semicolon }
      | (SyntaxKind.MarkupSection, [markup_hashbang; markup_suffix]) ->
        MarkupSection { markup_hashbang; markup_suffix }
      | ( SyntaxKind.MarkupSuffix,
          [markup_suffix_less_than_question; markup_suffix_name] ) ->
        MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name }
      | ( SyntaxKind.UnsetStatement,
          [
            unset_keyword;
            unset_left_paren;
            unset_variables;
            unset_right_paren;
            unset_semicolon;
          ] ) ->
        UnsetStatement
          {
            unset_keyword;
            unset_left_paren;
            unset_variables;
            unset_right_paren;
            unset_semicolon;
          }
      | ( SyntaxKind.DeclareLocalStatement,
          [
            declare_local_keyword;
            declare_local_variable;
            declare_local_colon;
            declare_local_type;
            declare_local_initializer;
            declare_local_semicolon;
          ] ) ->
        DeclareLocalStatement
          {
            declare_local_keyword;
            declare_local_variable;
            declare_local_colon;
            declare_local_type;
            declare_local_initializer;
            declare_local_semicolon;
          }
      | ( SyntaxKind.UsingStatementBlockScoped,
          [
            using_block_await_keyword;
            using_block_using_keyword;
            using_block_left_paren;
            using_block_expressions;
            using_block_right_paren;
            using_block_body;
          ] ) ->
        UsingStatementBlockScoped
          {
            using_block_await_keyword;
            using_block_using_keyword;
            using_block_left_paren;
            using_block_expressions;
            using_block_right_paren;
            using_block_body;
          }
      | ( SyntaxKind.UsingStatementFunctionScoped,
          [
            using_function_await_keyword;
            using_function_using_keyword;
            using_function_expression;
            using_function_semicolon;
          ] ) ->
        UsingStatementFunctionScoped
          {
            using_function_await_keyword;
            using_function_using_keyword;
            using_function_expression;
            using_function_semicolon;
          }
      | ( SyntaxKind.WhileStatement,
          [
            while_keyword;
            while_left_paren;
            while_condition;
            while_right_paren;
            while_body;
          ] ) ->
        WhileStatement
          {
            while_keyword;
            while_left_paren;
            while_condition;
            while_right_paren;
            while_body;
          }
      | ( SyntaxKind.IfStatement,
          [
            if_keyword;
            if_left_paren;
            if_condition;
            if_right_paren;
            if_statement;
            if_else_clause;
          ] ) ->
        IfStatement
          {
            if_keyword;
            if_left_paren;
            if_condition;
            if_right_paren;
            if_statement;
            if_else_clause;
          }
      | (SyntaxKind.ElseClause, [else_keyword; else_statement]) ->
        ElseClause { else_keyword; else_statement }
      | ( SyntaxKind.TryStatement,
          [
            try_keyword;
            try_compound_statement;
            try_catch_clauses;
            try_finally_clause;
          ] ) ->
        TryStatement
          {
            try_keyword;
            try_compound_statement;
            try_catch_clauses;
            try_finally_clause;
          }
      | ( SyntaxKind.CatchClause,
          [
            catch_keyword;
            catch_left_paren;
            catch_type;
            catch_variable;
            catch_right_paren;
            catch_body;
          ] ) ->
        CatchClause
          {
            catch_keyword;
            catch_left_paren;
            catch_type;
            catch_variable;
            catch_right_paren;
            catch_body;
          }
      | (SyntaxKind.FinallyClause, [finally_keyword; finally_body]) ->
        FinallyClause { finally_keyword; finally_body }
      | ( SyntaxKind.DoStatement,
          [
            do_keyword;
            do_body;
            do_while_keyword;
            do_left_paren;
            do_condition;
            do_right_paren;
            do_semicolon;
          ] ) ->
        DoStatement
          {
            do_keyword;
            do_body;
            do_while_keyword;
            do_left_paren;
            do_condition;
            do_right_paren;
            do_semicolon;
          }
      | ( SyntaxKind.ForStatement,
          [
            for_keyword;
            for_left_paren;
            for_initializer;
            for_first_semicolon;
            for_control;
            for_second_semicolon;
            for_end_of_loop;
            for_right_paren;
            for_body;
          ] ) ->
        ForStatement
          {
            for_keyword;
            for_left_paren;
            for_initializer;
            for_first_semicolon;
            for_control;
            for_second_semicolon;
            for_end_of_loop;
            for_right_paren;
            for_body;
          }
      | ( SyntaxKind.ForeachStatement,
          [
            foreach_keyword;
            foreach_left_paren;
            foreach_collection;
            foreach_await_keyword;
            foreach_as;
            foreach_key;
            foreach_arrow;
            foreach_value;
            foreach_right_paren;
            foreach_body;
          ] ) ->
        ForeachStatement
          {
            foreach_keyword;
            foreach_left_paren;
            foreach_collection;
            foreach_await_keyword;
            foreach_as;
            foreach_key;
            foreach_arrow;
            foreach_value;
            foreach_right_paren;
            foreach_body;
          }
      | ( SyntaxKind.SwitchStatement,
          [
            switch_keyword;
            switch_left_paren;
            switch_expression;
            switch_right_paren;
            switch_left_brace;
            switch_sections;
            switch_right_brace;
          ] ) ->
        SwitchStatement
          {
            switch_keyword;
            switch_left_paren;
            switch_expression;
            switch_right_paren;
            switch_left_brace;
            switch_sections;
            switch_right_brace;
          }
      | ( SyntaxKind.SwitchSection,
          [
            switch_section_labels;
            switch_section_statements;
            switch_section_fallthrough;
          ] ) ->
        SwitchSection
          {
            switch_section_labels;
            switch_section_statements;
            switch_section_fallthrough;
          }
      | ( SyntaxKind.SwitchFallthrough,
          [fallthrough_keyword; fallthrough_semicolon] ) ->
        SwitchFallthrough { fallthrough_keyword; fallthrough_semicolon }
      | (SyntaxKind.CaseLabel, [case_keyword; case_expression; case_colon]) ->
        CaseLabel { case_keyword; case_expression; case_colon }
      | (SyntaxKind.DefaultLabel, [default_keyword; default_colon]) ->
        DefaultLabel { default_keyword; default_colon }
      | ( SyntaxKind.MatchStatement,
          [
            match_statement_keyword;
            match_statement_left_paren;
            match_statement_expression;
            match_statement_right_paren;
            match_statement_left_brace;
            match_statement_arms;
            match_statement_right_brace;
          ] ) ->
        MatchStatement
          {
            match_statement_keyword;
            match_statement_left_paren;
            match_statement_expression;
            match_statement_right_paren;
            match_statement_left_brace;
            match_statement_arms;
            match_statement_right_brace;
          }
      | ( SyntaxKind.MatchStatementArm,
          [
            match_statement_arm_pattern;
            match_statement_arm_arrow;
            match_statement_arm_body;
          ] ) ->
        MatchStatementArm
          {
            match_statement_arm_pattern;
            match_statement_arm_arrow;
            match_statement_arm_body;
          }
      | ( SyntaxKind.ReturnStatement,
          [return_keyword; return_expression; return_semicolon] ) ->
        ReturnStatement { return_keyword; return_expression; return_semicolon }
      | ( SyntaxKind.YieldBreakStatement,
          [yield_break_keyword; yield_break_break; yield_break_semicolon] ) ->
        YieldBreakStatement
          { yield_break_keyword; yield_break_break; yield_break_semicolon }
      | ( SyntaxKind.ThrowStatement,
          [throw_keyword; throw_expression; throw_semicolon] ) ->
        ThrowStatement { throw_keyword; throw_expression; throw_semicolon }
      | (SyntaxKind.BreakStatement, [break_keyword; break_semicolon]) ->
        BreakStatement { break_keyword; break_semicolon }
      | (SyntaxKind.ContinueStatement, [continue_keyword; continue_semicolon])
        ->
        ContinueStatement { continue_keyword; continue_semicolon }
      | ( SyntaxKind.EchoStatement,
          [echo_keyword; echo_expressions; echo_semicolon] ) ->
        EchoStatement { echo_keyword; echo_expressions; echo_semicolon }
      | ( SyntaxKind.ConcurrentStatement,
          [concurrent_keyword; concurrent_statement] ) ->
        ConcurrentStatement { concurrent_keyword; concurrent_statement }
      | ( SyntaxKind.SimpleInitializer,
          [simple_initializer_equal; simple_initializer_value] ) ->
        SimpleInitializer { simple_initializer_equal; simple_initializer_value }
      | ( SyntaxKind.AnonymousClass,
          [
            anonymous_class_class_keyword;
            anonymous_class_left_paren;
            anonymous_class_argument_list;
            anonymous_class_right_paren;
            anonymous_class_extends_keyword;
            anonymous_class_extends_list;
            anonymous_class_implements_keyword;
            anonymous_class_implements_list;
            anonymous_class_body;
          ] ) ->
        AnonymousClass
          {
            anonymous_class_class_keyword;
            anonymous_class_left_paren;
            anonymous_class_argument_list;
            anonymous_class_right_paren;
            anonymous_class_extends_keyword;
            anonymous_class_extends_list;
            anonymous_class_implements_keyword;
            anonymous_class_implements_list;
            anonymous_class_body;
          }
      | ( SyntaxKind.AnonymousFunction,
          [
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          ] ) ->
        AnonymousFunction
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          }
      | ( SyntaxKind.AnonymousFunctionUseClause,
          [
            anonymous_use_keyword;
            anonymous_use_left_paren;
            anonymous_use_variables;
            anonymous_use_right_paren;
          ] ) ->
        AnonymousFunctionUseClause
          {
            anonymous_use_keyword;
            anonymous_use_left_paren;
            anonymous_use_variables;
            anonymous_use_right_paren;
          }
      | (SyntaxKind.VariablePattern, [variable_pattern_variable]) ->
        VariablePattern { variable_pattern_variable }
      | ( SyntaxKind.ConstructorPattern,
          [
            constructor_pattern_constructor;
            constructor_pattern_left_paren;
            constructor_pattern_members;
            constructor_pattern_right_paren;
          ] ) ->
        ConstructorPattern
          {
            constructor_pattern_constructor;
            constructor_pattern_left_paren;
            constructor_pattern_members;
            constructor_pattern_right_paren;
          }
      | ( SyntaxKind.RefinementPattern,
          [
            refinement_pattern_variable;
            refinement_pattern_colon;
            refinement_pattern_specifier;
          ] ) ->
        RefinementPattern
          {
            refinement_pattern_variable;
            refinement_pattern_colon;
            refinement_pattern_specifier;
          }
      | ( SyntaxKind.LambdaExpression,
          [
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          ] ) ->
        LambdaExpression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          }
      | ( SyntaxKind.LambdaSignature,
          [
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          ] ) ->
        LambdaSignature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          }
      | ( SyntaxKind.CastExpression,
          [cast_left_paren; cast_type; cast_right_paren; cast_operand] ) ->
        CastExpression
          { cast_left_paren; cast_type; cast_right_paren; cast_operand }
      | ( SyntaxKind.ScopeResolutionExpression,
          [
            scope_resolution_qualifier;
            scope_resolution_operator;
            scope_resolution_name;
          ] ) ->
        ScopeResolutionExpression
          {
            scope_resolution_qualifier;
            scope_resolution_operator;
            scope_resolution_name;
          }
      | ( SyntaxKind.MemberSelectionExpression,
          [member_object; member_operator; member_name] ) ->
        MemberSelectionExpression
          { member_object; member_operator; member_name }
      | ( SyntaxKind.SafeMemberSelectionExpression,
          [safe_member_object; safe_member_operator; safe_member_name] ) ->
        SafeMemberSelectionExpression
          { safe_member_object; safe_member_operator; safe_member_name }
      | ( SyntaxKind.EmbeddedMemberSelectionExpression,
          [
            embedded_member_object;
            embedded_member_operator;
            embedded_member_name;
          ] ) ->
        EmbeddedMemberSelectionExpression
          {
            embedded_member_object;
            embedded_member_operator;
            embedded_member_name;
          }
      | (SyntaxKind.YieldExpression, [yield_keyword; yield_operand]) ->
        YieldExpression { yield_keyword; yield_operand }
      | ( SyntaxKind.PrefixUnaryExpression,
          [prefix_unary_operator; prefix_unary_operand] ) ->
        PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
      | ( SyntaxKind.PostfixUnaryExpression,
          [postfix_unary_operand; postfix_unary_operator] ) ->
        PostfixUnaryExpression { postfix_unary_operand; postfix_unary_operator }
      | ( SyntaxKind.BinaryExpression,
          [binary_left_operand; binary_operator; binary_right_operand] ) ->
        BinaryExpression
          { binary_left_operand; binary_operator; binary_right_operand }
      | ( SyntaxKind.IsExpression,
          [is_left_operand; is_operator; is_right_operand] ) ->
        IsExpression { is_left_operand; is_operator; is_right_operand }
      | ( SyntaxKind.AsExpression,
          [as_left_operand; as_operator; as_right_operand] ) ->
        AsExpression { as_left_operand; as_operator; as_right_operand }
      | ( SyntaxKind.NullableAsExpression,
          [
            nullable_as_left_operand;
            nullable_as_operator;
            nullable_as_right_operand;
          ] ) ->
        NullableAsExpression
          {
            nullable_as_left_operand;
            nullable_as_operator;
            nullable_as_right_operand;
          }
      | ( SyntaxKind.UpcastExpression,
          [upcast_left_operand; upcast_operator; upcast_right_operand] ) ->
        UpcastExpression
          { upcast_left_operand; upcast_operator; upcast_right_operand }
      | ( SyntaxKind.ConditionalExpression,
          [
            conditional_test;
            conditional_question;
            conditional_consequence;
            conditional_colon;
            conditional_alternative;
          ] ) ->
        ConditionalExpression
          {
            conditional_test;
            conditional_question;
            conditional_consequence;
            conditional_colon;
            conditional_alternative;
          }
      | ( SyntaxKind.EvalExpression,
          [eval_keyword; eval_left_paren; eval_argument; eval_right_paren] ) ->
        EvalExpression
          { eval_keyword; eval_left_paren; eval_argument; eval_right_paren }
      | ( SyntaxKind.IssetExpression,
          [
            isset_keyword;
            isset_left_paren;
            isset_argument_list;
            isset_right_paren;
          ] ) ->
        IssetExpression
          {
            isset_keyword;
            isset_left_paren;
            isset_argument_list;
            isset_right_paren;
          }
      | (SyntaxKind.NameofExpression, [nameof_keyword; nameof_target]) ->
        NameofExpression { nameof_keyword; nameof_target }
      | ( SyntaxKind.FunctionCallExpression,
          [
            function_call_receiver;
            function_call_type_args;
            function_call_left_paren;
            function_call_argument_list;
            function_call_right_paren;
          ] ) ->
        FunctionCallExpression
          {
            function_call_receiver;
            function_call_type_args;
            function_call_left_paren;
            function_call_argument_list;
            function_call_right_paren;
          }
      | ( SyntaxKind.FunctionPointerExpression,
          [function_pointer_receiver; function_pointer_type_args] ) ->
        FunctionPointerExpression
          { function_pointer_receiver; function_pointer_type_args }
      | ( SyntaxKind.ParenthesizedExpression,
          [
            parenthesized_expression_left_paren;
            parenthesized_expression_expression;
            parenthesized_expression_right_paren;
          ] ) ->
        ParenthesizedExpression
          {
            parenthesized_expression_left_paren;
            parenthesized_expression_expression;
            parenthesized_expression_right_paren;
          }
      | ( SyntaxKind.BracedExpression,
          [
            braced_expression_left_brace;
            braced_expression_expression;
            braced_expression_right_brace;
          ] ) ->
        BracedExpression
          {
            braced_expression_left_brace;
            braced_expression_expression;
            braced_expression_right_brace;
          }
      | ( SyntaxKind.ETSpliceExpression,
          [
            et_splice_expression_dollar;
            et_splice_expression_left_brace;
            et_splice_expression_expression;
            et_splice_expression_right_brace;
          ] ) ->
        ETSpliceExpression
          {
            et_splice_expression_dollar;
            et_splice_expression_left_brace;
            et_splice_expression_expression;
            et_splice_expression_right_brace;
          }
      | ( SyntaxKind.EmbeddedBracedExpression,
          [
            embedded_braced_expression_left_brace;
            embedded_braced_expression_expression;
            embedded_braced_expression_right_brace;
          ] ) ->
        EmbeddedBracedExpression
          {
            embedded_braced_expression_left_brace;
            embedded_braced_expression_expression;
            embedded_braced_expression_right_brace;
          }
      | ( SyntaxKind.ListExpression,
          [list_keyword; list_left_paren; list_members; list_right_paren] ) ->
        ListExpression
          { list_keyword; list_left_paren; list_members; list_right_paren }
      | ( SyntaxKind.CollectionLiteralExpression,
          [
            collection_literal_name;
            collection_literal_left_brace;
            collection_literal_initializers;
            collection_literal_right_brace;
          ] ) ->
        CollectionLiteralExpression
          {
            collection_literal_name;
            collection_literal_left_brace;
            collection_literal_initializers;
            collection_literal_right_brace;
          }
      | ( SyntaxKind.ObjectCreationExpression,
          [object_creation_new_keyword; object_creation_object] ) ->
        ObjectCreationExpression
          { object_creation_new_keyword; object_creation_object }
      | ( SyntaxKind.ConstructorCall,
          [
            constructor_call_type;
            constructor_call_left_paren;
            constructor_call_argument_list;
            constructor_call_right_paren;
          ] ) ->
        ConstructorCall
          {
            constructor_call_type;
            constructor_call_left_paren;
            constructor_call_argument_list;
            constructor_call_right_paren;
          }
      | ( SyntaxKind.DarrayIntrinsicExpression,
          [
            darray_intrinsic_keyword;
            darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket;
            darray_intrinsic_members;
            darray_intrinsic_right_bracket;
          ] ) ->
        DarrayIntrinsicExpression
          {
            darray_intrinsic_keyword;
            darray_intrinsic_explicit_type;
            darray_intrinsic_left_bracket;
            darray_intrinsic_members;
            darray_intrinsic_right_bracket;
          }
      | ( SyntaxKind.DictionaryIntrinsicExpression,
          [
            dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket;
          ] ) ->
        DictionaryIntrinsicExpression
          {
            dictionary_intrinsic_keyword;
            dictionary_intrinsic_explicit_type;
            dictionary_intrinsic_left_bracket;
            dictionary_intrinsic_members;
            dictionary_intrinsic_right_bracket;
          }
      | ( SyntaxKind.KeysetIntrinsicExpression,
          [
            keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket;
            keyset_intrinsic_members;
            keyset_intrinsic_right_bracket;
          ] ) ->
        KeysetIntrinsicExpression
          {
            keyset_intrinsic_keyword;
            keyset_intrinsic_explicit_type;
            keyset_intrinsic_left_bracket;
            keyset_intrinsic_members;
            keyset_intrinsic_right_bracket;
          }
      | ( SyntaxKind.VarrayIntrinsicExpression,
          [
            varray_intrinsic_keyword;
            varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket;
            varray_intrinsic_members;
            varray_intrinsic_right_bracket;
          ] ) ->
        VarrayIntrinsicExpression
          {
            varray_intrinsic_keyword;
            varray_intrinsic_explicit_type;
            varray_intrinsic_left_bracket;
            varray_intrinsic_members;
            varray_intrinsic_right_bracket;
          }
      | ( SyntaxKind.VectorIntrinsicExpression,
          [
            vector_intrinsic_keyword;
            vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket;
            vector_intrinsic_members;
            vector_intrinsic_right_bracket;
          ] ) ->
        VectorIntrinsicExpression
          {
            vector_intrinsic_keyword;
            vector_intrinsic_explicit_type;
            vector_intrinsic_left_bracket;
            vector_intrinsic_members;
            vector_intrinsic_right_bracket;
          }
      | ( SyntaxKind.ElementInitializer,
          [element_key; element_arrow; element_value] ) ->
        ElementInitializer { element_key; element_arrow; element_value }
      | ( SyntaxKind.SubscriptExpression,
          [
            subscript_receiver;
            subscript_left_bracket;
            subscript_index;
            subscript_right_bracket;
          ] ) ->
        SubscriptExpression
          {
            subscript_receiver;
            subscript_left_bracket;
            subscript_index;
            subscript_right_bracket;
          }
      | ( SyntaxKind.EmbeddedSubscriptExpression,
          [
            embedded_subscript_receiver;
            embedded_subscript_left_bracket;
            embedded_subscript_index;
            embedded_subscript_right_bracket;
          ] ) ->
        EmbeddedSubscriptExpression
          {
            embedded_subscript_receiver;
            embedded_subscript_left_bracket;
            embedded_subscript_index;
            embedded_subscript_right_bracket;
          }
      | ( SyntaxKind.AwaitableCreationExpression,
          [
            awaitable_attribute_spec;
            awaitable_async;
            awaitable_compound_statement;
          ] ) ->
        AwaitableCreationExpression
          {
            awaitable_attribute_spec;
            awaitable_async;
            awaitable_compound_statement;
          }
      | ( SyntaxKind.XHPChildrenDeclaration,
          [
            xhp_children_keyword; xhp_children_expression; xhp_children_semicolon;
          ] ) ->
        XHPChildrenDeclaration
          {
            xhp_children_keyword;
            xhp_children_expression;
            xhp_children_semicolon;
          }
      | ( SyntaxKind.XHPChildrenParenthesizedList,
          [
            xhp_children_list_left_paren;
            xhp_children_list_xhp_children;
            xhp_children_list_right_paren;
          ] ) ->
        XHPChildrenParenthesizedList
          {
            xhp_children_list_left_paren;
            xhp_children_list_xhp_children;
            xhp_children_list_right_paren;
          }
      | ( SyntaxKind.XHPCategoryDeclaration,
          [
            xhp_category_keyword; xhp_category_categories; xhp_category_semicolon;
          ] ) ->
        XHPCategoryDeclaration
          {
            xhp_category_keyword;
            xhp_category_categories;
            xhp_category_semicolon;
          }
      | ( SyntaxKind.XHPEnumType,
          [
            xhp_enum_like;
            xhp_enum_keyword;
            xhp_enum_left_brace;
            xhp_enum_values;
            xhp_enum_right_brace;
          ] ) ->
        XHPEnumType
          {
            xhp_enum_like;
            xhp_enum_keyword;
            xhp_enum_left_brace;
            xhp_enum_values;
            xhp_enum_right_brace;
          }
      | (SyntaxKind.XHPLateinit, [xhp_lateinit_at; xhp_lateinit_keyword]) ->
        XHPLateinit { xhp_lateinit_at; xhp_lateinit_keyword }
      | (SyntaxKind.XHPRequired, [xhp_required_at; xhp_required_keyword]) ->
        XHPRequired { xhp_required_at; xhp_required_keyword }
      | ( SyntaxKind.XHPClassAttributeDeclaration,
          [
            xhp_attribute_keyword;
            xhp_attribute_attributes;
            xhp_attribute_semicolon;
          ] ) ->
        XHPClassAttributeDeclaration
          {
            xhp_attribute_keyword;
            xhp_attribute_attributes;
            xhp_attribute_semicolon;
          }
      | ( SyntaxKind.XHPClassAttribute,
          [
            xhp_attribute_decl_type;
            xhp_attribute_decl_name;
            xhp_attribute_decl_initializer;
            xhp_attribute_decl_required;
          ] ) ->
        XHPClassAttribute
          {
            xhp_attribute_decl_type;
            xhp_attribute_decl_name;
            xhp_attribute_decl_initializer;
            xhp_attribute_decl_required;
          }
      | (SyntaxKind.XHPSimpleClassAttribute, [xhp_simple_class_attribute_type])
        ->
        XHPSimpleClassAttribute { xhp_simple_class_attribute_type }
      | ( SyntaxKind.XHPSimpleAttribute,
          [
            xhp_simple_attribute_name;
            xhp_simple_attribute_equal;
            xhp_simple_attribute_expression;
          ] ) ->
        XHPSimpleAttribute
          {
            xhp_simple_attribute_name;
            xhp_simple_attribute_equal;
            xhp_simple_attribute_expression;
          }
      | ( SyntaxKind.XHPSpreadAttribute,
          [
            xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace;
          ] ) ->
        XHPSpreadAttribute
          {
            xhp_spread_attribute_left_brace;
            xhp_spread_attribute_spread_operator;
            xhp_spread_attribute_expression;
            xhp_spread_attribute_right_brace;
          }
      | ( SyntaxKind.XHPOpen,
          [
            xhp_open_left_angle;
            xhp_open_name;
            xhp_open_attributes;
            xhp_open_right_angle;
          ] ) ->
        XHPOpen
          {
            xhp_open_left_angle;
            xhp_open_name;
            xhp_open_attributes;
            xhp_open_right_angle;
          }
      | (SyntaxKind.XHPExpression, [xhp_open; xhp_body; xhp_close]) ->
        XHPExpression { xhp_open; xhp_body; xhp_close }
      | ( SyntaxKind.XHPClose,
          [xhp_close_left_angle; xhp_close_name; xhp_close_right_angle] ) ->
        XHPClose { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
      | ( SyntaxKind.TypeConstant,
          [
            type_constant_left_type;
            type_constant_separator;
            type_constant_right_type;
          ] ) ->
        TypeConstant
          {
            type_constant_left_type;
            type_constant_separator;
            type_constant_right_type;
          }
      | ( SyntaxKind.VectorTypeSpecifier,
          [
            vector_type_keyword;
            vector_type_left_angle;
            vector_type_type;
            vector_type_trailing_comma;
            vector_type_right_angle;
          ] ) ->
        VectorTypeSpecifier
          {
            vector_type_keyword;
            vector_type_left_angle;
            vector_type_type;
            vector_type_trailing_comma;
            vector_type_right_angle;
          }
      | ( SyntaxKind.KeysetTypeSpecifier,
          [
            keyset_type_keyword;
            keyset_type_left_angle;
            keyset_type_type;
            keyset_type_trailing_comma;
            keyset_type_right_angle;
          ] ) ->
        KeysetTypeSpecifier
          {
            keyset_type_keyword;
            keyset_type_left_angle;
            keyset_type_type;
            keyset_type_trailing_comma;
            keyset_type_right_angle;
          }
      | ( SyntaxKind.TupleTypeExplicitSpecifier,
          [
            tuple_type_keyword;
            tuple_type_left_angle;
            tuple_type_types;
            tuple_type_right_angle;
          ] ) ->
        TupleTypeExplicitSpecifier
          {
            tuple_type_keyword;
            tuple_type_left_angle;
            tuple_type_types;
            tuple_type_right_angle;
          }
      | ( SyntaxKind.VarrayTypeSpecifier,
          [
            varray_keyword;
            varray_left_angle;
            varray_type;
            varray_trailing_comma;
            varray_right_angle;
          ] ) ->
        VarrayTypeSpecifier
          {
            varray_keyword;
            varray_left_angle;
            varray_type;
            varray_trailing_comma;
            varray_right_angle;
          }
      | ( SyntaxKind.FunctionCtxTypeSpecifier,
          [function_ctx_type_keyword; function_ctx_type_variable] ) ->
        FunctionCtxTypeSpecifier
          { function_ctx_type_keyword; function_ctx_type_variable }
      | ( SyntaxKind.TypeParameter,
          [
            type_attribute_spec;
            type_reified;
            type_variance;
            type_name;
            type_param_params;
            type_constraints;
          ] ) ->
        TypeParameter
          {
            type_attribute_spec;
            type_reified;
            type_variance;
            type_name;
            type_param_params;
            type_constraints;
          }
      | (SyntaxKind.TypeConstraint, [constraint_keyword; constraint_type]) ->
        TypeConstraint { constraint_keyword; constraint_type }
      | ( SyntaxKind.ContextConstraint,
          [ctx_constraint_keyword; ctx_constraint_ctx_list] ) ->
        ContextConstraint { ctx_constraint_keyword; ctx_constraint_ctx_list }
      | ( SyntaxKind.DarrayTypeSpecifier,
          [
            darray_keyword;
            darray_left_angle;
            darray_key;
            darray_comma;
            darray_value;
            darray_trailing_comma;
            darray_right_angle;
          ] ) ->
        DarrayTypeSpecifier
          {
            darray_keyword;
            darray_left_angle;
            darray_key;
            darray_comma;
            darray_value;
            darray_trailing_comma;
            darray_right_angle;
          }
      | ( SyntaxKind.DictionaryTypeSpecifier,
          [
            dictionary_type_keyword;
            dictionary_type_left_angle;
            dictionary_type_members;
            dictionary_type_right_angle;
          ] ) ->
        DictionaryTypeSpecifier
          {
            dictionary_type_keyword;
            dictionary_type_left_angle;
            dictionary_type_members;
            dictionary_type_right_angle;
          }
      | ( SyntaxKind.ClosureTypeSpecifier,
          [
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          ] ) ->
        ClosureTypeSpecifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          }
      | ( SyntaxKind.ClosureParameterTypeSpecifier,
          [
            closure_parameter_call_convention;
            closure_parameter_readonly;
            closure_parameter_type;
          ] ) ->
        ClosureParameterTypeSpecifier
          {
            closure_parameter_call_convention;
            closure_parameter_readonly;
            closure_parameter_type;
          }
      | ( SyntaxKind.TypeRefinement,
          [
            type_refinement_type;
            type_refinement_keyword;
            type_refinement_left_brace;
            type_refinement_members;
            type_refinement_right_brace;
          ] ) ->
        TypeRefinement
          {
            type_refinement_type;
            type_refinement_keyword;
            type_refinement_left_brace;
            type_refinement_members;
            type_refinement_right_brace;
          }
      | ( SyntaxKind.TypeInRefinement,
          [
            type_in_refinement_keyword;
            type_in_refinement_name;
            type_in_refinement_type_parameters;
            type_in_refinement_constraints;
            type_in_refinement_equal;
            type_in_refinement_type;
          ] ) ->
        TypeInRefinement
          {
            type_in_refinement_keyword;
            type_in_refinement_name;
            type_in_refinement_type_parameters;
            type_in_refinement_constraints;
            type_in_refinement_equal;
            type_in_refinement_type;
          }
      | ( SyntaxKind.CtxInRefinement,
          [
            ctx_in_refinement_keyword;
            ctx_in_refinement_name;
            ctx_in_refinement_type_parameters;
            ctx_in_refinement_constraints;
            ctx_in_refinement_equal;
            ctx_in_refinement_ctx_list;
          ] ) ->
        CtxInRefinement
          {
            ctx_in_refinement_keyword;
            ctx_in_refinement_name;
            ctx_in_refinement_type_parameters;
            ctx_in_refinement_constraints;
            ctx_in_refinement_equal;
            ctx_in_refinement_ctx_list;
          }
      | ( SyntaxKind.ClassnameTypeSpecifier,
          [
            classname_keyword;
            classname_left_angle;
            classname_type;
            classname_trailing_comma;
            classname_right_angle;
          ] ) ->
        ClassnameTypeSpecifier
          {
            classname_keyword;
            classname_left_angle;
            classname_type;
            classname_trailing_comma;
            classname_right_angle;
          }
      | ( SyntaxKind.FieldSpecifier,
          [field_question; field_name; field_arrow; field_type] ) ->
        FieldSpecifier { field_question; field_name; field_arrow; field_type }
      | ( SyntaxKind.FieldInitializer,
          [
            field_initializer_name;
            field_initializer_arrow;
            field_initializer_value;
          ] ) ->
        FieldInitializer
          {
            field_initializer_name;
            field_initializer_arrow;
            field_initializer_value;
          }
      | ( SyntaxKind.ShapeTypeSpecifier,
          [
            shape_type_keyword;
            shape_type_left_paren;
            shape_type_fields;
            shape_type_ellipsis;
            shape_type_right_paren;
          ] ) ->
        ShapeTypeSpecifier
          {
            shape_type_keyword;
            shape_type_left_paren;
            shape_type_fields;
            shape_type_ellipsis;
            shape_type_right_paren;
          }
      | ( SyntaxKind.ShapeExpression,
          [
            shape_expression_keyword;
            shape_expression_left_paren;
            shape_expression_fields;
            shape_expression_right_paren;
          ] ) ->
        ShapeExpression
          {
            shape_expression_keyword;
            shape_expression_left_paren;
            shape_expression_fields;
            shape_expression_right_paren;
          }
      | ( SyntaxKind.TupleExpression,
          [
            tuple_expression_keyword;
            tuple_expression_left_paren;
            tuple_expression_items;
            tuple_expression_right_paren;
          ] ) ->
        TupleExpression
          {
            tuple_expression_keyword;
            tuple_expression_left_paren;
            tuple_expression_items;
            tuple_expression_right_paren;
          }
      | ( SyntaxKind.GenericTypeSpecifier,
          [generic_class_type; generic_argument_list] ) ->
        GenericTypeSpecifier { generic_class_type; generic_argument_list }
      | (SyntaxKind.NullableTypeSpecifier, [nullable_question; nullable_type])
        ->
        NullableTypeSpecifier { nullable_question; nullable_type }
      | (SyntaxKind.LikeTypeSpecifier, [like_tilde; like_type]) ->
        LikeTypeSpecifier { like_tilde; like_type }
      | (SyntaxKind.SoftTypeSpecifier, [soft_at; soft_type]) ->
        SoftTypeSpecifier { soft_at; soft_type }
      | ( SyntaxKind.AttributizedSpecifier,
          [attributized_specifier_attribute_spec; attributized_specifier_type]
        ) ->
        AttributizedSpecifier
          { attributized_specifier_attribute_spec; attributized_specifier_type }
      | ( SyntaxKind.ReifiedTypeArgument,
          [reified_type_argument_reified; reified_type_argument_type] ) ->
        ReifiedTypeArgument
          { reified_type_argument_reified; reified_type_argument_type }
      | ( SyntaxKind.TypeArguments,
          [
            type_arguments_left_angle;
            type_arguments_types;
            type_arguments_right_angle;
          ] ) ->
        TypeArguments
          {
            type_arguments_left_angle;
            type_arguments_types;
            type_arguments_right_angle;
          }
      | ( SyntaxKind.TypeParameters,
          [
            type_parameters_left_angle;
            type_parameters_parameters;
            type_parameters_right_angle;
          ] ) ->
        TypeParameters
          {
            type_parameters_left_angle;
            type_parameters_parameters;
            type_parameters_right_angle;
          }
      | ( SyntaxKind.TupleTypeSpecifier,
          [tuple_left_paren; tuple_types; tuple_right_paren] ) ->
        TupleTypeSpecifier { tuple_left_paren; tuple_types; tuple_right_paren }
      | ( SyntaxKind.UnionTypeSpecifier,
          [union_left_paren; union_types; union_right_paren] ) ->
        UnionTypeSpecifier { union_left_paren; union_types; union_right_paren }
      | ( SyntaxKind.IntersectionTypeSpecifier,
          [
            intersection_left_paren; intersection_types; intersection_right_paren;
          ] ) ->
        IntersectionTypeSpecifier
          {
            intersection_left_paren;
            intersection_types;
            intersection_right_paren;
          }
      | (SyntaxKind.ErrorSyntax, [error_error]) -> ErrorSyntax { error_error }
      | (SyntaxKind.ListItem, [list_item; list_separator]) ->
        ListItem { list_item; list_separator }
      | ( SyntaxKind.EnumClassLabelExpression,
          [
            enum_class_label_qualifier;
            enum_class_label_hash;
            enum_class_label_expression;
          ] ) ->
        EnumClassLabelExpression
          {
            enum_class_label_qualifier;
            enum_class_label_hash;
            enum_class_label_expression;
          }
      | ( SyntaxKind.ModuleDeclaration,
          [
            module_declaration_attribute_spec;
            module_declaration_new_keyword;
            module_declaration_module_keyword;
            module_declaration_name;
            module_declaration_left_brace;
            module_declaration_exports;
            module_declaration_imports;
            module_declaration_right_brace;
          ] ) ->
        ModuleDeclaration
          {
            module_declaration_attribute_spec;
            module_declaration_new_keyword;
            module_declaration_module_keyword;
            module_declaration_name;
            module_declaration_left_brace;
            module_declaration_exports;
            module_declaration_imports;
            module_declaration_right_brace;
          }
      | ( SyntaxKind.ModuleExports,
          [
            module_exports_exports_keyword;
            module_exports_left_brace;
            module_exports_exports;
            module_exports_right_brace;
          ] ) ->
        ModuleExports
          {
            module_exports_exports_keyword;
            module_exports_left_brace;
            module_exports_exports;
            module_exports_right_brace;
          }
      | ( SyntaxKind.ModuleImports,
          [
            module_imports_imports_keyword;
            module_imports_left_brace;
            module_imports_imports;
            module_imports_right_brace;
          ] ) ->
        ModuleImports
          {
            module_imports_imports_keyword;
            module_imports_left_brace;
            module_imports_imports;
            module_imports_right_brace;
          }
      | ( SyntaxKind.ModuleMembershipDeclaration,
          [
            module_membership_declaration_module_keyword;
            module_membership_declaration_name;
            module_membership_declaration_semicolon;
          ] ) ->
        ModuleMembershipDeclaration
          {
            module_membership_declaration_module_keyword;
            module_membership_declaration_name;
            module_membership_declaration_semicolon;
          }
      | ( SyntaxKind.PackageExpression,
          [package_expression_keyword; package_expression_name] ) ->
        PackageExpression
          { package_expression_keyword; package_expression_name }
      | (SyntaxKind.Missing, []) -> Missing
      | (SyntaxKind.SyntaxList, items) -> SyntaxList items
      | _ ->
        failwith "syntax_from_children called with wrong number of children"

    let all_tokens node =
      let rec aux acc nodes =
        match nodes with
        | [] -> acc
        | h :: t -> begin
          match syntax h with
          | Token token -> aux (token :: acc) t
          | _ -> aux (aux acc (children h)) t
        end
      in
      List.rev (aux [] [node])

    module type ValueBuilderType = sig
      val value_from_children :
        Full_fidelity_source_text.t ->
        int ->
        (* offset *)
        Full_fidelity_syntax_kind.t ->
        t list ->
        SyntaxValue.t

      val value_from_token : Token.t -> SyntaxValue.t

      val value_from_syntax : syntax -> SyntaxValue.t
    end

    module WithValueBuilder (ValueBuilder : ValueBuilderType) = struct
      let from_children text offset kind ts =
        let syntax = syntax_from_children kind ts in
        let value = ValueBuilder.value_from_children text offset kind ts in
        make syntax value

      let make_token token =
        let syntax = Token token in
        let value = ValueBuilder.value_from_token token in
        make syntax value

      let make_missing text offset =
        from_children text offset SyntaxKind.Missing []

      (* An empty list is represented by Missing; everything else is a
         SyntaxList, even if the list has only one item. *)
      let make_list text offset items =
        match items with
        | [] -> make_missing text offset
        | _ -> from_children text offset SyntaxKind.SyntaxList items

      let make_end_of_file end_of_file_token =
        let syntax = EndOfFile { end_of_file_token } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_script script_declarations =
        let syntax = Script { script_declarations } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_qualified_name qualified_name_parts =
        let syntax = QualifiedName { qualified_name_parts } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_module_name module_name_parts =
        let syntax = ModuleName { module_name_parts } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_simple_type_specifier simple_type_specifier =
        let syntax = SimpleTypeSpecifier { simple_type_specifier } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_literal_expression literal_expression =
        let syntax = LiteralExpression { literal_expression } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_prefixed_string_expression
          prefixed_string_name prefixed_string_str =
        let syntax =
          PrefixedStringExpression { prefixed_string_name; prefixed_string_str }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_prefixed_code_expression
          prefixed_code_prefix
          prefixed_code_left_backtick
          prefixed_code_body
          prefixed_code_right_backtick =
        let syntax =
          PrefixedCodeExpression
            {
              prefixed_code_prefix;
              prefixed_code_left_backtick;
              prefixed_code_body;
              prefixed_code_right_backtick;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_variable_expression variable_expression =
        let syntax = VariableExpression { variable_expression } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_pipe_variable_expression pipe_variable_expression =
        let syntax = PipeVariableExpression { pipe_variable_expression } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_file_attribute_specification
          file_attribute_specification_left_double_angle
          file_attribute_specification_keyword
          file_attribute_specification_colon
          file_attribute_specification_attributes
          file_attribute_specification_right_double_angle =
        let syntax =
          FileAttributeSpecification
            {
              file_attribute_specification_left_double_angle;
              file_attribute_specification_keyword;
              file_attribute_specification_colon;
              file_attribute_specification_attributes;
              file_attribute_specification_right_double_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_declaration
          enum_attribute_spec
          enum_modifiers
          enum_keyword
          enum_name
          enum_colon
          enum_base
          enum_type
          enum_left_brace
          enum_use_clauses
          enum_enumerators
          enum_right_brace =
        let syntax =
          EnumDeclaration
            {
              enum_attribute_spec;
              enum_modifiers;
              enum_keyword;
              enum_name;
              enum_colon;
              enum_base;
              enum_type;
              enum_left_brace;
              enum_use_clauses;
              enum_enumerators;
              enum_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_use enum_use_keyword enum_use_names enum_use_semicolon =
        let syntax =
          EnumUse { enum_use_keyword; enum_use_names; enum_use_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enumerator
          enumerator_name enumerator_equal enumerator_value enumerator_semicolon
          =
        let syntax =
          Enumerator
            {
              enumerator_name;
              enumerator_equal;
              enumerator_value;
              enumerator_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_class_declaration
          enum_class_attribute_spec
          enum_class_modifiers
          enum_class_enum_keyword
          enum_class_class_keyword
          enum_class_name
          enum_class_colon
          enum_class_base
          enum_class_extends
          enum_class_extends_list
          enum_class_left_brace
          enum_class_elements
          enum_class_right_brace =
        let syntax =
          EnumClassDeclaration
            {
              enum_class_attribute_spec;
              enum_class_modifiers;
              enum_class_enum_keyword;
              enum_class_class_keyword;
              enum_class_name;
              enum_class_colon;
              enum_class_base;
              enum_class_extends;
              enum_class_extends_list;
              enum_class_left_brace;
              enum_class_elements;
              enum_class_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_class_enumerator
          enum_class_enumerator_modifiers
          enum_class_enumerator_type
          enum_class_enumerator_name
          enum_class_enumerator_initializer
          enum_class_enumerator_semicolon =
        let syntax =
          EnumClassEnumerator
            {
              enum_class_enumerator_modifiers;
              enum_class_enumerator_type;
              enum_class_enumerator_name;
              enum_class_enumerator_initializer;
              enum_class_enumerator_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_alias_declaration
          alias_attribute_spec
          alias_modifiers
          alias_module_kw_opt
          alias_keyword
          alias_name
          alias_generic_parameter
          alias_constraint
          alias_equal
          alias_type
          alias_semicolon =
        let syntax =
          AliasDeclaration
            {
              alias_attribute_spec;
              alias_modifiers;
              alias_module_kw_opt;
              alias_keyword;
              alias_name;
              alias_generic_parameter;
              alias_constraint;
              alias_equal;
              alias_type;
              alias_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_context_alias_declaration
          ctx_alias_attribute_spec
          ctx_alias_keyword
          ctx_alias_name
          ctx_alias_generic_parameter
          ctx_alias_as_constraint
          ctx_alias_equal
          ctx_alias_context
          ctx_alias_semicolon =
        let syntax =
          ContextAliasDeclaration
            {
              ctx_alias_attribute_spec;
              ctx_alias_keyword;
              ctx_alias_name;
              ctx_alias_generic_parameter;
              ctx_alias_as_constraint;
              ctx_alias_equal;
              ctx_alias_context;
              ctx_alias_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_case_type_declaration
          case_type_attribute_spec
          case_type_modifiers
          case_type_case_keyword
          case_type_type_keyword
          case_type_name
          case_type_generic_parameter
          case_type_as
          case_type_bounds
          case_type_equal
          case_type_variants
          case_type_semicolon =
        let syntax =
          CaseTypeDeclaration
            {
              case_type_attribute_spec;
              case_type_modifiers;
              case_type_case_keyword;
              case_type_type_keyword;
              case_type_name;
              case_type_generic_parameter;
              case_type_as;
              case_type_bounds;
              case_type_equal;
              case_type_variants;
              case_type_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_case_type_variant case_type_variant_bar case_type_variant_type =
        let syntax =
          CaseTypeVariant { case_type_variant_bar; case_type_variant_type }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_property_declaration
          property_attribute_spec
          property_modifiers
          property_type
          property_declarators
          property_semicolon =
        let syntax =
          PropertyDeclaration
            {
              property_attribute_spec;
              property_modifiers;
              property_type;
              property_declarators;
              property_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_property_declarator property_name property_initializer =
        let syntax =
          PropertyDeclarator { property_name; property_initializer }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_declaration namespace_header namespace_body =
        let syntax =
          NamespaceDeclaration { namespace_header; namespace_body }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_declaration_header namespace_keyword namespace_name =
        let syntax =
          NamespaceDeclarationHeader { namespace_keyword; namespace_name }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_body
          namespace_left_brace namespace_declarations namespace_right_brace =
        let syntax =
          NamespaceBody
            {
              namespace_left_brace;
              namespace_declarations;
              namespace_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_empty_body namespace_semicolon =
        let syntax = NamespaceEmptyBody { namespace_semicolon } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_use_declaration
          namespace_use_keyword
          namespace_use_kind
          namespace_use_clauses
          namespace_use_semicolon =
        let syntax =
          NamespaceUseDeclaration
            {
              namespace_use_keyword;
              namespace_use_kind;
              namespace_use_clauses;
              namespace_use_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_group_use_declaration
          namespace_group_use_keyword
          namespace_group_use_kind
          namespace_group_use_prefix
          namespace_group_use_left_brace
          namespace_group_use_clauses
          namespace_group_use_right_brace
          namespace_group_use_semicolon =
        let syntax =
          NamespaceGroupUseDeclaration
            {
              namespace_group_use_keyword;
              namespace_group_use_kind;
              namespace_group_use_prefix;
              namespace_group_use_left_brace;
              namespace_group_use_clauses;
              namespace_group_use_right_brace;
              namespace_group_use_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_use_clause
          namespace_use_clause_kind
          namespace_use_name
          namespace_use_as
          namespace_use_alias =
        let syntax =
          NamespaceUseClause
            {
              namespace_use_clause_kind;
              namespace_use_name;
              namespace_use_as;
              namespace_use_alias;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_declaration
          function_attribute_spec function_declaration_header function_body =
        let syntax =
          FunctionDeclaration
            {
              function_attribute_spec;
              function_declaration_header;
              function_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_declaration_header
          function_modifiers
          function_keyword
          function_name
          function_type_parameter_list
          function_left_paren
          function_parameter_list
          function_right_paren
          function_contexts
          function_colon
          function_readonly_return
          function_type
          function_where_clause =
        let syntax =
          FunctionDeclarationHeader
            {
              function_modifiers;
              function_keyword;
              function_name;
              function_type_parameter_list;
              function_left_paren;
              function_parameter_list;
              function_right_paren;
              function_contexts;
              function_colon;
              function_readonly_return;
              function_type;
              function_where_clause;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_contexts
          contexts_left_bracket contexts_types contexts_right_bracket =
        let syntax =
          Contexts
            { contexts_left_bracket; contexts_types; contexts_right_bracket }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_where_clause where_clause_keyword where_clause_constraints =
        let syntax =
          WhereClause { where_clause_keyword; where_clause_constraints }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_where_constraint
          where_constraint_left_type
          where_constraint_operator
          where_constraint_right_type =
        let syntax =
          WhereConstraint
            {
              where_constraint_left_type;
              where_constraint_operator;
              where_constraint_right_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_methodish_declaration
          methodish_attribute
          methodish_function_decl_header
          methodish_function_body
          methodish_semicolon =
        let syntax =
          MethodishDeclaration
            {
              methodish_attribute;
              methodish_function_decl_header;
              methodish_function_body;
              methodish_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_methodish_trait_resolution
          methodish_trait_attribute
          methodish_trait_function_decl_header
          methodish_trait_equal
          methodish_trait_name
          methodish_trait_semicolon =
        let syntax =
          MethodishTraitResolution
            {
              methodish_trait_attribute;
              methodish_trait_function_decl_header;
              methodish_trait_equal;
              methodish_trait_name;
              methodish_trait_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classish_declaration
          classish_attribute
          classish_modifiers
          classish_xhp
          classish_keyword
          classish_name
          classish_type_parameters
          classish_extends_keyword
          classish_extends_list
          classish_implements_keyword
          classish_implements_list
          classish_where_clause
          classish_body =
        let syntax =
          ClassishDeclaration
            {
              classish_attribute;
              classish_modifiers;
              classish_xhp;
              classish_keyword;
              classish_name;
              classish_type_parameters;
              classish_extends_keyword;
              classish_extends_list;
              classish_implements_keyword;
              classish_implements_list;
              classish_where_clause;
              classish_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classish_body
          classish_body_left_brace
          classish_body_elements
          classish_body_right_brace =
        let syntax =
          ClassishBody
            {
              classish_body_left_brace;
              classish_body_elements;
              classish_body_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_trait_use trait_use_keyword trait_use_names trait_use_semicolon =
        let syntax =
          TraitUse { trait_use_keyword; trait_use_names; trait_use_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_require_clause
          require_keyword require_kind require_name require_semicolon =
        let syntax =
          RequireClause
            { require_keyword; require_kind; require_name; require_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_const_declaration
          const_attribute_spec
          const_modifiers
          const_keyword
          const_type_specifier
          const_declarators
          const_semicolon =
        let syntax =
          ConstDeclaration
            {
              const_attribute_spec;
              const_modifiers;
              const_keyword;
              const_type_specifier;
              const_declarators;
              const_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_constant_declarator
          constant_declarator_name constant_declarator_initializer =
        let syntax =
          ConstantDeclarator
            { constant_declarator_name; constant_declarator_initializer }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_const_declaration
          type_const_attribute_spec
          type_const_modifiers
          type_const_keyword
          type_const_type_keyword
          type_const_name
          type_const_type_parameters
          type_const_type_constraints
          type_const_equal
          type_const_type_specifier
          type_const_semicolon =
        let syntax =
          TypeConstDeclaration
            {
              type_const_attribute_spec;
              type_const_modifiers;
              type_const_keyword;
              type_const_type_keyword;
              type_const_name;
              type_const_type_parameters;
              type_const_type_constraints;
              type_const_equal;
              type_const_type_specifier;
              type_const_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_context_const_declaration
          context_const_modifiers
          context_const_const_keyword
          context_const_ctx_keyword
          context_const_name
          context_const_type_parameters
          context_const_constraint
          context_const_equal
          context_const_ctx_list
          context_const_semicolon =
        let syntax =
          ContextConstDeclaration
            {
              context_const_modifiers;
              context_const_const_keyword;
              context_const_ctx_keyword;
              context_const_name;
              context_const_type_parameters;
              context_const_constraint;
              context_const_equal;
              context_const_ctx_list;
              context_const_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_decorated_expression
          decorated_expression_decorator decorated_expression_expression =
        let syntax =
          DecoratedExpression
            { decorated_expression_decorator; decorated_expression_expression }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_parameter_declaration
          parameter_attribute
          parameter_visibility
          parameter_call_convention
          parameter_readonly
          parameter_type
          parameter_name
          parameter_default_value =
        let syntax =
          ParameterDeclaration
            {
              parameter_attribute;
              parameter_visibility;
              parameter_call_convention;
              parameter_readonly;
              parameter_type;
              parameter_name;
              parameter_default_value;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_variadic_parameter
          variadic_parameter_call_convention
          variadic_parameter_type
          variadic_parameter_ellipsis =
        let syntax =
          VariadicParameter
            {
              variadic_parameter_call_convention;
              variadic_parameter_type;
              variadic_parameter_ellipsis;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_old_attribute_specification
          old_attribute_specification_left_double_angle
          old_attribute_specification_attributes
          old_attribute_specification_right_double_angle =
        let syntax =
          OldAttributeSpecification
            {
              old_attribute_specification_left_double_angle;
              old_attribute_specification_attributes;
              old_attribute_specification_right_double_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_attribute_specification attribute_specification_attributes =
        let syntax =
          AttributeSpecification { attribute_specification_attributes }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_attribute attribute_at attribute_attribute_name =
        let syntax = Attribute { attribute_at; attribute_attribute_name } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_inclusion_expression inclusion_require inclusion_filename =
        let syntax =
          InclusionExpression { inclusion_require; inclusion_filename }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_inclusion_directive inclusion_expression inclusion_semicolon =
        let syntax =
          InclusionDirective { inclusion_expression; inclusion_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_compound_statement
          compound_left_brace compound_statements compound_right_brace =
        let syntax =
          CompoundStatement
            { compound_left_brace; compound_statements; compound_right_brace }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_expression_statement
          expression_statement_expression expression_statement_semicolon =
        let syntax =
          ExpressionStatement
            { expression_statement_expression; expression_statement_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_markup_section markup_hashbang markup_suffix =
        let syntax = MarkupSection { markup_hashbang; markup_suffix } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_markup_suffix markup_suffix_less_than_question markup_suffix_name
          =
        let syntax =
          MarkupSuffix { markup_suffix_less_than_question; markup_suffix_name }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_unset_statement
          unset_keyword
          unset_left_paren
          unset_variables
          unset_right_paren
          unset_semicolon =
        let syntax =
          UnsetStatement
            {
              unset_keyword;
              unset_left_paren;
              unset_variables;
              unset_right_paren;
              unset_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_declare_local_statement
          declare_local_keyword
          declare_local_variable
          declare_local_colon
          declare_local_type
          declare_local_initializer
          declare_local_semicolon =
        let syntax =
          DeclareLocalStatement
            {
              declare_local_keyword;
              declare_local_variable;
              declare_local_colon;
              declare_local_type;
              declare_local_initializer;
              declare_local_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_using_statement_block_scoped
          using_block_await_keyword
          using_block_using_keyword
          using_block_left_paren
          using_block_expressions
          using_block_right_paren
          using_block_body =
        let syntax =
          UsingStatementBlockScoped
            {
              using_block_await_keyword;
              using_block_using_keyword;
              using_block_left_paren;
              using_block_expressions;
              using_block_right_paren;
              using_block_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_using_statement_function_scoped
          using_function_await_keyword
          using_function_using_keyword
          using_function_expression
          using_function_semicolon =
        let syntax =
          UsingStatementFunctionScoped
            {
              using_function_await_keyword;
              using_function_using_keyword;
              using_function_expression;
              using_function_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_while_statement
          while_keyword
          while_left_paren
          while_condition
          while_right_paren
          while_body =
        let syntax =
          WhileStatement
            {
              while_keyword;
              while_left_paren;
              while_condition;
              while_right_paren;
              while_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_if_statement
          if_keyword
          if_left_paren
          if_condition
          if_right_paren
          if_statement
          if_else_clause =
        let syntax =
          IfStatement
            {
              if_keyword;
              if_left_paren;
              if_condition;
              if_right_paren;
              if_statement;
              if_else_clause;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_else_clause else_keyword else_statement =
        let syntax = ElseClause { else_keyword; else_statement } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_try_statement
          try_keyword
          try_compound_statement
          try_catch_clauses
          try_finally_clause =
        let syntax =
          TryStatement
            {
              try_keyword;
              try_compound_statement;
              try_catch_clauses;
              try_finally_clause;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_catch_clause
          catch_keyword
          catch_left_paren
          catch_type
          catch_variable
          catch_right_paren
          catch_body =
        let syntax =
          CatchClause
            {
              catch_keyword;
              catch_left_paren;
              catch_type;
              catch_variable;
              catch_right_paren;
              catch_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_finally_clause finally_keyword finally_body =
        let syntax = FinallyClause { finally_keyword; finally_body } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_do_statement
          do_keyword
          do_body
          do_while_keyword
          do_left_paren
          do_condition
          do_right_paren
          do_semicolon =
        let syntax =
          DoStatement
            {
              do_keyword;
              do_body;
              do_while_keyword;
              do_left_paren;
              do_condition;
              do_right_paren;
              do_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_for_statement
          for_keyword
          for_left_paren
          for_initializer
          for_first_semicolon
          for_control
          for_second_semicolon
          for_end_of_loop
          for_right_paren
          for_body =
        let syntax =
          ForStatement
            {
              for_keyword;
              for_left_paren;
              for_initializer;
              for_first_semicolon;
              for_control;
              for_second_semicolon;
              for_end_of_loop;
              for_right_paren;
              for_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_foreach_statement
          foreach_keyword
          foreach_left_paren
          foreach_collection
          foreach_await_keyword
          foreach_as
          foreach_key
          foreach_arrow
          foreach_value
          foreach_right_paren
          foreach_body =
        let syntax =
          ForeachStatement
            {
              foreach_keyword;
              foreach_left_paren;
              foreach_collection;
              foreach_await_keyword;
              foreach_as;
              foreach_key;
              foreach_arrow;
              foreach_value;
              foreach_right_paren;
              foreach_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_statement
          switch_keyword
          switch_left_paren
          switch_expression
          switch_right_paren
          switch_left_brace
          switch_sections
          switch_right_brace =
        let syntax =
          SwitchStatement
            {
              switch_keyword;
              switch_left_paren;
              switch_expression;
              switch_right_paren;
              switch_left_brace;
              switch_sections;
              switch_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_section
          switch_section_labels
          switch_section_statements
          switch_section_fallthrough =
        let syntax =
          SwitchSection
            {
              switch_section_labels;
              switch_section_statements;
              switch_section_fallthrough;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_fallthrough fallthrough_keyword fallthrough_semicolon =
        let syntax =
          SwitchFallthrough { fallthrough_keyword; fallthrough_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_case_label case_keyword case_expression case_colon =
        let syntax = CaseLabel { case_keyword; case_expression; case_colon } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_default_label default_keyword default_colon =
        let syntax = DefaultLabel { default_keyword; default_colon } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_match_statement
          match_statement_keyword
          match_statement_left_paren
          match_statement_expression
          match_statement_right_paren
          match_statement_left_brace
          match_statement_arms
          match_statement_right_brace =
        let syntax =
          MatchStatement
            {
              match_statement_keyword;
              match_statement_left_paren;
              match_statement_expression;
              match_statement_right_paren;
              match_statement_left_brace;
              match_statement_arms;
              match_statement_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_match_statement_arm
          match_statement_arm_pattern
          match_statement_arm_arrow
          match_statement_arm_body =
        let syntax =
          MatchStatementArm
            {
              match_statement_arm_pattern;
              match_statement_arm_arrow;
              match_statement_arm_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_return_statement
          return_keyword return_expression return_semicolon =
        let syntax =
          ReturnStatement
            { return_keyword; return_expression; return_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_yield_break_statement
          yield_break_keyword yield_break_break yield_break_semicolon =
        let syntax =
          YieldBreakStatement
            { yield_break_keyword; yield_break_break; yield_break_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_throw_statement throw_keyword throw_expression throw_semicolon =
        let syntax =
          ThrowStatement { throw_keyword; throw_expression; throw_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_break_statement break_keyword break_semicolon =
        let syntax = BreakStatement { break_keyword; break_semicolon } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_continue_statement continue_keyword continue_semicolon =
        let syntax =
          ContinueStatement { continue_keyword; continue_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_echo_statement echo_keyword echo_expressions echo_semicolon =
        let syntax =
          EchoStatement { echo_keyword; echo_expressions; echo_semicolon }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_concurrent_statement concurrent_keyword concurrent_statement =
        let syntax =
          ConcurrentStatement { concurrent_keyword; concurrent_statement }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_simple_initializer
          simple_initializer_equal simple_initializer_value =
        let syntax =
          SimpleInitializer
            { simple_initializer_equal; simple_initializer_value }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_anonymous_class
          anonymous_class_class_keyword
          anonymous_class_left_paren
          anonymous_class_argument_list
          anonymous_class_right_paren
          anonymous_class_extends_keyword
          anonymous_class_extends_list
          anonymous_class_implements_keyword
          anonymous_class_implements_list
          anonymous_class_body =
        let syntax =
          AnonymousClass
            {
              anonymous_class_class_keyword;
              anonymous_class_left_paren;
              anonymous_class_argument_list;
              anonymous_class_right_paren;
              anonymous_class_extends_keyword;
              anonymous_class_extends_list;
              anonymous_class_implements_keyword;
              anonymous_class_implements_list;
              anonymous_class_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_anonymous_function
          anonymous_attribute_spec
          anonymous_async_keyword
          anonymous_function_keyword
          anonymous_left_paren
          anonymous_parameters
          anonymous_right_paren
          anonymous_ctx_list
          anonymous_colon
          anonymous_readonly_return
          anonymous_type
          anonymous_use
          anonymous_body =
        let syntax =
          AnonymousFunction
            {
              anonymous_attribute_spec;
              anonymous_async_keyword;
              anonymous_function_keyword;
              anonymous_left_paren;
              anonymous_parameters;
              anonymous_right_paren;
              anonymous_ctx_list;
              anonymous_colon;
              anonymous_readonly_return;
              anonymous_type;
              anonymous_use;
              anonymous_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_anonymous_function_use_clause
          anonymous_use_keyword
          anonymous_use_left_paren
          anonymous_use_variables
          anonymous_use_right_paren =
        let syntax =
          AnonymousFunctionUseClause
            {
              anonymous_use_keyword;
              anonymous_use_left_paren;
              anonymous_use_variables;
              anonymous_use_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_variable_pattern variable_pattern_variable =
        let syntax = VariablePattern { variable_pattern_variable } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_constructor_pattern
          constructor_pattern_constructor
          constructor_pattern_left_paren
          constructor_pattern_members
          constructor_pattern_right_paren =
        let syntax =
          ConstructorPattern
            {
              constructor_pattern_constructor;
              constructor_pattern_left_paren;
              constructor_pattern_members;
              constructor_pattern_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_refinement_pattern
          refinement_pattern_variable
          refinement_pattern_colon
          refinement_pattern_specifier =
        let syntax =
          RefinementPattern
            {
              refinement_pattern_variable;
              refinement_pattern_colon;
              refinement_pattern_specifier;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_lambda_expression
          lambda_attribute_spec
          lambda_async
          lambda_signature
          lambda_arrow
          lambda_body =
        let syntax =
          LambdaExpression
            {
              lambda_attribute_spec;
              lambda_async;
              lambda_signature;
              lambda_arrow;
              lambda_body;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_lambda_signature
          lambda_left_paren
          lambda_parameters
          lambda_right_paren
          lambda_contexts
          lambda_colon
          lambda_readonly_return
          lambda_type =
        let syntax =
          LambdaSignature
            {
              lambda_left_paren;
              lambda_parameters;
              lambda_right_paren;
              lambda_contexts;
              lambda_colon;
              lambda_readonly_return;
              lambda_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_cast_expression
          cast_left_paren cast_type cast_right_paren cast_operand =
        let syntax =
          CastExpression
            { cast_left_paren; cast_type; cast_right_paren; cast_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_scope_resolution_expression
          scope_resolution_qualifier
          scope_resolution_operator
          scope_resolution_name =
        let syntax =
          ScopeResolutionExpression
            {
              scope_resolution_qualifier;
              scope_resolution_operator;
              scope_resolution_name;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_member_selection_expression
          member_object member_operator member_name =
        let syntax =
          MemberSelectionExpression
            { member_object; member_operator; member_name }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_safe_member_selection_expression
          safe_member_object safe_member_operator safe_member_name =
        let syntax =
          SafeMemberSelectionExpression
            { safe_member_object; safe_member_operator; safe_member_name }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_member_selection_expression
          embedded_member_object embedded_member_operator embedded_member_name =
        let syntax =
          EmbeddedMemberSelectionExpression
            {
              embedded_member_object;
              embedded_member_operator;
              embedded_member_name;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_yield_expression yield_keyword yield_operand =
        let syntax = YieldExpression { yield_keyword; yield_operand } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_prefix_unary_expression
          prefix_unary_operator prefix_unary_operand =
        let syntax =
          PrefixUnaryExpression { prefix_unary_operator; prefix_unary_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_postfix_unary_expression
          postfix_unary_operand postfix_unary_operator =
        let syntax =
          PostfixUnaryExpression
            { postfix_unary_operand; postfix_unary_operator }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_binary_expression
          binary_left_operand binary_operator binary_right_operand =
        let syntax =
          BinaryExpression
            { binary_left_operand; binary_operator; binary_right_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_is_expression is_left_operand is_operator is_right_operand =
        let syntax =
          IsExpression { is_left_operand; is_operator; is_right_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_as_expression as_left_operand as_operator as_right_operand =
        let syntax =
          AsExpression { as_left_operand; as_operator; as_right_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_nullable_as_expression
          nullable_as_left_operand
          nullable_as_operator
          nullable_as_right_operand =
        let syntax =
          NullableAsExpression
            {
              nullable_as_left_operand;
              nullable_as_operator;
              nullable_as_right_operand;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_upcast_expression
          upcast_left_operand upcast_operator upcast_right_operand =
        let syntax =
          UpcastExpression
            { upcast_left_operand; upcast_operator; upcast_right_operand }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_conditional_expression
          conditional_test
          conditional_question
          conditional_consequence
          conditional_colon
          conditional_alternative =
        let syntax =
          ConditionalExpression
            {
              conditional_test;
              conditional_question;
              conditional_consequence;
              conditional_colon;
              conditional_alternative;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_eval_expression
          eval_keyword eval_left_paren eval_argument eval_right_paren =
        let syntax =
          EvalExpression
            { eval_keyword; eval_left_paren; eval_argument; eval_right_paren }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_isset_expression
          isset_keyword isset_left_paren isset_argument_list isset_right_paren =
        let syntax =
          IssetExpression
            {
              isset_keyword;
              isset_left_paren;
              isset_argument_list;
              isset_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_nameof_expression nameof_keyword nameof_target =
        let syntax = NameofExpression { nameof_keyword; nameof_target } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_call_expression
          function_call_receiver
          function_call_type_args
          function_call_left_paren
          function_call_argument_list
          function_call_right_paren =
        let syntax =
          FunctionCallExpression
            {
              function_call_receiver;
              function_call_type_args;
              function_call_left_paren;
              function_call_argument_list;
              function_call_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_pointer_expression
          function_pointer_receiver function_pointer_type_args =
        let syntax =
          FunctionPointerExpression
            { function_pointer_receiver; function_pointer_type_args }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_parenthesized_expression
          parenthesized_expression_left_paren
          parenthesized_expression_expression
          parenthesized_expression_right_paren =
        let syntax =
          ParenthesizedExpression
            {
              parenthesized_expression_left_paren;
              parenthesized_expression_expression;
              parenthesized_expression_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_braced_expression
          braced_expression_left_brace
          braced_expression_expression
          braced_expression_right_brace =
        let syntax =
          BracedExpression
            {
              braced_expression_left_brace;
              braced_expression_expression;
              braced_expression_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_et_splice_expression
          et_splice_expression_dollar
          et_splice_expression_left_brace
          et_splice_expression_expression
          et_splice_expression_right_brace =
        let syntax =
          ETSpliceExpression
            {
              et_splice_expression_dollar;
              et_splice_expression_left_brace;
              et_splice_expression_expression;
              et_splice_expression_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_braced_expression
          embedded_braced_expression_left_brace
          embedded_braced_expression_expression
          embedded_braced_expression_right_brace =
        let syntax =
          EmbeddedBracedExpression
            {
              embedded_braced_expression_left_brace;
              embedded_braced_expression_expression;
              embedded_braced_expression_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_list_expression
          list_keyword list_left_paren list_members list_right_paren =
        let syntax =
          ListExpression
            { list_keyword; list_left_paren; list_members; list_right_paren }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_collection_literal_expression
          collection_literal_name
          collection_literal_left_brace
          collection_literal_initializers
          collection_literal_right_brace =
        let syntax =
          CollectionLiteralExpression
            {
              collection_literal_name;
              collection_literal_left_brace;
              collection_literal_initializers;
              collection_literal_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_object_creation_expression
          object_creation_new_keyword object_creation_object =
        let syntax =
          ObjectCreationExpression
            { object_creation_new_keyword; object_creation_object }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_constructor_call
          constructor_call_type
          constructor_call_left_paren
          constructor_call_argument_list
          constructor_call_right_paren =
        let syntax =
          ConstructorCall
            {
              constructor_call_type;
              constructor_call_left_paren;
              constructor_call_argument_list;
              constructor_call_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_darray_intrinsic_expression
          darray_intrinsic_keyword
          darray_intrinsic_explicit_type
          darray_intrinsic_left_bracket
          darray_intrinsic_members
          darray_intrinsic_right_bracket =
        let syntax =
          DarrayIntrinsicExpression
            {
              darray_intrinsic_keyword;
              darray_intrinsic_explicit_type;
              darray_intrinsic_left_bracket;
              darray_intrinsic_members;
              darray_intrinsic_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_dictionary_intrinsic_expression
          dictionary_intrinsic_keyword
          dictionary_intrinsic_explicit_type
          dictionary_intrinsic_left_bracket
          dictionary_intrinsic_members
          dictionary_intrinsic_right_bracket =
        let syntax =
          DictionaryIntrinsicExpression
            {
              dictionary_intrinsic_keyword;
              dictionary_intrinsic_explicit_type;
              dictionary_intrinsic_left_bracket;
              dictionary_intrinsic_members;
              dictionary_intrinsic_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_keyset_intrinsic_expression
          keyset_intrinsic_keyword
          keyset_intrinsic_explicit_type
          keyset_intrinsic_left_bracket
          keyset_intrinsic_members
          keyset_intrinsic_right_bracket =
        let syntax =
          KeysetIntrinsicExpression
            {
              keyset_intrinsic_keyword;
              keyset_intrinsic_explicit_type;
              keyset_intrinsic_left_bracket;
              keyset_intrinsic_members;
              keyset_intrinsic_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_varray_intrinsic_expression
          varray_intrinsic_keyword
          varray_intrinsic_explicit_type
          varray_intrinsic_left_bracket
          varray_intrinsic_members
          varray_intrinsic_right_bracket =
        let syntax =
          VarrayIntrinsicExpression
            {
              varray_intrinsic_keyword;
              varray_intrinsic_explicit_type;
              varray_intrinsic_left_bracket;
              varray_intrinsic_members;
              varray_intrinsic_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_vector_intrinsic_expression
          vector_intrinsic_keyword
          vector_intrinsic_explicit_type
          vector_intrinsic_left_bracket
          vector_intrinsic_members
          vector_intrinsic_right_bracket =
        let syntax =
          VectorIntrinsicExpression
            {
              vector_intrinsic_keyword;
              vector_intrinsic_explicit_type;
              vector_intrinsic_left_bracket;
              vector_intrinsic_members;
              vector_intrinsic_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_element_initializer element_key element_arrow element_value =
        let syntax =
          ElementInitializer { element_key; element_arrow; element_value }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_subscript_expression
          subscript_receiver
          subscript_left_bracket
          subscript_index
          subscript_right_bracket =
        let syntax =
          SubscriptExpression
            {
              subscript_receiver;
              subscript_left_bracket;
              subscript_index;
              subscript_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_subscript_expression
          embedded_subscript_receiver
          embedded_subscript_left_bracket
          embedded_subscript_index
          embedded_subscript_right_bracket =
        let syntax =
          EmbeddedSubscriptExpression
            {
              embedded_subscript_receiver;
              embedded_subscript_left_bracket;
              embedded_subscript_index;
              embedded_subscript_right_bracket;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_awaitable_creation_expression
          awaitable_attribute_spec awaitable_async awaitable_compound_statement
          =
        let syntax =
          AwaitableCreationExpression
            {
              awaitable_attribute_spec;
              awaitable_async;
              awaitable_compound_statement;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_children_declaration
          xhp_children_keyword xhp_children_expression xhp_children_semicolon =
        let syntax =
          XHPChildrenDeclaration
            {
              xhp_children_keyword;
              xhp_children_expression;
              xhp_children_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_children_parenthesized_list
          xhp_children_list_left_paren
          xhp_children_list_xhp_children
          xhp_children_list_right_paren =
        let syntax =
          XHPChildrenParenthesizedList
            {
              xhp_children_list_left_paren;
              xhp_children_list_xhp_children;
              xhp_children_list_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_category_declaration
          xhp_category_keyword xhp_category_categories xhp_category_semicolon =
        let syntax =
          XHPCategoryDeclaration
            {
              xhp_category_keyword;
              xhp_category_categories;
              xhp_category_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_enum_type
          xhp_enum_like
          xhp_enum_keyword
          xhp_enum_left_brace
          xhp_enum_values
          xhp_enum_right_brace =
        let syntax =
          XHPEnumType
            {
              xhp_enum_like;
              xhp_enum_keyword;
              xhp_enum_left_brace;
              xhp_enum_values;
              xhp_enum_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_lateinit xhp_lateinit_at xhp_lateinit_keyword =
        let syntax = XHPLateinit { xhp_lateinit_at; xhp_lateinit_keyword } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_required xhp_required_at xhp_required_keyword =
        let syntax = XHPRequired { xhp_required_at; xhp_required_keyword } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_class_attribute_declaration
          xhp_attribute_keyword xhp_attribute_attributes xhp_attribute_semicolon
          =
        let syntax =
          XHPClassAttributeDeclaration
            {
              xhp_attribute_keyword;
              xhp_attribute_attributes;
              xhp_attribute_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_class_attribute
          xhp_attribute_decl_type
          xhp_attribute_decl_name
          xhp_attribute_decl_initializer
          xhp_attribute_decl_required =
        let syntax =
          XHPClassAttribute
            {
              xhp_attribute_decl_type;
              xhp_attribute_decl_name;
              xhp_attribute_decl_initializer;
              xhp_attribute_decl_required;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_simple_class_attribute xhp_simple_class_attribute_type =
        let syntax =
          XHPSimpleClassAttribute { xhp_simple_class_attribute_type }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_simple_attribute
          xhp_simple_attribute_name
          xhp_simple_attribute_equal
          xhp_simple_attribute_expression =
        let syntax =
          XHPSimpleAttribute
            {
              xhp_simple_attribute_name;
              xhp_simple_attribute_equal;
              xhp_simple_attribute_expression;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_spread_attribute
          xhp_spread_attribute_left_brace
          xhp_spread_attribute_spread_operator
          xhp_spread_attribute_expression
          xhp_spread_attribute_right_brace =
        let syntax =
          XHPSpreadAttribute
            {
              xhp_spread_attribute_left_brace;
              xhp_spread_attribute_spread_operator;
              xhp_spread_attribute_expression;
              xhp_spread_attribute_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_open
          xhp_open_left_angle
          xhp_open_name
          xhp_open_attributes
          xhp_open_right_angle =
        let syntax =
          XHPOpen
            {
              xhp_open_left_angle;
              xhp_open_name;
              xhp_open_attributes;
              xhp_open_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_expression xhp_open xhp_body xhp_close =
        let syntax = XHPExpression { xhp_open; xhp_body; xhp_close } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_close
          xhp_close_left_angle xhp_close_name xhp_close_right_angle =
        let syntax =
          XHPClose
            { xhp_close_left_angle; xhp_close_name; xhp_close_right_angle }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_constant
          type_constant_left_type
          type_constant_separator
          type_constant_right_type =
        let syntax =
          TypeConstant
            {
              type_constant_left_type;
              type_constant_separator;
              type_constant_right_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_vector_type_specifier
          vector_type_keyword
          vector_type_left_angle
          vector_type_type
          vector_type_trailing_comma
          vector_type_right_angle =
        let syntax =
          VectorTypeSpecifier
            {
              vector_type_keyword;
              vector_type_left_angle;
              vector_type_type;
              vector_type_trailing_comma;
              vector_type_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_keyset_type_specifier
          keyset_type_keyword
          keyset_type_left_angle
          keyset_type_type
          keyset_type_trailing_comma
          keyset_type_right_angle =
        let syntax =
          KeysetTypeSpecifier
            {
              keyset_type_keyword;
              keyset_type_left_angle;
              keyset_type_type;
              keyset_type_trailing_comma;
              keyset_type_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_type_explicit_specifier
          tuple_type_keyword
          tuple_type_left_angle
          tuple_type_types
          tuple_type_right_angle =
        let syntax =
          TupleTypeExplicitSpecifier
            {
              tuple_type_keyword;
              tuple_type_left_angle;
              tuple_type_types;
              tuple_type_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_varray_type_specifier
          varray_keyword
          varray_left_angle
          varray_type
          varray_trailing_comma
          varray_right_angle =
        let syntax =
          VarrayTypeSpecifier
            {
              varray_keyword;
              varray_left_angle;
              varray_type;
              varray_trailing_comma;
              varray_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_ctx_type_specifier
          function_ctx_type_keyword function_ctx_type_variable =
        let syntax =
          FunctionCtxTypeSpecifier
            { function_ctx_type_keyword; function_ctx_type_variable }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_parameter
          type_attribute_spec
          type_reified
          type_variance
          type_name
          type_param_params
          type_constraints =
        let syntax =
          TypeParameter
            {
              type_attribute_spec;
              type_reified;
              type_variance;
              type_name;
              type_param_params;
              type_constraints;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_constraint constraint_keyword constraint_type =
        let syntax = TypeConstraint { constraint_keyword; constraint_type } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_context_constraint ctx_constraint_keyword ctx_constraint_ctx_list
          =
        let syntax =
          ContextConstraint { ctx_constraint_keyword; ctx_constraint_ctx_list }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_darray_type_specifier
          darray_keyword
          darray_left_angle
          darray_key
          darray_comma
          darray_value
          darray_trailing_comma
          darray_right_angle =
        let syntax =
          DarrayTypeSpecifier
            {
              darray_keyword;
              darray_left_angle;
              darray_key;
              darray_comma;
              darray_value;
              darray_trailing_comma;
              darray_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_dictionary_type_specifier
          dictionary_type_keyword
          dictionary_type_left_angle
          dictionary_type_members
          dictionary_type_right_angle =
        let syntax =
          DictionaryTypeSpecifier
            {
              dictionary_type_keyword;
              dictionary_type_left_angle;
              dictionary_type_members;
              dictionary_type_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_closure_type_specifier
          closure_outer_left_paren
          closure_readonly_keyword
          closure_function_keyword
          closure_inner_left_paren
          closure_parameter_list
          closure_inner_right_paren
          closure_contexts
          closure_colon
          closure_readonly_return
          closure_return_type
          closure_outer_right_paren =
        let syntax =
          ClosureTypeSpecifier
            {
              closure_outer_left_paren;
              closure_readonly_keyword;
              closure_function_keyword;
              closure_inner_left_paren;
              closure_parameter_list;
              closure_inner_right_paren;
              closure_contexts;
              closure_colon;
              closure_readonly_return;
              closure_return_type;
              closure_outer_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_closure_parameter_type_specifier
          closure_parameter_call_convention
          closure_parameter_readonly
          closure_parameter_type =
        let syntax =
          ClosureParameterTypeSpecifier
            {
              closure_parameter_call_convention;
              closure_parameter_readonly;
              closure_parameter_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_refinement
          type_refinement_type
          type_refinement_keyword
          type_refinement_left_brace
          type_refinement_members
          type_refinement_right_brace =
        let syntax =
          TypeRefinement
            {
              type_refinement_type;
              type_refinement_keyword;
              type_refinement_left_brace;
              type_refinement_members;
              type_refinement_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_in_refinement
          type_in_refinement_keyword
          type_in_refinement_name
          type_in_refinement_type_parameters
          type_in_refinement_constraints
          type_in_refinement_equal
          type_in_refinement_type =
        let syntax =
          TypeInRefinement
            {
              type_in_refinement_keyword;
              type_in_refinement_name;
              type_in_refinement_type_parameters;
              type_in_refinement_constraints;
              type_in_refinement_equal;
              type_in_refinement_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_ctx_in_refinement
          ctx_in_refinement_keyword
          ctx_in_refinement_name
          ctx_in_refinement_type_parameters
          ctx_in_refinement_constraints
          ctx_in_refinement_equal
          ctx_in_refinement_ctx_list =
        let syntax =
          CtxInRefinement
            {
              ctx_in_refinement_keyword;
              ctx_in_refinement_name;
              ctx_in_refinement_type_parameters;
              ctx_in_refinement_constraints;
              ctx_in_refinement_equal;
              ctx_in_refinement_ctx_list;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classname_type_specifier
          classname_keyword
          classname_left_angle
          classname_type
          classname_trailing_comma
          classname_right_angle =
        let syntax =
          ClassnameTypeSpecifier
            {
              classname_keyword;
              classname_left_angle;
              classname_type;
              classname_trailing_comma;
              classname_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_field_specifier field_question field_name field_arrow field_type
          =
        let syntax =
          FieldSpecifier { field_question; field_name; field_arrow; field_type }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_field_initializer
          field_initializer_name field_initializer_arrow field_initializer_value
          =
        let syntax =
          FieldInitializer
            {
              field_initializer_name;
              field_initializer_arrow;
              field_initializer_value;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_shape_type_specifier
          shape_type_keyword
          shape_type_left_paren
          shape_type_fields
          shape_type_ellipsis
          shape_type_right_paren =
        let syntax =
          ShapeTypeSpecifier
            {
              shape_type_keyword;
              shape_type_left_paren;
              shape_type_fields;
              shape_type_ellipsis;
              shape_type_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_shape_expression
          shape_expression_keyword
          shape_expression_left_paren
          shape_expression_fields
          shape_expression_right_paren =
        let syntax =
          ShapeExpression
            {
              shape_expression_keyword;
              shape_expression_left_paren;
              shape_expression_fields;
              shape_expression_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_expression
          tuple_expression_keyword
          tuple_expression_left_paren
          tuple_expression_items
          tuple_expression_right_paren =
        let syntax =
          TupleExpression
            {
              tuple_expression_keyword;
              tuple_expression_left_paren;
              tuple_expression_items;
              tuple_expression_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_generic_type_specifier generic_class_type generic_argument_list =
        let syntax =
          GenericTypeSpecifier { generic_class_type; generic_argument_list }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_nullable_type_specifier nullable_question nullable_type =
        let syntax =
          NullableTypeSpecifier { nullable_question; nullable_type }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_like_type_specifier like_tilde like_type =
        let syntax = LikeTypeSpecifier { like_tilde; like_type } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_soft_type_specifier soft_at soft_type =
        let syntax = SoftTypeSpecifier { soft_at; soft_type } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_attributized_specifier
          attributized_specifier_attribute_spec attributized_specifier_type =
        let syntax =
          AttributizedSpecifier
            {
              attributized_specifier_attribute_spec;
              attributized_specifier_type;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_reified_type_argument
          reified_type_argument_reified reified_type_argument_type =
        let syntax =
          ReifiedTypeArgument
            { reified_type_argument_reified; reified_type_argument_type }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_arguments
          type_arguments_left_angle
          type_arguments_types
          type_arguments_right_angle =
        let syntax =
          TypeArguments
            {
              type_arguments_left_angle;
              type_arguments_types;
              type_arguments_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_parameters
          type_parameters_left_angle
          type_parameters_parameters
          type_parameters_right_angle =
        let syntax =
          TypeParameters
            {
              type_parameters_left_angle;
              type_parameters_parameters;
              type_parameters_right_angle;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_type_specifier
          tuple_left_paren tuple_types tuple_right_paren =
        let syntax =
          TupleTypeSpecifier
            { tuple_left_paren; tuple_types; tuple_right_paren }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_union_type_specifier
          union_left_paren union_types union_right_paren =
        let syntax =
          UnionTypeSpecifier
            { union_left_paren; union_types; union_right_paren }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_intersection_type_specifier
          intersection_left_paren intersection_types intersection_right_paren =
        let syntax =
          IntersectionTypeSpecifier
            {
              intersection_left_paren;
              intersection_types;
              intersection_right_paren;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_error error_error =
        let syntax = ErrorSyntax { error_error } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_list_item list_item list_separator =
        let syntax = ListItem { list_item; list_separator } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_class_label_expression
          enum_class_label_qualifier
          enum_class_label_hash
          enum_class_label_expression =
        let syntax =
          EnumClassLabelExpression
            {
              enum_class_label_qualifier;
              enum_class_label_hash;
              enum_class_label_expression;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_module_declaration
          module_declaration_attribute_spec
          module_declaration_new_keyword
          module_declaration_module_keyword
          module_declaration_name
          module_declaration_left_brace
          module_declaration_exports
          module_declaration_imports
          module_declaration_right_brace =
        let syntax =
          ModuleDeclaration
            {
              module_declaration_attribute_spec;
              module_declaration_new_keyword;
              module_declaration_module_keyword;
              module_declaration_name;
              module_declaration_left_brace;
              module_declaration_exports;
              module_declaration_imports;
              module_declaration_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_module_exports
          module_exports_exports_keyword
          module_exports_left_brace
          module_exports_exports
          module_exports_right_brace =
        let syntax =
          ModuleExports
            {
              module_exports_exports_keyword;
              module_exports_left_brace;
              module_exports_exports;
              module_exports_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_module_imports
          module_imports_imports_keyword
          module_imports_left_brace
          module_imports_imports
          module_imports_right_brace =
        let syntax =
          ModuleImports
            {
              module_imports_imports_keyword;
              module_imports_left_brace;
              module_imports_imports;
              module_imports_right_brace;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_module_membership_declaration
          module_membership_declaration_module_keyword
          module_membership_declaration_name
          module_membership_declaration_semicolon =
        let syntax =
          ModuleMembershipDeclaration
            {
              module_membership_declaration_module_keyword;
              module_membership_declaration_name;
              module_membership_declaration_semicolon;
            }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_package_expression
          package_expression_keyword package_expression_name =
        let syntax =
          PackageExpression
            { package_expression_keyword; package_expression_name }
        in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let from_function_declaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          } =
        FunctionDeclaration
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          }

      let from_function_declaration_header
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          } =
        FunctionDeclarationHeader
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          }

      let from_methodish_declaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          } =
        MethodishDeclaration
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          }

      let from_anonymous_function
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          } =
        AnonymousFunction
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          }

      let from_lambda_expression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          } =
        LambdaExpression
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          }

      let from_lambda_signature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          } =
        LambdaSignature
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          }

      let from_closure_type_specifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          } =
        ClosureTypeSpecifier
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          }

      let get_function_declaration x =
        match x with
        | FunctionDeclaration
            {
              function_attribute_spec;
              function_declaration_header;
              function_body;
            } ->
          {
            function_attribute_spec;
            function_declaration_header;
            function_body;
          }
        | _ -> failwith "get_function_declaration: not a FunctionDeclaration"

      let get_function_declaration_header x =
        match x with
        | FunctionDeclarationHeader
            {
              function_modifiers;
              function_keyword;
              function_name;
              function_type_parameter_list;
              function_left_paren;
              function_parameter_list;
              function_right_paren;
              function_contexts;
              function_colon;
              function_readonly_return;
              function_type;
              function_where_clause;
            } ->
          {
            function_modifiers;
            function_keyword;
            function_name;
            function_type_parameter_list;
            function_left_paren;
            function_parameter_list;
            function_right_paren;
            function_contexts;
            function_colon;
            function_readonly_return;
            function_type;
            function_where_clause;
          }
        | _ ->
          failwith
            "get_function_declaration_header: not a FunctionDeclarationHeader"

      let get_methodish_declaration x =
        match x with
        | MethodishDeclaration
            {
              methodish_attribute;
              methodish_function_decl_header;
              methodish_function_body;
              methodish_semicolon;
            } ->
          {
            methodish_attribute;
            methodish_function_decl_header;
            methodish_function_body;
            methodish_semicolon;
          }
        | _ -> failwith "get_methodish_declaration: not a MethodishDeclaration"

      let get_anonymous_function x =
        match x with
        | AnonymousFunction
            {
              anonymous_attribute_spec;
              anonymous_async_keyword;
              anonymous_function_keyword;
              anonymous_left_paren;
              anonymous_parameters;
              anonymous_right_paren;
              anonymous_ctx_list;
              anonymous_colon;
              anonymous_readonly_return;
              anonymous_type;
              anonymous_use;
              anonymous_body;
            } ->
          {
            anonymous_attribute_spec;
            anonymous_async_keyword;
            anonymous_function_keyword;
            anonymous_left_paren;
            anonymous_parameters;
            anonymous_right_paren;
            anonymous_ctx_list;
            anonymous_colon;
            anonymous_readonly_return;
            anonymous_type;
            anonymous_use;
            anonymous_body;
          }
        | _ -> failwith "get_anonymous_function: not a AnonymousFunction"

      let get_lambda_expression x =
        match x with
        | LambdaExpression
            {
              lambda_attribute_spec;
              lambda_async;
              lambda_signature;
              lambda_arrow;
              lambda_body;
            } ->
          {
            lambda_attribute_spec;
            lambda_async;
            lambda_signature;
            lambda_arrow;
            lambda_body;
          }
        | _ -> failwith "get_lambda_expression: not a LambdaExpression"

      let get_lambda_signature x =
        match x with
        | LambdaSignature
            {
              lambda_left_paren;
              lambda_parameters;
              lambda_right_paren;
              lambda_contexts;
              lambda_colon;
              lambda_readonly_return;
              lambda_type;
            } ->
          {
            lambda_left_paren;
            lambda_parameters;
            lambda_right_paren;
            lambda_contexts;
            lambda_colon;
            lambda_readonly_return;
            lambda_type;
          }
        | _ -> failwith "get_lambda_signature: not a LambdaSignature"

      let get_closure_type_specifier x =
        match x with
        | ClosureTypeSpecifier
            {
              closure_outer_left_paren;
              closure_readonly_keyword;
              closure_function_keyword;
              closure_inner_left_paren;
              closure_parameter_list;
              closure_inner_right_paren;
              closure_contexts;
              closure_colon;
              closure_readonly_return;
              closure_return_type;
              closure_outer_right_paren;
            } ->
          {
            closure_outer_left_paren;
            closure_readonly_keyword;
            closure_function_keyword;
            closure_inner_left_paren;
            closure_parameter_list;
            closure_inner_right_paren;
            closure_contexts;
            closure_colon;
            closure_readonly_return;
            closure_return_type;
            closure_outer_right_paren;
          }
        | _ -> failwith "get_closure_type_specifier: not a ClosureTypeSpecifier"
    end
  end
end
