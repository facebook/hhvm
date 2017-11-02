(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
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

open Full_fidelity_syntax_type
module SyntaxKind = Full_fidelity_syntax_kind
module Operator = Full_fidelity_operator

module WithToken(Token: TokenType) = struct
  module WithSyntaxValue(SyntaxValue: SyntaxValueType) = struct

    include MakeSyntaxType(Token)(SyntaxValue)

    let make syntax value =
      { syntax; value }

    let syntax node =
      node.syntax

    let value node =
      node.value

    let syntax_node_to_list node =
      match syntax node with
      | SyntaxList x -> x
      | Missing -> []
      | _ -> [node]

    let to_kind syntax =
      match syntax with
      | Missing                             -> SyntaxKind.Missing
      | Token                             _ -> SyntaxKind.Token
      | SyntaxList                        _ -> SyntaxKind.SyntaxList
      | EndOfFile                               _ -> SyntaxKind.EndOfFile
      | Script                                  _ -> SyntaxKind.Script
      | SimpleTypeSpecifier                     _ -> SyntaxKind.SimpleTypeSpecifier
      | LiteralExpression                       _ -> SyntaxKind.LiteralExpression
      | VariableExpression                      _ -> SyntaxKind.VariableExpression
      | QualifiedNameExpression                 _ -> SyntaxKind.QualifiedNameExpression
      | PipeVariableExpression                  _ -> SyntaxKind.PipeVariableExpression
      | EnumDeclaration                         _ -> SyntaxKind.EnumDeclaration
      | Enumerator                              _ -> SyntaxKind.Enumerator
      | AliasDeclaration                        _ -> SyntaxKind.AliasDeclaration
      | PropertyDeclaration                     _ -> SyntaxKind.PropertyDeclaration
      | PropertyDeclarator                      _ -> SyntaxKind.PropertyDeclarator
      | NamespaceDeclaration                    _ -> SyntaxKind.NamespaceDeclaration
      | NamespaceBody                           _ -> SyntaxKind.NamespaceBody
      | NamespaceEmptyBody                      _ -> SyntaxKind.NamespaceEmptyBody
      | NamespaceUseDeclaration                 _ -> SyntaxKind.NamespaceUseDeclaration
      | NamespaceGroupUseDeclaration            _ -> SyntaxKind.NamespaceGroupUseDeclaration
      | NamespaceUseClause                      _ -> SyntaxKind.NamespaceUseClause
      | FunctionDeclaration                     _ -> SyntaxKind.FunctionDeclaration
      | FunctionDeclarationHeader               _ -> SyntaxKind.FunctionDeclarationHeader
      | WhereClause                             _ -> SyntaxKind.WhereClause
      | WhereConstraint                         _ -> SyntaxKind.WhereConstraint
      | MethodishDeclaration                    _ -> SyntaxKind.MethodishDeclaration
      | ClassishDeclaration                     _ -> SyntaxKind.ClassishDeclaration
      | ClassishBody                            _ -> SyntaxKind.ClassishBody
      | TraitUsePrecedenceItem                  _ -> SyntaxKind.TraitUsePrecedenceItem
      | TraitUseAliasItem                       _ -> SyntaxKind.TraitUseAliasItem
      | TraitUseConflictResolution              _ -> SyntaxKind.TraitUseConflictResolution
      | TraitUse                                _ -> SyntaxKind.TraitUse
      | RequireClause                           _ -> SyntaxKind.RequireClause
      | ConstDeclaration                        _ -> SyntaxKind.ConstDeclaration
      | ConstantDeclarator                      _ -> SyntaxKind.ConstantDeclarator
      | TypeConstDeclaration                    _ -> SyntaxKind.TypeConstDeclaration
      | DecoratedExpression                     _ -> SyntaxKind.DecoratedExpression
      | ParameterDeclaration                    _ -> SyntaxKind.ParameterDeclaration
      | VariadicParameter                       _ -> SyntaxKind.VariadicParameter
      | AttributeSpecification                  _ -> SyntaxKind.AttributeSpecification
      | Attribute                               _ -> SyntaxKind.Attribute
      | InclusionExpression                     _ -> SyntaxKind.InclusionExpression
      | InclusionDirective                      _ -> SyntaxKind.InclusionDirective
      | CompoundStatement                       _ -> SyntaxKind.CompoundStatement
      | ExpressionStatement                     _ -> SyntaxKind.ExpressionStatement
      | MarkupSection                           _ -> SyntaxKind.MarkupSection
      | MarkupSuffix                            _ -> SyntaxKind.MarkupSuffix
      | UnsetStatement                          _ -> SyntaxKind.UnsetStatement
      | UsingStatementBlockScoped               _ -> SyntaxKind.UsingStatementBlockScoped
      | UsingStatementFunctionScoped            _ -> SyntaxKind.UsingStatementFunctionScoped
      | WhileStatement                          _ -> SyntaxKind.WhileStatement
      | IfStatement                             _ -> SyntaxKind.IfStatement
      | ElseifClause                            _ -> SyntaxKind.ElseifClause
      | ElseClause                              _ -> SyntaxKind.ElseClause
      | IfEndIfStatement                        _ -> SyntaxKind.IfEndIfStatement
      | ElseifColonClause                       _ -> SyntaxKind.ElseifColonClause
      | ElseColonClause                         _ -> SyntaxKind.ElseColonClause
      | TryStatement                            _ -> SyntaxKind.TryStatement
      | CatchClause                             _ -> SyntaxKind.CatchClause
      | FinallyClause                           _ -> SyntaxKind.FinallyClause
      | DoStatement                             _ -> SyntaxKind.DoStatement
      | ForStatement                            _ -> SyntaxKind.ForStatement
      | ForeachStatement                        _ -> SyntaxKind.ForeachStatement
      | SwitchStatement                         _ -> SyntaxKind.SwitchStatement
      | SwitchSection                           _ -> SyntaxKind.SwitchSection
      | SwitchFallthrough                       _ -> SyntaxKind.SwitchFallthrough
      | CaseLabel                               _ -> SyntaxKind.CaseLabel
      | DefaultLabel                            _ -> SyntaxKind.DefaultLabel
      | ReturnStatement                         _ -> SyntaxKind.ReturnStatement
      | GotoLabel                               _ -> SyntaxKind.GotoLabel
      | GotoStatement                           _ -> SyntaxKind.GotoStatement
      | ThrowStatement                          _ -> SyntaxKind.ThrowStatement
      | BreakStatement                          _ -> SyntaxKind.BreakStatement
      | ContinueStatement                       _ -> SyntaxKind.ContinueStatement
      | FunctionStaticStatement                 _ -> SyntaxKind.FunctionStaticStatement
      | StaticDeclarator                        _ -> SyntaxKind.StaticDeclarator
      | EchoStatement                           _ -> SyntaxKind.EchoStatement
      | GlobalStatement                         _ -> SyntaxKind.GlobalStatement
      | SimpleInitializer                       _ -> SyntaxKind.SimpleInitializer
      | AnonymousFunction                       _ -> SyntaxKind.AnonymousFunction
      | AnonymousFunctionUseClause              _ -> SyntaxKind.AnonymousFunctionUseClause
      | LambdaExpression                        _ -> SyntaxKind.LambdaExpression
      | LambdaSignature                         _ -> SyntaxKind.LambdaSignature
      | CastExpression                          _ -> SyntaxKind.CastExpression
      | ScopeResolutionExpression               _ -> SyntaxKind.ScopeResolutionExpression
      | MemberSelectionExpression               _ -> SyntaxKind.MemberSelectionExpression
      | SafeMemberSelectionExpression           _ -> SyntaxKind.SafeMemberSelectionExpression
      | EmbeddedMemberSelectionExpression       _ -> SyntaxKind.EmbeddedMemberSelectionExpression
      | YieldExpression                         _ -> SyntaxKind.YieldExpression
      | YieldFromExpression                     _ -> SyntaxKind.YieldFromExpression
      | PrefixUnaryExpression                   _ -> SyntaxKind.PrefixUnaryExpression
      | PostfixUnaryExpression                  _ -> SyntaxKind.PostfixUnaryExpression
      | BinaryExpression                        _ -> SyntaxKind.BinaryExpression
      | InstanceofExpression                    _ -> SyntaxKind.InstanceofExpression
      | IsExpression                            _ -> SyntaxKind.IsExpression
      | ConditionalExpression                   _ -> SyntaxKind.ConditionalExpression
      | EvalExpression                          _ -> SyntaxKind.EvalExpression
      | EmptyExpression                         _ -> SyntaxKind.EmptyExpression
      | DefineExpression                        _ -> SyntaxKind.DefineExpression
      | IssetExpression                         _ -> SyntaxKind.IssetExpression
      | FunctionCallExpression                  _ -> SyntaxKind.FunctionCallExpression
      | FunctionCallWithTypeArgumentsExpression _ -> SyntaxKind.FunctionCallWithTypeArgumentsExpression
      | ParenthesizedExpression                 _ -> SyntaxKind.ParenthesizedExpression
      | BracedExpression                        _ -> SyntaxKind.BracedExpression
      | EmbeddedBracedExpression                _ -> SyntaxKind.EmbeddedBracedExpression
      | ListExpression                          _ -> SyntaxKind.ListExpression
      | CollectionLiteralExpression             _ -> SyntaxKind.CollectionLiteralExpression
      | ObjectCreationExpression                _ -> SyntaxKind.ObjectCreationExpression
      | ArrayCreationExpression                 _ -> SyntaxKind.ArrayCreationExpression
      | ArrayIntrinsicExpression                _ -> SyntaxKind.ArrayIntrinsicExpression
      | DarrayIntrinsicExpression               _ -> SyntaxKind.DarrayIntrinsicExpression
      | DictionaryIntrinsicExpression           _ -> SyntaxKind.DictionaryIntrinsicExpression
      | KeysetIntrinsicExpression               _ -> SyntaxKind.KeysetIntrinsicExpression
      | VarrayIntrinsicExpression               _ -> SyntaxKind.VarrayIntrinsicExpression
      | VectorIntrinsicExpression               _ -> SyntaxKind.VectorIntrinsicExpression
      | ElementInitializer                      _ -> SyntaxKind.ElementInitializer
      | SubscriptExpression                     _ -> SyntaxKind.SubscriptExpression
      | EmbeddedSubscriptExpression             _ -> SyntaxKind.EmbeddedSubscriptExpression
      | AwaitableCreationExpression             _ -> SyntaxKind.AwaitableCreationExpression
      | XHPChildrenDeclaration                  _ -> SyntaxKind.XHPChildrenDeclaration
      | XHPChildrenParenthesizedList            _ -> SyntaxKind.XHPChildrenParenthesizedList
      | XHPCategoryDeclaration                  _ -> SyntaxKind.XHPCategoryDeclaration
      | XHPEnumType                             _ -> SyntaxKind.XHPEnumType
      | XHPRequired                             _ -> SyntaxKind.XHPRequired
      | XHPClassAttributeDeclaration            _ -> SyntaxKind.XHPClassAttributeDeclaration
      | XHPClassAttribute                       _ -> SyntaxKind.XHPClassAttribute
      | XHPSimpleClassAttribute                 _ -> SyntaxKind.XHPSimpleClassAttribute
      | XHPAttribute                            _ -> SyntaxKind.XHPAttribute
      | XHPOpen                                 _ -> SyntaxKind.XHPOpen
      | XHPExpression                           _ -> SyntaxKind.XHPExpression
      | XHPClose                                _ -> SyntaxKind.XHPClose
      | TypeConstant                            _ -> SyntaxKind.TypeConstant
      | VectorTypeSpecifier                     _ -> SyntaxKind.VectorTypeSpecifier
      | KeysetTypeSpecifier                     _ -> SyntaxKind.KeysetTypeSpecifier
      | TupleTypeExplicitSpecifier              _ -> SyntaxKind.TupleTypeExplicitSpecifier
      | VarrayTypeSpecifier                     _ -> SyntaxKind.VarrayTypeSpecifier
      | VectorArrayTypeSpecifier                _ -> SyntaxKind.VectorArrayTypeSpecifier
      | TypeParameter                           _ -> SyntaxKind.TypeParameter
      | TypeConstraint                          _ -> SyntaxKind.TypeConstraint
      | DarrayTypeSpecifier                     _ -> SyntaxKind.DarrayTypeSpecifier
      | MapArrayTypeSpecifier                   _ -> SyntaxKind.MapArrayTypeSpecifier
      | DictionaryTypeSpecifier                 _ -> SyntaxKind.DictionaryTypeSpecifier
      | ClosureTypeSpecifier                    _ -> SyntaxKind.ClosureTypeSpecifier
      | ClassnameTypeSpecifier                  _ -> SyntaxKind.ClassnameTypeSpecifier
      | FieldSpecifier                          _ -> SyntaxKind.FieldSpecifier
      | FieldInitializer                        _ -> SyntaxKind.FieldInitializer
      | ShapeTypeSpecifier                      _ -> SyntaxKind.ShapeTypeSpecifier
      | ShapeExpression                         _ -> SyntaxKind.ShapeExpression
      | TupleExpression                         _ -> SyntaxKind.TupleExpression
      | GenericTypeSpecifier                    _ -> SyntaxKind.GenericTypeSpecifier
      | NullableTypeSpecifier                   _ -> SyntaxKind.NullableTypeSpecifier
      | SoftTypeSpecifier                       _ -> SyntaxKind.SoftTypeSpecifier
      | TypeArguments                           _ -> SyntaxKind.TypeArguments
      | TypeParameters                          _ -> SyntaxKind.TypeParameters
      | TupleTypeSpecifier                      _ -> SyntaxKind.TupleTypeSpecifier
      | ErrorSyntax                             _ -> SyntaxKind.ErrorSyntax
      | ListItem                                _ -> SyntaxKind.ListItem


    let kind node =
      to_kind (syntax node)

    let has_kind syntax_kind node =
      kind node = syntax_kind

    let is_missing node =
      kind node = SyntaxKind.Missing

    let is_list node =
      kind node = SyntaxKind.SyntaxList

    let is_end_of_file                                  = has_kind SyntaxKind.EndOfFile
    let is_script                                       = has_kind SyntaxKind.Script
    let is_simple_type_specifier                        = has_kind SyntaxKind.SimpleTypeSpecifier
    let is_literal_expression                           = has_kind SyntaxKind.LiteralExpression
    let is_variable_expression                          = has_kind SyntaxKind.VariableExpression
    let is_qualified_name_expression                    = has_kind SyntaxKind.QualifiedNameExpression
    let is_pipe_variable_expression                     = has_kind SyntaxKind.PipeVariableExpression
    let is_enum_declaration                             = has_kind SyntaxKind.EnumDeclaration
    let is_enumerator                                   = has_kind SyntaxKind.Enumerator
    let is_alias_declaration                            = has_kind SyntaxKind.AliasDeclaration
    let is_property_declaration                         = has_kind SyntaxKind.PropertyDeclaration
    let is_property_declarator                          = has_kind SyntaxKind.PropertyDeclarator
    let is_namespace_declaration                        = has_kind SyntaxKind.NamespaceDeclaration
    let is_namespace_body                               = has_kind SyntaxKind.NamespaceBody
    let is_namespace_empty_body                         = has_kind SyntaxKind.NamespaceEmptyBody
    let is_namespace_use_declaration                    = has_kind SyntaxKind.NamespaceUseDeclaration
    let is_namespace_group_use_declaration              = has_kind SyntaxKind.NamespaceGroupUseDeclaration
    let is_namespace_use_clause                         = has_kind SyntaxKind.NamespaceUseClause
    let is_function_declaration                         = has_kind SyntaxKind.FunctionDeclaration
    let is_function_declaration_header                  = has_kind SyntaxKind.FunctionDeclarationHeader
    let is_where_clause                                 = has_kind SyntaxKind.WhereClause
    let is_where_constraint                             = has_kind SyntaxKind.WhereConstraint
    let is_methodish_declaration                        = has_kind SyntaxKind.MethodishDeclaration
    let is_classish_declaration                         = has_kind SyntaxKind.ClassishDeclaration
    let is_classish_body                                = has_kind SyntaxKind.ClassishBody
    let is_trait_use_precedence_item                    = has_kind SyntaxKind.TraitUsePrecedenceItem
    let is_trait_use_alias_item                         = has_kind SyntaxKind.TraitUseAliasItem
    let is_trait_use_conflict_resolution                = has_kind SyntaxKind.TraitUseConflictResolution
    let is_trait_use                                    = has_kind SyntaxKind.TraitUse
    let is_require_clause                               = has_kind SyntaxKind.RequireClause
    let is_const_declaration                            = has_kind SyntaxKind.ConstDeclaration
    let is_constant_declarator                          = has_kind SyntaxKind.ConstantDeclarator
    let is_type_const_declaration                       = has_kind SyntaxKind.TypeConstDeclaration
    let is_decorated_expression                         = has_kind SyntaxKind.DecoratedExpression
    let is_parameter_declaration                        = has_kind SyntaxKind.ParameterDeclaration
    let is_variadic_parameter                           = has_kind SyntaxKind.VariadicParameter
    let is_attribute_specification                      = has_kind SyntaxKind.AttributeSpecification
    let is_attribute                                    = has_kind SyntaxKind.Attribute
    let is_inclusion_expression                         = has_kind SyntaxKind.InclusionExpression
    let is_inclusion_directive                          = has_kind SyntaxKind.InclusionDirective
    let is_compound_statement                           = has_kind SyntaxKind.CompoundStatement
    let is_expression_statement                         = has_kind SyntaxKind.ExpressionStatement
    let is_markup_section                               = has_kind SyntaxKind.MarkupSection
    let is_markup_suffix                                = has_kind SyntaxKind.MarkupSuffix
    let is_unset_statement                              = has_kind SyntaxKind.UnsetStatement
    let is_using_statement_block_scoped                 = has_kind SyntaxKind.UsingStatementBlockScoped
    let is_using_statement_function_scoped              = has_kind SyntaxKind.UsingStatementFunctionScoped
    let is_while_statement                              = has_kind SyntaxKind.WhileStatement
    let is_if_statement                                 = has_kind SyntaxKind.IfStatement
    let is_elseif_clause                                = has_kind SyntaxKind.ElseifClause
    let is_else_clause                                  = has_kind SyntaxKind.ElseClause
    let is_if_endif_statement                           = has_kind SyntaxKind.IfEndIfStatement
    let is_elseif_colon_clause                          = has_kind SyntaxKind.ElseifColonClause
    let is_else_colon_clause                            = has_kind SyntaxKind.ElseColonClause
    let is_try_statement                                = has_kind SyntaxKind.TryStatement
    let is_catch_clause                                 = has_kind SyntaxKind.CatchClause
    let is_finally_clause                               = has_kind SyntaxKind.FinallyClause
    let is_do_statement                                 = has_kind SyntaxKind.DoStatement
    let is_for_statement                                = has_kind SyntaxKind.ForStatement
    let is_foreach_statement                            = has_kind SyntaxKind.ForeachStatement
    let is_switch_statement                             = has_kind SyntaxKind.SwitchStatement
    let is_switch_section                               = has_kind SyntaxKind.SwitchSection
    let is_switch_fallthrough                           = has_kind SyntaxKind.SwitchFallthrough
    let is_case_label                                   = has_kind SyntaxKind.CaseLabel
    let is_default_label                                = has_kind SyntaxKind.DefaultLabel
    let is_return_statement                             = has_kind SyntaxKind.ReturnStatement
    let is_goto_label                                   = has_kind SyntaxKind.GotoLabel
    let is_goto_statement                               = has_kind SyntaxKind.GotoStatement
    let is_throw_statement                              = has_kind SyntaxKind.ThrowStatement
    let is_break_statement                              = has_kind SyntaxKind.BreakStatement
    let is_continue_statement                           = has_kind SyntaxKind.ContinueStatement
    let is_function_static_statement                    = has_kind SyntaxKind.FunctionStaticStatement
    let is_static_declarator                            = has_kind SyntaxKind.StaticDeclarator
    let is_echo_statement                               = has_kind SyntaxKind.EchoStatement
    let is_global_statement                             = has_kind SyntaxKind.GlobalStatement
    let is_simple_initializer                           = has_kind SyntaxKind.SimpleInitializer
    let is_anonymous_function                           = has_kind SyntaxKind.AnonymousFunction
    let is_anonymous_function_use_clause                = has_kind SyntaxKind.AnonymousFunctionUseClause
    let is_lambda_expression                            = has_kind SyntaxKind.LambdaExpression
    let is_lambda_signature                             = has_kind SyntaxKind.LambdaSignature
    let is_cast_expression                              = has_kind SyntaxKind.CastExpression
    let is_scope_resolution_expression                  = has_kind SyntaxKind.ScopeResolutionExpression
    let is_member_selection_expression                  = has_kind SyntaxKind.MemberSelectionExpression
    let is_safe_member_selection_expression             = has_kind SyntaxKind.SafeMemberSelectionExpression
    let is_embedded_member_selection_expression         = has_kind SyntaxKind.EmbeddedMemberSelectionExpression
    let is_yield_expression                             = has_kind SyntaxKind.YieldExpression
    let is_yield_from_expression                        = has_kind SyntaxKind.YieldFromExpression
    let is_prefix_unary_expression                      = has_kind SyntaxKind.PrefixUnaryExpression
    let is_postfix_unary_expression                     = has_kind SyntaxKind.PostfixUnaryExpression
    let is_binary_expression                            = has_kind SyntaxKind.BinaryExpression
    let is_instanceof_expression                        = has_kind SyntaxKind.InstanceofExpression
    let is_is_expression                                = has_kind SyntaxKind.IsExpression
    let is_conditional_expression                       = has_kind SyntaxKind.ConditionalExpression
    let is_eval_expression                              = has_kind SyntaxKind.EvalExpression
    let is_empty_expression                             = has_kind SyntaxKind.EmptyExpression
    let is_define_expression                            = has_kind SyntaxKind.DefineExpression
    let is_isset_expression                             = has_kind SyntaxKind.IssetExpression
    let is_function_call_expression                     = has_kind SyntaxKind.FunctionCallExpression
    let is_function_call_with_type_arguments_expression = has_kind SyntaxKind.FunctionCallWithTypeArgumentsExpression
    let is_parenthesized_expression                     = has_kind SyntaxKind.ParenthesizedExpression
    let is_braced_expression                            = has_kind SyntaxKind.BracedExpression
    let is_embedded_braced_expression                   = has_kind SyntaxKind.EmbeddedBracedExpression
    let is_list_expression                              = has_kind SyntaxKind.ListExpression
    let is_collection_literal_expression                = has_kind SyntaxKind.CollectionLiteralExpression
    let is_object_creation_expression                   = has_kind SyntaxKind.ObjectCreationExpression
    let is_array_creation_expression                    = has_kind SyntaxKind.ArrayCreationExpression
    let is_array_intrinsic_expression                   = has_kind SyntaxKind.ArrayIntrinsicExpression
    let is_darray_intrinsic_expression                  = has_kind SyntaxKind.DarrayIntrinsicExpression
    let is_dictionary_intrinsic_expression              = has_kind SyntaxKind.DictionaryIntrinsicExpression
    let is_keyset_intrinsic_expression                  = has_kind SyntaxKind.KeysetIntrinsicExpression
    let is_varray_intrinsic_expression                  = has_kind SyntaxKind.VarrayIntrinsicExpression
    let is_vector_intrinsic_expression                  = has_kind SyntaxKind.VectorIntrinsicExpression
    let is_element_initializer                          = has_kind SyntaxKind.ElementInitializer
    let is_subscript_expression                         = has_kind SyntaxKind.SubscriptExpression
    let is_embedded_subscript_expression                = has_kind SyntaxKind.EmbeddedSubscriptExpression
    let is_awaitable_creation_expression                = has_kind SyntaxKind.AwaitableCreationExpression
    let is_xhp_children_declaration                     = has_kind SyntaxKind.XHPChildrenDeclaration
    let is_xhp_children_parenthesized_list              = has_kind SyntaxKind.XHPChildrenParenthesizedList
    let is_xhp_category_declaration                     = has_kind SyntaxKind.XHPCategoryDeclaration
    let is_xhp_enum_type                                = has_kind SyntaxKind.XHPEnumType
    let is_xhp_required                                 = has_kind SyntaxKind.XHPRequired
    let is_xhp_class_attribute_declaration              = has_kind SyntaxKind.XHPClassAttributeDeclaration
    let is_xhp_class_attribute                          = has_kind SyntaxKind.XHPClassAttribute
    let is_xhp_simple_class_attribute                   = has_kind SyntaxKind.XHPSimpleClassAttribute
    let is_xhp_attribute                                = has_kind SyntaxKind.XHPAttribute
    let is_xhp_open                                     = has_kind SyntaxKind.XHPOpen
    let is_xhp_expression                               = has_kind SyntaxKind.XHPExpression
    let is_xhp_close                                    = has_kind SyntaxKind.XHPClose
    let is_type_constant                                = has_kind SyntaxKind.TypeConstant
    let is_vector_type_specifier                        = has_kind SyntaxKind.VectorTypeSpecifier
    let is_keyset_type_specifier                        = has_kind SyntaxKind.KeysetTypeSpecifier
    let is_tuple_type_explicit_specifier                = has_kind SyntaxKind.TupleTypeExplicitSpecifier
    let is_varray_type_specifier                        = has_kind SyntaxKind.VarrayTypeSpecifier
    let is_vector_array_type_specifier                  = has_kind SyntaxKind.VectorArrayTypeSpecifier
    let is_type_parameter                               = has_kind SyntaxKind.TypeParameter
    let is_type_constraint                              = has_kind SyntaxKind.TypeConstraint
    let is_darray_type_specifier                        = has_kind SyntaxKind.DarrayTypeSpecifier
    let is_map_array_type_specifier                     = has_kind SyntaxKind.MapArrayTypeSpecifier
    let is_dictionary_type_specifier                    = has_kind SyntaxKind.DictionaryTypeSpecifier
    let is_closure_type_specifier                       = has_kind SyntaxKind.ClosureTypeSpecifier
    let is_classname_type_specifier                     = has_kind SyntaxKind.ClassnameTypeSpecifier
    let is_field_specifier                              = has_kind SyntaxKind.FieldSpecifier
    let is_field_initializer                            = has_kind SyntaxKind.FieldInitializer
    let is_shape_type_specifier                         = has_kind SyntaxKind.ShapeTypeSpecifier
    let is_shape_expression                             = has_kind SyntaxKind.ShapeExpression
    let is_tuple_expression                             = has_kind SyntaxKind.TupleExpression
    let is_generic_type_specifier                       = has_kind SyntaxKind.GenericTypeSpecifier
    let is_nullable_type_specifier                      = has_kind SyntaxKind.NullableTypeSpecifier
    let is_soft_type_specifier                          = has_kind SyntaxKind.SoftTypeSpecifier
    let is_type_arguments                               = has_kind SyntaxKind.TypeArguments
    let is_type_parameters                              = has_kind SyntaxKind.TypeParameters
    let is_tuple_type_specifier                         = has_kind SyntaxKind.TupleTypeSpecifier
    let is_error                                        = has_kind SyntaxKind.ErrorSyntax
    let is_list_item                                    = has_kind SyntaxKind.ListItem


    let is_loop_statement node =
      is_for_statement node ||
      is_foreach_statement node ||
      is_while_statement node ||
      is_do_statement node

    let is_separable_prefix node =
      match syntax node with
      | Token t -> begin
        Full_fidelity_token_kind.(match Token.kind t with
        | PlusPlus | MinusMinus -> false
        | _ -> true) end
      | _ -> true

    let is_specific_token kind node =
      match syntax node with
      | Token t -> Token.kind t = kind
      | _ -> false


    let is_semicolon  = is_specific_token Full_fidelity_token_kind.Semicolon
    let is_name       = is_specific_token Full_fidelity_token_kind.Name
    let is_construct  = is_specific_token Full_fidelity_token_kind.Construct
    let is_destruct   = is_specific_token Full_fidelity_token_kind.Destruct
    let is_static     = is_specific_token Full_fidelity_token_kind.Static
    let is_private    = is_specific_token Full_fidelity_token_kind.Private
    let is_public     = is_specific_token Full_fidelity_token_kind.Public
    let is_protected  = is_specific_token Full_fidelity_token_kind.Protected
    let is_abstract   = is_specific_token Full_fidelity_token_kind.Abstract
    let is_final      = is_specific_token Full_fidelity_token_kind.Final
    let is_void       = is_specific_token Full_fidelity_token_kind.Void
    let is_left_brace = is_specific_token Full_fidelity_token_kind.LeftBrace
    let is_ellipsis   = is_specific_token Full_fidelity_token_kind.DotDotDot
    let is_comma      = is_specific_token Full_fidelity_token_kind.Comma
    let is_array      = is_specific_token Full_fidelity_token_kind.Array
    let is_var        = is_specific_token Full_fidelity_token_kind.Var
    let is_ampersand  = is_specific_token Full_fidelity_token_kind.Ampersand
    let is_inout      = is_specific_token Full_fidelity_token_kind.Inout

    let get_end_of_file_children {
      end_of_file_token;
    } = (
      end_of_file_token
    )

    let get_script_children {
      script_declarations;
    } = (
      script_declarations
    )

    let get_simple_type_specifier_children {
      simple_type_specifier;
    } = (
      simple_type_specifier
    )

    let get_literal_expression_children {
      literal_expression;
    } = (
      literal_expression
    )

    let get_variable_expression_children {
      variable_expression;
    } = (
      variable_expression
    )

    let get_qualified_name_expression_children {
      qualified_name_expression;
    } = (
      qualified_name_expression
    )

    let get_pipe_variable_expression_children {
      pipe_variable_expression;
    } = (
      pipe_variable_expression
    )

    let get_enum_declaration_children {
      enum_attribute_spec;
      enum_keyword;
      enum_name;
      enum_colon;
      enum_base;
      enum_type;
      enum_left_brace;
      enum_enumerators;
      enum_right_brace;
    } = (
      enum_attribute_spec,
      enum_keyword,
      enum_name,
      enum_colon,
      enum_base,
      enum_type,
      enum_left_brace,
      enum_enumerators,
      enum_right_brace
    )

    let get_enumerator_children {
      enumerator_name;
      enumerator_equal;
      enumerator_value;
      enumerator_semicolon;
    } = (
      enumerator_name,
      enumerator_equal,
      enumerator_value,
      enumerator_semicolon
    )

    let get_alias_declaration_children {
      alias_attribute_spec;
      alias_keyword;
      alias_name;
      alias_generic_parameter;
      alias_constraint;
      alias_equal;
      alias_type;
      alias_semicolon;
    } = (
      alias_attribute_spec,
      alias_keyword,
      alias_name,
      alias_generic_parameter,
      alias_constraint,
      alias_equal,
      alias_type,
      alias_semicolon
    )

    let get_property_declaration_children {
      property_modifiers;
      property_type;
      property_declarators;
      property_semicolon;
    } = (
      property_modifiers,
      property_type,
      property_declarators,
      property_semicolon
    )

    let get_property_declarator_children {
      property_name;
      property_initializer;
    } = (
      property_name,
      property_initializer
    )

    let get_namespace_declaration_children {
      namespace_keyword;
      namespace_name;
      namespace_body;
    } = (
      namespace_keyword,
      namespace_name,
      namespace_body
    )

    let get_namespace_body_children {
      namespace_left_brace;
      namespace_declarations;
      namespace_right_brace;
    } = (
      namespace_left_brace,
      namespace_declarations,
      namespace_right_brace
    )

    let get_namespace_empty_body_children {
      namespace_semicolon;
    } = (
      namespace_semicolon
    )

    let get_namespace_use_declaration_children {
      namespace_use_keyword;
      namespace_use_kind;
      namespace_use_clauses;
      namespace_use_semicolon;
    } = (
      namespace_use_keyword,
      namespace_use_kind,
      namespace_use_clauses,
      namespace_use_semicolon
    )

    let get_namespace_group_use_declaration_children {
      namespace_group_use_keyword;
      namespace_group_use_kind;
      namespace_group_use_prefix;
      namespace_group_use_left_brace;
      namespace_group_use_clauses;
      namespace_group_use_right_brace;
      namespace_group_use_semicolon;
    } = (
      namespace_group_use_keyword,
      namespace_group_use_kind,
      namespace_group_use_prefix,
      namespace_group_use_left_brace,
      namespace_group_use_clauses,
      namespace_group_use_right_brace,
      namespace_group_use_semicolon
    )

    let get_namespace_use_clause_children {
      namespace_use_clause_kind;
      namespace_use_name;
      namespace_use_as;
      namespace_use_alias;
    } = (
      namespace_use_clause_kind,
      namespace_use_name,
      namespace_use_as,
      namespace_use_alias
    )

    let get_function_declaration_children {
      function_attribute_spec;
      function_declaration_header;
      function_body;
    } = (
      function_attribute_spec,
      function_declaration_header,
      function_body
    )

    let get_function_declaration_header_children {
      function_async;
      function_coroutine;
      function_keyword;
      function_ampersand;
      function_name;
      function_type_parameter_list;
      function_left_paren;
      function_parameter_list;
      function_right_paren;
      function_colon;
      function_type;
      function_where_clause;
    } = (
      function_async,
      function_coroutine,
      function_keyword,
      function_ampersand,
      function_name,
      function_type_parameter_list,
      function_left_paren,
      function_parameter_list,
      function_right_paren,
      function_colon,
      function_type,
      function_where_clause
    )

    let get_where_clause_children {
      where_clause_keyword;
      where_clause_constraints;
    } = (
      where_clause_keyword,
      where_clause_constraints
    )

    let get_where_constraint_children {
      where_constraint_left_type;
      where_constraint_operator;
      where_constraint_right_type;
    } = (
      where_constraint_left_type,
      where_constraint_operator,
      where_constraint_right_type
    )

    let get_methodish_declaration_children {
      methodish_attribute;
      methodish_modifiers;
      methodish_function_decl_header;
      methodish_function_body;
      methodish_semicolon;
    } = (
      methodish_attribute,
      methodish_modifiers,
      methodish_function_decl_header,
      methodish_function_body,
      methodish_semicolon
    )

    let get_classish_declaration_children {
      classish_attribute;
      classish_modifiers;
      classish_keyword;
      classish_name;
      classish_type_parameters;
      classish_extends_keyword;
      classish_extends_list;
      classish_implements_keyword;
      classish_implements_list;
      classish_body;
    } = (
      classish_attribute,
      classish_modifiers,
      classish_keyword,
      classish_name,
      classish_type_parameters,
      classish_extends_keyword,
      classish_extends_list,
      classish_implements_keyword,
      classish_implements_list,
      classish_body
    )

    let get_classish_body_children {
      classish_body_left_brace;
      classish_body_elements;
      classish_body_right_brace;
    } = (
      classish_body_left_brace,
      classish_body_elements,
      classish_body_right_brace
    )

    let get_trait_use_precedence_item_children {
      trait_use_precedence_item_name;
      trait_use_precedence_item_keyword;
      trait_use_precedence_item_removed_names;
    } = (
      trait_use_precedence_item_name,
      trait_use_precedence_item_keyword,
      trait_use_precedence_item_removed_names
    )

    let get_trait_use_alias_item_children {
      trait_use_alias_item_aliasing_name;
      trait_use_alias_item_keyword;
      trait_use_alias_item_visibility;
      trait_use_alias_item_aliased_name;
    } = (
      trait_use_alias_item_aliasing_name,
      trait_use_alias_item_keyword,
      trait_use_alias_item_visibility,
      trait_use_alias_item_aliased_name
    )

    let get_trait_use_conflict_resolution_children {
      trait_use_conflict_resolution_keyword;
      trait_use_conflict_resolution_names;
      trait_use_conflict_resolution_left_brace;
      trait_use_conflict_resolution_clauses;
      trait_use_conflict_resolution_right_brace;
    } = (
      trait_use_conflict_resolution_keyword,
      trait_use_conflict_resolution_names,
      trait_use_conflict_resolution_left_brace,
      trait_use_conflict_resolution_clauses,
      trait_use_conflict_resolution_right_brace
    )

    let get_trait_use_children {
      trait_use_keyword;
      trait_use_names;
      trait_use_semicolon;
    } = (
      trait_use_keyword,
      trait_use_names,
      trait_use_semicolon
    )

    let get_require_clause_children {
      require_keyword;
      require_kind;
      require_name;
      require_semicolon;
    } = (
      require_keyword,
      require_kind,
      require_name,
      require_semicolon
    )

    let get_const_declaration_children {
      const_abstract;
      const_keyword;
      const_type_specifier;
      const_declarators;
      const_semicolon;
    } = (
      const_abstract,
      const_keyword,
      const_type_specifier,
      const_declarators,
      const_semicolon
    )

    let get_constant_declarator_children {
      constant_declarator_name;
      constant_declarator_initializer;
    } = (
      constant_declarator_name,
      constant_declarator_initializer
    )

    let get_type_const_declaration_children {
      type_const_abstract;
      type_const_keyword;
      type_const_type_keyword;
      type_const_name;
      type_const_type_constraint;
      type_const_equal;
      type_const_type_specifier;
      type_const_semicolon;
    } = (
      type_const_abstract,
      type_const_keyword,
      type_const_type_keyword,
      type_const_name,
      type_const_type_constraint,
      type_const_equal,
      type_const_type_specifier,
      type_const_semicolon
    )

    let get_decorated_expression_children {
      decorated_expression_decorator;
      decorated_expression_expression;
    } = (
      decorated_expression_decorator,
      decorated_expression_expression
    )

    let get_parameter_declaration_children {
      parameter_attribute;
      parameter_visibility;
      parameter_call_convention;
      parameter_type;
      parameter_name;
      parameter_default_value;
    } = (
      parameter_attribute,
      parameter_visibility,
      parameter_call_convention,
      parameter_type,
      parameter_name,
      parameter_default_value
    )

    let get_variadic_parameter_children {
      variadic_parameter_type;
      variadic_parameter_ellipsis;
    } = (
      variadic_parameter_type,
      variadic_parameter_ellipsis
    )

    let get_attribute_specification_children {
      attribute_specification_left_double_angle;
      attribute_specification_attributes;
      attribute_specification_right_double_angle;
    } = (
      attribute_specification_left_double_angle,
      attribute_specification_attributes,
      attribute_specification_right_double_angle
    )

    let get_attribute_children {
      attribute_name;
      attribute_left_paren;
      attribute_values;
      attribute_right_paren;
    } = (
      attribute_name,
      attribute_left_paren,
      attribute_values,
      attribute_right_paren
    )

    let get_inclusion_expression_children {
      inclusion_require;
      inclusion_filename;
    } = (
      inclusion_require,
      inclusion_filename
    )

    let get_inclusion_directive_children {
      inclusion_expression;
      inclusion_semicolon;
    } = (
      inclusion_expression,
      inclusion_semicolon
    )

    let get_compound_statement_children {
      compound_left_brace;
      compound_statements;
      compound_right_brace;
    } = (
      compound_left_brace,
      compound_statements,
      compound_right_brace
    )

    let get_expression_statement_children {
      expression_statement_expression;
      expression_statement_semicolon;
    } = (
      expression_statement_expression,
      expression_statement_semicolon
    )

    let get_markup_section_children {
      markup_prefix;
      markup_text;
      markup_suffix;
      markup_expression;
    } = (
      markup_prefix,
      markup_text,
      markup_suffix,
      markup_expression
    )

    let get_markup_suffix_children {
      markup_suffix_less_than_question;
      markup_suffix_name;
    } = (
      markup_suffix_less_than_question,
      markup_suffix_name
    )

    let get_unset_statement_children {
      unset_keyword;
      unset_left_paren;
      unset_variables;
      unset_right_paren;
      unset_semicolon;
    } = (
      unset_keyword,
      unset_left_paren,
      unset_variables,
      unset_right_paren,
      unset_semicolon
    )

    let get_using_statement_block_scoped_children {
      using_block_await_keyword;
      using_block_using_keyword;
      using_block_left_paren;
      using_block_expressions;
      using_block_right_paren;
      using_block_body;
    } = (
      using_block_await_keyword,
      using_block_using_keyword,
      using_block_left_paren,
      using_block_expressions,
      using_block_right_paren,
      using_block_body
    )

    let get_using_statement_function_scoped_children {
      using_function_await_keyword;
      using_function_using_keyword;
      using_function_expression;
      using_function_semicolon;
    } = (
      using_function_await_keyword,
      using_function_using_keyword,
      using_function_expression,
      using_function_semicolon
    )

    let get_while_statement_children {
      while_keyword;
      while_left_paren;
      while_condition;
      while_right_paren;
      while_body;
    } = (
      while_keyword,
      while_left_paren,
      while_condition,
      while_right_paren,
      while_body
    )

    let get_if_statement_children {
      if_keyword;
      if_left_paren;
      if_condition;
      if_right_paren;
      if_statement;
      if_elseif_clauses;
      if_else_clause;
    } = (
      if_keyword,
      if_left_paren,
      if_condition,
      if_right_paren,
      if_statement,
      if_elseif_clauses,
      if_else_clause
    )

    let get_elseif_clause_children {
      elseif_keyword;
      elseif_left_paren;
      elseif_condition;
      elseif_right_paren;
      elseif_statement;
    } = (
      elseif_keyword,
      elseif_left_paren,
      elseif_condition,
      elseif_right_paren,
      elseif_statement
    )

    let get_else_clause_children {
      else_keyword;
      else_statement;
    } = (
      else_keyword,
      else_statement
    )

    let get_if_endif_statement_children {
      if_endif_keyword;
      if_endif_left_paren;
      if_endif_condition;
      if_endif_right_paren;
      if_endif_colon;
      if_endif_statement;
      if_endif_elseif_colon_clauses;
      if_endif_else_colon_clause;
      if_endif_endif_keyword;
      if_endif_semicolon;
    } = (
      if_endif_keyword,
      if_endif_left_paren,
      if_endif_condition,
      if_endif_right_paren,
      if_endif_colon,
      if_endif_statement,
      if_endif_elseif_colon_clauses,
      if_endif_else_colon_clause,
      if_endif_endif_keyword,
      if_endif_semicolon
    )

    let get_elseif_colon_clause_children {
      elseif_colon_keyword;
      elseif_colon_left_paren;
      elseif_colon_condition;
      elseif_colon_right_paren;
      elseif_colon_colon;
      elseif_colon_statement;
    } = (
      elseif_colon_keyword,
      elseif_colon_left_paren,
      elseif_colon_condition,
      elseif_colon_right_paren,
      elseif_colon_colon,
      elseif_colon_statement
    )

    let get_else_colon_clause_children {
      else_colon_keyword;
      else_colon_colon;
      else_colon_statement;
    } = (
      else_colon_keyword,
      else_colon_colon,
      else_colon_statement
    )

    let get_try_statement_children {
      try_keyword;
      try_compound_statement;
      try_catch_clauses;
      try_finally_clause;
    } = (
      try_keyword,
      try_compound_statement,
      try_catch_clauses,
      try_finally_clause
    )

    let get_catch_clause_children {
      catch_keyword;
      catch_left_paren;
      catch_type;
      catch_variable;
      catch_right_paren;
      catch_body;
    } = (
      catch_keyword,
      catch_left_paren,
      catch_type,
      catch_variable,
      catch_right_paren,
      catch_body
    )

    let get_finally_clause_children {
      finally_keyword;
      finally_body;
    } = (
      finally_keyword,
      finally_body
    )

    let get_do_statement_children {
      do_keyword;
      do_body;
      do_while_keyword;
      do_left_paren;
      do_condition;
      do_right_paren;
      do_semicolon;
    } = (
      do_keyword,
      do_body,
      do_while_keyword,
      do_left_paren,
      do_condition,
      do_right_paren,
      do_semicolon
    )

    let get_for_statement_children {
      for_keyword;
      for_left_paren;
      for_initializer;
      for_first_semicolon;
      for_control;
      for_second_semicolon;
      for_end_of_loop;
      for_right_paren;
      for_body;
    } = (
      for_keyword,
      for_left_paren,
      for_initializer,
      for_first_semicolon,
      for_control,
      for_second_semicolon,
      for_end_of_loop,
      for_right_paren,
      for_body
    )

    let get_foreach_statement_children {
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
    } = (
      foreach_keyword,
      foreach_left_paren,
      foreach_collection,
      foreach_await_keyword,
      foreach_as,
      foreach_key,
      foreach_arrow,
      foreach_value,
      foreach_right_paren,
      foreach_body
    )

    let get_switch_statement_children {
      switch_keyword;
      switch_left_paren;
      switch_expression;
      switch_right_paren;
      switch_left_brace;
      switch_sections;
      switch_right_brace;
    } = (
      switch_keyword,
      switch_left_paren,
      switch_expression,
      switch_right_paren,
      switch_left_brace,
      switch_sections,
      switch_right_brace
    )

    let get_switch_section_children {
      switch_section_labels;
      switch_section_statements;
      switch_section_fallthrough;
    } = (
      switch_section_labels,
      switch_section_statements,
      switch_section_fallthrough
    )

    let get_switch_fallthrough_children {
      fallthrough_keyword;
      fallthrough_semicolon;
    } = (
      fallthrough_keyword,
      fallthrough_semicolon
    )

    let get_case_label_children {
      case_keyword;
      case_expression;
      case_colon;
    } = (
      case_keyword,
      case_expression,
      case_colon
    )

    let get_default_label_children {
      default_keyword;
      default_colon;
    } = (
      default_keyword,
      default_colon
    )

    let get_return_statement_children {
      return_keyword;
      return_expression;
      return_semicolon;
    } = (
      return_keyword,
      return_expression,
      return_semicolon
    )

    let get_goto_label_children {
      goto_label_name;
      goto_label_colon;
    } = (
      goto_label_name,
      goto_label_colon
    )

    let get_goto_statement_children {
      goto_statement_keyword;
      goto_statement_label_name;
      goto_statement_semicolon;
    } = (
      goto_statement_keyword,
      goto_statement_label_name,
      goto_statement_semicolon
    )

    let get_throw_statement_children {
      throw_keyword;
      throw_expression;
      throw_semicolon;
    } = (
      throw_keyword,
      throw_expression,
      throw_semicolon
    )

    let get_break_statement_children {
      break_keyword;
      break_level;
      break_semicolon;
    } = (
      break_keyword,
      break_level,
      break_semicolon
    )

    let get_continue_statement_children {
      continue_keyword;
      continue_level;
      continue_semicolon;
    } = (
      continue_keyword,
      continue_level,
      continue_semicolon
    )

    let get_function_static_statement_children {
      static_static_keyword;
      static_declarations;
      static_semicolon;
    } = (
      static_static_keyword,
      static_declarations,
      static_semicolon
    )

    let get_static_declarator_children {
      static_name;
      static_initializer;
    } = (
      static_name,
      static_initializer
    )

    let get_echo_statement_children {
      echo_keyword;
      echo_expressions;
      echo_semicolon;
    } = (
      echo_keyword,
      echo_expressions,
      echo_semicolon
    )

    let get_global_statement_children {
      global_keyword;
      global_variables;
      global_semicolon;
    } = (
      global_keyword,
      global_variables,
      global_semicolon
    )

    let get_simple_initializer_children {
      simple_initializer_equal;
      simple_initializer_value;
    } = (
      simple_initializer_equal,
      simple_initializer_value
    )

    let get_anonymous_function_children {
      anonymous_static_keyword;
      anonymous_async_keyword;
      anonymous_coroutine_keyword;
      anonymous_function_keyword;
      anonymous_left_paren;
      anonymous_parameters;
      anonymous_right_paren;
      anonymous_colon;
      anonymous_type;
      anonymous_use;
      anonymous_body;
    } = (
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
      anonymous_body
    )

    let get_anonymous_function_use_clause_children {
      anonymous_use_keyword;
      anonymous_use_left_paren;
      anonymous_use_variables;
      anonymous_use_right_paren;
    } = (
      anonymous_use_keyword,
      anonymous_use_left_paren,
      anonymous_use_variables,
      anonymous_use_right_paren
    )

    let get_lambda_expression_children {
      lambda_async;
      lambda_coroutine;
      lambda_signature;
      lambda_arrow;
      lambda_body;
    } = (
      lambda_async,
      lambda_coroutine,
      lambda_signature,
      lambda_arrow,
      lambda_body
    )

    let get_lambda_signature_children {
      lambda_left_paren;
      lambda_parameters;
      lambda_right_paren;
      lambda_colon;
      lambda_type;
    } = (
      lambda_left_paren,
      lambda_parameters,
      lambda_right_paren,
      lambda_colon,
      lambda_type
    )

    let get_cast_expression_children {
      cast_left_paren;
      cast_type;
      cast_right_paren;
      cast_operand;
    } = (
      cast_left_paren,
      cast_type,
      cast_right_paren,
      cast_operand
    )

    let get_scope_resolution_expression_children {
      scope_resolution_qualifier;
      scope_resolution_operator;
      scope_resolution_name;
    } = (
      scope_resolution_qualifier,
      scope_resolution_operator,
      scope_resolution_name
    )

    let get_member_selection_expression_children {
      member_object;
      member_operator;
      member_name;
    } = (
      member_object,
      member_operator,
      member_name
    )

    let get_safe_member_selection_expression_children {
      safe_member_object;
      safe_member_operator;
      safe_member_name;
    } = (
      safe_member_object,
      safe_member_operator,
      safe_member_name
    )

    let get_embedded_member_selection_expression_children {
      embedded_member_object;
      embedded_member_operator;
      embedded_member_name;
    } = (
      embedded_member_object,
      embedded_member_operator,
      embedded_member_name
    )

    let get_yield_expression_children {
      yield_keyword;
      yield_operand;
    } = (
      yield_keyword,
      yield_operand
    )

    let get_yield_from_expression_children {
      yield_from_yield_keyword;
      yield_from_from_keyword;
      yield_from_operand;
    } = (
      yield_from_yield_keyword,
      yield_from_from_keyword,
      yield_from_operand
    )

    let get_prefix_unary_expression_children {
      prefix_unary_operator;
      prefix_unary_operand;
    } = (
      prefix_unary_operator,
      prefix_unary_operand
    )

    let get_postfix_unary_expression_children {
      postfix_unary_operand;
      postfix_unary_operator;
    } = (
      postfix_unary_operand,
      postfix_unary_operator
    )

    let get_binary_expression_children {
      binary_left_operand;
      binary_operator;
      binary_right_operand;
    } = (
      binary_left_operand,
      binary_operator,
      binary_right_operand
    )

    let get_instanceof_expression_children {
      instanceof_left_operand;
      instanceof_operator;
      instanceof_right_operand;
    } = (
      instanceof_left_operand,
      instanceof_operator,
      instanceof_right_operand
    )

    let get_is_expression_children {
      is_left_operand;
      is_operator;
      is_right_operand;
    } = (
      is_left_operand,
      is_operator,
      is_right_operand
    )

    let get_conditional_expression_children {
      conditional_test;
      conditional_question;
      conditional_consequence;
      conditional_colon;
      conditional_alternative;
    } = (
      conditional_test,
      conditional_question,
      conditional_consequence,
      conditional_colon,
      conditional_alternative
    )

    let get_eval_expression_children {
      eval_keyword;
      eval_left_paren;
      eval_argument;
      eval_right_paren;
    } = (
      eval_keyword,
      eval_left_paren,
      eval_argument,
      eval_right_paren
    )

    let get_empty_expression_children {
      empty_keyword;
      empty_left_paren;
      empty_argument;
      empty_right_paren;
    } = (
      empty_keyword,
      empty_left_paren,
      empty_argument,
      empty_right_paren
    )

    let get_define_expression_children {
      define_keyword;
      define_left_paren;
      define_argument_list;
      define_right_paren;
    } = (
      define_keyword,
      define_left_paren,
      define_argument_list,
      define_right_paren
    )

    let get_isset_expression_children {
      isset_keyword;
      isset_left_paren;
      isset_argument_list;
      isset_right_paren;
    } = (
      isset_keyword,
      isset_left_paren,
      isset_argument_list,
      isset_right_paren
    )

    let get_function_call_expression_children {
      function_call_receiver;
      function_call_left_paren;
      function_call_argument_list;
      function_call_right_paren;
    } = (
      function_call_receiver,
      function_call_left_paren,
      function_call_argument_list,
      function_call_right_paren
    )

    let get_function_call_with_type_arguments_expression_children {
      function_call_with_type_arguments_receiver;
      function_call_with_type_arguments_type_args;
      function_call_with_type_arguments_left_paren;
      function_call_with_type_arguments_argument_list;
      function_call_with_type_arguments_right_paren;
    } = (
      function_call_with_type_arguments_receiver,
      function_call_with_type_arguments_type_args,
      function_call_with_type_arguments_left_paren,
      function_call_with_type_arguments_argument_list,
      function_call_with_type_arguments_right_paren
    )

    let get_parenthesized_expression_children {
      parenthesized_expression_left_paren;
      parenthesized_expression_expression;
      parenthesized_expression_right_paren;
    } = (
      parenthesized_expression_left_paren,
      parenthesized_expression_expression,
      parenthesized_expression_right_paren
    )

    let get_braced_expression_children {
      braced_expression_left_brace;
      braced_expression_expression;
      braced_expression_right_brace;
    } = (
      braced_expression_left_brace,
      braced_expression_expression,
      braced_expression_right_brace
    )

    let get_embedded_braced_expression_children {
      embedded_braced_expression_left_brace;
      embedded_braced_expression_expression;
      embedded_braced_expression_right_brace;
    } = (
      embedded_braced_expression_left_brace,
      embedded_braced_expression_expression,
      embedded_braced_expression_right_brace
    )

    let get_list_expression_children {
      list_keyword;
      list_left_paren;
      list_members;
      list_right_paren;
    } = (
      list_keyword,
      list_left_paren,
      list_members,
      list_right_paren
    )

    let get_collection_literal_expression_children {
      collection_literal_name;
      collection_literal_left_brace;
      collection_literal_initializers;
      collection_literal_right_brace;
    } = (
      collection_literal_name,
      collection_literal_left_brace,
      collection_literal_initializers,
      collection_literal_right_brace
    )

    let get_object_creation_expression_children {
      object_creation_new_keyword;
      object_creation_type;
      object_creation_left_paren;
      object_creation_argument_list;
      object_creation_right_paren;
    } = (
      object_creation_new_keyword,
      object_creation_type,
      object_creation_left_paren,
      object_creation_argument_list,
      object_creation_right_paren
    )

    let get_array_creation_expression_children {
      array_creation_left_bracket;
      array_creation_members;
      array_creation_right_bracket;
    } = (
      array_creation_left_bracket,
      array_creation_members,
      array_creation_right_bracket
    )

    let get_array_intrinsic_expression_children {
      array_intrinsic_keyword;
      array_intrinsic_left_paren;
      array_intrinsic_members;
      array_intrinsic_right_paren;
    } = (
      array_intrinsic_keyword,
      array_intrinsic_left_paren,
      array_intrinsic_members,
      array_intrinsic_right_paren
    )

    let get_darray_intrinsic_expression_children {
      darray_intrinsic_keyword;
      darray_intrinsic_left_bracket;
      darray_intrinsic_members;
      darray_intrinsic_right_bracket;
    } = (
      darray_intrinsic_keyword,
      darray_intrinsic_left_bracket,
      darray_intrinsic_members,
      darray_intrinsic_right_bracket
    )

    let get_dictionary_intrinsic_expression_children {
      dictionary_intrinsic_keyword;
      dictionary_intrinsic_left_bracket;
      dictionary_intrinsic_members;
      dictionary_intrinsic_right_bracket;
    } = (
      dictionary_intrinsic_keyword,
      dictionary_intrinsic_left_bracket,
      dictionary_intrinsic_members,
      dictionary_intrinsic_right_bracket
    )

    let get_keyset_intrinsic_expression_children {
      keyset_intrinsic_keyword;
      keyset_intrinsic_left_bracket;
      keyset_intrinsic_members;
      keyset_intrinsic_right_bracket;
    } = (
      keyset_intrinsic_keyword,
      keyset_intrinsic_left_bracket,
      keyset_intrinsic_members,
      keyset_intrinsic_right_bracket
    )

    let get_varray_intrinsic_expression_children {
      varray_intrinsic_keyword;
      varray_intrinsic_left_bracket;
      varray_intrinsic_members;
      varray_intrinsic_right_bracket;
    } = (
      varray_intrinsic_keyword,
      varray_intrinsic_left_bracket,
      varray_intrinsic_members,
      varray_intrinsic_right_bracket
    )

    let get_vector_intrinsic_expression_children {
      vector_intrinsic_keyword;
      vector_intrinsic_left_bracket;
      vector_intrinsic_members;
      vector_intrinsic_right_bracket;
    } = (
      vector_intrinsic_keyword,
      vector_intrinsic_left_bracket,
      vector_intrinsic_members,
      vector_intrinsic_right_bracket
    )

    let get_element_initializer_children {
      element_key;
      element_arrow;
      element_value;
    } = (
      element_key,
      element_arrow,
      element_value
    )

    let get_subscript_expression_children {
      subscript_receiver;
      subscript_left_bracket;
      subscript_index;
      subscript_right_bracket;
    } = (
      subscript_receiver,
      subscript_left_bracket,
      subscript_index,
      subscript_right_bracket
    )

    let get_embedded_subscript_expression_children {
      embedded_subscript_receiver;
      embedded_subscript_left_bracket;
      embedded_subscript_index;
      embedded_subscript_right_bracket;
    } = (
      embedded_subscript_receiver,
      embedded_subscript_left_bracket,
      embedded_subscript_index,
      embedded_subscript_right_bracket
    )

    let get_awaitable_creation_expression_children {
      awaitable_async;
      awaitable_coroutine;
      awaitable_compound_statement;
    } = (
      awaitable_async,
      awaitable_coroutine,
      awaitable_compound_statement
    )

    let get_xhp_children_declaration_children {
      xhp_children_keyword;
      xhp_children_expression;
      xhp_children_semicolon;
    } = (
      xhp_children_keyword,
      xhp_children_expression,
      xhp_children_semicolon
    )

    let get_xhp_children_parenthesized_list_children {
      xhp_children_list_left_paren;
      xhp_children_list_xhp_children;
      xhp_children_list_right_paren;
    } = (
      xhp_children_list_left_paren,
      xhp_children_list_xhp_children,
      xhp_children_list_right_paren
    )

    let get_xhp_category_declaration_children {
      xhp_category_keyword;
      xhp_category_categories;
      xhp_category_semicolon;
    } = (
      xhp_category_keyword,
      xhp_category_categories,
      xhp_category_semicolon
    )

    let get_xhp_enum_type_children {
      xhp_enum_keyword;
      xhp_enum_left_brace;
      xhp_enum_values;
      xhp_enum_right_brace;
    } = (
      xhp_enum_keyword,
      xhp_enum_left_brace,
      xhp_enum_values,
      xhp_enum_right_brace
    )

    let get_xhp_required_children {
      xhp_required_at;
      xhp_required_keyword;
    } = (
      xhp_required_at,
      xhp_required_keyword
    )

    let get_xhp_class_attribute_declaration_children {
      xhp_attribute_keyword;
      xhp_attribute_attributes;
      xhp_attribute_semicolon;
    } = (
      xhp_attribute_keyword,
      xhp_attribute_attributes,
      xhp_attribute_semicolon
    )

    let get_xhp_class_attribute_children {
      xhp_attribute_decl_type;
      xhp_attribute_decl_name;
      xhp_attribute_decl_initializer;
      xhp_attribute_decl_required;
    } = (
      xhp_attribute_decl_type,
      xhp_attribute_decl_name,
      xhp_attribute_decl_initializer,
      xhp_attribute_decl_required
    )

    let get_xhp_simple_class_attribute_children {
      xhp_simple_class_attribute_type;
    } = (
      xhp_simple_class_attribute_type
    )

    let get_xhp_attribute_children {
      xhp_attribute_name;
      xhp_attribute_equal;
      xhp_attribute_expression;
    } = (
      xhp_attribute_name,
      xhp_attribute_equal,
      xhp_attribute_expression
    )

    let get_xhp_open_children {
      xhp_open_left_angle;
      xhp_open_name;
      xhp_open_attributes;
      xhp_open_right_angle;
    } = (
      xhp_open_left_angle,
      xhp_open_name,
      xhp_open_attributes,
      xhp_open_right_angle
    )

    let get_xhp_expression_children {
      xhp_open;
      xhp_body;
      xhp_close;
    } = (
      xhp_open,
      xhp_body,
      xhp_close
    )

    let get_xhp_close_children {
      xhp_close_left_angle;
      xhp_close_name;
      xhp_close_right_angle;
    } = (
      xhp_close_left_angle,
      xhp_close_name,
      xhp_close_right_angle
    )

    let get_type_constant_children {
      type_constant_left_type;
      type_constant_separator;
      type_constant_right_type;
    } = (
      type_constant_left_type,
      type_constant_separator,
      type_constant_right_type
    )

    let get_vector_type_specifier_children {
      vector_type_keyword;
      vector_type_left_angle;
      vector_type_type;
      vector_type_trailing_comma;
      vector_type_right_angle;
    } = (
      vector_type_keyword,
      vector_type_left_angle,
      vector_type_type,
      vector_type_trailing_comma,
      vector_type_right_angle
    )

    let get_keyset_type_specifier_children {
      keyset_type_keyword;
      keyset_type_left_angle;
      keyset_type_type;
      keyset_type_trailing_comma;
      keyset_type_right_angle;
    } = (
      keyset_type_keyword,
      keyset_type_left_angle,
      keyset_type_type,
      keyset_type_trailing_comma,
      keyset_type_right_angle
    )

    let get_tuple_type_explicit_specifier_children {
      tuple_type_keyword;
      tuple_type_left_angle;
      tuple_type_types;
      tuple_type_right_angle;
    } = (
      tuple_type_keyword,
      tuple_type_left_angle,
      tuple_type_types,
      tuple_type_right_angle
    )

    let get_varray_type_specifier_children {
      varray_keyword;
      varray_left_angle;
      varray_type;
      varray_trailing_comma;
      varray_right_angle;
    } = (
      varray_keyword,
      varray_left_angle,
      varray_type,
      varray_trailing_comma,
      varray_right_angle
    )

    let get_vector_array_type_specifier_children {
      vector_array_keyword;
      vector_array_left_angle;
      vector_array_type;
      vector_array_right_angle;
    } = (
      vector_array_keyword,
      vector_array_left_angle,
      vector_array_type,
      vector_array_right_angle
    )

    let get_type_parameter_children {
      type_variance;
      type_name;
      type_constraints;
    } = (
      type_variance,
      type_name,
      type_constraints
    )

    let get_type_constraint_children {
      constraint_keyword;
      constraint_type;
    } = (
      constraint_keyword,
      constraint_type
    )

    let get_darray_type_specifier_children {
      darray_keyword;
      darray_left_angle;
      darray_key;
      darray_comma;
      darray_value;
      darray_trailing_comma;
      darray_right_angle;
    } = (
      darray_keyword,
      darray_left_angle,
      darray_key,
      darray_comma,
      darray_value,
      darray_trailing_comma,
      darray_right_angle
    )

    let get_map_array_type_specifier_children {
      map_array_keyword;
      map_array_left_angle;
      map_array_key;
      map_array_comma;
      map_array_value;
      map_array_right_angle;
    } = (
      map_array_keyword,
      map_array_left_angle,
      map_array_key,
      map_array_comma,
      map_array_value,
      map_array_right_angle
    )

    let get_dictionary_type_specifier_children {
      dictionary_type_keyword;
      dictionary_type_left_angle;
      dictionary_type_members;
      dictionary_type_right_angle;
    } = (
      dictionary_type_keyword,
      dictionary_type_left_angle,
      dictionary_type_members,
      dictionary_type_right_angle
    )

    let get_closure_type_specifier_children {
      closure_outer_left_paren;
      closure_coroutine;
      closure_function_keyword;
      closure_inner_left_paren;
      closure_parameter_types;
      closure_inner_right_paren;
      closure_colon;
      closure_return_type;
      closure_outer_right_paren;
    } = (
      closure_outer_left_paren,
      closure_coroutine,
      closure_function_keyword,
      closure_inner_left_paren,
      closure_parameter_types,
      closure_inner_right_paren,
      closure_colon,
      closure_return_type,
      closure_outer_right_paren
    )

    let get_classname_type_specifier_children {
      classname_keyword;
      classname_left_angle;
      classname_type;
      classname_trailing_comma;
      classname_right_angle;
    } = (
      classname_keyword,
      classname_left_angle,
      classname_type,
      classname_trailing_comma,
      classname_right_angle
    )

    let get_field_specifier_children {
      field_question;
      field_name;
      field_arrow;
      field_type;
    } = (
      field_question,
      field_name,
      field_arrow,
      field_type
    )

    let get_field_initializer_children {
      field_initializer_name;
      field_initializer_arrow;
      field_initializer_value;
    } = (
      field_initializer_name,
      field_initializer_arrow,
      field_initializer_value
    )

    let get_shape_type_specifier_children {
      shape_type_keyword;
      shape_type_left_paren;
      shape_type_fields;
      shape_type_ellipsis;
      shape_type_right_paren;
    } = (
      shape_type_keyword,
      shape_type_left_paren,
      shape_type_fields,
      shape_type_ellipsis,
      shape_type_right_paren
    )

    let get_shape_expression_children {
      shape_expression_keyword;
      shape_expression_left_paren;
      shape_expression_fields;
      shape_expression_right_paren;
    } = (
      shape_expression_keyword,
      shape_expression_left_paren,
      shape_expression_fields,
      shape_expression_right_paren
    )

    let get_tuple_expression_children {
      tuple_expression_keyword;
      tuple_expression_left_paren;
      tuple_expression_items;
      tuple_expression_right_paren;
    } = (
      tuple_expression_keyword,
      tuple_expression_left_paren,
      tuple_expression_items,
      tuple_expression_right_paren
    )

    let get_generic_type_specifier_children {
      generic_class_type;
      generic_argument_list;
    } = (
      generic_class_type,
      generic_argument_list
    )

    let get_nullable_type_specifier_children {
      nullable_question;
      nullable_type;
    } = (
      nullable_question,
      nullable_type
    )

    let get_soft_type_specifier_children {
      soft_at;
      soft_type;
    } = (
      soft_at,
      soft_type
    )

    let get_type_arguments_children {
      type_arguments_left_angle;
      type_arguments_types;
      type_arguments_right_angle;
    } = (
      type_arguments_left_angle,
      type_arguments_types,
      type_arguments_right_angle
    )

    let get_type_parameters_children {
      type_parameters_left_angle;
      type_parameters_parameters;
      type_parameters_right_angle;
    } = (
      type_parameters_left_angle,
      type_parameters_parameters,
      type_parameters_right_angle
    )

    let get_tuple_type_specifier_children {
      tuple_left_paren;
      tuple_types;
      tuple_right_paren;
    } = (
      tuple_left_paren,
      tuple_types,
      tuple_right_paren
    )

    let get_error_children {
      error_error;
    } = (
      error_error
    )

    let get_list_item_children {
      list_item;
      list_separator;
    } = (
      list_item,
      list_separator
    )



    let fold_over_children f acc syntax =
      match syntax with
      | Missing -> acc
      | Token _ -> acc
      | SyntaxList _ -> acc
      | EndOfFile {
        end_of_file_token;
      } ->
         let acc = f acc end_of_file_token in
         acc
      | Script {
        script_declarations;
      } ->
         let acc = f acc script_declarations in
         acc
      | SimpleTypeSpecifier {
        simple_type_specifier;
      } ->
         let acc = f acc simple_type_specifier in
         acc
      | LiteralExpression {
        literal_expression;
      } ->
         let acc = f acc literal_expression in
         acc
      | VariableExpression {
        variable_expression;
      } ->
         let acc = f acc variable_expression in
         acc
      | QualifiedNameExpression {
        qualified_name_expression;
      } ->
         let acc = f acc qualified_name_expression in
         acc
      | PipeVariableExpression {
        pipe_variable_expression;
      } ->
         let acc = f acc pipe_variable_expression in
         acc
      | EnumDeclaration {
        enum_attribute_spec;
        enum_keyword;
        enum_name;
        enum_colon;
        enum_base;
        enum_type;
        enum_left_brace;
        enum_enumerators;
        enum_right_brace;
      } ->
         let acc = f acc enum_attribute_spec in
         let acc = f acc enum_keyword in
         let acc = f acc enum_name in
         let acc = f acc enum_colon in
         let acc = f acc enum_base in
         let acc = f acc enum_type in
         let acc = f acc enum_left_brace in
         let acc = f acc enum_enumerators in
         let acc = f acc enum_right_brace in
         acc
      | Enumerator {
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
      | AliasDeclaration {
        alias_attribute_spec;
        alias_keyword;
        alias_name;
        alias_generic_parameter;
        alias_constraint;
        alias_equal;
        alias_type;
        alias_semicolon;
      } ->
         let acc = f acc alias_attribute_spec in
         let acc = f acc alias_keyword in
         let acc = f acc alias_name in
         let acc = f acc alias_generic_parameter in
         let acc = f acc alias_constraint in
         let acc = f acc alias_equal in
         let acc = f acc alias_type in
         let acc = f acc alias_semicolon in
         acc
      | PropertyDeclaration {
        property_modifiers;
        property_type;
        property_declarators;
        property_semicolon;
      } ->
         let acc = f acc property_modifiers in
         let acc = f acc property_type in
         let acc = f acc property_declarators in
         let acc = f acc property_semicolon in
         acc
      | PropertyDeclarator {
        property_name;
        property_initializer;
      } ->
         let acc = f acc property_name in
         let acc = f acc property_initializer in
         acc
      | NamespaceDeclaration {
        namespace_keyword;
        namespace_name;
        namespace_body;
      } ->
         let acc = f acc namespace_keyword in
         let acc = f acc namespace_name in
         let acc = f acc namespace_body in
         acc
      | NamespaceBody {
        namespace_left_brace;
        namespace_declarations;
        namespace_right_brace;
      } ->
         let acc = f acc namespace_left_brace in
         let acc = f acc namespace_declarations in
         let acc = f acc namespace_right_brace in
         acc
      | NamespaceEmptyBody {
        namespace_semicolon;
      } ->
         let acc = f acc namespace_semicolon in
         acc
      | NamespaceUseDeclaration {
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
      | NamespaceGroupUseDeclaration {
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
      | NamespaceUseClause {
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
      | FunctionDeclaration {
        function_attribute_spec;
        function_declaration_header;
        function_body;
      } ->
         let acc = f acc function_attribute_spec in
         let acc = f acc function_declaration_header in
         let acc = f acc function_body in
         acc
      | FunctionDeclarationHeader {
        function_async;
        function_coroutine;
        function_keyword;
        function_ampersand;
        function_name;
        function_type_parameter_list;
        function_left_paren;
        function_parameter_list;
        function_right_paren;
        function_colon;
        function_type;
        function_where_clause;
      } ->
         let acc = f acc function_async in
         let acc = f acc function_coroutine in
         let acc = f acc function_keyword in
         let acc = f acc function_ampersand in
         let acc = f acc function_name in
         let acc = f acc function_type_parameter_list in
         let acc = f acc function_left_paren in
         let acc = f acc function_parameter_list in
         let acc = f acc function_right_paren in
         let acc = f acc function_colon in
         let acc = f acc function_type in
         let acc = f acc function_where_clause in
         acc
      | WhereClause {
        where_clause_keyword;
        where_clause_constraints;
      } ->
         let acc = f acc where_clause_keyword in
         let acc = f acc where_clause_constraints in
         acc
      | WhereConstraint {
        where_constraint_left_type;
        where_constraint_operator;
        where_constraint_right_type;
      } ->
         let acc = f acc where_constraint_left_type in
         let acc = f acc where_constraint_operator in
         let acc = f acc where_constraint_right_type in
         acc
      | MethodishDeclaration {
        methodish_attribute;
        methodish_modifiers;
        methodish_function_decl_header;
        methodish_function_body;
        methodish_semicolon;
      } ->
         let acc = f acc methodish_attribute in
         let acc = f acc methodish_modifiers in
         let acc = f acc methodish_function_decl_header in
         let acc = f acc methodish_function_body in
         let acc = f acc methodish_semicolon in
         acc
      | ClassishDeclaration {
        classish_attribute;
        classish_modifiers;
        classish_keyword;
        classish_name;
        classish_type_parameters;
        classish_extends_keyword;
        classish_extends_list;
        classish_implements_keyword;
        classish_implements_list;
        classish_body;
      } ->
         let acc = f acc classish_attribute in
         let acc = f acc classish_modifiers in
         let acc = f acc classish_keyword in
         let acc = f acc classish_name in
         let acc = f acc classish_type_parameters in
         let acc = f acc classish_extends_keyword in
         let acc = f acc classish_extends_list in
         let acc = f acc classish_implements_keyword in
         let acc = f acc classish_implements_list in
         let acc = f acc classish_body in
         acc
      | ClassishBody {
        classish_body_left_brace;
        classish_body_elements;
        classish_body_right_brace;
      } ->
         let acc = f acc classish_body_left_brace in
         let acc = f acc classish_body_elements in
         let acc = f acc classish_body_right_brace in
         acc
      | TraitUsePrecedenceItem {
        trait_use_precedence_item_name;
        trait_use_precedence_item_keyword;
        trait_use_precedence_item_removed_names;
      } ->
         let acc = f acc trait_use_precedence_item_name in
         let acc = f acc trait_use_precedence_item_keyword in
         let acc = f acc trait_use_precedence_item_removed_names in
         acc
      | TraitUseAliasItem {
        trait_use_alias_item_aliasing_name;
        trait_use_alias_item_keyword;
        trait_use_alias_item_visibility;
        trait_use_alias_item_aliased_name;
      } ->
         let acc = f acc trait_use_alias_item_aliasing_name in
         let acc = f acc trait_use_alias_item_keyword in
         let acc = f acc trait_use_alias_item_visibility in
         let acc = f acc trait_use_alias_item_aliased_name in
         acc
      | TraitUseConflictResolution {
        trait_use_conflict_resolution_keyword;
        trait_use_conflict_resolution_names;
        trait_use_conflict_resolution_left_brace;
        trait_use_conflict_resolution_clauses;
        trait_use_conflict_resolution_right_brace;
      } ->
         let acc = f acc trait_use_conflict_resolution_keyword in
         let acc = f acc trait_use_conflict_resolution_names in
         let acc = f acc trait_use_conflict_resolution_left_brace in
         let acc = f acc trait_use_conflict_resolution_clauses in
         let acc = f acc trait_use_conflict_resolution_right_brace in
         acc
      | TraitUse {
        trait_use_keyword;
        trait_use_names;
        trait_use_semicolon;
      } ->
         let acc = f acc trait_use_keyword in
         let acc = f acc trait_use_names in
         let acc = f acc trait_use_semicolon in
         acc
      | RequireClause {
        require_keyword;
        require_kind;
        require_name;
        require_semicolon;
      } ->
         let acc = f acc require_keyword in
         let acc = f acc require_kind in
         let acc = f acc require_name in
         let acc = f acc require_semicolon in
         acc
      | ConstDeclaration {
        const_abstract;
        const_keyword;
        const_type_specifier;
        const_declarators;
        const_semicolon;
      } ->
         let acc = f acc const_abstract in
         let acc = f acc const_keyword in
         let acc = f acc const_type_specifier in
         let acc = f acc const_declarators in
         let acc = f acc const_semicolon in
         acc
      | ConstantDeclarator {
        constant_declarator_name;
        constant_declarator_initializer;
      } ->
         let acc = f acc constant_declarator_name in
         let acc = f acc constant_declarator_initializer in
         acc
      | TypeConstDeclaration {
        type_const_abstract;
        type_const_keyword;
        type_const_type_keyword;
        type_const_name;
        type_const_type_constraint;
        type_const_equal;
        type_const_type_specifier;
        type_const_semicolon;
      } ->
         let acc = f acc type_const_abstract in
         let acc = f acc type_const_keyword in
         let acc = f acc type_const_type_keyword in
         let acc = f acc type_const_name in
         let acc = f acc type_const_type_constraint in
         let acc = f acc type_const_equal in
         let acc = f acc type_const_type_specifier in
         let acc = f acc type_const_semicolon in
         acc
      | DecoratedExpression {
        decorated_expression_decorator;
        decorated_expression_expression;
      } ->
         let acc = f acc decorated_expression_decorator in
         let acc = f acc decorated_expression_expression in
         acc
      | ParameterDeclaration {
        parameter_attribute;
        parameter_visibility;
        parameter_call_convention;
        parameter_type;
        parameter_name;
        parameter_default_value;
      } ->
         let acc = f acc parameter_attribute in
         let acc = f acc parameter_visibility in
         let acc = f acc parameter_call_convention in
         let acc = f acc parameter_type in
         let acc = f acc parameter_name in
         let acc = f acc parameter_default_value in
         acc
      | VariadicParameter {
        variadic_parameter_type;
        variadic_parameter_ellipsis;
      } ->
         let acc = f acc variadic_parameter_type in
         let acc = f acc variadic_parameter_ellipsis in
         acc
      | AttributeSpecification {
        attribute_specification_left_double_angle;
        attribute_specification_attributes;
        attribute_specification_right_double_angle;
      } ->
         let acc = f acc attribute_specification_left_double_angle in
         let acc = f acc attribute_specification_attributes in
         let acc = f acc attribute_specification_right_double_angle in
         acc
      | Attribute {
        attribute_name;
        attribute_left_paren;
        attribute_values;
        attribute_right_paren;
      } ->
         let acc = f acc attribute_name in
         let acc = f acc attribute_left_paren in
         let acc = f acc attribute_values in
         let acc = f acc attribute_right_paren in
         acc
      | InclusionExpression {
        inclusion_require;
        inclusion_filename;
      } ->
         let acc = f acc inclusion_require in
         let acc = f acc inclusion_filename in
         acc
      | InclusionDirective {
        inclusion_expression;
        inclusion_semicolon;
      } ->
         let acc = f acc inclusion_expression in
         let acc = f acc inclusion_semicolon in
         acc
      | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      } ->
         let acc = f acc compound_left_brace in
         let acc = f acc compound_statements in
         let acc = f acc compound_right_brace in
         acc
      | ExpressionStatement {
        expression_statement_expression;
        expression_statement_semicolon;
      } ->
         let acc = f acc expression_statement_expression in
         let acc = f acc expression_statement_semicolon in
         acc
      | MarkupSection {
        markup_prefix;
        markup_text;
        markup_suffix;
        markup_expression;
      } ->
         let acc = f acc markup_prefix in
         let acc = f acc markup_text in
         let acc = f acc markup_suffix in
         let acc = f acc markup_expression in
         acc
      | MarkupSuffix {
        markup_suffix_less_than_question;
        markup_suffix_name;
      } ->
         let acc = f acc markup_suffix_less_than_question in
         let acc = f acc markup_suffix_name in
         acc
      | UnsetStatement {
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
      | UsingStatementBlockScoped {
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
      | UsingStatementFunctionScoped {
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
      | WhileStatement {
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
      | IfStatement {
        if_keyword;
        if_left_paren;
        if_condition;
        if_right_paren;
        if_statement;
        if_elseif_clauses;
        if_else_clause;
      } ->
         let acc = f acc if_keyword in
         let acc = f acc if_left_paren in
         let acc = f acc if_condition in
         let acc = f acc if_right_paren in
         let acc = f acc if_statement in
         let acc = f acc if_elseif_clauses in
         let acc = f acc if_else_clause in
         acc
      | ElseifClause {
        elseif_keyword;
        elseif_left_paren;
        elseif_condition;
        elseif_right_paren;
        elseif_statement;
      } ->
         let acc = f acc elseif_keyword in
         let acc = f acc elseif_left_paren in
         let acc = f acc elseif_condition in
         let acc = f acc elseif_right_paren in
         let acc = f acc elseif_statement in
         acc
      | ElseClause {
        else_keyword;
        else_statement;
      } ->
         let acc = f acc else_keyword in
         let acc = f acc else_statement in
         acc
      | IfEndIfStatement {
        if_endif_keyword;
        if_endif_left_paren;
        if_endif_condition;
        if_endif_right_paren;
        if_endif_colon;
        if_endif_statement;
        if_endif_elseif_colon_clauses;
        if_endif_else_colon_clause;
        if_endif_endif_keyword;
        if_endif_semicolon;
      } ->
         let acc = f acc if_endif_keyword in
         let acc = f acc if_endif_left_paren in
         let acc = f acc if_endif_condition in
         let acc = f acc if_endif_right_paren in
         let acc = f acc if_endif_colon in
         let acc = f acc if_endif_statement in
         let acc = f acc if_endif_elseif_colon_clauses in
         let acc = f acc if_endif_else_colon_clause in
         let acc = f acc if_endif_endif_keyword in
         let acc = f acc if_endif_semicolon in
         acc
      | ElseifColonClause {
        elseif_colon_keyword;
        elseif_colon_left_paren;
        elseif_colon_condition;
        elseif_colon_right_paren;
        elseif_colon_colon;
        elseif_colon_statement;
      } ->
         let acc = f acc elseif_colon_keyword in
         let acc = f acc elseif_colon_left_paren in
         let acc = f acc elseif_colon_condition in
         let acc = f acc elseif_colon_right_paren in
         let acc = f acc elseif_colon_colon in
         let acc = f acc elseif_colon_statement in
         acc
      | ElseColonClause {
        else_colon_keyword;
        else_colon_colon;
        else_colon_statement;
      } ->
         let acc = f acc else_colon_keyword in
         let acc = f acc else_colon_colon in
         let acc = f acc else_colon_statement in
         acc
      | TryStatement {
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
      | CatchClause {
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
      | FinallyClause {
        finally_keyword;
        finally_body;
      } ->
         let acc = f acc finally_keyword in
         let acc = f acc finally_body in
         acc
      | DoStatement {
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
      | ForStatement {
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
      | ForeachStatement {
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
      | SwitchStatement {
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
      | SwitchSection {
        switch_section_labels;
        switch_section_statements;
        switch_section_fallthrough;
      } ->
         let acc = f acc switch_section_labels in
         let acc = f acc switch_section_statements in
         let acc = f acc switch_section_fallthrough in
         acc
      | SwitchFallthrough {
        fallthrough_keyword;
        fallthrough_semicolon;
      } ->
         let acc = f acc fallthrough_keyword in
         let acc = f acc fallthrough_semicolon in
         acc
      | CaseLabel {
        case_keyword;
        case_expression;
        case_colon;
      } ->
         let acc = f acc case_keyword in
         let acc = f acc case_expression in
         let acc = f acc case_colon in
         acc
      | DefaultLabel {
        default_keyword;
        default_colon;
      } ->
         let acc = f acc default_keyword in
         let acc = f acc default_colon in
         acc
      | ReturnStatement {
        return_keyword;
        return_expression;
        return_semicolon;
      } ->
         let acc = f acc return_keyword in
         let acc = f acc return_expression in
         let acc = f acc return_semicolon in
         acc
      | GotoLabel {
        goto_label_name;
        goto_label_colon;
      } ->
         let acc = f acc goto_label_name in
         let acc = f acc goto_label_colon in
         acc
      | GotoStatement {
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon;
      } ->
         let acc = f acc goto_statement_keyword in
         let acc = f acc goto_statement_label_name in
         let acc = f acc goto_statement_semicolon in
         acc
      | ThrowStatement {
        throw_keyword;
        throw_expression;
        throw_semicolon;
      } ->
         let acc = f acc throw_keyword in
         let acc = f acc throw_expression in
         let acc = f acc throw_semicolon in
         acc
      | BreakStatement {
        break_keyword;
        break_level;
        break_semicolon;
      } ->
         let acc = f acc break_keyword in
         let acc = f acc break_level in
         let acc = f acc break_semicolon in
         acc
      | ContinueStatement {
        continue_keyword;
        continue_level;
        continue_semicolon;
      } ->
         let acc = f acc continue_keyword in
         let acc = f acc continue_level in
         let acc = f acc continue_semicolon in
         acc
      | FunctionStaticStatement {
        static_static_keyword;
        static_declarations;
        static_semicolon;
      } ->
         let acc = f acc static_static_keyword in
         let acc = f acc static_declarations in
         let acc = f acc static_semicolon in
         acc
      | StaticDeclarator {
        static_name;
        static_initializer;
      } ->
         let acc = f acc static_name in
         let acc = f acc static_initializer in
         acc
      | EchoStatement {
        echo_keyword;
        echo_expressions;
        echo_semicolon;
      } ->
         let acc = f acc echo_keyword in
         let acc = f acc echo_expressions in
         let acc = f acc echo_semicolon in
         acc
      | GlobalStatement {
        global_keyword;
        global_variables;
        global_semicolon;
      } ->
         let acc = f acc global_keyword in
         let acc = f acc global_variables in
         let acc = f acc global_semicolon in
         acc
      | SimpleInitializer {
        simple_initializer_equal;
        simple_initializer_value;
      } ->
         let acc = f acc simple_initializer_equal in
         let acc = f acc simple_initializer_value in
         acc
      | AnonymousFunction {
        anonymous_static_keyword;
        anonymous_async_keyword;
        anonymous_coroutine_keyword;
        anonymous_function_keyword;
        anonymous_left_paren;
        anonymous_parameters;
        anonymous_right_paren;
        anonymous_colon;
        anonymous_type;
        anonymous_use;
        anonymous_body;
      } ->
         let acc = f acc anonymous_static_keyword in
         let acc = f acc anonymous_async_keyword in
         let acc = f acc anonymous_coroutine_keyword in
         let acc = f acc anonymous_function_keyword in
         let acc = f acc anonymous_left_paren in
         let acc = f acc anonymous_parameters in
         let acc = f acc anonymous_right_paren in
         let acc = f acc anonymous_colon in
         let acc = f acc anonymous_type in
         let acc = f acc anonymous_use in
         let acc = f acc anonymous_body in
         acc
      | AnonymousFunctionUseClause {
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
      | LambdaExpression {
        lambda_async;
        lambda_coroutine;
        lambda_signature;
        lambda_arrow;
        lambda_body;
      } ->
         let acc = f acc lambda_async in
         let acc = f acc lambda_coroutine in
         let acc = f acc lambda_signature in
         let acc = f acc lambda_arrow in
         let acc = f acc lambda_body in
         acc
      | LambdaSignature {
        lambda_left_paren;
        lambda_parameters;
        lambda_right_paren;
        lambda_colon;
        lambda_type;
      } ->
         let acc = f acc lambda_left_paren in
         let acc = f acc lambda_parameters in
         let acc = f acc lambda_right_paren in
         let acc = f acc lambda_colon in
         let acc = f acc lambda_type in
         acc
      | CastExpression {
        cast_left_paren;
        cast_type;
        cast_right_paren;
        cast_operand;
      } ->
         let acc = f acc cast_left_paren in
         let acc = f acc cast_type in
         let acc = f acc cast_right_paren in
         let acc = f acc cast_operand in
         acc
      | ScopeResolutionExpression {
        scope_resolution_qualifier;
        scope_resolution_operator;
        scope_resolution_name;
      } ->
         let acc = f acc scope_resolution_qualifier in
         let acc = f acc scope_resolution_operator in
         let acc = f acc scope_resolution_name in
         acc
      | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name;
      } ->
         let acc = f acc member_object in
         let acc = f acc member_operator in
         let acc = f acc member_name in
         acc
      | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name;
      } ->
         let acc = f acc safe_member_object in
         let acc = f acc safe_member_operator in
         let acc = f acc safe_member_name in
         acc
      | EmbeddedMemberSelectionExpression {
        embedded_member_object;
        embedded_member_operator;
        embedded_member_name;
      } ->
         let acc = f acc embedded_member_object in
         let acc = f acc embedded_member_operator in
         let acc = f acc embedded_member_name in
         acc
      | YieldExpression {
        yield_keyword;
        yield_operand;
      } ->
         let acc = f acc yield_keyword in
         let acc = f acc yield_operand in
         acc
      | YieldFromExpression {
        yield_from_yield_keyword;
        yield_from_from_keyword;
        yield_from_operand;
      } ->
         let acc = f acc yield_from_yield_keyword in
         let acc = f acc yield_from_from_keyword in
         let acc = f acc yield_from_operand in
         acc
      | PrefixUnaryExpression {
        prefix_unary_operator;
        prefix_unary_operand;
      } ->
         let acc = f acc prefix_unary_operator in
         let acc = f acc prefix_unary_operand in
         acc
      | PostfixUnaryExpression {
        postfix_unary_operand;
        postfix_unary_operator;
      } ->
         let acc = f acc postfix_unary_operand in
         let acc = f acc postfix_unary_operator in
         acc
      | BinaryExpression {
        binary_left_operand;
        binary_operator;
        binary_right_operand;
      } ->
         let acc = f acc binary_left_operand in
         let acc = f acc binary_operator in
         let acc = f acc binary_right_operand in
         acc
      | InstanceofExpression {
        instanceof_left_operand;
        instanceof_operator;
        instanceof_right_operand;
      } ->
         let acc = f acc instanceof_left_operand in
         let acc = f acc instanceof_operator in
         let acc = f acc instanceof_right_operand in
         acc
      | IsExpression {
        is_left_operand;
        is_operator;
        is_right_operand;
      } ->
         let acc = f acc is_left_operand in
         let acc = f acc is_operator in
         let acc = f acc is_right_operand in
         acc
      | ConditionalExpression {
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
      | EvalExpression {
        eval_keyword;
        eval_left_paren;
        eval_argument;
        eval_right_paren;
      } ->
         let acc = f acc eval_keyword in
         let acc = f acc eval_left_paren in
         let acc = f acc eval_argument in
         let acc = f acc eval_right_paren in
         acc
      | EmptyExpression {
        empty_keyword;
        empty_left_paren;
        empty_argument;
        empty_right_paren;
      } ->
         let acc = f acc empty_keyword in
         let acc = f acc empty_left_paren in
         let acc = f acc empty_argument in
         let acc = f acc empty_right_paren in
         acc
      | DefineExpression {
        define_keyword;
        define_left_paren;
        define_argument_list;
        define_right_paren;
      } ->
         let acc = f acc define_keyword in
         let acc = f acc define_left_paren in
         let acc = f acc define_argument_list in
         let acc = f acc define_right_paren in
         acc
      | IssetExpression {
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
      | FunctionCallExpression {
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren;
      } ->
         let acc = f acc function_call_receiver in
         let acc = f acc function_call_left_paren in
         let acc = f acc function_call_argument_list in
         let acc = f acc function_call_right_paren in
         acc
      | FunctionCallWithTypeArgumentsExpression {
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren;
      } ->
         let acc = f acc function_call_with_type_arguments_receiver in
         let acc = f acc function_call_with_type_arguments_type_args in
         let acc = f acc function_call_with_type_arguments_left_paren in
         let acc = f acc function_call_with_type_arguments_argument_list in
         let acc = f acc function_call_with_type_arguments_right_paren in
         acc
      | ParenthesizedExpression {
        parenthesized_expression_left_paren;
        parenthesized_expression_expression;
        parenthesized_expression_right_paren;
      } ->
         let acc = f acc parenthesized_expression_left_paren in
         let acc = f acc parenthesized_expression_expression in
         let acc = f acc parenthesized_expression_right_paren in
         acc
      | BracedExpression {
        braced_expression_left_brace;
        braced_expression_expression;
        braced_expression_right_brace;
      } ->
         let acc = f acc braced_expression_left_brace in
         let acc = f acc braced_expression_expression in
         let acc = f acc braced_expression_right_brace in
         acc
      | EmbeddedBracedExpression {
        embedded_braced_expression_left_brace;
        embedded_braced_expression_expression;
        embedded_braced_expression_right_brace;
      } ->
         let acc = f acc embedded_braced_expression_left_brace in
         let acc = f acc embedded_braced_expression_expression in
         let acc = f acc embedded_braced_expression_right_brace in
         acc
      | ListExpression {
        list_keyword;
        list_left_paren;
        list_members;
        list_right_paren;
      } ->
         let acc = f acc list_keyword in
         let acc = f acc list_left_paren in
         let acc = f acc list_members in
         let acc = f acc list_right_paren in
         acc
      | CollectionLiteralExpression {
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
      | ObjectCreationExpression {
        object_creation_new_keyword;
        object_creation_type;
        object_creation_left_paren;
        object_creation_argument_list;
        object_creation_right_paren;
      } ->
         let acc = f acc object_creation_new_keyword in
         let acc = f acc object_creation_type in
         let acc = f acc object_creation_left_paren in
         let acc = f acc object_creation_argument_list in
         let acc = f acc object_creation_right_paren in
         acc
      | ArrayCreationExpression {
        array_creation_left_bracket;
        array_creation_members;
        array_creation_right_bracket;
      } ->
         let acc = f acc array_creation_left_bracket in
         let acc = f acc array_creation_members in
         let acc = f acc array_creation_right_bracket in
         acc
      | ArrayIntrinsicExpression {
        array_intrinsic_keyword;
        array_intrinsic_left_paren;
        array_intrinsic_members;
        array_intrinsic_right_paren;
      } ->
         let acc = f acc array_intrinsic_keyword in
         let acc = f acc array_intrinsic_left_paren in
         let acc = f acc array_intrinsic_members in
         let acc = f acc array_intrinsic_right_paren in
         acc
      | DarrayIntrinsicExpression {
        darray_intrinsic_keyword;
        darray_intrinsic_left_bracket;
        darray_intrinsic_members;
        darray_intrinsic_right_bracket;
      } ->
         let acc = f acc darray_intrinsic_keyword in
         let acc = f acc darray_intrinsic_left_bracket in
         let acc = f acc darray_intrinsic_members in
         let acc = f acc darray_intrinsic_right_bracket in
         acc
      | DictionaryIntrinsicExpression {
        dictionary_intrinsic_keyword;
        dictionary_intrinsic_left_bracket;
        dictionary_intrinsic_members;
        dictionary_intrinsic_right_bracket;
      } ->
         let acc = f acc dictionary_intrinsic_keyword in
         let acc = f acc dictionary_intrinsic_left_bracket in
         let acc = f acc dictionary_intrinsic_members in
         let acc = f acc dictionary_intrinsic_right_bracket in
         acc
      | KeysetIntrinsicExpression {
        keyset_intrinsic_keyword;
        keyset_intrinsic_left_bracket;
        keyset_intrinsic_members;
        keyset_intrinsic_right_bracket;
      } ->
         let acc = f acc keyset_intrinsic_keyword in
         let acc = f acc keyset_intrinsic_left_bracket in
         let acc = f acc keyset_intrinsic_members in
         let acc = f acc keyset_intrinsic_right_bracket in
         acc
      | VarrayIntrinsicExpression {
        varray_intrinsic_keyword;
        varray_intrinsic_left_bracket;
        varray_intrinsic_members;
        varray_intrinsic_right_bracket;
      } ->
         let acc = f acc varray_intrinsic_keyword in
         let acc = f acc varray_intrinsic_left_bracket in
         let acc = f acc varray_intrinsic_members in
         let acc = f acc varray_intrinsic_right_bracket in
         acc
      | VectorIntrinsicExpression {
        vector_intrinsic_keyword;
        vector_intrinsic_left_bracket;
        vector_intrinsic_members;
        vector_intrinsic_right_bracket;
      } ->
         let acc = f acc vector_intrinsic_keyword in
         let acc = f acc vector_intrinsic_left_bracket in
         let acc = f acc vector_intrinsic_members in
         let acc = f acc vector_intrinsic_right_bracket in
         acc
      | ElementInitializer {
        element_key;
        element_arrow;
        element_value;
      } ->
         let acc = f acc element_key in
         let acc = f acc element_arrow in
         let acc = f acc element_value in
         acc
      | SubscriptExpression {
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
      | EmbeddedSubscriptExpression {
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
      | AwaitableCreationExpression {
        awaitable_async;
        awaitable_coroutine;
        awaitable_compound_statement;
      } ->
         let acc = f acc awaitable_async in
         let acc = f acc awaitable_coroutine in
         let acc = f acc awaitable_compound_statement in
         acc
      | XHPChildrenDeclaration {
        xhp_children_keyword;
        xhp_children_expression;
        xhp_children_semicolon;
      } ->
         let acc = f acc xhp_children_keyword in
         let acc = f acc xhp_children_expression in
         let acc = f acc xhp_children_semicolon in
         acc
      | XHPChildrenParenthesizedList {
        xhp_children_list_left_paren;
        xhp_children_list_xhp_children;
        xhp_children_list_right_paren;
      } ->
         let acc = f acc xhp_children_list_left_paren in
         let acc = f acc xhp_children_list_xhp_children in
         let acc = f acc xhp_children_list_right_paren in
         acc
      | XHPCategoryDeclaration {
        xhp_category_keyword;
        xhp_category_categories;
        xhp_category_semicolon;
      } ->
         let acc = f acc xhp_category_keyword in
         let acc = f acc xhp_category_categories in
         let acc = f acc xhp_category_semicolon in
         acc
      | XHPEnumType {
        xhp_enum_keyword;
        xhp_enum_left_brace;
        xhp_enum_values;
        xhp_enum_right_brace;
      } ->
         let acc = f acc xhp_enum_keyword in
         let acc = f acc xhp_enum_left_brace in
         let acc = f acc xhp_enum_values in
         let acc = f acc xhp_enum_right_brace in
         acc
      | XHPRequired {
        xhp_required_at;
        xhp_required_keyword;
      } ->
         let acc = f acc xhp_required_at in
         let acc = f acc xhp_required_keyword in
         acc
      | XHPClassAttributeDeclaration {
        xhp_attribute_keyword;
        xhp_attribute_attributes;
        xhp_attribute_semicolon;
      } ->
         let acc = f acc xhp_attribute_keyword in
         let acc = f acc xhp_attribute_attributes in
         let acc = f acc xhp_attribute_semicolon in
         acc
      | XHPClassAttribute {
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
      | XHPSimpleClassAttribute {
        xhp_simple_class_attribute_type;
      } ->
         let acc = f acc xhp_simple_class_attribute_type in
         acc
      | XHPAttribute {
        xhp_attribute_name;
        xhp_attribute_equal;
        xhp_attribute_expression;
      } ->
         let acc = f acc xhp_attribute_name in
         let acc = f acc xhp_attribute_equal in
         let acc = f acc xhp_attribute_expression in
         acc
      | XHPOpen {
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
      | XHPExpression {
        xhp_open;
        xhp_body;
        xhp_close;
      } ->
         let acc = f acc xhp_open in
         let acc = f acc xhp_body in
         let acc = f acc xhp_close in
         acc
      | XHPClose {
        xhp_close_left_angle;
        xhp_close_name;
        xhp_close_right_angle;
      } ->
         let acc = f acc xhp_close_left_angle in
         let acc = f acc xhp_close_name in
         let acc = f acc xhp_close_right_angle in
         acc
      | TypeConstant {
        type_constant_left_type;
        type_constant_separator;
        type_constant_right_type;
      } ->
         let acc = f acc type_constant_left_type in
         let acc = f acc type_constant_separator in
         let acc = f acc type_constant_right_type in
         acc
      | VectorTypeSpecifier {
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
      | KeysetTypeSpecifier {
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
      | TupleTypeExplicitSpecifier {
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
      | VarrayTypeSpecifier {
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
      | VectorArrayTypeSpecifier {
        vector_array_keyword;
        vector_array_left_angle;
        vector_array_type;
        vector_array_right_angle;
      } ->
         let acc = f acc vector_array_keyword in
         let acc = f acc vector_array_left_angle in
         let acc = f acc vector_array_type in
         let acc = f acc vector_array_right_angle in
         acc
      | TypeParameter {
        type_variance;
        type_name;
        type_constraints;
      } ->
         let acc = f acc type_variance in
         let acc = f acc type_name in
         let acc = f acc type_constraints in
         acc
      | TypeConstraint {
        constraint_keyword;
        constraint_type;
      } ->
         let acc = f acc constraint_keyword in
         let acc = f acc constraint_type in
         acc
      | DarrayTypeSpecifier {
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
      | MapArrayTypeSpecifier {
        map_array_keyword;
        map_array_left_angle;
        map_array_key;
        map_array_comma;
        map_array_value;
        map_array_right_angle;
      } ->
         let acc = f acc map_array_keyword in
         let acc = f acc map_array_left_angle in
         let acc = f acc map_array_key in
         let acc = f acc map_array_comma in
         let acc = f acc map_array_value in
         let acc = f acc map_array_right_angle in
         acc
      | DictionaryTypeSpecifier {
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
      | ClosureTypeSpecifier {
        closure_outer_left_paren;
        closure_coroutine;
        closure_function_keyword;
        closure_inner_left_paren;
        closure_parameter_types;
        closure_inner_right_paren;
        closure_colon;
        closure_return_type;
        closure_outer_right_paren;
      } ->
         let acc = f acc closure_outer_left_paren in
         let acc = f acc closure_coroutine in
         let acc = f acc closure_function_keyword in
         let acc = f acc closure_inner_left_paren in
         let acc = f acc closure_parameter_types in
         let acc = f acc closure_inner_right_paren in
         let acc = f acc closure_colon in
         let acc = f acc closure_return_type in
         let acc = f acc closure_outer_right_paren in
         acc
      | ClassnameTypeSpecifier {
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
      | FieldSpecifier {
        field_question;
        field_name;
        field_arrow;
        field_type;
      } ->
         let acc = f acc field_question in
         let acc = f acc field_name in
         let acc = f acc field_arrow in
         let acc = f acc field_type in
         acc
      | FieldInitializer {
        field_initializer_name;
        field_initializer_arrow;
        field_initializer_value;
      } ->
         let acc = f acc field_initializer_name in
         let acc = f acc field_initializer_arrow in
         let acc = f acc field_initializer_value in
         acc
      | ShapeTypeSpecifier {
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
      | ShapeExpression {
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
      | TupleExpression {
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
      | GenericTypeSpecifier {
        generic_class_type;
        generic_argument_list;
      } ->
         let acc = f acc generic_class_type in
         let acc = f acc generic_argument_list in
         acc
      | NullableTypeSpecifier {
        nullable_question;
        nullable_type;
      } ->
         let acc = f acc nullable_question in
         let acc = f acc nullable_type in
         acc
      | SoftTypeSpecifier {
        soft_at;
        soft_type;
      } ->
         let acc = f acc soft_at in
         let acc = f acc soft_type in
         acc
      | TypeArguments {
        type_arguments_left_angle;
        type_arguments_types;
        type_arguments_right_angle;
      } ->
         let acc = f acc type_arguments_left_angle in
         let acc = f acc type_arguments_types in
         let acc = f acc type_arguments_right_angle in
         acc
      | TypeParameters {
        type_parameters_left_angle;
        type_parameters_parameters;
        type_parameters_right_angle;
      } ->
         let acc = f acc type_parameters_left_angle in
         let acc = f acc type_parameters_parameters in
         let acc = f acc type_parameters_right_angle in
         acc
      | TupleTypeSpecifier {
        tuple_left_paren;
        tuple_types;
        tuple_right_paren;
      } ->
         let acc = f acc tuple_left_paren in
         let acc = f acc tuple_types in
         let acc = f acc tuple_right_paren in
         acc
      | ErrorSyntax {
        error_error;
      } ->
         let acc = f acc error_error in
         acc
      | ListItem {
        list_item;
        list_separator;
      } ->
         let acc = f acc list_item in
         let acc = f acc list_separator in
         acc


    (* The order that the children are returned in should match the order
       that they appear in the source text *)
    let children_from_syntax s =
      match s with
      | Missing -> []
      | Token _ -> []
      | SyntaxList x -> x
      | EndOfFile {
        end_of_file_token;
      } -> [
        end_of_file_token;
      ]
      | Script {
        script_declarations;
      } -> [
        script_declarations;
      ]
      | SimpleTypeSpecifier {
        simple_type_specifier;
      } -> [
        simple_type_specifier;
      ]
      | LiteralExpression {
        literal_expression;
      } -> [
        literal_expression;
      ]
      | VariableExpression {
        variable_expression;
      } -> [
        variable_expression;
      ]
      | QualifiedNameExpression {
        qualified_name_expression;
      } -> [
        qualified_name_expression;
      ]
      | PipeVariableExpression {
        pipe_variable_expression;
      } -> [
        pipe_variable_expression;
      ]
      | EnumDeclaration {
        enum_attribute_spec;
        enum_keyword;
        enum_name;
        enum_colon;
        enum_base;
        enum_type;
        enum_left_brace;
        enum_enumerators;
        enum_right_brace;
      } -> [
        enum_attribute_spec;
        enum_keyword;
        enum_name;
        enum_colon;
        enum_base;
        enum_type;
        enum_left_brace;
        enum_enumerators;
        enum_right_brace;
      ]
      | Enumerator {
        enumerator_name;
        enumerator_equal;
        enumerator_value;
        enumerator_semicolon;
      } -> [
        enumerator_name;
        enumerator_equal;
        enumerator_value;
        enumerator_semicolon;
      ]
      | AliasDeclaration {
        alias_attribute_spec;
        alias_keyword;
        alias_name;
        alias_generic_parameter;
        alias_constraint;
        alias_equal;
        alias_type;
        alias_semicolon;
      } -> [
        alias_attribute_spec;
        alias_keyword;
        alias_name;
        alias_generic_parameter;
        alias_constraint;
        alias_equal;
        alias_type;
        alias_semicolon;
      ]
      | PropertyDeclaration {
        property_modifiers;
        property_type;
        property_declarators;
        property_semicolon;
      } -> [
        property_modifiers;
        property_type;
        property_declarators;
        property_semicolon;
      ]
      | PropertyDeclarator {
        property_name;
        property_initializer;
      } -> [
        property_name;
        property_initializer;
      ]
      | NamespaceDeclaration {
        namespace_keyword;
        namespace_name;
        namespace_body;
      } -> [
        namespace_keyword;
        namespace_name;
        namespace_body;
      ]
      | NamespaceBody {
        namespace_left_brace;
        namespace_declarations;
        namespace_right_brace;
      } -> [
        namespace_left_brace;
        namespace_declarations;
        namespace_right_brace;
      ]
      | NamespaceEmptyBody {
        namespace_semicolon;
      } -> [
        namespace_semicolon;
      ]
      | NamespaceUseDeclaration {
        namespace_use_keyword;
        namespace_use_kind;
        namespace_use_clauses;
        namespace_use_semicolon;
      } -> [
        namespace_use_keyword;
        namespace_use_kind;
        namespace_use_clauses;
        namespace_use_semicolon;
      ]
      | NamespaceGroupUseDeclaration {
        namespace_group_use_keyword;
        namespace_group_use_kind;
        namespace_group_use_prefix;
        namespace_group_use_left_brace;
        namespace_group_use_clauses;
        namespace_group_use_right_brace;
        namespace_group_use_semicolon;
      } -> [
        namespace_group_use_keyword;
        namespace_group_use_kind;
        namespace_group_use_prefix;
        namespace_group_use_left_brace;
        namespace_group_use_clauses;
        namespace_group_use_right_brace;
        namespace_group_use_semicolon;
      ]
      | NamespaceUseClause {
        namespace_use_clause_kind;
        namespace_use_name;
        namespace_use_as;
        namespace_use_alias;
      } -> [
        namespace_use_clause_kind;
        namespace_use_name;
        namespace_use_as;
        namespace_use_alias;
      ]
      | FunctionDeclaration {
        function_attribute_spec;
        function_declaration_header;
        function_body;
      } -> [
        function_attribute_spec;
        function_declaration_header;
        function_body;
      ]
      | FunctionDeclarationHeader {
        function_async;
        function_coroutine;
        function_keyword;
        function_ampersand;
        function_name;
        function_type_parameter_list;
        function_left_paren;
        function_parameter_list;
        function_right_paren;
        function_colon;
        function_type;
        function_where_clause;
      } -> [
        function_async;
        function_coroutine;
        function_keyword;
        function_ampersand;
        function_name;
        function_type_parameter_list;
        function_left_paren;
        function_parameter_list;
        function_right_paren;
        function_colon;
        function_type;
        function_where_clause;
      ]
      | WhereClause {
        where_clause_keyword;
        where_clause_constraints;
      } -> [
        where_clause_keyword;
        where_clause_constraints;
      ]
      | WhereConstraint {
        where_constraint_left_type;
        where_constraint_operator;
        where_constraint_right_type;
      } -> [
        where_constraint_left_type;
        where_constraint_operator;
        where_constraint_right_type;
      ]
      | MethodishDeclaration {
        methodish_attribute;
        methodish_modifiers;
        methodish_function_decl_header;
        methodish_function_body;
        methodish_semicolon;
      } -> [
        methodish_attribute;
        methodish_modifiers;
        methodish_function_decl_header;
        methodish_function_body;
        methodish_semicolon;
      ]
      | ClassishDeclaration {
        classish_attribute;
        classish_modifiers;
        classish_keyword;
        classish_name;
        classish_type_parameters;
        classish_extends_keyword;
        classish_extends_list;
        classish_implements_keyword;
        classish_implements_list;
        classish_body;
      } -> [
        classish_attribute;
        classish_modifiers;
        classish_keyword;
        classish_name;
        classish_type_parameters;
        classish_extends_keyword;
        classish_extends_list;
        classish_implements_keyword;
        classish_implements_list;
        classish_body;
      ]
      | ClassishBody {
        classish_body_left_brace;
        classish_body_elements;
        classish_body_right_brace;
      } -> [
        classish_body_left_brace;
        classish_body_elements;
        classish_body_right_brace;
      ]
      | TraitUsePrecedenceItem {
        trait_use_precedence_item_name;
        trait_use_precedence_item_keyword;
        trait_use_precedence_item_removed_names;
      } -> [
        trait_use_precedence_item_name;
        trait_use_precedence_item_keyword;
        trait_use_precedence_item_removed_names;
      ]
      | TraitUseAliasItem {
        trait_use_alias_item_aliasing_name;
        trait_use_alias_item_keyword;
        trait_use_alias_item_visibility;
        trait_use_alias_item_aliased_name;
      } -> [
        trait_use_alias_item_aliasing_name;
        trait_use_alias_item_keyword;
        trait_use_alias_item_visibility;
        trait_use_alias_item_aliased_name;
      ]
      | TraitUseConflictResolution {
        trait_use_conflict_resolution_keyword;
        trait_use_conflict_resolution_names;
        trait_use_conflict_resolution_left_brace;
        trait_use_conflict_resolution_clauses;
        trait_use_conflict_resolution_right_brace;
      } -> [
        trait_use_conflict_resolution_keyword;
        trait_use_conflict_resolution_names;
        trait_use_conflict_resolution_left_brace;
        trait_use_conflict_resolution_clauses;
        trait_use_conflict_resolution_right_brace;
      ]
      | TraitUse {
        trait_use_keyword;
        trait_use_names;
        trait_use_semicolon;
      } -> [
        trait_use_keyword;
        trait_use_names;
        trait_use_semicolon;
      ]
      | RequireClause {
        require_keyword;
        require_kind;
        require_name;
        require_semicolon;
      } -> [
        require_keyword;
        require_kind;
        require_name;
        require_semicolon;
      ]
      | ConstDeclaration {
        const_abstract;
        const_keyword;
        const_type_specifier;
        const_declarators;
        const_semicolon;
      } -> [
        const_abstract;
        const_keyword;
        const_type_specifier;
        const_declarators;
        const_semicolon;
      ]
      | ConstantDeclarator {
        constant_declarator_name;
        constant_declarator_initializer;
      } -> [
        constant_declarator_name;
        constant_declarator_initializer;
      ]
      | TypeConstDeclaration {
        type_const_abstract;
        type_const_keyword;
        type_const_type_keyword;
        type_const_name;
        type_const_type_constraint;
        type_const_equal;
        type_const_type_specifier;
        type_const_semicolon;
      } -> [
        type_const_abstract;
        type_const_keyword;
        type_const_type_keyword;
        type_const_name;
        type_const_type_constraint;
        type_const_equal;
        type_const_type_specifier;
        type_const_semicolon;
      ]
      | DecoratedExpression {
        decorated_expression_decorator;
        decorated_expression_expression;
      } -> [
        decorated_expression_decorator;
        decorated_expression_expression;
      ]
      | ParameterDeclaration {
        parameter_attribute;
        parameter_visibility;
        parameter_call_convention;
        parameter_type;
        parameter_name;
        parameter_default_value;
      } -> [
        parameter_attribute;
        parameter_visibility;
        parameter_call_convention;
        parameter_type;
        parameter_name;
        parameter_default_value;
      ]
      | VariadicParameter {
        variadic_parameter_type;
        variadic_parameter_ellipsis;
      } -> [
        variadic_parameter_type;
        variadic_parameter_ellipsis;
      ]
      | AttributeSpecification {
        attribute_specification_left_double_angle;
        attribute_specification_attributes;
        attribute_specification_right_double_angle;
      } -> [
        attribute_specification_left_double_angle;
        attribute_specification_attributes;
        attribute_specification_right_double_angle;
      ]
      | Attribute {
        attribute_name;
        attribute_left_paren;
        attribute_values;
        attribute_right_paren;
      } -> [
        attribute_name;
        attribute_left_paren;
        attribute_values;
        attribute_right_paren;
      ]
      | InclusionExpression {
        inclusion_require;
        inclusion_filename;
      } -> [
        inclusion_require;
        inclusion_filename;
      ]
      | InclusionDirective {
        inclusion_expression;
        inclusion_semicolon;
      } -> [
        inclusion_expression;
        inclusion_semicolon;
      ]
      | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      } -> [
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      ]
      | ExpressionStatement {
        expression_statement_expression;
        expression_statement_semicolon;
      } -> [
        expression_statement_expression;
        expression_statement_semicolon;
      ]
      | MarkupSection {
        markup_prefix;
        markup_text;
        markup_suffix;
        markup_expression;
      } -> [
        markup_prefix;
        markup_text;
        markup_suffix;
        markup_expression;
      ]
      | MarkupSuffix {
        markup_suffix_less_than_question;
        markup_suffix_name;
      } -> [
        markup_suffix_less_than_question;
        markup_suffix_name;
      ]
      | UnsetStatement {
        unset_keyword;
        unset_left_paren;
        unset_variables;
        unset_right_paren;
        unset_semicolon;
      } -> [
        unset_keyword;
        unset_left_paren;
        unset_variables;
        unset_right_paren;
        unset_semicolon;
      ]
      | UsingStatementBlockScoped {
        using_block_await_keyword;
        using_block_using_keyword;
        using_block_left_paren;
        using_block_expressions;
        using_block_right_paren;
        using_block_body;
      } -> [
        using_block_await_keyword;
        using_block_using_keyword;
        using_block_left_paren;
        using_block_expressions;
        using_block_right_paren;
        using_block_body;
      ]
      | UsingStatementFunctionScoped {
        using_function_await_keyword;
        using_function_using_keyword;
        using_function_expression;
        using_function_semicolon;
      } -> [
        using_function_await_keyword;
        using_function_using_keyword;
        using_function_expression;
        using_function_semicolon;
      ]
      | WhileStatement {
        while_keyword;
        while_left_paren;
        while_condition;
        while_right_paren;
        while_body;
      } -> [
        while_keyword;
        while_left_paren;
        while_condition;
        while_right_paren;
        while_body;
      ]
      | IfStatement {
        if_keyword;
        if_left_paren;
        if_condition;
        if_right_paren;
        if_statement;
        if_elseif_clauses;
        if_else_clause;
      } -> [
        if_keyword;
        if_left_paren;
        if_condition;
        if_right_paren;
        if_statement;
        if_elseif_clauses;
        if_else_clause;
      ]
      | ElseifClause {
        elseif_keyword;
        elseif_left_paren;
        elseif_condition;
        elseif_right_paren;
        elseif_statement;
      } -> [
        elseif_keyword;
        elseif_left_paren;
        elseif_condition;
        elseif_right_paren;
        elseif_statement;
      ]
      | ElseClause {
        else_keyword;
        else_statement;
      } -> [
        else_keyword;
        else_statement;
      ]
      | IfEndIfStatement {
        if_endif_keyword;
        if_endif_left_paren;
        if_endif_condition;
        if_endif_right_paren;
        if_endif_colon;
        if_endif_statement;
        if_endif_elseif_colon_clauses;
        if_endif_else_colon_clause;
        if_endif_endif_keyword;
        if_endif_semicolon;
      } -> [
        if_endif_keyword;
        if_endif_left_paren;
        if_endif_condition;
        if_endif_right_paren;
        if_endif_colon;
        if_endif_statement;
        if_endif_elseif_colon_clauses;
        if_endif_else_colon_clause;
        if_endif_endif_keyword;
        if_endif_semicolon;
      ]
      | ElseifColonClause {
        elseif_colon_keyword;
        elseif_colon_left_paren;
        elseif_colon_condition;
        elseif_colon_right_paren;
        elseif_colon_colon;
        elseif_colon_statement;
      } -> [
        elseif_colon_keyword;
        elseif_colon_left_paren;
        elseif_colon_condition;
        elseif_colon_right_paren;
        elseif_colon_colon;
        elseif_colon_statement;
      ]
      | ElseColonClause {
        else_colon_keyword;
        else_colon_colon;
        else_colon_statement;
      } -> [
        else_colon_keyword;
        else_colon_colon;
        else_colon_statement;
      ]
      | TryStatement {
        try_keyword;
        try_compound_statement;
        try_catch_clauses;
        try_finally_clause;
      } -> [
        try_keyword;
        try_compound_statement;
        try_catch_clauses;
        try_finally_clause;
      ]
      | CatchClause {
        catch_keyword;
        catch_left_paren;
        catch_type;
        catch_variable;
        catch_right_paren;
        catch_body;
      } -> [
        catch_keyword;
        catch_left_paren;
        catch_type;
        catch_variable;
        catch_right_paren;
        catch_body;
      ]
      | FinallyClause {
        finally_keyword;
        finally_body;
      } -> [
        finally_keyword;
        finally_body;
      ]
      | DoStatement {
        do_keyword;
        do_body;
        do_while_keyword;
        do_left_paren;
        do_condition;
        do_right_paren;
        do_semicolon;
      } -> [
        do_keyword;
        do_body;
        do_while_keyword;
        do_left_paren;
        do_condition;
        do_right_paren;
        do_semicolon;
      ]
      | ForStatement {
        for_keyword;
        for_left_paren;
        for_initializer;
        for_first_semicolon;
        for_control;
        for_second_semicolon;
        for_end_of_loop;
        for_right_paren;
        for_body;
      } -> [
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
      | ForeachStatement {
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
      } -> [
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
      | SwitchStatement {
        switch_keyword;
        switch_left_paren;
        switch_expression;
        switch_right_paren;
        switch_left_brace;
        switch_sections;
        switch_right_brace;
      } -> [
        switch_keyword;
        switch_left_paren;
        switch_expression;
        switch_right_paren;
        switch_left_brace;
        switch_sections;
        switch_right_brace;
      ]
      | SwitchSection {
        switch_section_labels;
        switch_section_statements;
        switch_section_fallthrough;
      } -> [
        switch_section_labels;
        switch_section_statements;
        switch_section_fallthrough;
      ]
      | SwitchFallthrough {
        fallthrough_keyword;
        fallthrough_semicolon;
      } -> [
        fallthrough_keyword;
        fallthrough_semicolon;
      ]
      | CaseLabel {
        case_keyword;
        case_expression;
        case_colon;
      } -> [
        case_keyword;
        case_expression;
        case_colon;
      ]
      | DefaultLabel {
        default_keyword;
        default_colon;
      } -> [
        default_keyword;
        default_colon;
      ]
      | ReturnStatement {
        return_keyword;
        return_expression;
        return_semicolon;
      } -> [
        return_keyword;
        return_expression;
        return_semicolon;
      ]
      | GotoLabel {
        goto_label_name;
        goto_label_colon;
      } -> [
        goto_label_name;
        goto_label_colon;
      ]
      | GotoStatement {
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon;
      } -> [
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon;
      ]
      | ThrowStatement {
        throw_keyword;
        throw_expression;
        throw_semicolon;
      } -> [
        throw_keyword;
        throw_expression;
        throw_semicolon;
      ]
      | BreakStatement {
        break_keyword;
        break_level;
        break_semicolon;
      } -> [
        break_keyword;
        break_level;
        break_semicolon;
      ]
      | ContinueStatement {
        continue_keyword;
        continue_level;
        continue_semicolon;
      } -> [
        continue_keyword;
        continue_level;
        continue_semicolon;
      ]
      | FunctionStaticStatement {
        static_static_keyword;
        static_declarations;
        static_semicolon;
      } -> [
        static_static_keyword;
        static_declarations;
        static_semicolon;
      ]
      | StaticDeclarator {
        static_name;
        static_initializer;
      } -> [
        static_name;
        static_initializer;
      ]
      | EchoStatement {
        echo_keyword;
        echo_expressions;
        echo_semicolon;
      } -> [
        echo_keyword;
        echo_expressions;
        echo_semicolon;
      ]
      | GlobalStatement {
        global_keyword;
        global_variables;
        global_semicolon;
      } -> [
        global_keyword;
        global_variables;
        global_semicolon;
      ]
      | SimpleInitializer {
        simple_initializer_equal;
        simple_initializer_value;
      } -> [
        simple_initializer_equal;
        simple_initializer_value;
      ]
      | AnonymousFunction {
        anonymous_static_keyword;
        anonymous_async_keyword;
        anonymous_coroutine_keyword;
        anonymous_function_keyword;
        anonymous_left_paren;
        anonymous_parameters;
        anonymous_right_paren;
        anonymous_colon;
        anonymous_type;
        anonymous_use;
        anonymous_body;
      } -> [
        anonymous_static_keyword;
        anonymous_async_keyword;
        anonymous_coroutine_keyword;
        anonymous_function_keyword;
        anonymous_left_paren;
        anonymous_parameters;
        anonymous_right_paren;
        anonymous_colon;
        anonymous_type;
        anonymous_use;
        anonymous_body;
      ]
      | AnonymousFunctionUseClause {
        anonymous_use_keyword;
        anonymous_use_left_paren;
        anonymous_use_variables;
        anonymous_use_right_paren;
      } -> [
        anonymous_use_keyword;
        anonymous_use_left_paren;
        anonymous_use_variables;
        anonymous_use_right_paren;
      ]
      | LambdaExpression {
        lambda_async;
        lambda_coroutine;
        lambda_signature;
        lambda_arrow;
        lambda_body;
      } -> [
        lambda_async;
        lambda_coroutine;
        lambda_signature;
        lambda_arrow;
        lambda_body;
      ]
      | LambdaSignature {
        lambda_left_paren;
        lambda_parameters;
        lambda_right_paren;
        lambda_colon;
        lambda_type;
      } -> [
        lambda_left_paren;
        lambda_parameters;
        lambda_right_paren;
        lambda_colon;
        lambda_type;
      ]
      | CastExpression {
        cast_left_paren;
        cast_type;
        cast_right_paren;
        cast_operand;
      } -> [
        cast_left_paren;
        cast_type;
        cast_right_paren;
        cast_operand;
      ]
      | ScopeResolutionExpression {
        scope_resolution_qualifier;
        scope_resolution_operator;
        scope_resolution_name;
      } -> [
        scope_resolution_qualifier;
        scope_resolution_operator;
        scope_resolution_name;
      ]
      | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name;
      } -> [
        member_object;
        member_operator;
        member_name;
      ]
      | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name;
      } -> [
        safe_member_object;
        safe_member_operator;
        safe_member_name;
      ]
      | EmbeddedMemberSelectionExpression {
        embedded_member_object;
        embedded_member_operator;
        embedded_member_name;
      } -> [
        embedded_member_object;
        embedded_member_operator;
        embedded_member_name;
      ]
      | YieldExpression {
        yield_keyword;
        yield_operand;
      } -> [
        yield_keyword;
        yield_operand;
      ]
      | YieldFromExpression {
        yield_from_yield_keyword;
        yield_from_from_keyword;
        yield_from_operand;
      } -> [
        yield_from_yield_keyword;
        yield_from_from_keyword;
        yield_from_operand;
      ]
      | PrefixUnaryExpression {
        prefix_unary_operator;
        prefix_unary_operand;
      } -> [
        prefix_unary_operator;
        prefix_unary_operand;
      ]
      | PostfixUnaryExpression {
        postfix_unary_operand;
        postfix_unary_operator;
      } -> [
        postfix_unary_operand;
        postfix_unary_operator;
      ]
      | BinaryExpression {
        binary_left_operand;
        binary_operator;
        binary_right_operand;
      } -> [
        binary_left_operand;
        binary_operator;
        binary_right_operand;
      ]
      | InstanceofExpression {
        instanceof_left_operand;
        instanceof_operator;
        instanceof_right_operand;
      } -> [
        instanceof_left_operand;
        instanceof_operator;
        instanceof_right_operand;
      ]
      | IsExpression {
        is_left_operand;
        is_operator;
        is_right_operand;
      } -> [
        is_left_operand;
        is_operator;
        is_right_operand;
      ]
      | ConditionalExpression {
        conditional_test;
        conditional_question;
        conditional_consequence;
        conditional_colon;
        conditional_alternative;
      } -> [
        conditional_test;
        conditional_question;
        conditional_consequence;
        conditional_colon;
        conditional_alternative;
      ]
      | EvalExpression {
        eval_keyword;
        eval_left_paren;
        eval_argument;
        eval_right_paren;
      } -> [
        eval_keyword;
        eval_left_paren;
        eval_argument;
        eval_right_paren;
      ]
      | EmptyExpression {
        empty_keyword;
        empty_left_paren;
        empty_argument;
        empty_right_paren;
      } -> [
        empty_keyword;
        empty_left_paren;
        empty_argument;
        empty_right_paren;
      ]
      | DefineExpression {
        define_keyword;
        define_left_paren;
        define_argument_list;
        define_right_paren;
      } -> [
        define_keyword;
        define_left_paren;
        define_argument_list;
        define_right_paren;
      ]
      | IssetExpression {
        isset_keyword;
        isset_left_paren;
        isset_argument_list;
        isset_right_paren;
      } -> [
        isset_keyword;
        isset_left_paren;
        isset_argument_list;
        isset_right_paren;
      ]
      | FunctionCallExpression {
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren;
      } -> [
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren;
      ]
      | FunctionCallWithTypeArgumentsExpression {
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren;
      } -> [
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren;
      ]
      | ParenthesizedExpression {
        parenthesized_expression_left_paren;
        parenthesized_expression_expression;
        parenthesized_expression_right_paren;
      } -> [
        parenthesized_expression_left_paren;
        parenthesized_expression_expression;
        parenthesized_expression_right_paren;
      ]
      | BracedExpression {
        braced_expression_left_brace;
        braced_expression_expression;
        braced_expression_right_brace;
      } -> [
        braced_expression_left_brace;
        braced_expression_expression;
        braced_expression_right_brace;
      ]
      | EmbeddedBracedExpression {
        embedded_braced_expression_left_brace;
        embedded_braced_expression_expression;
        embedded_braced_expression_right_brace;
      } -> [
        embedded_braced_expression_left_brace;
        embedded_braced_expression_expression;
        embedded_braced_expression_right_brace;
      ]
      | ListExpression {
        list_keyword;
        list_left_paren;
        list_members;
        list_right_paren;
      } -> [
        list_keyword;
        list_left_paren;
        list_members;
        list_right_paren;
      ]
      | CollectionLiteralExpression {
        collection_literal_name;
        collection_literal_left_brace;
        collection_literal_initializers;
        collection_literal_right_brace;
      } -> [
        collection_literal_name;
        collection_literal_left_brace;
        collection_literal_initializers;
        collection_literal_right_brace;
      ]
      | ObjectCreationExpression {
        object_creation_new_keyword;
        object_creation_type;
        object_creation_left_paren;
        object_creation_argument_list;
        object_creation_right_paren;
      } -> [
        object_creation_new_keyword;
        object_creation_type;
        object_creation_left_paren;
        object_creation_argument_list;
        object_creation_right_paren;
      ]
      | ArrayCreationExpression {
        array_creation_left_bracket;
        array_creation_members;
        array_creation_right_bracket;
      } -> [
        array_creation_left_bracket;
        array_creation_members;
        array_creation_right_bracket;
      ]
      | ArrayIntrinsicExpression {
        array_intrinsic_keyword;
        array_intrinsic_left_paren;
        array_intrinsic_members;
        array_intrinsic_right_paren;
      } -> [
        array_intrinsic_keyword;
        array_intrinsic_left_paren;
        array_intrinsic_members;
        array_intrinsic_right_paren;
      ]
      | DarrayIntrinsicExpression {
        darray_intrinsic_keyword;
        darray_intrinsic_left_bracket;
        darray_intrinsic_members;
        darray_intrinsic_right_bracket;
      } -> [
        darray_intrinsic_keyword;
        darray_intrinsic_left_bracket;
        darray_intrinsic_members;
        darray_intrinsic_right_bracket;
      ]
      | DictionaryIntrinsicExpression {
        dictionary_intrinsic_keyword;
        dictionary_intrinsic_left_bracket;
        dictionary_intrinsic_members;
        dictionary_intrinsic_right_bracket;
      } -> [
        dictionary_intrinsic_keyword;
        dictionary_intrinsic_left_bracket;
        dictionary_intrinsic_members;
        dictionary_intrinsic_right_bracket;
      ]
      | KeysetIntrinsicExpression {
        keyset_intrinsic_keyword;
        keyset_intrinsic_left_bracket;
        keyset_intrinsic_members;
        keyset_intrinsic_right_bracket;
      } -> [
        keyset_intrinsic_keyword;
        keyset_intrinsic_left_bracket;
        keyset_intrinsic_members;
        keyset_intrinsic_right_bracket;
      ]
      | VarrayIntrinsicExpression {
        varray_intrinsic_keyword;
        varray_intrinsic_left_bracket;
        varray_intrinsic_members;
        varray_intrinsic_right_bracket;
      } -> [
        varray_intrinsic_keyword;
        varray_intrinsic_left_bracket;
        varray_intrinsic_members;
        varray_intrinsic_right_bracket;
      ]
      | VectorIntrinsicExpression {
        vector_intrinsic_keyword;
        vector_intrinsic_left_bracket;
        vector_intrinsic_members;
        vector_intrinsic_right_bracket;
      } -> [
        vector_intrinsic_keyword;
        vector_intrinsic_left_bracket;
        vector_intrinsic_members;
        vector_intrinsic_right_bracket;
      ]
      | ElementInitializer {
        element_key;
        element_arrow;
        element_value;
      } -> [
        element_key;
        element_arrow;
        element_value;
      ]
      | SubscriptExpression {
        subscript_receiver;
        subscript_left_bracket;
        subscript_index;
        subscript_right_bracket;
      } -> [
        subscript_receiver;
        subscript_left_bracket;
        subscript_index;
        subscript_right_bracket;
      ]
      | EmbeddedSubscriptExpression {
        embedded_subscript_receiver;
        embedded_subscript_left_bracket;
        embedded_subscript_index;
        embedded_subscript_right_bracket;
      } -> [
        embedded_subscript_receiver;
        embedded_subscript_left_bracket;
        embedded_subscript_index;
        embedded_subscript_right_bracket;
      ]
      | AwaitableCreationExpression {
        awaitable_async;
        awaitable_coroutine;
        awaitable_compound_statement;
      } -> [
        awaitable_async;
        awaitable_coroutine;
        awaitable_compound_statement;
      ]
      | XHPChildrenDeclaration {
        xhp_children_keyword;
        xhp_children_expression;
        xhp_children_semicolon;
      } -> [
        xhp_children_keyword;
        xhp_children_expression;
        xhp_children_semicolon;
      ]
      | XHPChildrenParenthesizedList {
        xhp_children_list_left_paren;
        xhp_children_list_xhp_children;
        xhp_children_list_right_paren;
      } -> [
        xhp_children_list_left_paren;
        xhp_children_list_xhp_children;
        xhp_children_list_right_paren;
      ]
      | XHPCategoryDeclaration {
        xhp_category_keyword;
        xhp_category_categories;
        xhp_category_semicolon;
      } -> [
        xhp_category_keyword;
        xhp_category_categories;
        xhp_category_semicolon;
      ]
      | XHPEnumType {
        xhp_enum_keyword;
        xhp_enum_left_brace;
        xhp_enum_values;
        xhp_enum_right_brace;
      } -> [
        xhp_enum_keyword;
        xhp_enum_left_brace;
        xhp_enum_values;
        xhp_enum_right_brace;
      ]
      | XHPRequired {
        xhp_required_at;
        xhp_required_keyword;
      } -> [
        xhp_required_at;
        xhp_required_keyword;
      ]
      | XHPClassAttributeDeclaration {
        xhp_attribute_keyword;
        xhp_attribute_attributes;
        xhp_attribute_semicolon;
      } -> [
        xhp_attribute_keyword;
        xhp_attribute_attributes;
        xhp_attribute_semicolon;
      ]
      | XHPClassAttribute {
        xhp_attribute_decl_type;
        xhp_attribute_decl_name;
        xhp_attribute_decl_initializer;
        xhp_attribute_decl_required;
      } -> [
        xhp_attribute_decl_type;
        xhp_attribute_decl_name;
        xhp_attribute_decl_initializer;
        xhp_attribute_decl_required;
      ]
      | XHPSimpleClassAttribute {
        xhp_simple_class_attribute_type;
      } -> [
        xhp_simple_class_attribute_type;
      ]
      | XHPAttribute {
        xhp_attribute_name;
        xhp_attribute_equal;
        xhp_attribute_expression;
      } -> [
        xhp_attribute_name;
        xhp_attribute_equal;
        xhp_attribute_expression;
      ]
      | XHPOpen {
        xhp_open_left_angle;
        xhp_open_name;
        xhp_open_attributes;
        xhp_open_right_angle;
      } -> [
        xhp_open_left_angle;
        xhp_open_name;
        xhp_open_attributes;
        xhp_open_right_angle;
      ]
      | XHPExpression {
        xhp_open;
        xhp_body;
        xhp_close;
      } -> [
        xhp_open;
        xhp_body;
        xhp_close;
      ]
      | XHPClose {
        xhp_close_left_angle;
        xhp_close_name;
        xhp_close_right_angle;
      } -> [
        xhp_close_left_angle;
        xhp_close_name;
        xhp_close_right_angle;
      ]
      | TypeConstant {
        type_constant_left_type;
        type_constant_separator;
        type_constant_right_type;
      } -> [
        type_constant_left_type;
        type_constant_separator;
        type_constant_right_type;
      ]
      | VectorTypeSpecifier {
        vector_type_keyword;
        vector_type_left_angle;
        vector_type_type;
        vector_type_trailing_comma;
        vector_type_right_angle;
      } -> [
        vector_type_keyword;
        vector_type_left_angle;
        vector_type_type;
        vector_type_trailing_comma;
        vector_type_right_angle;
      ]
      | KeysetTypeSpecifier {
        keyset_type_keyword;
        keyset_type_left_angle;
        keyset_type_type;
        keyset_type_trailing_comma;
        keyset_type_right_angle;
      } -> [
        keyset_type_keyword;
        keyset_type_left_angle;
        keyset_type_type;
        keyset_type_trailing_comma;
        keyset_type_right_angle;
      ]
      | TupleTypeExplicitSpecifier {
        tuple_type_keyword;
        tuple_type_left_angle;
        tuple_type_types;
        tuple_type_right_angle;
      } -> [
        tuple_type_keyword;
        tuple_type_left_angle;
        tuple_type_types;
        tuple_type_right_angle;
      ]
      | VarrayTypeSpecifier {
        varray_keyword;
        varray_left_angle;
        varray_type;
        varray_trailing_comma;
        varray_right_angle;
      } -> [
        varray_keyword;
        varray_left_angle;
        varray_type;
        varray_trailing_comma;
        varray_right_angle;
      ]
      | VectorArrayTypeSpecifier {
        vector_array_keyword;
        vector_array_left_angle;
        vector_array_type;
        vector_array_right_angle;
      } -> [
        vector_array_keyword;
        vector_array_left_angle;
        vector_array_type;
        vector_array_right_angle;
      ]
      | TypeParameter {
        type_variance;
        type_name;
        type_constraints;
      } -> [
        type_variance;
        type_name;
        type_constraints;
      ]
      | TypeConstraint {
        constraint_keyword;
        constraint_type;
      } -> [
        constraint_keyword;
        constraint_type;
      ]
      | DarrayTypeSpecifier {
        darray_keyword;
        darray_left_angle;
        darray_key;
        darray_comma;
        darray_value;
        darray_trailing_comma;
        darray_right_angle;
      } -> [
        darray_keyword;
        darray_left_angle;
        darray_key;
        darray_comma;
        darray_value;
        darray_trailing_comma;
        darray_right_angle;
      ]
      | MapArrayTypeSpecifier {
        map_array_keyword;
        map_array_left_angle;
        map_array_key;
        map_array_comma;
        map_array_value;
        map_array_right_angle;
      } -> [
        map_array_keyword;
        map_array_left_angle;
        map_array_key;
        map_array_comma;
        map_array_value;
        map_array_right_angle;
      ]
      | DictionaryTypeSpecifier {
        dictionary_type_keyword;
        dictionary_type_left_angle;
        dictionary_type_members;
        dictionary_type_right_angle;
      } -> [
        dictionary_type_keyword;
        dictionary_type_left_angle;
        dictionary_type_members;
        dictionary_type_right_angle;
      ]
      | ClosureTypeSpecifier {
        closure_outer_left_paren;
        closure_coroutine;
        closure_function_keyword;
        closure_inner_left_paren;
        closure_parameter_types;
        closure_inner_right_paren;
        closure_colon;
        closure_return_type;
        closure_outer_right_paren;
      } -> [
        closure_outer_left_paren;
        closure_coroutine;
        closure_function_keyword;
        closure_inner_left_paren;
        closure_parameter_types;
        closure_inner_right_paren;
        closure_colon;
        closure_return_type;
        closure_outer_right_paren;
      ]
      | ClassnameTypeSpecifier {
        classname_keyword;
        classname_left_angle;
        classname_type;
        classname_trailing_comma;
        classname_right_angle;
      } -> [
        classname_keyword;
        classname_left_angle;
        classname_type;
        classname_trailing_comma;
        classname_right_angle;
      ]
      | FieldSpecifier {
        field_question;
        field_name;
        field_arrow;
        field_type;
      } -> [
        field_question;
        field_name;
        field_arrow;
        field_type;
      ]
      | FieldInitializer {
        field_initializer_name;
        field_initializer_arrow;
        field_initializer_value;
      } -> [
        field_initializer_name;
        field_initializer_arrow;
        field_initializer_value;
      ]
      | ShapeTypeSpecifier {
        shape_type_keyword;
        shape_type_left_paren;
        shape_type_fields;
        shape_type_ellipsis;
        shape_type_right_paren;
      } -> [
        shape_type_keyword;
        shape_type_left_paren;
        shape_type_fields;
        shape_type_ellipsis;
        shape_type_right_paren;
      ]
      | ShapeExpression {
        shape_expression_keyword;
        shape_expression_left_paren;
        shape_expression_fields;
        shape_expression_right_paren;
      } -> [
        shape_expression_keyword;
        shape_expression_left_paren;
        shape_expression_fields;
        shape_expression_right_paren;
      ]
      | TupleExpression {
        tuple_expression_keyword;
        tuple_expression_left_paren;
        tuple_expression_items;
        tuple_expression_right_paren;
      } -> [
        tuple_expression_keyword;
        tuple_expression_left_paren;
        tuple_expression_items;
        tuple_expression_right_paren;
      ]
      | GenericTypeSpecifier {
        generic_class_type;
        generic_argument_list;
      } -> [
        generic_class_type;
        generic_argument_list;
      ]
      | NullableTypeSpecifier {
        nullable_question;
        nullable_type;
      } -> [
        nullable_question;
        nullable_type;
      ]
      | SoftTypeSpecifier {
        soft_at;
        soft_type;
      } -> [
        soft_at;
        soft_type;
      ]
      | TypeArguments {
        type_arguments_left_angle;
        type_arguments_types;
        type_arguments_right_angle;
      } -> [
        type_arguments_left_angle;
        type_arguments_types;
        type_arguments_right_angle;
      ]
      | TypeParameters {
        type_parameters_left_angle;
        type_parameters_parameters;
        type_parameters_right_angle;
      } -> [
        type_parameters_left_angle;
        type_parameters_parameters;
        type_parameters_right_angle;
      ]
      | TupleTypeSpecifier {
        tuple_left_paren;
        tuple_types;
        tuple_right_paren;
      } -> [
        tuple_left_paren;
        tuple_types;
        tuple_right_paren;
      ]
      | ErrorSyntax {
        error_error;
      } -> [
        error_error;
      ]
      | ListItem {
        list_item;
        list_separator;
      } -> [
        list_item;
        list_separator;
      ]


    let children node =
      children_from_syntax node.syntax

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList _ -> []
      | EndOfFile {
        end_of_file_token;
      } -> [
        "end_of_file_token";
      ]
      | Script {
        script_declarations;
      } -> [
        "script_declarations";
      ]
      | SimpleTypeSpecifier {
        simple_type_specifier;
      } -> [
        "simple_type_specifier";
      ]
      | LiteralExpression {
        literal_expression;
      } -> [
        "literal_expression";
      ]
      | VariableExpression {
        variable_expression;
      } -> [
        "variable_expression";
      ]
      | QualifiedNameExpression {
        qualified_name_expression;
      } -> [
        "qualified_name_expression";
      ]
      | PipeVariableExpression {
        pipe_variable_expression;
      } -> [
        "pipe_variable_expression";
      ]
      | EnumDeclaration {
        enum_attribute_spec;
        enum_keyword;
        enum_name;
        enum_colon;
        enum_base;
        enum_type;
        enum_left_brace;
        enum_enumerators;
        enum_right_brace;
      } -> [
        "enum_attribute_spec";
        "enum_keyword";
        "enum_name";
        "enum_colon";
        "enum_base";
        "enum_type";
        "enum_left_brace";
        "enum_enumerators";
        "enum_right_brace";
      ]
      | Enumerator {
        enumerator_name;
        enumerator_equal;
        enumerator_value;
        enumerator_semicolon;
      } -> [
        "enumerator_name";
        "enumerator_equal";
        "enumerator_value";
        "enumerator_semicolon";
      ]
      | AliasDeclaration {
        alias_attribute_spec;
        alias_keyword;
        alias_name;
        alias_generic_parameter;
        alias_constraint;
        alias_equal;
        alias_type;
        alias_semicolon;
      } -> [
        "alias_attribute_spec";
        "alias_keyword";
        "alias_name";
        "alias_generic_parameter";
        "alias_constraint";
        "alias_equal";
        "alias_type";
        "alias_semicolon";
      ]
      | PropertyDeclaration {
        property_modifiers;
        property_type;
        property_declarators;
        property_semicolon;
      } -> [
        "property_modifiers";
        "property_type";
        "property_declarators";
        "property_semicolon";
      ]
      | PropertyDeclarator {
        property_name;
        property_initializer;
      } -> [
        "property_name";
        "property_initializer";
      ]
      | NamespaceDeclaration {
        namespace_keyword;
        namespace_name;
        namespace_body;
      } -> [
        "namespace_keyword";
        "namespace_name";
        "namespace_body";
      ]
      | NamespaceBody {
        namespace_left_brace;
        namespace_declarations;
        namespace_right_brace;
      } -> [
        "namespace_left_brace";
        "namespace_declarations";
        "namespace_right_brace";
      ]
      | NamespaceEmptyBody {
        namespace_semicolon;
      } -> [
        "namespace_semicolon";
      ]
      | NamespaceUseDeclaration {
        namespace_use_keyword;
        namespace_use_kind;
        namespace_use_clauses;
        namespace_use_semicolon;
      } -> [
        "namespace_use_keyword";
        "namespace_use_kind";
        "namespace_use_clauses";
        "namespace_use_semicolon";
      ]
      | NamespaceGroupUseDeclaration {
        namespace_group_use_keyword;
        namespace_group_use_kind;
        namespace_group_use_prefix;
        namespace_group_use_left_brace;
        namespace_group_use_clauses;
        namespace_group_use_right_brace;
        namespace_group_use_semicolon;
      } -> [
        "namespace_group_use_keyword";
        "namespace_group_use_kind";
        "namespace_group_use_prefix";
        "namespace_group_use_left_brace";
        "namespace_group_use_clauses";
        "namespace_group_use_right_brace";
        "namespace_group_use_semicolon";
      ]
      | NamespaceUseClause {
        namespace_use_clause_kind;
        namespace_use_name;
        namespace_use_as;
        namespace_use_alias;
      } -> [
        "namespace_use_clause_kind";
        "namespace_use_name";
        "namespace_use_as";
        "namespace_use_alias";
      ]
      | FunctionDeclaration {
        function_attribute_spec;
        function_declaration_header;
        function_body;
      } -> [
        "function_attribute_spec";
        "function_declaration_header";
        "function_body";
      ]
      | FunctionDeclarationHeader {
        function_async;
        function_coroutine;
        function_keyword;
        function_ampersand;
        function_name;
        function_type_parameter_list;
        function_left_paren;
        function_parameter_list;
        function_right_paren;
        function_colon;
        function_type;
        function_where_clause;
      } -> [
        "function_async";
        "function_coroutine";
        "function_keyword";
        "function_ampersand";
        "function_name";
        "function_type_parameter_list";
        "function_left_paren";
        "function_parameter_list";
        "function_right_paren";
        "function_colon";
        "function_type";
        "function_where_clause";
      ]
      | WhereClause {
        where_clause_keyword;
        where_clause_constraints;
      } -> [
        "where_clause_keyword";
        "where_clause_constraints";
      ]
      | WhereConstraint {
        where_constraint_left_type;
        where_constraint_operator;
        where_constraint_right_type;
      } -> [
        "where_constraint_left_type";
        "where_constraint_operator";
        "where_constraint_right_type";
      ]
      | MethodishDeclaration {
        methodish_attribute;
        methodish_modifiers;
        methodish_function_decl_header;
        methodish_function_body;
        methodish_semicolon;
      } -> [
        "methodish_attribute";
        "methodish_modifiers";
        "methodish_function_decl_header";
        "methodish_function_body";
        "methodish_semicolon";
      ]
      | ClassishDeclaration {
        classish_attribute;
        classish_modifiers;
        classish_keyword;
        classish_name;
        classish_type_parameters;
        classish_extends_keyword;
        classish_extends_list;
        classish_implements_keyword;
        classish_implements_list;
        classish_body;
      } -> [
        "classish_attribute";
        "classish_modifiers";
        "classish_keyword";
        "classish_name";
        "classish_type_parameters";
        "classish_extends_keyword";
        "classish_extends_list";
        "classish_implements_keyword";
        "classish_implements_list";
        "classish_body";
      ]
      | ClassishBody {
        classish_body_left_brace;
        classish_body_elements;
        classish_body_right_brace;
      } -> [
        "classish_body_left_brace";
        "classish_body_elements";
        "classish_body_right_brace";
      ]
      | TraitUsePrecedenceItem {
        trait_use_precedence_item_name;
        trait_use_precedence_item_keyword;
        trait_use_precedence_item_removed_names;
      } -> [
        "trait_use_precedence_item_name";
        "trait_use_precedence_item_keyword";
        "trait_use_precedence_item_removed_names";
      ]
      | TraitUseAliasItem {
        trait_use_alias_item_aliasing_name;
        trait_use_alias_item_keyword;
        trait_use_alias_item_visibility;
        trait_use_alias_item_aliased_name;
      } -> [
        "trait_use_alias_item_aliasing_name";
        "trait_use_alias_item_keyword";
        "trait_use_alias_item_visibility";
        "trait_use_alias_item_aliased_name";
      ]
      | TraitUseConflictResolution {
        trait_use_conflict_resolution_keyword;
        trait_use_conflict_resolution_names;
        trait_use_conflict_resolution_left_brace;
        trait_use_conflict_resolution_clauses;
        trait_use_conflict_resolution_right_brace;
      } -> [
        "trait_use_conflict_resolution_keyword";
        "trait_use_conflict_resolution_names";
        "trait_use_conflict_resolution_left_brace";
        "trait_use_conflict_resolution_clauses";
        "trait_use_conflict_resolution_right_brace";
      ]
      | TraitUse {
        trait_use_keyword;
        trait_use_names;
        trait_use_semicolon;
      } -> [
        "trait_use_keyword";
        "trait_use_names";
        "trait_use_semicolon";
      ]
      | RequireClause {
        require_keyword;
        require_kind;
        require_name;
        require_semicolon;
      } -> [
        "require_keyword";
        "require_kind";
        "require_name";
        "require_semicolon";
      ]
      | ConstDeclaration {
        const_abstract;
        const_keyword;
        const_type_specifier;
        const_declarators;
        const_semicolon;
      } -> [
        "const_abstract";
        "const_keyword";
        "const_type_specifier";
        "const_declarators";
        "const_semicolon";
      ]
      | ConstantDeclarator {
        constant_declarator_name;
        constant_declarator_initializer;
      } -> [
        "constant_declarator_name";
        "constant_declarator_initializer";
      ]
      | TypeConstDeclaration {
        type_const_abstract;
        type_const_keyword;
        type_const_type_keyword;
        type_const_name;
        type_const_type_constraint;
        type_const_equal;
        type_const_type_specifier;
        type_const_semicolon;
      } -> [
        "type_const_abstract";
        "type_const_keyword";
        "type_const_type_keyword";
        "type_const_name";
        "type_const_type_constraint";
        "type_const_equal";
        "type_const_type_specifier";
        "type_const_semicolon";
      ]
      | DecoratedExpression {
        decorated_expression_decorator;
        decorated_expression_expression;
      } -> [
        "decorated_expression_decorator";
        "decorated_expression_expression";
      ]
      | ParameterDeclaration {
        parameter_attribute;
        parameter_visibility;
        parameter_call_convention;
        parameter_type;
        parameter_name;
        parameter_default_value;
      } -> [
        "parameter_attribute";
        "parameter_visibility";
        "parameter_call_convention";
        "parameter_type";
        "parameter_name";
        "parameter_default_value";
      ]
      | VariadicParameter {
        variadic_parameter_type;
        variadic_parameter_ellipsis;
      } -> [
        "variadic_parameter_type";
        "variadic_parameter_ellipsis";
      ]
      | AttributeSpecification {
        attribute_specification_left_double_angle;
        attribute_specification_attributes;
        attribute_specification_right_double_angle;
      } -> [
        "attribute_specification_left_double_angle";
        "attribute_specification_attributes";
        "attribute_specification_right_double_angle";
      ]
      | Attribute {
        attribute_name;
        attribute_left_paren;
        attribute_values;
        attribute_right_paren;
      } -> [
        "attribute_name";
        "attribute_left_paren";
        "attribute_values";
        "attribute_right_paren";
      ]
      | InclusionExpression {
        inclusion_require;
        inclusion_filename;
      } -> [
        "inclusion_require";
        "inclusion_filename";
      ]
      | InclusionDirective {
        inclusion_expression;
        inclusion_semicolon;
      } -> [
        "inclusion_expression";
        "inclusion_semicolon";
      ]
      | CompoundStatement {
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      } -> [
        "compound_left_brace";
        "compound_statements";
        "compound_right_brace";
      ]
      | ExpressionStatement {
        expression_statement_expression;
        expression_statement_semicolon;
      } -> [
        "expression_statement_expression";
        "expression_statement_semicolon";
      ]
      | MarkupSection {
        markup_prefix;
        markup_text;
        markup_suffix;
        markup_expression;
      } -> [
        "markup_prefix";
        "markup_text";
        "markup_suffix";
        "markup_expression";
      ]
      | MarkupSuffix {
        markup_suffix_less_than_question;
        markup_suffix_name;
      } -> [
        "markup_suffix_less_than_question";
        "markup_suffix_name";
      ]
      | UnsetStatement {
        unset_keyword;
        unset_left_paren;
        unset_variables;
        unset_right_paren;
        unset_semicolon;
      } -> [
        "unset_keyword";
        "unset_left_paren";
        "unset_variables";
        "unset_right_paren";
        "unset_semicolon";
      ]
      | UsingStatementBlockScoped {
        using_block_await_keyword;
        using_block_using_keyword;
        using_block_left_paren;
        using_block_expressions;
        using_block_right_paren;
        using_block_body;
      } -> [
        "using_block_await_keyword";
        "using_block_using_keyword";
        "using_block_left_paren";
        "using_block_expressions";
        "using_block_right_paren";
        "using_block_body";
      ]
      | UsingStatementFunctionScoped {
        using_function_await_keyword;
        using_function_using_keyword;
        using_function_expression;
        using_function_semicolon;
      } -> [
        "using_function_await_keyword";
        "using_function_using_keyword";
        "using_function_expression";
        "using_function_semicolon";
      ]
      | WhileStatement {
        while_keyword;
        while_left_paren;
        while_condition;
        while_right_paren;
        while_body;
      } -> [
        "while_keyword";
        "while_left_paren";
        "while_condition";
        "while_right_paren";
        "while_body";
      ]
      | IfStatement {
        if_keyword;
        if_left_paren;
        if_condition;
        if_right_paren;
        if_statement;
        if_elseif_clauses;
        if_else_clause;
      } -> [
        "if_keyword";
        "if_left_paren";
        "if_condition";
        "if_right_paren";
        "if_statement";
        "if_elseif_clauses";
        "if_else_clause";
      ]
      | ElseifClause {
        elseif_keyword;
        elseif_left_paren;
        elseif_condition;
        elseif_right_paren;
        elseif_statement;
      } -> [
        "elseif_keyword";
        "elseif_left_paren";
        "elseif_condition";
        "elseif_right_paren";
        "elseif_statement";
      ]
      | ElseClause {
        else_keyword;
        else_statement;
      } -> [
        "else_keyword";
        "else_statement";
      ]
      | IfEndIfStatement {
        if_endif_keyword;
        if_endif_left_paren;
        if_endif_condition;
        if_endif_right_paren;
        if_endif_colon;
        if_endif_statement;
        if_endif_elseif_colon_clauses;
        if_endif_else_colon_clause;
        if_endif_endif_keyword;
        if_endif_semicolon;
      } -> [
        "if_endif_keyword";
        "if_endif_left_paren";
        "if_endif_condition";
        "if_endif_right_paren";
        "if_endif_colon";
        "if_endif_statement";
        "if_endif_elseif_colon_clauses";
        "if_endif_else_colon_clause";
        "if_endif_endif_keyword";
        "if_endif_semicolon";
      ]
      | ElseifColonClause {
        elseif_colon_keyword;
        elseif_colon_left_paren;
        elseif_colon_condition;
        elseif_colon_right_paren;
        elseif_colon_colon;
        elseif_colon_statement;
      } -> [
        "elseif_colon_keyword";
        "elseif_colon_left_paren";
        "elseif_colon_condition";
        "elseif_colon_right_paren";
        "elseif_colon_colon";
        "elseif_colon_statement";
      ]
      | ElseColonClause {
        else_colon_keyword;
        else_colon_colon;
        else_colon_statement;
      } -> [
        "else_colon_keyword";
        "else_colon_colon";
        "else_colon_statement";
      ]
      | TryStatement {
        try_keyword;
        try_compound_statement;
        try_catch_clauses;
        try_finally_clause;
      } -> [
        "try_keyword";
        "try_compound_statement";
        "try_catch_clauses";
        "try_finally_clause";
      ]
      | CatchClause {
        catch_keyword;
        catch_left_paren;
        catch_type;
        catch_variable;
        catch_right_paren;
        catch_body;
      } -> [
        "catch_keyword";
        "catch_left_paren";
        "catch_type";
        "catch_variable";
        "catch_right_paren";
        "catch_body";
      ]
      | FinallyClause {
        finally_keyword;
        finally_body;
      } -> [
        "finally_keyword";
        "finally_body";
      ]
      | DoStatement {
        do_keyword;
        do_body;
        do_while_keyword;
        do_left_paren;
        do_condition;
        do_right_paren;
        do_semicolon;
      } -> [
        "do_keyword";
        "do_body";
        "do_while_keyword";
        "do_left_paren";
        "do_condition";
        "do_right_paren";
        "do_semicolon";
      ]
      | ForStatement {
        for_keyword;
        for_left_paren;
        for_initializer;
        for_first_semicolon;
        for_control;
        for_second_semicolon;
        for_end_of_loop;
        for_right_paren;
        for_body;
      } -> [
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
      | ForeachStatement {
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
      } -> [
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
      | SwitchStatement {
        switch_keyword;
        switch_left_paren;
        switch_expression;
        switch_right_paren;
        switch_left_brace;
        switch_sections;
        switch_right_brace;
      } -> [
        "switch_keyword";
        "switch_left_paren";
        "switch_expression";
        "switch_right_paren";
        "switch_left_brace";
        "switch_sections";
        "switch_right_brace";
      ]
      | SwitchSection {
        switch_section_labels;
        switch_section_statements;
        switch_section_fallthrough;
      } -> [
        "switch_section_labels";
        "switch_section_statements";
        "switch_section_fallthrough";
      ]
      | SwitchFallthrough {
        fallthrough_keyword;
        fallthrough_semicolon;
      } -> [
        "fallthrough_keyword";
        "fallthrough_semicolon";
      ]
      | CaseLabel {
        case_keyword;
        case_expression;
        case_colon;
      } -> [
        "case_keyword";
        "case_expression";
        "case_colon";
      ]
      | DefaultLabel {
        default_keyword;
        default_colon;
      } -> [
        "default_keyword";
        "default_colon";
      ]
      | ReturnStatement {
        return_keyword;
        return_expression;
        return_semicolon;
      } -> [
        "return_keyword";
        "return_expression";
        "return_semicolon";
      ]
      | GotoLabel {
        goto_label_name;
        goto_label_colon;
      } -> [
        "goto_label_name";
        "goto_label_colon";
      ]
      | GotoStatement {
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon;
      } -> [
        "goto_statement_keyword";
        "goto_statement_label_name";
        "goto_statement_semicolon";
      ]
      | ThrowStatement {
        throw_keyword;
        throw_expression;
        throw_semicolon;
      } -> [
        "throw_keyword";
        "throw_expression";
        "throw_semicolon";
      ]
      | BreakStatement {
        break_keyword;
        break_level;
        break_semicolon;
      } -> [
        "break_keyword";
        "break_level";
        "break_semicolon";
      ]
      | ContinueStatement {
        continue_keyword;
        continue_level;
        continue_semicolon;
      } -> [
        "continue_keyword";
        "continue_level";
        "continue_semicolon";
      ]
      | FunctionStaticStatement {
        static_static_keyword;
        static_declarations;
        static_semicolon;
      } -> [
        "static_static_keyword";
        "static_declarations";
        "static_semicolon";
      ]
      | StaticDeclarator {
        static_name;
        static_initializer;
      } -> [
        "static_name";
        "static_initializer";
      ]
      | EchoStatement {
        echo_keyword;
        echo_expressions;
        echo_semicolon;
      } -> [
        "echo_keyword";
        "echo_expressions";
        "echo_semicolon";
      ]
      | GlobalStatement {
        global_keyword;
        global_variables;
        global_semicolon;
      } -> [
        "global_keyword";
        "global_variables";
        "global_semicolon";
      ]
      | SimpleInitializer {
        simple_initializer_equal;
        simple_initializer_value;
      } -> [
        "simple_initializer_equal";
        "simple_initializer_value";
      ]
      | AnonymousFunction {
        anonymous_static_keyword;
        anonymous_async_keyword;
        anonymous_coroutine_keyword;
        anonymous_function_keyword;
        anonymous_left_paren;
        anonymous_parameters;
        anonymous_right_paren;
        anonymous_colon;
        anonymous_type;
        anonymous_use;
        anonymous_body;
      } -> [
        "anonymous_static_keyword";
        "anonymous_async_keyword";
        "anonymous_coroutine_keyword";
        "anonymous_function_keyword";
        "anonymous_left_paren";
        "anonymous_parameters";
        "anonymous_right_paren";
        "anonymous_colon";
        "anonymous_type";
        "anonymous_use";
        "anonymous_body";
      ]
      | AnonymousFunctionUseClause {
        anonymous_use_keyword;
        anonymous_use_left_paren;
        anonymous_use_variables;
        anonymous_use_right_paren;
      } -> [
        "anonymous_use_keyword";
        "anonymous_use_left_paren";
        "anonymous_use_variables";
        "anonymous_use_right_paren";
      ]
      | LambdaExpression {
        lambda_async;
        lambda_coroutine;
        lambda_signature;
        lambda_arrow;
        lambda_body;
      } -> [
        "lambda_async";
        "lambda_coroutine";
        "lambda_signature";
        "lambda_arrow";
        "lambda_body";
      ]
      | LambdaSignature {
        lambda_left_paren;
        lambda_parameters;
        lambda_right_paren;
        lambda_colon;
        lambda_type;
      } -> [
        "lambda_left_paren";
        "lambda_parameters";
        "lambda_right_paren";
        "lambda_colon";
        "lambda_type";
      ]
      | CastExpression {
        cast_left_paren;
        cast_type;
        cast_right_paren;
        cast_operand;
      } -> [
        "cast_left_paren";
        "cast_type";
        "cast_right_paren";
        "cast_operand";
      ]
      | ScopeResolutionExpression {
        scope_resolution_qualifier;
        scope_resolution_operator;
        scope_resolution_name;
      } -> [
        "scope_resolution_qualifier";
        "scope_resolution_operator";
        "scope_resolution_name";
      ]
      | MemberSelectionExpression {
        member_object;
        member_operator;
        member_name;
      } -> [
        "member_object";
        "member_operator";
        "member_name";
      ]
      | SafeMemberSelectionExpression {
        safe_member_object;
        safe_member_operator;
        safe_member_name;
      } -> [
        "safe_member_object";
        "safe_member_operator";
        "safe_member_name";
      ]
      | EmbeddedMemberSelectionExpression {
        embedded_member_object;
        embedded_member_operator;
        embedded_member_name;
      } -> [
        "embedded_member_object";
        "embedded_member_operator";
        "embedded_member_name";
      ]
      | YieldExpression {
        yield_keyword;
        yield_operand;
      } -> [
        "yield_keyword";
        "yield_operand";
      ]
      | YieldFromExpression {
        yield_from_yield_keyword;
        yield_from_from_keyword;
        yield_from_operand;
      } -> [
        "yield_from_yield_keyword";
        "yield_from_from_keyword";
        "yield_from_operand";
      ]
      | PrefixUnaryExpression {
        prefix_unary_operator;
        prefix_unary_operand;
      } -> [
        "prefix_unary_operator";
        "prefix_unary_operand";
      ]
      | PostfixUnaryExpression {
        postfix_unary_operand;
        postfix_unary_operator;
      } -> [
        "postfix_unary_operand";
        "postfix_unary_operator";
      ]
      | BinaryExpression {
        binary_left_operand;
        binary_operator;
        binary_right_operand;
      } -> [
        "binary_left_operand";
        "binary_operator";
        "binary_right_operand";
      ]
      | InstanceofExpression {
        instanceof_left_operand;
        instanceof_operator;
        instanceof_right_operand;
      } -> [
        "instanceof_left_operand";
        "instanceof_operator";
        "instanceof_right_operand";
      ]
      | IsExpression {
        is_left_operand;
        is_operator;
        is_right_operand;
      } -> [
        "is_left_operand";
        "is_operator";
        "is_right_operand";
      ]
      | ConditionalExpression {
        conditional_test;
        conditional_question;
        conditional_consequence;
        conditional_colon;
        conditional_alternative;
      } -> [
        "conditional_test";
        "conditional_question";
        "conditional_consequence";
        "conditional_colon";
        "conditional_alternative";
      ]
      | EvalExpression {
        eval_keyword;
        eval_left_paren;
        eval_argument;
        eval_right_paren;
      } -> [
        "eval_keyword";
        "eval_left_paren";
        "eval_argument";
        "eval_right_paren";
      ]
      | EmptyExpression {
        empty_keyword;
        empty_left_paren;
        empty_argument;
        empty_right_paren;
      } -> [
        "empty_keyword";
        "empty_left_paren";
        "empty_argument";
        "empty_right_paren";
      ]
      | DefineExpression {
        define_keyword;
        define_left_paren;
        define_argument_list;
        define_right_paren;
      } -> [
        "define_keyword";
        "define_left_paren";
        "define_argument_list";
        "define_right_paren";
      ]
      | IssetExpression {
        isset_keyword;
        isset_left_paren;
        isset_argument_list;
        isset_right_paren;
      } -> [
        "isset_keyword";
        "isset_left_paren";
        "isset_argument_list";
        "isset_right_paren";
      ]
      | FunctionCallExpression {
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren;
      } -> [
        "function_call_receiver";
        "function_call_left_paren";
        "function_call_argument_list";
        "function_call_right_paren";
      ]
      | FunctionCallWithTypeArgumentsExpression {
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren;
      } -> [
        "function_call_with_type_arguments_receiver";
        "function_call_with_type_arguments_type_args";
        "function_call_with_type_arguments_left_paren";
        "function_call_with_type_arguments_argument_list";
        "function_call_with_type_arguments_right_paren";
      ]
      | ParenthesizedExpression {
        parenthesized_expression_left_paren;
        parenthesized_expression_expression;
        parenthesized_expression_right_paren;
      } -> [
        "parenthesized_expression_left_paren";
        "parenthesized_expression_expression";
        "parenthesized_expression_right_paren";
      ]
      | BracedExpression {
        braced_expression_left_brace;
        braced_expression_expression;
        braced_expression_right_brace;
      } -> [
        "braced_expression_left_brace";
        "braced_expression_expression";
        "braced_expression_right_brace";
      ]
      | EmbeddedBracedExpression {
        embedded_braced_expression_left_brace;
        embedded_braced_expression_expression;
        embedded_braced_expression_right_brace;
      } -> [
        "embedded_braced_expression_left_brace";
        "embedded_braced_expression_expression";
        "embedded_braced_expression_right_brace";
      ]
      | ListExpression {
        list_keyword;
        list_left_paren;
        list_members;
        list_right_paren;
      } -> [
        "list_keyword";
        "list_left_paren";
        "list_members";
        "list_right_paren";
      ]
      | CollectionLiteralExpression {
        collection_literal_name;
        collection_literal_left_brace;
        collection_literal_initializers;
        collection_literal_right_brace;
      } -> [
        "collection_literal_name";
        "collection_literal_left_brace";
        "collection_literal_initializers";
        "collection_literal_right_brace";
      ]
      | ObjectCreationExpression {
        object_creation_new_keyword;
        object_creation_type;
        object_creation_left_paren;
        object_creation_argument_list;
        object_creation_right_paren;
      } -> [
        "object_creation_new_keyword";
        "object_creation_type";
        "object_creation_left_paren";
        "object_creation_argument_list";
        "object_creation_right_paren";
      ]
      | ArrayCreationExpression {
        array_creation_left_bracket;
        array_creation_members;
        array_creation_right_bracket;
      } -> [
        "array_creation_left_bracket";
        "array_creation_members";
        "array_creation_right_bracket";
      ]
      | ArrayIntrinsicExpression {
        array_intrinsic_keyword;
        array_intrinsic_left_paren;
        array_intrinsic_members;
        array_intrinsic_right_paren;
      } -> [
        "array_intrinsic_keyword";
        "array_intrinsic_left_paren";
        "array_intrinsic_members";
        "array_intrinsic_right_paren";
      ]
      | DarrayIntrinsicExpression {
        darray_intrinsic_keyword;
        darray_intrinsic_left_bracket;
        darray_intrinsic_members;
        darray_intrinsic_right_bracket;
      } -> [
        "darray_intrinsic_keyword";
        "darray_intrinsic_left_bracket";
        "darray_intrinsic_members";
        "darray_intrinsic_right_bracket";
      ]
      | DictionaryIntrinsicExpression {
        dictionary_intrinsic_keyword;
        dictionary_intrinsic_left_bracket;
        dictionary_intrinsic_members;
        dictionary_intrinsic_right_bracket;
      } -> [
        "dictionary_intrinsic_keyword";
        "dictionary_intrinsic_left_bracket";
        "dictionary_intrinsic_members";
        "dictionary_intrinsic_right_bracket";
      ]
      | KeysetIntrinsicExpression {
        keyset_intrinsic_keyword;
        keyset_intrinsic_left_bracket;
        keyset_intrinsic_members;
        keyset_intrinsic_right_bracket;
      } -> [
        "keyset_intrinsic_keyword";
        "keyset_intrinsic_left_bracket";
        "keyset_intrinsic_members";
        "keyset_intrinsic_right_bracket";
      ]
      | VarrayIntrinsicExpression {
        varray_intrinsic_keyword;
        varray_intrinsic_left_bracket;
        varray_intrinsic_members;
        varray_intrinsic_right_bracket;
      } -> [
        "varray_intrinsic_keyword";
        "varray_intrinsic_left_bracket";
        "varray_intrinsic_members";
        "varray_intrinsic_right_bracket";
      ]
      | VectorIntrinsicExpression {
        vector_intrinsic_keyword;
        vector_intrinsic_left_bracket;
        vector_intrinsic_members;
        vector_intrinsic_right_bracket;
      } -> [
        "vector_intrinsic_keyword";
        "vector_intrinsic_left_bracket";
        "vector_intrinsic_members";
        "vector_intrinsic_right_bracket";
      ]
      | ElementInitializer {
        element_key;
        element_arrow;
        element_value;
      } -> [
        "element_key";
        "element_arrow";
        "element_value";
      ]
      | SubscriptExpression {
        subscript_receiver;
        subscript_left_bracket;
        subscript_index;
        subscript_right_bracket;
      } -> [
        "subscript_receiver";
        "subscript_left_bracket";
        "subscript_index";
        "subscript_right_bracket";
      ]
      | EmbeddedSubscriptExpression {
        embedded_subscript_receiver;
        embedded_subscript_left_bracket;
        embedded_subscript_index;
        embedded_subscript_right_bracket;
      } -> [
        "embedded_subscript_receiver";
        "embedded_subscript_left_bracket";
        "embedded_subscript_index";
        "embedded_subscript_right_bracket";
      ]
      | AwaitableCreationExpression {
        awaitable_async;
        awaitable_coroutine;
        awaitable_compound_statement;
      } -> [
        "awaitable_async";
        "awaitable_coroutine";
        "awaitable_compound_statement";
      ]
      | XHPChildrenDeclaration {
        xhp_children_keyword;
        xhp_children_expression;
        xhp_children_semicolon;
      } -> [
        "xhp_children_keyword";
        "xhp_children_expression";
        "xhp_children_semicolon";
      ]
      | XHPChildrenParenthesizedList {
        xhp_children_list_left_paren;
        xhp_children_list_xhp_children;
        xhp_children_list_right_paren;
      } -> [
        "xhp_children_list_left_paren";
        "xhp_children_list_xhp_children";
        "xhp_children_list_right_paren";
      ]
      | XHPCategoryDeclaration {
        xhp_category_keyword;
        xhp_category_categories;
        xhp_category_semicolon;
      } -> [
        "xhp_category_keyword";
        "xhp_category_categories";
        "xhp_category_semicolon";
      ]
      | XHPEnumType {
        xhp_enum_keyword;
        xhp_enum_left_brace;
        xhp_enum_values;
        xhp_enum_right_brace;
      } -> [
        "xhp_enum_keyword";
        "xhp_enum_left_brace";
        "xhp_enum_values";
        "xhp_enum_right_brace";
      ]
      | XHPRequired {
        xhp_required_at;
        xhp_required_keyword;
      } -> [
        "xhp_required_at";
        "xhp_required_keyword";
      ]
      | XHPClassAttributeDeclaration {
        xhp_attribute_keyword;
        xhp_attribute_attributes;
        xhp_attribute_semicolon;
      } -> [
        "xhp_attribute_keyword";
        "xhp_attribute_attributes";
        "xhp_attribute_semicolon";
      ]
      | XHPClassAttribute {
        xhp_attribute_decl_type;
        xhp_attribute_decl_name;
        xhp_attribute_decl_initializer;
        xhp_attribute_decl_required;
      } -> [
        "xhp_attribute_decl_type";
        "xhp_attribute_decl_name";
        "xhp_attribute_decl_initializer";
        "xhp_attribute_decl_required";
      ]
      | XHPSimpleClassAttribute {
        xhp_simple_class_attribute_type;
      } -> [
        "xhp_simple_class_attribute_type";
      ]
      | XHPAttribute {
        xhp_attribute_name;
        xhp_attribute_equal;
        xhp_attribute_expression;
      } -> [
        "xhp_attribute_name";
        "xhp_attribute_equal";
        "xhp_attribute_expression";
      ]
      | XHPOpen {
        xhp_open_left_angle;
        xhp_open_name;
        xhp_open_attributes;
        xhp_open_right_angle;
      } -> [
        "xhp_open_left_angle";
        "xhp_open_name";
        "xhp_open_attributes";
        "xhp_open_right_angle";
      ]
      | XHPExpression {
        xhp_open;
        xhp_body;
        xhp_close;
      } -> [
        "xhp_open";
        "xhp_body";
        "xhp_close";
      ]
      | XHPClose {
        xhp_close_left_angle;
        xhp_close_name;
        xhp_close_right_angle;
      } -> [
        "xhp_close_left_angle";
        "xhp_close_name";
        "xhp_close_right_angle";
      ]
      | TypeConstant {
        type_constant_left_type;
        type_constant_separator;
        type_constant_right_type;
      } -> [
        "type_constant_left_type";
        "type_constant_separator";
        "type_constant_right_type";
      ]
      | VectorTypeSpecifier {
        vector_type_keyword;
        vector_type_left_angle;
        vector_type_type;
        vector_type_trailing_comma;
        vector_type_right_angle;
      } -> [
        "vector_type_keyword";
        "vector_type_left_angle";
        "vector_type_type";
        "vector_type_trailing_comma";
        "vector_type_right_angle";
      ]
      | KeysetTypeSpecifier {
        keyset_type_keyword;
        keyset_type_left_angle;
        keyset_type_type;
        keyset_type_trailing_comma;
        keyset_type_right_angle;
      } -> [
        "keyset_type_keyword";
        "keyset_type_left_angle";
        "keyset_type_type";
        "keyset_type_trailing_comma";
        "keyset_type_right_angle";
      ]
      | TupleTypeExplicitSpecifier {
        tuple_type_keyword;
        tuple_type_left_angle;
        tuple_type_types;
        tuple_type_right_angle;
      } -> [
        "tuple_type_keyword";
        "tuple_type_left_angle";
        "tuple_type_types";
        "tuple_type_right_angle";
      ]
      | VarrayTypeSpecifier {
        varray_keyword;
        varray_left_angle;
        varray_type;
        varray_trailing_comma;
        varray_right_angle;
      } -> [
        "varray_keyword";
        "varray_left_angle";
        "varray_type";
        "varray_trailing_comma";
        "varray_right_angle";
      ]
      | VectorArrayTypeSpecifier {
        vector_array_keyword;
        vector_array_left_angle;
        vector_array_type;
        vector_array_right_angle;
      } -> [
        "vector_array_keyword";
        "vector_array_left_angle";
        "vector_array_type";
        "vector_array_right_angle";
      ]
      | TypeParameter {
        type_variance;
        type_name;
        type_constraints;
      } -> [
        "type_variance";
        "type_name";
        "type_constraints";
      ]
      | TypeConstraint {
        constraint_keyword;
        constraint_type;
      } -> [
        "constraint_keyword";
        "constraint_type";
      ]
      | DarrayTypeSpecifier {
        darray_keyword;
        darray_left_angle;
        darray_key;
        darray_comma;
        darray_value;
        darray_trailing_comma;
        darray_right_angle;
      } -> [
        "darray_keyword";
        "darray_left_angle";
        "darray_key";
        "darray_comma";
        "darray_value";
        "darray_trailing_comma";
        "darray_right_angle";
      ]
      | MapArrayTypeSpecifier {
        map_array_keyword;
        map_array_left_angle;
        map_array_key;
        map_array_comma;
        map_array_value;
        map_array_right_angle;
      } -> [
        "map_array_keyword";
        "map_array_left_angle";
        "map_array_key";
        "map_array_comma";
        "map_array_value";
        "map_array_right_angle";
      ]
      | DictionaryTypeSpecifier {
        dictionary_type_keyword;
        dictionary_type_left_angle;
        dictionary_type_members;
        dictionary_type_right_angle;
      } -> [
        "dictionary_type_keyword";
        "dictionary_type_left_angle";
        "dictionary_type_members";
        "dictionary_type_right_angle";
      ]
      | ClosureTypeSpecifier {
        closure_outer_left_paren;
        closure_coroutine;
        closure_function_keyword;
        closure_inner_left_paren;
        closure_parameter_types;
        closure_inner_right_paren;
        closure_colon;
        closure_return_type;
        closure_outer_right_paren;
      } -> [
        "closure_outer_left_paren";
        "closure_coroutine";
        "closure_function_keyword";
        "closure_inner_left_paren";
        "closure_parameter_types";
        "closure_inner_right_paren";
        "closure_colon";
        "closure_return_type";
        "closure_outer_right_paren";
      ]
      | ClassnameTypeSpecifier {
        classname_keyword;
        classname_left_angle;
        classname_type;
        classname_trailing_comma;
        classname_right_angle;
      } -> [
        "classname_keyword";
        "classname_left_angle";
        "classname_type";
        "classname_trailing_comma";
        "classname_right_angle";
      ]
      | FieldSpecifier {
        field_question;
        field_name;
        field_arrow;
        field_type;
      } -> [
        "field_question";
        "field_name";
        "field_arrow";
        "field_type";
      ]
      | FieldInitializer {
        field_initializer_name;
        field_initializer_arrow;
        field_initializer_value;
      } -> [
        "field_initializer_name";
        "field_initializer_arrow";
        "field_initializer_value";
      ]
      | ShapeTypeSpecifier {
        shape_type_keyword;
        shape_type_left_paren;
        shape_type_fields;
        shape_type_ellipsis;
        shape_type_right_paren;
      } -> [
        "shape_type_keyword";
        "shape_type_left_paren";
        "shape_type_fields";
        "shape_type_ellipsis";
        "shape_type_right_paren";
      ]
      | ShapeExpression {
        shape_expression_keyword;
        shape_expression_left_paren;
        shape_expression_fields;
        shape_expression_right_paren;
      } -> [
        "shape_expression_keyword";
        "shape_expression_left_paren";
        "shape_expression_fields";
        "shape_expression_right_paren";
      ]
      | TupleExpression {
        tuple_expression_keyword;
        tuple_expression_left_paren;
        tuple_expression_items;
        tuple_expression_right_paren;
      } -> [
        "tuple_expression_keyword";
        "tuple_expression_left_paren";
        "tuple_expression_items";
        "tuple_expression_right_paren";
      ]
      | GenericTypeSpecifier {
        generic_class_type;
        generic_argument_list;
      } -> [
        "generic_class_type";
        "generic_argument_list";
      ]
      | NullableTypeSpecifier {
        nullable_question;
        nullable_type;
      } -> [
        "nullable_question";
        "nullable_type";
      ]
      | SoftTypeSpecifier {
        soft_at;
        soft_type;
      } -> [
        "soft_at";
        "soft_type";
      ]
      | TypeArguments {
        type_arguments_left_angle;
        type_arguments_types;
        type_arguments_right_angle;
      } -> [
        "type_arguments_left_angle";
        "type_arguments_types";
        "type_arguments_right_angle";
      ]
      | TypeParameters {
        type_parameters_left_angle;
        type_parameters_parameters;
        type_parameters_right_angle;
      } -> [
        "type_parameters_left_angle";
        "type_parameters_parameters";
        "type_parameters_right_angle";
      ]
      | TupleTypeSpecifier {
        tuple_left_paren;
        tuple_types;
        tuple_right_paren;
      } -> [
        "tuple_left_paren";
        "tuple_types";
        "tuple_right_paren";
      ]
      | ErrorSyntax {
        error_error;
      } -> [
        "error_error";
      ]
      | ListItem {
        list_item;
        list_separator;
      } -> [
        "list_item";
        "list_separator";
      ]


    let rec to_json node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ "token", Token.to_json t ]
      | SyntaxList x -> [ ("elements", JSON_Array (List.map to_json x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            aux ((hn, to_json hc) :: acc) tc tn
          | _ -> failwith "mismatch between children and names" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = ("kind", JSON_String (SyntaxKind.to_string (kind node))) in
      JSON_Object (k :: ch)

    let binary_operator_kind b =
      match syntax b.binary_operator with
      | Token token ->
        let kind = Token.kind token in
        if Operator.is_trailing_operator_token kind then
          Some (Operator.trailing_from_token kind)
        else
          None
      | _ -> None

    let get_token node =
      match (syntax node) with
      | Token token -> Some token
      | _ -> None

    let leading_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (children h) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let trailing_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (List.rev (children h)) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let syntax_from_children kind ts =
      match kind, ts with
      | (SyntaxKind.EndOfFile, [
          end_of_file_token;
        ]) ->
        EndOfFile {
          end_of_file_token;
        }
      | (SyntaxKind.Script, [
          script_declarations;
        ]) ->
        Script {
          script_declarations;
        }
      | (SyntaxKind.SimpleTypeSpecifier, [
          simple_type_specifier;
        ]) ->
        SimpleTypeSpecifier {
          simple_type_specifier;
        }
      | (SyntaxKind.LiteralExpression, [
          literal_expression;
        ]) ->
        LiteralExpression {
          literal_expression;
        }
      | (SyntaxKind.VariableExpression, [
          variable_expression;
        ]) ->
        VariableExpression {
          variable_expression;
        }
      | (SyntaxKind.QualifiedNameExpression, [
          qualified_name_expression;
        ]) ->
        QualifiedNameExpression {
          qualified_name_expression;
        }
      | (SyntaxKind.PipeVariableExpression, [
          pipe_variable_expression;
        ]) ->
        PipeVariableExpression {
          pipe_variable_expression;
        }
      | (SyntaxKind.EnumDeclaration, [
          enum_attribute_spec;
          enum_keyword;
          enum_name;
          enum_colon;
          enum_base;
          enum_type;
          enum_left_brace;
          enum_enumerators;
          enum_right_brace;
        ]) ->
        EnumDeclaration {
          enum_attribute_spec;
          enum_keyword;
          enum_name;
          enum_colon;
          enum_base;
          enum_type;
          enum_left_brace;
          enum_enumerators;
          enum_right_brace;
        }
      | (SyntaxKind.Enumerator, [
          enumerator_name;
          enumerator_equal;
          enumerator_value;
          enumerator_semicolon;
        ]) ->
        Enumerator {
          enumerator_name;
          enumerator_equal;
          enumerator_value;
          enumerator_semicolon;
        }
      | (SyntaxKind.AliasDeclaration, [
          alias_attribute_spec;
          alias_keyword;
          alias_name;
          alias_generic_parameter;
          alias_constraint;
          alias_equal;
          alias_type;
          alias_semicolon;
        ]) ->
        AliasDeclaration {
          alias_attribute_spec;
          alias_keyword;
          alias_name;
          alias_generic_parameter;
          alias_constraint;
          alias_equal;
          alias_type;
          alias_semicolon;
        }
      | (SyntaxKind.PropertyDeclaration, [
          property_modifiers;
          property_type;
          property_declarators;
          property_semicolon;
        ]) ->
        PropertyDeclaration {
          property_modifiers;
          property_type;
          property_declarators;
          property_semicolon;
        }
      | (SyntaxKind.PropertyDeclarator, [
          property_name;
          property_initializer;
        ]) ->
        PropertyDeclarator {
          property_name;
          property_initializer;
        }
      | (SyntaxKind.NamespaceDeclaration, [
          namespace_keyword;
          namespace_name;
          namespace_body;
        ]) ->
        NamespaceDeclaration {
          namespace_keyword;
          namespace_name;
          namespace_body;
        }
      | (SyntaxKind.NamespaceBody, [
          namespace_left_brace;
          namespace_declarations;
          namespace_right_brace;
        ]) ->
        NamespaceBody {
          namespace_left_brace;
          namespace_declarations;
          namespace_right_brace;
        }
      | (SyntaxKind.NamespaceEmptyBody, [
          namespace_semicolon;
        ]) ->
        NamespaceEmptyBody {
          namespace_semicolon;
        }
      | (SyntaxKind.NamespaceUseDeclaration, [
          namespace_use_keyword;
          namespace_use_kind;
          namespace_use_clauses;
          namespace_use_semicolon;
        ]) ->
        NamespaceUseDeclaration {
          namespace_use_keyword;
          namespace_use_kind;
          namespace_use_clauses;
          namespace_use_semicolon;
        }
      | (SyntaxKind.NamespaceGroupUseDeclaration, [
          namespace_group_use_keyword;
          namespace_group_use_kind;
          namespace_group_use_prefix;
          namespace_group_use_left_brace;
          namespace_group_use_clauses;
          namespace_group_use_right_brace;
          namespace_group_use_semicolon;
        ]) ->
        NamespaceGroupUseDeclaration {
          namespace_group_use_keyword;
          namespace_group_use_kind;
          namespace_group_use_prefix;
          namespace_group_use_left_brace;
          namespace_group_use_clauses;
          namespace_group_use_right_brace;
          namespace_group_use_semicolon;
        }
      | (SyntaxKind.NamespaceUseClause, [
          namespace_use_clause_kind;
          namespace_use_name;
          namespace_use_as;
          namespace_use_alias;
        ]) ->
        NamespaceUseClause {
          namespace_use_clause_kind;
          namespace_use_name;
          namespace_use_as;
          namespace_use_alias;
        }
      | (SyntaxKind.FunctionDeclaration, [
          function_attribute_spec;
          function_declaration_header;
          function_body;
        ]) ->
        FunctionDeclaration {
          function_attribute_spec;
          function_declaration_header;
          function_body;
        }
      | (SyntaxKind.FunctionDeclarationHeader, [
          function_async;
          function_coroutine;
          function_keyword;
          function_ampersand;
          function_name;
          function_type_parameter_list;
          function_left_paren;
          function_parameter_list;
          function_right_paren;
          function_colon;
          function_type;
          function_where_clause;
        ]) ->
        FunctionDeclarationHeader {
          function_async;
          function_coroutine;
          function_keyword;
          function_ampersand;
          function_name;
          function_type_parameter_list;
          function_left_paren;
          function_parameter_list;
          function_right_paren;
          function_colon;
          function_type;
          function_where_clause;
        }
      | (SyntaxKind.WhereClause, [
          where_clause_keyword;
          where_clause_constraints;
        ]) ->
        WhereClause {
          where_clause_keyword;
          where_clause_constraints;
        }
      | (SyntaxKind.WhereConstraint, [
          where_constraint_left_type;
          where_constraint_operator;
          where_constraint_right_type;
        ]) ->
        WhereConstraint {
          where_constraint_left_type;
          where_constraint_operator;
          where_constraint_right_type;
        }
      | (SyntaxKind.MethodishDeclaration, [
          methodish_attribute;
          methodish_modifiers;
          methodish_function_decl_header;
          methodish_function_body;
          methodish_semicolon;
        ]) ->
        MethodishDeclaration {
          methodish_attribute;
          methodish_modifiers;
          methodish_function_decl_header;
          methodish_function_body;
          methodish_semicolon;
        }
      | (SyntaxKind.ClassishDeclaration, [
          classish_attribute;
          classish_modifiers;
          classish_keyword;
          classish_name;
          classish_type_parameters;
          classish_extends_keyword;
          classish_extends_list;
          classish_implements_keyword;
          classish_implements_list;
          classish_body;
        ]) ->
        ClassishDeclaration {
          classish_attribute;
          classish_modifiers;
          classish_keyword;
          classish_name;
          classish_type_parameters;
          classish_extends_keyword;
          classish_extends_list;
          classish_implements_keyword;
          classish_implements_list;
          classish_body;
        }
      | (SyntaxKind.ClassishBody, [
          classish_body_left_brace;
          classish_body_elements;
          classish_body_right_brace;
        ]) ->
        ClassishBody {
          classish_body_left_brace;
          classish_body_elements;
          classish_body_right_brace;
        }
      | (SyntaxKind.TraitUsePrecedenceItem, [
          trait_use_precedence_item_name;
          trait_use_precedence_item_keyword;
          trait_use_precedence_item_removed_names;
        ]) ->
        TraitUsePrecedenceItem {
          trait_use_precedence_item_name;
          trait_use_precedence_item_keyword;
          trait_use_precedence_item_removed_names;
        }
      | (SyntaxKind.TraitUseAliasItem, [
          trait_use_alias_item_aliasing_name;
          trait_use_alias_item_keyword;
          trait_use_alias_item_visibility;
          trait_use_alias_item_aliased_name;
        ]) ->
        TraitUseAliasItem {
          trait_use_alias_item_aliasing_name;
          trait_use_alias_item_keyword;
          trait_use_alias_item_visibility;
          trait_use_alias_item_aliased_name;
        }
      | (SyntaxKind.TraitUseConflictResolution, [
          trait_use_conflict_resolution_keyword;
          trait_use_conflict_resolution_names;
          trait_use_conflict_resolution_left_brace;
          trait_use_conflict_resolution_clauses;
          trait_use_conflict_resolution_right_brace;
        ]) ->
        TraitUseConflictResolution {
          trait_use_conflict_resolution_keyword;
          trait_use_conflict_resolution_names;
          trait_use_conflict_resolution_left_brace;
          trait_use_conflict_resolution_clauses;
          trait_use_conflict_resolution_right_brace;
        }
      | (SyntaxKind.TraitUse, [
          trait_use_keyword;
          trait_use_names;
          trait_use_semicolon;
        ]) ->
        TraitUse {
          trait_use_keyword;
          trait_use_names;
          trait_use_semicolon;
        }
      | (SyntaxKind.RequireClause, [
          require_keyword;
          require_kind;
          require_name;
          require_semicolon;
        ]) ->
        RequireClause {
          require_keyword;
          require_kind;
          require_name;
          require_semicolon;
        }
      | (SyntaxKind.ConstDeclaration, [
          const_abstract;
          const_keyword;
          const_type_specifier;
          const_declarators;
          const_semicolon;
        ]) ->
        ConstDeclaration {
          const_abstract;
          const_keyword;
          const_type_specifier;
          const_declarators;
          const_semicolon;
        }
      | (SyntaxKind.ConstantDeclarator, [
          constant_declarator_name;
          constant_declarator_initializer;
        ]) ->
        ConstantDeclarator {
          constant_declarator_name;
          constant_declarator_initializer;
        }
      | (SyntaxKind.TypeConstDeclaration, [
          type_const_abstract;
          type_const_keyword;
          type_const_type_keyword;
          type_const_name;
          type_const_type_constraint;
          type_const_equal;
          type_const_type_specifier;
          type_const_semicolon;
        ]) ->
        TypeConstDeclaration {
          type_const_abstract;
          type_const_keyword;
          type_const_type_keyword;
          type_const_name;
          type_const_type_constraint;
          type_const_equal;
          type_const_type_specifier;
          type_const_semicolon;
        }
      | (SyntaxKind.DecoratedExpression, [
          decorated_expression_decorator;
          decorated_expression_expression;
        ]) ->
        DecoratedExpression {
          decorated_expression_decorator;
          decorated_expression_expression;
        }
      | (SyntaxKind.ParameterDeclaration, [
          parameter_attribute;
          parameter_visibility;
          parameter_call_convention;
          parameter_type;
          parameter_name;
          parameter_default_value;
        ]) ->
        ParameterDeclaration {
          parameter_attribute;
          parameter_visibility;
          parameter_call_convention;
          parameter_type;
          parameter_name;
          parameter_default_value;
        }
      | (SyntaxKind.VariadicParameter, [
          variadic_parameter_type;
          variadic_parameter_ellipsis;
        ]) ->
        VariadicParameter {
          variadic_parameter_type;
          variadic_parameter_ellipsis;
        }
      | (SyntaxKind.AttributeSpecification, [
          attribute_specification_left_double_angle;
          attribute_specification_attributes;
          attribute_specification_right_double_angle;
        ]) ->
        AttributeSpecification {
          attribute_specification_left_double_angle;
          attribute_specification_attributes;
          attribute_specification_right_double_angle;
        }
      | (SyntaxKind.Attribute, [
          attribute_name;
          attribute_left_paren;
          attribute_values;
          attribute_right_paren;
        ]) ->
        Attribute {
          attribute_name;
          attribute_left_paren;
          attribute_values;
          attribute_right_paren;
        }
      | (SyntaxKind.InclusionExpression, [
          inclusion_require;
          inclusion_filename;
        ]) ->
        InclusionExpression {
          inclusion_require;
          inclusion_filename;
        }
      | (SyntaxKind.InclusionDirective, [
          inclusion_expression;
          inclusion_semicolon;
        ]) ->
        InclusionDirective {
          inclusion_expression;
          inclusion_semicolon;
        }
      | (SyntaxKind.CompoundStatement, [
          compound_left_brace;
          compound_statements;
          compound_right_brace;
        ]) ->
        CompoundStatement {
          compound_left_brace;
          compound_statements;
          compound_right_brace;
        }
      | (SyntaxKind.ExpressionStatement, [
          expression_statement_expression;
          expression_statement_semicolon;
        ]) ->
        ExpressionStatement {
          expression_statement_expression;
          expression_statement_semicolon;
        }
      | (SyntaxKind.MarkupSection, [
          markup_prefix;
          markup_text;
          markup_suffix;
          markup_expression;
        ]) ->
        MarkupSection {
          markup_prefix;
          markup_text;
          markup_suffix;
          markup_expression;
        }
      | (SyntaxKind.MarkupSuffix, [
          markup_suffix_less_than_question;
          markup_suffix_name;
        ]) ->
        MarkupSuffix {
          markup_suffix_less_than_question;
          markup_suffix_name;
        }
      | (SyntaxKind.UnsetStatement, [
          unset_keyword;
          unset_left_paren;
          unset_variables;
          unset_right_paren;
          unset_semicolon;
        ]) ->
        UnsetStatement {
          unset_keyword;
          unset_left_paren;
          unset_variables;
          unset_right_paren;
          unset_semicolon;
        }
      | (SyntaxKind.UsingStatementBlockScoped, [
          using_block_await_keyword;
          using_block_using_keyword;
          using_block_left_paren;
          using_block_expressions;
          using_block_right_paren;
          using_block_body;
        ]) ->
        UsingStatementBlockScoped {
          using_block_await_keyword;
          using_block_using_keyword;
          using_block_left_paren;
          using_block_expressions;
          using_block_right_paren;
          using_block_body;
        }
      | (SyntaxKind.UsingStatementFunctionScoped, [
          using_function_await_keyword;
          using_function_using_keyword;
          using_function_expression;
          using_function_semicolon;
        ]) ->
        UsingStatementFunctionScoped {
          using_function_await_keyword;
          using_function_using_keyword;
          using_function_expression;
          using_function_semicolon;
        }
      | (SyntaxKind.WhileStatement, [
          while_keyword;
          while_left_paren;
          while_condition;
          while_right_paren;
          while_body;
        ]) ->
        WhileStatement {
          while_keyword;
          while_left_paren;
          while_condition;
          while_right_paren;
          while_body;
        }
      | (SyntaxKind.IfStatement, [
          if_keyword;
          if_left_paren;
          if_condition;
          if_right_paren;
          if_statement;
          if_elseif_clauses;
          if_else_clause;
        ]) ->
        IfStatement {
          if_keyword;
          if_left_paren;
          if_condition;
          if_right_paren;
          if_statement;
          if_elseif_clauses;
          if_else_clause;
        }
      | (SyntaxKind.ElseifClause, [
          elseif_keyword;
          elseif_left_paren;
          elseif_condition;
          elseif_right_paren;
          elseif_statement;
        ]) ->
        ElseifClause {
          elseif_keyword;
          elseif_left_paren;
          elseif_condition;
          elseif_right_paren;
          elseif_statement;
        }
      | (SyntaxKind.ElseClause, [
          else_keyword;
          else_statement;
        ]) ->
        ElseClause {
          else_keyword;
          else_statement;
        }
      | (SyntaxKind.IfEndIfStatement, [
          if_endif_keyword;
          if_endif_left_paren;
          if_endif_condition;
          if_endif_right_paren;
          if_endif_colon;
          if_endif_statement;
          if_endif_elseif_colon_clauses;
          if_endif_else_colon_clause;
          if_endif_endif_keyword;
          if_endif_semicolon;
        ]) ->
        IfEndIfStatement {
          if_endif_keyword;
          if_endif_left_paren;
          if_endif_condition;
          if_endif_right_paren;
          if_endif_colon;
          if_endif_statement;
          if_endif_elseif_colon_clauses;
          if_endif_else_colon_clause;
          if_endif_endif_keyword;
          if_endif_semicolon;
        }
      | (SyntaxKind.ElseifColonClause, [
          elseif_colon_keyword;
          elseif_colon_left_paren;
          elseif_colon_condition;
          elseif_colon_right_paren;
          elseif_colon_colon;
          elseif_colon_statement;
        ]) ->
        ElseifColonClause {
          elseif_colon_keyword;
          elseif_colon_left_paren;
          elseif_colon_condition;
          elseif_colon_right_paren;
          elseif_colon_colon;
          elseif_colon_statement;
        }
      | (SyntaxKind.ElseColonClause, [
          else_colon_keyword;
          else_colon_colon;
          else_colon_statement;
        ]) ->
        ElseColonClause {
          else_colon_keyword;
          else_colon_colon;
          else_colon_statement;
        }
      | (SyntaxKind.TryStatement, [
          try_keyword;
          try_compound_statement;
          try_catch_clauses;
          try_finally_clause;
        ]) ->
        TryStatement {
          try_keyword;
          try_compound_statement;
          try_catch_clauses;
          try_finally_clause;
        }
      | (SyntaxKind.CatchClause, [
          catch_keyword;
          catch_left_paren;
          catch_type;
          catch_variable;
          catch_right_paren;
          catch_body;
        ]) ->
        CatchClause {
          catch_keyword;
          catch_left_paren;
          catch_type;
          catch_variable;
          catch_right_paren;
          catch_body;
        }
      | (SyntaxKind.FinallyClause, [
          finally_keyword;
          finally_body;
        ]) ->
        FinallyClause {
          finally_keyword;
          finally_body;
        }
      | (SyntaxKind.DoStatement, [
          do_keyword;
          do_body;
          do_while_keyword;
          do_left_paren;
          do_condition;
          do_right_paren;
          do_semicolon;
        ]) ->
        DoStatement {
          do_keyword;
          do_body;
          do_while_keyword;
          do_left_paren;
          do_condition;
          do_right_paren;
          do_semicolon;
        }
      | (SyntaxKind.ForStatement, [
          for_keyword;
          for_left_paren;
          for_initializer;
          for_first_semicolon;
          for_control;
          for_second_semicolon;
          for_end_of_loop;
          for_right_paren;
          for_body;
        ]) ->
        ForStatement {
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
      | (SyntaxKind.ForeachStatement, [
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
        ]) ->
        ForeachStatement {
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
      | (SyntaxKind.SwitchStatement, [
          switch_keyword;
          switch_left_paren;
          switch_expression;
          switch_right_paren;
          switch_left_brace;
          switch_sections;
          switch_right_brace;
        ]) ->
        SwitchStatement {
          switch_keyword;
          switch_left_paren;
          switch_expression;
          switch_right_paren;
          switch_left_brace;
          switch_sections;
          switch_right_brace;
        }
      | (SyntaxKind.SwitchSection, [
          switch_section_labels;
          switch_section_statements;
          switch_section_fallthrough;
        ]) ->
        SwitchSection {
          switch_section_labels;
          switch_section_statements;
          switch_section_fallthrough;
        }
      | (SyntaxKind.SwitchFallthrough, [
          fallthrough_keyword;
          fallthrough_semicolon;
        ]) ->
        SwitchFallthrough {
          fallthrough_keyword;
          fallthrough_semicolon;
        }
      | (SyntaxKind.CaseLabel, [
          case_keyword;
          case_expression;
          case_colon;
        ]) ->
        CaseLabel {
          case_keyword;
          case_expression;
          case_colon;
        }
      | (SyntaxKind.DefaultLabel, [
          default_keyword;
          default_colon;
        ]) ->
        DefaultLabel {
          default_keyword;
          default_colon;
        }
      | (SyntaxKind.ReturnStatement, [
          return_keyword;
          return_expression;
          return_semicolon;
        ]) ->
        ReturnStatement {
          return_keyword;
          return_expression;
          return_semicolon;
        }
      | (SyntaxKind.GotoLabel, [
          goto_label_name;
          goto_label_colon;
        ]) ->
        GotoLabel {
          goto_label_name;
          goto_label_colon;
        }
      | (SyntaxKind.GotoStatement, [
          goto_statement_keyword;
          goto_statement_label_name;
          goto_statement_semicolon;
        ]) ->
        GotoStatement {
          goto_statement_keyword;
          goto_statement_label_name;
          goto_statement_semicolon;
        }
      | (SyntaxKind.ThrowStatement, [
          throw_keyword;
          throw_expression;
          throw_semicolon;
        ]) ->
        ThrowStatement {
          throw_keyword;
          throw_expression;
          throw_semicolon;
        }
      | (SyntaxKind.BreakStatement, [
          break_keyword;
          break_level;
          break_semicolon;
        ]) ->
        BreakStatement {
          break_keyword;
          break_level;
          break_semicolon;
        }
      | (SyntaxKind.ContinueStatement, [
          continue_keyword;
          continue_level;
          continue_semicolon;
        ]) ->
        ContinueStatement {
          continue_keyword;
          continue_level;
          continue_semicolon;
        }
      | (SyntaxKind.FunctionStaticStatement, [
          static_static_keyword;
          static_declarations;
          static_semicolon;
        ]) ->
        FunctionStaticStatement {
          static_static_keyword;
          static_declarations;
          static_semicolon;
        }
      | (SyntaxKind.StaticDeclarator, [
          static_name;
          static_initializer;
        ]) ->
        StaticDeclarator {
          static_name;
          static_initializer;
        }
      | (SyntaxKind.EchoStatement, [
          echo_keyword;
          echo_expressions;
          echo_semicolon;
        ]) ->
        EchoStatement {
          echo_keyword;
          echo_expressions;
          echo_semicolon;
        }
      | (SyntaxKind.GlobalStatement, [
          global_keyword;
          global_variables;
          global_semicolon;
        ]) ->
        GlobalStatement {
          global_keyword;
          global_variables;
          global_semicolon;
        }
      | (SyntaxKind.SimpleInitializer, [
          simple_initializer_equal;
          simple_initializer_value;
        ]) ->
        SimpleInitializer {
          simple_initializer_equal;
          simple_initializer_value;
        }
      | (SyntaxKind.AnonymousFunction, [
          anonymous_static_keyword;
          anonymous_async_keyword;
          anonymous_coroutine_keyword;
          anonymous_function_keyword;
          anonymous_left_paren;
          anonymous_parameters;
          anonymous_right_paren;
          anonymous_colon;
          anonymous_type;
          anonymous_use;
          anonymous_body;
        ]) ->
        AnonymousFunction {
          anonymous_static_keyword;
          anonymous_async_keyword;
          anonymous_coroutine_keyword;
          anonymous_function_keyword;
          anonymous_left_paren;
          anonymous_parameters;
          anonymous_right_paren;
          anonymous_colon;
          anonymous_type;
          anonymous_use;
          anonymous_body;
        }
      | (SyntaxKind.AnonymousFunctionUseClause, [
          anonymous_use_keyword;
          anonymous_use_left_paren;
          anonymous_use_variables;
          anonymous_use_right_paren;
        ]) ->
        AnonymousFunctionUseClause {
          anonymous_use_keyword;
          anonymous_use_left_paren;
          anonymous_use_variables;
          anonymous_use_right_paren;
        }
      | (SyntaxKind.LambdaExpression, [
          lambda_async;
          lambda_coroutine;
          lambda_signature;
          lambda_arrow;
          lambda_body;
        ]) ->
        LambdaExpression {
          lambda_async;
          lambda_coroutine;
          lambda_signature;
          lambda_arrow;
          lambda_body;
        }
      | (SyntaxKind.LambdaSignature, [
          lambda_left_paren;
          lambda_parameters;
          lambda_right_paren;
          lambda_colon;
          lambda_type;
        ]) ->
        LambdaSignature {
          lambda_left_paren;
          lambda_parameters;
          lambda_right_paren;
          lambda_colon;
          lambda_type;
        }
      | (SyntaxKind.CastExpression, [
          cast_left_paren;
          cast_type;
          cast_right_paren;
          cast_operand;
        ]) ->
        CastExpression {
          cast_left_paren;
          cast_type;
          cast_right_paren;
          cast_operand;
        }
      | (SyntaxKind.ScopeResolutionExpression, [
          scope_resolution_qualifier;
          scope_resolution_operator;
          scope_resolution_name;
        ]) ->
        ScopeResolutionExpression {
          scope_resolution_qualifier;
          scope_resolution_operator;
          scope_resolution_name;
        }
      | (SyntaxKind.MemberSelectionExpression, [
          member_object;
          member_operator;
          member_name;
        ]) ->
        MemberSelectionExpression {
          member_object;
          member_operator;
          member_name;
        }
      | (SyntaxKind.SafeMemberSelectionExpression, [
          safe_member_object;
          safe_member_operator;
          safe_member_name;
        ]) ->
        SafeMemberSelectionExpression {
          safe_member_object;
          safe_member_operator;
          safe_member_name;
        }
      | (SyntaxKind.EmbeddedMemberSelectionExpression, [
          embedded_member_object;
          embedded_member_operator;
          embedded_member_name;
        ]) ->
        EmbeddedMemberSelectionExpression {
          embedded_member_object;
          embedded_member_operator;
          embedded_member_name;
        }
      | (SyntaxKind.YieldExpression, [
          yield_keyword;
          yield_operand;
        ]) ->
        YieldExpression {
          yield_keyword;
          yield_operand;
        }
      | (SyntaxKind.YieldFromExpression, [
          yield_from_yield_keyword;
          yield_from_from_keyword;
          yield_from_operand;
        ]) ->
        YieldFromExpression {
          yield_from_yield_keyword;
          yield_from_from_keyword;
          yield_from_operand;
        }
      | (SyntaxKind.PrefixUnaryExpression, [
          prefix_unary_operator;
          prefix_unary_operand;
        ]) ->
        PrefixUnaryExpression {
          prefix_unary_operator;
          prefix_unary_operand;
        }
      | (SyntaxKind.PostfixUnaryExpression, [
          postfix_unary_operand;
          postfix_unary_operator;
        ]) ->
        PostfixUnaryExpression {
          postfix_unary_operand;
          postfix_unary_operator;
        }
      | (SyntaxKind.BinaryExpression, [
          binary_left_operand;
          binary_operator;
          binary_right_operand;
        ]) ->
        BinaryExpression {
          binary_left_operand;
          binary_operator;
          binary_right_operand;
        }
      | (SyntaxKind.InstanceofExpression, [
          instanceof_left_operand;
          instanceof_operator;
          instanceof_right_operand;
        ]) ->
        InstanceofExpression {
          instanceof_left_operand;
          instanceof_operator;
          instanceof_right_operand;
        }
      | (SyntaxKind.IsExpression, [
          is_left_operand;
          is_operator;
          is_right_operand;
        ]) ->
        IsExpression {
          is_left_operand;
          is_operator;
          is_right_operand;
        }
      | (SyntaxKind.ConditionalExpression, [
          conditional_test;
          conditional_question;
          conditional_consequence;
          conditional_colon;
          conditional_alternative;
        ]) ->
        ConditionalExpression {
          conditional_test;
          conditional_question;
          conditional_consequence;
          conditional_colon;
          conditional_alternative;
        }
      | (SyntaxKind.EvalExpression, [
          eval_keyword;
          eval_left_paren;
          eval_argument;
          eval_right_paren;
        ]) ->
        EvalExpression {
          eval_keyword;
          eval_left_paren;
          eval_argument;
          eval_right_paren;
        }
      | (SyntaxKind.EmptyExpression, [
          empty_keyword;
          empty_left_paren;
          empty_argument;
          empty_right_paren;
        ]) ->
        EmptyExpression {
          empty_keyword;
          empty_left_paren;
          empty_argument;
          empty_right_paren;
        }
      | (SyntaxKind.DefineExpression, [
          define_keyword;
          define_left_paren;
          define_argument_list;
          define_right_paren;
        ]) ->
        DefineExpression {
          define_keyword;
          define_left_paren;
          define_argument_list;
          define_right_paren;
        }
      | (SyntaxKind.IssetExpression, [
          isset_keyword;
          isset_left_paren;
          isset_argument_list;
          isset_right_paren;
        ]) ->
        IssetExpression {
          isset_keyword;
          isset_left_paren;
          isset_argument_list;
          isset_right_paren;
        }
      | (SyntaxKind.FunctionCallExpression, [
          function_call_receiver;
          function_call_left_paren;
          function_call_argument_list;
          function_call_right_paren;
        ]) ->
        FunctionCallExpression {
          function_call_receiver;
          function_call_left_paren;
          function_call_argument_list;
          function_call_right_paren;
        }
      | (SyntaxKind.FunctionCallWithTypeArgumentsExpression, [
          function_call_with_type_arguments_receiver;
          function_call_with_type_arguments_type_args;
          function_call_with_type_arguments_left_paren;
          function_call_with_type_arguments_argument_list;
          function_call_with_type_arguments_right_paren;
        ]) ->
        FunctionCallWithTypeArgumentsExpression {
          function_call_with_type_arguments_receiver;
          function_call_with_type_arguments_type_args;
          function_call_with_type_arguments_left_paren;
          function_call_with_type_arguments_argument_list;
          function_call_with_type_arguments_right_paren;
        }
      | (SyntaxKind.ParenthesizedExpression, [
          parenthesized_expression_left_paren;
          parenthesized_expression_expression;
          parenthesized_expression_right_paren;
        ]) ->
        ParenthesizedExpression {
          parenthesized_expression_left_paren;
          parenthesized_expression_expression;
          parenthesized_expression_right_paren;
        }
      | (SyntaxKind.BracedExpression, [
          braced_expression_left_brace;
          braced_expression_expression;
          braced_expression_right_brace;
        ]) ->
        BracedExpression {
          braced_expression_left_brace;
          braced_expression_expression;
          braced_expression_right_brace;
        }
      | (SyntaxKind.EmbeddedBracedExpression, [
          embedded_braced_expression_left_brace;
          embedded_braced_expression_expression;
          embedded_braced_expression_right_brace;
        ]) ->
        EmbeddedBracedExpression {
          embedded_braced_expression_left_brace;
          embedded_braced_expression_expression;
          embedded_braced_expression_right_brace;
        }
      | (SyntaxKind.ListExpression, [
          list_keyword;
          list_left_paren;
          list_members;
          list_right_paren;
        ]) ->
        ListExpression {
          list_keyword;
          list_left_paren;
          list_members;
          list_right_paren;
        }
      | (SyntaxKind.CollectionLiteralExpression, [
          collection_literal_name;
          collection_literal_left_brace;
          collection_literal_initializers;
          collection_literal_right_brace;
        ]) ->
        CollectionLiteralExpression {
          collection_literal_name;
          collection_literal_left_brace;
          collection_literal_initializers;
          collection_literal_right_brace;
        }
      | (SyntaxKind.ObjectCreationExpression, [
          object_creation_new_keyword;
          object_creation_type;
          object_creation_left_paren;
          object_creation_argument_list;
          object_creation_right_paren;
        ]) ->
        ObjectCreationExpression {
          object_creation_new_keyword;
          object_creation_type;
          object_creation_left_paren;
          object_creation_argument_list;
          object_creation_right_paren;
        }
      | (SyntaxKind.ArrayCreationExpression, [
          array_creation_left_bracket;
          array_creation_members;
          array_creation_right_bracket;
        ]) ->
        ArrayCreationExpression {
          array_creation_left_bracket;
          array_creation_members;
          array_creation_right_bracket;
        }
      | (SyntaxKind.ArrayIntrinsicExpression, [
          array_intrinsic_keyword;
          array_intrinsic_left_paren;
          array_intrinsic_members;
          array_intrinsic_right_paren;
        ]) ->
        ArrayIntrinsicExpression {
          array_intrinsic_keyword;
          array_intrinsic_left_paren;
          array_intrinsic_members;
          array_intrinsic_right_paren;
        }
      | (SyntaxKind.DarrayIntrinsicExpression, [
          darray_intrinsic_keyword;
          darray_intrinsic_left_bracket;
          darray_intrinsic_members;
          darray_intrinsic_right_bracket;
        ]) ->
        DarrayIntrinsicExpression {
          darray_intrinsic_keyword;
          darray_intrinsic_left_bracket;
          darray_intrinsic_members;
          darray_intrinsic_right_bracket;
        }
      | (SyntaxKind.DictionaryIntrinsicExpression, [
          dictionary_intrinsic_keyword;
          dictionary_intrinsic_left_bracket;
          dictionary_intrinsic_members;
          dictionary_intrinsic_right_bracket;
        ]) ->
        DictionaryIntrinsicExpression {
          dictionary_intrinsic_keyword;
          dictionary_intrinsic_left_bracket;
          dictionary_intrinsic_members;
          dictionary_intrinsic_right_bracket;
        }
      | (SyntaxKind.KeysetIntrinsicExpression, [
          keyset_intrinsic_keyword;
          keyset_intrinsic_left_bracket;
          keyset_intrinsic_members;
          keyset_intrinsic_right_bracket;
        ]) ->
        KeysetIntrinsicExpression {
          keyset_intrinsic_keyword;
          keyset_intrinsic_left_bracket;
          keyset_intrinsic_members;
          keyset_intrinsic_right_bracket;
        }
      | (SyntaxKind.VarrayIntrinsicExpression, [
          varray_intrinsic_keyword;
          varray_intrinsic_left_bracket;
          varray_intrinsic_members;
          varray_intrinsic_right_bracket;
        ]) ->
        VarrayIntrinsicExpression {
          varray_intrinsic_keyword;
          varray_intrinsic_left_bracket;
          varray_intrinsic_members;
          varray_intrinsic_right_bracket;
        }
      | (SyntaxKind.VectorIntrinsicExpression, [
          vector_intrinsic_keyword;
          vector_intrinsic_left_bracket;
          vector_intrinsic_members;
          vector_intrinsic_right_bracket;
        ]) ->
        VectorIntrinsicExpression {
          vector_intrinsic_keyword;
          vector_intrinsic_left_bracket;
          vector_intrinsic_members;
          vector_intrinsic_right_bracket;
        }
      | (SyntaxKind.ElementInitializer, [
          element_key;
          element_arrow;
          element_value;
        ]) ->
        ElementInitializer {
          element_key;
          element_arrow;
          element_value;
        }
      | (SyntaxKind.SubscriptExpression, [
          subscript_receiver;
          subscript_left_bracket;
          subscript_index;
          subscript_right_bracket;
        ]) ->
        SubscriptExpression {
          subscript_receiver;
          subscript_left_bracket;
          subscript_index;
          subscript_right_bracket;
        }
      | (SyntaxKind.EmbeddedSubscriptExpression, [
          embedded_subscript_receiver;
          embedded_subscript_left_bracket;
          embedded_subscript_index;
          embedded_subscript_right_bracket;
        ]) ->
        EmbeddedSubscriptExpression {
          embedded_subscript_receiver;
          embedded_subscript_left_bracket;
          embedded_subscript_index;
          embedded_subscript_right_bracket;
        }
      | (SyntaxKind.AwaitableCreationExpression, [
          awaitable_async;
          awaitable_coroutine;
          awaitable_compound_statement;
        ]) ->
        AwaitableCreationExpression {
          awaitable_async;
          awaitable_coroutine;
          awaitable_compound_statement;
        }
      | (SyntaxKind.XHPChildrenDeclaration, [
          xhp_children_keyword;
          xhp_children_expression;
          xhp_children_semicolon;
        ]) ->
        XHPChildrenDeclaration {
          xhp_children_keyword;
          xhp_children_expression;
          xhp_children_semicolon;
        }
      | (SyntaxKind.XHPChildrenParenthesizedList, [
          xhp_children_list_left_paren;
          xhp_children_list_xhp_children;
          xhp_children_list_right_paren;
        ]) ->
        XHPChildrenParenthesizedList {
          xhp_children_list_left_paren;
          xhp_children_list_xhp_children;
          xhp_children_list_right_paren;
        }
      | (SyntaxKind.XHPCategoryDeclaration, [
          xhp_category_keyword;
          xhp_category_categories;
          xhp_category_semicolon;
        ]) ->
        XHPCategoryDeclaration {
          xhp_category_keyword;
          xhp_category_categories;
          xhp_category_semicolon;
        }
      | (SyntaxKind.XHPEnumType, [
          xhp_enum_keyword;
          xhp_enum_left_brace;
          xhp_enum_values;
          xhp_enum_right_brace;
        ]) ->
        XHPEnumType {
          xhp_enum_keyword;
          xhp_enum_left_brace;
          xhp_enum_values;
          xhp_enum_right_brace;
        }
      | (SyntaxKind.XHPRequired, [
          xhp_required_at;
          xhp_required_keyword;
        ]) ->
        XHPRequired {
          xhp_required_at;
          xhp_required_keyword;
        }
      | (SyntaxKind.XHPClassAttributeDeclaration, [
          xhp_attribute_keyword;
          xhp_attribute_attributes;
          xhp_attribute_semicolon;
        ]) ->
        XHPClassAttributeDeclaration {
          xhp_attribute_keyword;
          xhp_attribute_attributes;
          xhp_attribute_semicolon;
        }
      | (SyntaxKind.XHPClassAttribute, [
          xhp_attribute_decl_type;
          xhp_attribute_decl_name;
          xhp_attribute_decl_initializer;
          xhp_attribute_decl_required;
        ]) ->
        XHPClassAttribute {
          xhp_attribute_decl_type;
          xhp_attribute_decl_name;
          xhp_attribute_decl_initializer;
          xhp_attribute_decl_required;
        }
      | (SyntaxKind.XHPSimpleClassAttribute, [
          xhp_simple_class_attribute_type;
        ]) ->
        XHPSimpleClassAttribute {
          xhp_simple_class_attribute_type;
        }
      | (SyntaxKind.XHPAttribute, [
          xhp_attribute_name;
          xhp_attribute_equal;
          xhp_attribute_expression;
        ]) ->
        XHPAttribute {
          xhp_attribute_name;
          xhp_attribute_equal;
          xhp_attribute_expression;
        }
      | (SyntaxKind.XHPOpen, [
          xhp_open_left_angle;
          xhp_open_name;
          xhp_open_attributes;
          xhp_open_right_angle;
        ]) ->
        XHPOpen {
          xhp_open_left_angle;
          xhp_open_name;
          xhp_open_attributes;
          xhp_open_right_angle;
        }
      | (SyntaxKind.XHPExpression, [
          xhp_open;
          xhp_body;
          xhp_close;
        ]) ->
        XHPExpression {
          xhp_open;
          xhp_body;
          xhp_close;
        }
      | (SyntaxKind.XHPClose, [
          xhp_close_left_angle;
          xhp_close_name;
          xhp_close_right_angle;
        ]) ->
        XHPClose {
          xhp_close_left_angle;
          xhp_close_name;
          xhp_close_right_angle;
        }
      | (SyntaxKind.TypeConstant, [
          type_constant_left_type;
          type_constant_separator;
          type_constant_right_type;
        ]) ->
        TypeConstant {
          type_constant_left_type;
          type_constant_separator;
          type_constant_right_type;
        }
      | (SyntaxKind.VectorTypeSpecifier, [
          vector_type_keyword;
          vector_type_left_angle;
          vector_type_type;
          vector_type_trailing_comma;
          vector_type_right_angle;
        ]) ->
        VectorTypeSpecifier {
          vector_type_keyword;
          vector_type_left_angle;
          vector_type_type;
          vector_type_trailing_comma;
          vector_type_right_angle;
        }
      | (SyntaxKind.KeysetTypeSpecifier, [
          keyset_type_keyword;
          keyset_type_left_angle;
          keyset_type_type;
          keyset_type_trailing_comma;
          keyset_type_right_angle;
        ]) ->
        KeysetTypeSpecifier {
          keyset_type_keyword;
          keyset_type_left_angle;
          keyset_type_type;
          keyset_type_trailing_comma;
          keyset_type_right_angle;
        }
      | (SyntaxKind.TupleTypeExplicitSpecifier, [
          tuple_type_keyword;
          tuple_type_left_angle;
          tuple_type_types;
          tuple_type_right_angle;
        ]) ->
        TupleTypeExplicitSpecifier {
          tuple_type_keyword;
          tuple_type_left_angle;
          tuple_type_types;
          tuple_type_right_angle;
        }
      | (SyntaxKind.VarrayTypeSpecifier, [
          varray_keyword;
          varray_left_angle;
          varray_type;
          varray_trailing_comma;
          varray_right_angle;
        ]) ->
        VarrayTypeSpecifier {
          varray_keyword;
          varray_left_angle;
          varray_type;
          varray_trailing_comma;
          varray_right_angle;
        }
      | (SyntaxKind.VectorArrayTypeSpecifier, [
          vector_array_keyword;
          vector_array_left_angle;
          vector_array_type;
          vector_array_right_angle;
        ]) ->
        VectorArrayTypeSpecifier {
          vector_array_keyword;
          vector_array_left_angle;
          vector_array_type;
          vector_array_right_angle;
        }
      | (SyntaxKind.TypeParameter, [
          type_variance;
          type_name;
          type_constraints;
        ]) ->
        TypeParameter {
          type_variance;
          type_name;
          type_constraints;
        }
      | (SyntaxKind.TypeConstraint, [
          constraint_keyword;
          constraint_type;
        ]) ->
        TypeConstraint {
          constraint_keyword;
          constraint_type;
        }
      | (SyntaxKind.DarrayTypeSpecifier, [
          darray_keyword;
          darray_left_angle;
          darray_key;
          darray_comma;
          darray_value;
          darray_trailing_comma;
          darray_right_angle;
        ]) ->
        DarrayTypeSpecifier {
          darray_keyword;
          darray_left_angle;
          darray_key;
          darray_comma;
          darray_value;
          darray_trailing_comma;
          darray_right_angle;
        }
      | (SyntaxKind.MapArrayTypeSpecifier, [
          map_array_keyword;
          map_array_left_angle;
          map_array_key;
          map_array_comma;
          map_array_value;
          map_array_right_angle;
        ]) ->
        MapArrayTypeSpecifier {
          map_array_keyword;
          map_array_left_angle;
          map_array_key;
          map_array_comma;
          map_array_value;
          map_array_right_angle;
        }
      | (SyntaxKind.DictionaryTypeSpecifier, [
          dictionary_type_keyword;
          dictionary_type_left_angle;
          dictionary_type_members;
          dictionary_type_right_angle;
        ]) ->
        DictionaryTypeSpecifier {
          dictionary_type_keyword;
          dictionary_type_left_angle;
          dictionary_type_members;
          dictionary_type_right_angle;
        }
      | (SyntaxKind.ClosureTypeSpecifier, [
          closure_outer_left_paren;
          closure_coroutine;
          closure_function_keyword;
          closure_inner_left_paren;
          closure_parameter_types;
          closure_inner_right_paren;
          closure_colon;
          closure_return_type;
          closure_outer_right_paren;
        ]) ->
        ClosureTypeSpecifier {
          closure_outer_left_paren;
          closure_coroutine;
          closure_function_keyword;
          closure_inner_left_paren;
          closure_parameter_types;
          closure_inner_right_paren;
          closure_colon;
          closure_return_type;
          closure_outer_right_paren;
        }
      | (SyntaxKind.ClassnameTypeSpecifier, [
          classname_keyword;
          classname_left_angle;
          classname_type;
          classname_trailing_comma;
          classname_right_angle;
        ]) ->
        ClassnameTypeSpecifier {
          classname_keyword;
          classname_left_angle;
          classname_type;
          classname_trailing_comma;
          classname_right_angle;
        }
      | (SyntaxKind.FieldSpecifier, [
          field_question;
          field_name;
          field_arrow;
          field_type;
        ]) ->
        FieldSpecifier {
          field_question;
          field_name;
          field_arrow;
          field_type;
        }
      | (SyntaxKind.FieldInitializer, [
          field_initializer_name;
          field_initializer_arrow;
          field_initializer_value;
        ]) ->
        FieldInitializer {
          field_initializer_name;
          field_initializer_arrow;
          field_initializer_value;
        }
      | (SyntaxKind.ShapeTypeSpecifier, [
          shape_type_keyword;
          shape_type_left_paren;
          shape_type_fields;
          shape_type_ellipsis;
          shape_type_right_paren;
        ]) ->
        ShapeTypeSpecifier {
          shape_type_keyword;
          shape_type_left_paren;
          shape_type_fields;
          shape_type_ellipsis;
          shape_type_right_paren;
        }
      | (SyntaxKind.ShapeExpression, [
          shape_expression_keyword;
          shape_expression_left_paren;
          shape_expression_fields;
          shape_expression_right_paren;
        ]) ->
        ShapeExpression {
          shape_expression_keyword;
          shape_expression_left_paren;
          shape_expression_fields;
          shape_expression_right_paren;
        }
      | (SyntaxKind.TupleExpression, [
          tuple_expression_keyword;
          tuple_expression_left_paren;
          tuple_expression_items;
          tuple_expression_right_paren;
        ]) ->
        TupleExpression {
          tuple_expression_keyword;
          tuple_expression_left_paren;
          tuple_expression_items;
          tuple_expression_right_paren;
        }
      | (SyntaxKind.GenericTypeSpecifier, [
          generic_class_type;
          generic_argument_list;
        ]) ->
        GenericTypeSpecifier {
          generic_class_type;
          generic_argument_list;
        }
      | (SyntaxKind.NullableTypeSpecifier, [
          nullable_question;
          nullable_type;
        ]) ->
        NullableTypeSpecifier {
          nullable_question;
          nullable_type;
        }
      | (SyntaxKind.SoftTypeSpecifier, [
          soft_at;
          soft_type;
        ]) ->
        SoftTypeSpecifier {
          soft_at;
          soft_type;
        }
      | (SyntaxKind.TypeArguments, [
          type_arguments_left_angle;
          type_arguments_types;
          type_arguments_right_angle;
        ]) ->
        TypeArguments {
          type_arguments_left_angle;
          type_arguments_types;
          type_arguments_right_angle;
        }
      | (SyntaxKind.TypeParameters, [
          type_parameters_left_angle;
          type_parameters_parameters;
          type_parameters_right_angle;
        ]) ->
        TypeParameters {
          type_parameters_left_angle;
          type_parameters_parameters;
          type_parameters_right_angle;
        }
      | (SyntaxKind.TupleTypeSpecifier, [
          tuple_left_paren;
          tuple_types;
          tuple_right_paren;
        ]) ->
        TupleTypeSpecifier {
          tuple_left_paren;
          tuple_types;
          tuple_right_paren;
        }
      | (SyntaxKind.ErrorSyntax, [
          error_error;
        ]) ->
        ErrorSyntax {
          error_error;
        }
      | (SyntaxKind.ListItem, [
          list_item;
          list_separator;
        ]) ->
        ListItem {
          list_item;
          list_separator;
        }
      | (SyntaxKind.Missing, []) -> Missing
      | (SyntaxKind.SyntaxList, items) -> SyntaxList items
      | _ -> failwith
        "syntax_from_children called with wrong number of children"

    let all_tokens node =
      let rec aux acc nodes =
        match nodes with
        | [] -> acc
        | h :: t ->
          begin
            match syntax h with
            | Token token -> aux (token :: acc) t
            | _ -> aux (aux acc (children h)) t
          end in
      List.rev (aux [] [node])

    module type ValueBuilderType = sig
      val value_from_children:
        Full_fidelity_syntax_kind.t -> t list -> SyntaxValue.t
      val value_from_token: Token.t -> SyntaxValue.t
      val value_from_syntax: syntax -> SyntaxValue.t
    end

    module WithValueBuilder(ValueBuilder: ValueBuilderType) = struct
      let from_children kind ts =
        let syntax = syntax_from_children kind ts in
        let value = ValueBuilder.value_from_children kind ts in
        make syntax value

      let make_token token =
        let syntax = Token token in
        let value = ValueBuilder.value_from_token token in
        make syntax value

      let make_missing () =
        from_children SyntaxKind.Missing []

      (* An empty list is represented by Missing; everything else is a
        SyntaxList, even if the list has only one item. *)
      let make_list items =
        match items with
        | [] -> make_missing()
        | _ -> from_children SyntaxKind.SyntaxList items

      let make_end_of_file
        end_of_file_token
      =
        let syntax = EndOfFile {
          end_of_file_token;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_script
        script_declarations
      =
        let syntax = Script {
          script_declarations;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_simple_type_specifier
        simple_type_specifier
      =
        let syntax = SimpleTypeSpecifier {
          simple_type_specifier;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_literal_expression
        literal_expression
      =
        let syntax = LiteralExpression {
          literal_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_variable_expression
        variable_expression
      =
        let syntax = VariableExpression {
          variable_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_qualified_name_expression
        qualified_name_expression
      =
        let syntax = QualifiedNameExpression {
          qualified_name_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_pipe_variable_expression
        pipe_variable_expression
      =
        let syntax = PipeVariableExpression {
          pipe_variable_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enum_declaration
        enum_attribute_spec
        enum_keyword
        enum_name
        enum_colon
        enum_base
        enum_type
        enum_left_brace
        enum_enumerators
        enum_right_brace
      =
        let syntax = EnumDeclaration {
          enum_attribute_spec;
          enum_keyword;
          enum_name;
          enum_colon;
          enum_base;
          enum_type;
          enum_left_brace;
          enum_enumerators;
          enum_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_enumerator
        enumerator_name
        enumerator_equal
        enumerator_value
        enumerator_semicolon
      =
        let syntax = Enumerator {
          enumerator_name;
          enumerator_equal;
          enumerator_value;
          enumerator_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_alias_declaration
        alias_attribute_spec
        alias_keyword
        alias_name
        alias_generic_parameter
        alias_constraint
        alias_equal
        alias_type
        alias_semicolon
      =
        let syntax = AliasDeclaration {
          alias_attribute_spec;
          alias_keyword;
          alias_name;
          alias_generic_parameter;
          alias_constraint;
          alias_equal;
          alias_type;
          alias_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_property_declaration
        property_modifiers
        property_type
        property_declarators
        property_semicolon
      =
        let syntax = PropertyDeclaration {
          property_modifiers;
          property_type;
          property_declarators;
          property_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_property_declarator
        property_name
        property_initializer
      =
        let syntax = PropertyDeclarator {
          property_name;
          property_initializer;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_declaration
        namespace_keyword
        namespace_name
        namespace_body
      =
        let syntax = NamespaceDeclaration {
          namespace_keyword;
          namespace_name;
          namespace_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_body
        namespace_left_brace
        namespace_declarations
        namespace_right_brace
      =
        let syntax = NamespaceBody {
          namespace_left_brace;
          namespace_declarations;
          namespace_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_empty_body
        namespace_semicolon
      =
        let syntax = NamespaceEmptyBody {
          namespace_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_use_declaration
        namespace_use_keyword
        namespace_use_kind
        namespace_use_clauses
        namespace_use_semicolon
      =
        let syntax = NamespaceUseDeclaration {
          namespace_use_keyword;
          namespace_use_kind;
          namespace_use_clauses;
          namespace_use_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_group_use_declaration
        namespace_group_use_keyword
        namespace_group_use_kind
        namespace_group_use_prefix
        namespace_group_use_left_brace
        namespace_group_use_clauses
        namespace_group_use_right_brace
        namespace_group_use_semicolon
      =
        let syntax = NamespaceGroupUseDeclaration {
          namespace_group_use_keyword;
          namespace_group_use_kind;
          namespace_group_use_prefix;
          namespace_group_use_left_brace;
          namespace_group_use_clauses;
          namespace_group_use_right_brace;
          namespace_group_use_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_namespace_use_clause
        namespace_use_clause_kind
        namespace_use_name
        namespace_use_as
        namespace_use_alias
      =
        let syntax = NamespaceUseClause {
          namespace_use_clause_kind;
          namespace_use_name;
          namespace_use_as;
          namespace_use_alias;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_declaration
        function_attribute_spec
        function_declaration_header
        function_body
      =
        let syntax = FunctionDeclaration {
          function_attribute_spec;
          function_declaration_header;
          function_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_declaration_header
        function_async
        function_coroutine
        function_keyword
        function_ampersand
        function_name
        function_type_parameter_list
        function_left_paren
        function_parameter_list
        function_right_paren
        function_colon
        function_type
        function_where_clause
      =
        let syntax = FunctionDeclarationHeader {
          function_async;
          function_coroutine;
          function_keyword;
          function_ampersand;
          function_name;
          function_type_parameter_list;
          function_left_paren;
          function_parameter_list;
          function_right_paren;
          function_colon;
          function_type;
          function_where_clause;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_where_clause
        where_clause_keyword
        where_clause_constraints
      =
        let syntax = WhereClause {
          where_clause_keyword;
          where_clause_constraints;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_where_constraint
        where_constraint_left_type
        where_constraint_operator
        where_constraint_right_type
      =
        let syntax = WhereConstraint {
          where_constraint_left_type;
          where_constraint_operator;
          where_constraint_right_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_methodish_declaration
        methodish_attribute
        methodish_modifiers
        methodish_function_decl_header
        methodish_function_body
        methodish_semicolon
      =
        let syntax = MethodishDeclaration {
          methodish_attribute;
          methodish_modifiers;
          methodish_function_decl_header;
          methodish_function_body;
          methodish_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classish_declaration
        classish_attribute
        classish_modifiers
        classish_keyword
        classish_name
        classish_type_parameters
        classish_extends_keyword
        classish_extends_list
        classish_implements_keyword
        classish_implements_list
        classish_body
      =
        let syntax = ClassishDeclaration {
          classish_attribute;
          classish_modifiers;
          classish_keyword;
          classish_name;
          classish_type_parameters;
          classish_extends_keyword;
          classish_extends_list;
          classish_implements_keyword;
          classish_implements_list;
          classish_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classish_body
        classish_body_left_brace
        classish_body_elements
        classish_body_right_brace
      =
        let syntax = ClassishBody {
          classish_body_left_brace;
          classish_body_elements;
          classish_body_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_trait_use_precedence_item
        trait_use_precedence_item_name
        trait_use_precedence_item_keyword
        trait_use_precedence_item_removed_names
      =
        let syntax = TraitUsePrecedenceItem {
          trait_use_precedence_item_name;
          trait_use_precedence_item_keyword;
          trait_use_precedence_item_removed_names;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_trait_use_alias_item
        trait_use_alias_item_aliasing_name
        trait_use_alias_item_keyword
        trait_use_alias_item_visibility
        trait_use_alias_item_aliased_name
      =
        let syntax = TraitUseAliasItem {
          trait_use_alias_item_aliasing_name;
          trait_use_alias_item_keyword;
          trait_use_alias_item_visibility;
          trait_use_alias_item_aliased_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_trait_use_conflict_resolution
        trait_use_conflict_resolution_keyword
        trait_use_conflict_resolution_names
        trait_use_conflict_resolution_left_brace
        trait_use_conflict_resolution_clauses
        trait_use_conflict_resolution_right_brace
      =
        let syntax = TraitUseConflictResolution {
          trait_use_conflict_resolution_keyword;
          trait_use_conflict_resolution_names;
          trait_use_conflict_resolution_left_brace;
          trait_use_conflict_resolution_clauses;
          trait_use_conflict_resolution_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_trait_use
        trait_use_keyword
        trait_use_names
        trait_use_semicolon
      =
        let syntax = TraitUse {
          trait_use_keyword;
          trait_use_names;
          trait_use_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_require_clause
        require_keyword
        require_kind
        require_name
        require_semicolon
      =
        let syntax = RequireClause {
          require_keyword;
          require_kind;
          require_name;
          require_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_const_declaration
        const_abstract
        const_keyword
        const_type_specifier
        const_declarators
        const_semicolon
      =
        let syntax = ConstDeclaration {
          const_abstract;
          const_keyword;
          const_type_specifier;
          const_declarators;
          const_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_constant_declarator
        constant_declarator_name
        constant_declarator_initializer
      =
        let syntax = ConstantDeclarator {
          constant_declarator_name;
          constant_declarator_initializer;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_const_declaration
        type_const_abstract
        type_const_keyword
        type_const_type_keyword
        type_const_name
        type_const_type_constraint
        type_const_equal
        type_const_type_specifier
        type_const_semicolon
      =
        let syntax = TypeConstDeclaration {
          type_const_abstract;
          type_const_keyword;
          type_const_type_keyword;
          type_const_name;
          type_const_type_constraint;
          type_const_equal;
          type_const_type_specifier;
          type_const_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_decorated_expression
        decorated_expression_decorator
        decorated_expression_expression
      =
        let syntax = DecoratedExpression {
          decorated_expression_decorator;
          decorated_expression_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_parameter_declaration
        parameter_attribute
        parameter_visibility
        parameter_call_convention
        parameter_type
        parameter_name
        parameter_default_value
      =
        let syntax = ParameterDeclaration {
          parameter_attribute;
          parameter_visibility;
          parameter_call_convention;
          parameter_type;
          parameter_name;
          parameter_default_value;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_variadic_parameter
        variadic_parameter_type
        variadic_parameter_ellipsis
      =
        let syntax = VariadicParameter {
          variadic_parameter_type;
          variadic_parameter_ellipsis;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_attribute_specification
        attribute_specification_left_double_angle
        attribute_specification_attributes
        attribute_specification_right_double_angle
      =
        let syntax = AttributeSpecification {
          attribute_specification_left_double_angle;
          attribute_specification_attributes;
          attribute_specification_right_double_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_attribute
        attribute_name
        attribute_left_paren
        attribute_values
        attribute_right_paren
      =
        let syntax = Attribute {
          attribute_name;
          attribute_left_paren;
          attribute_values;
          attribute_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_inclusion_expression
        inclusion_require
        inclusion_filename
      =
        let syntax = InclusionExpression {
          inclusion_require;
          inclusion_filename;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_inclusion_directive
        inclusion_expression
        inclusion_semicolon
      =
        let syntax = InclusionDirective {
          inclusion_expression;
          inclusion_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_compound_statement
        compound_left_brace
        compound_statements
        compound_right_brace
      =
        let syntax = CompoundStatement {
          compound_left_brace;
          compound_statements;
          compound_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_expression_statement
        expression_statement_expression
        expression_statement_semicolon
      =
        let syntax = ExpressionStatement {
          expression_statement_expression;
          expression_statement_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_markup_section
        markup_prefix
        markup_text
        markup_suffix
        markup_expression
      =
        let syntax = MarkupSection {
          markup_prefix;
          markup_text;
          markup_suffix;
          markup_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_markup_suffix
        markup_suffix_less_than_question
        markup_suffix_name
      =
        let syntax = MarkupSuffix {
          markup_suffix_less_than_question;
          markup_suffix_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_unset_statement
        unset_keyword
        unset_left_paren
        unset_variables
        unset_right_paren
        unset_semicolon
      =
        let syntax = UnsetStatement {
          unset_keyword;
          unset_left_paren;
          unset_variables;
          unset_right_paren;
          unset_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_using_statement_block_scoped
        using_block_await_keyword
        using_block_using_keyword
        using_block_left_paren
        using_block_expressions
        using_block_right_paren
        using_block_body
      =
        let syntax = UsingStatementBlockScoped {
          using_block_await_keyword;
          using_block_using_keyword;
          using_block_left_paren;
          using_block_expressions;
          using_block_right_paren;
          using_block_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_using_statement_function_scoped
        using_function_await_keyword
        using_function_using_keyword
        using_function_expression
        using_function_semicolon
      =
        let syntax = UsingStatementFunctionScoped {
          using_function_await_keyword;
          using_function_using_keyword;
          using_function_expression;
          using_function_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_while_statement
        while_keyword
        while_left_paren
        while_condition
        while_right_paren
        while_body
      =
        let syntax = WhileStatement {
          while_keyword;
          while_left_paren;
          while_condition;
          while_right_paren;
          while_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_if_statement
        if_keyword
        if_left_paren
        if_condition
        if_right_paren
        if_statement
        if_elseif_clauses
        if_else_clause
      =
        let syntax = IfStatement {
          if_keyword;
          if_left_paren;
          if_condition;
          if_right_paren;
          if_statement;
          if_elseif_clauses;
          if_else_clause;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_elseif_clause
        elseif_keyword
        elseif_left_paren
        elseif_condition
        elseif_right_paren
        elseif_statement
      =
        let syntax = ElseifClause {
          elseif_keyword;
          elseif_left_paren;
          elseif_condition;
          elseif_right_paren;
          elseif_statement;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_else_clause
        else_keyword
        else_statement
      =
        let syntax = ElseClause {
          else_keyword;
          else_statement;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_if_endif_statement
        if_endif_keyword
        if_endif_left_paren
        if_endif_condition
        if_endif_right_paren
        if_endif_colon
        if_endif_statement
        if_endif_elseif_colon_clauses
        if_endif_else_colon_clause
        if_endif_endif_keyword
        if_endif_semicolon
      =
        let syntax = IfEndIfStatement {
          if_endif_keyword;
          if_endif_left_paren;
          if_endif_condition;
          if_endif_right_paren;
          if_endif_colon;
          if_endif_statement;
          if_endif_elseif_colon_clauses;
          if_endif_else_colon_clause;
          if_endif_endif_keyword;
          if_endif_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_elseif_colon_clause
        elseif_colon_keyword
        elseif_colon_left_paren
        elseif_colon_condition
        elseif_colon_right_paren
        elseif_colon_colon
        elseif_colon_statement
      =
        let syntax = ElseifColonClause {
          elseif_colon_keyword;
          elseif_colon_left_paren;
          elseif_colon_condition;
          elseif_colon_right_paren;
          elseif_colon_colon;
          elseif_colon_statement;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_else_colon_clause
        else_colon_keyword
        else_colon_colon
        else_colon_statement
      =
        let syntax = ElseColonClause {
          else_colon_keyword;
          else_colon_colon;
          else_colon_statement;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_try_statement
        try_keyword
        try_compound_statement
        try_catch_clauses
        try_finally_clause
      =
        let syntax = TryStatement {
          try_keyword;
          try_compound_statement;
          try_catch_clauses;
          try_finally_clause;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_catch_clause
        catch_keyword
        catch_left_paren
        catch_type
        catch_variable
        catch_right_paren
        catch_body
      =
        let syntax = CatchClause {
          catch_keyword;
          catch_left_paren;
          catch_type;
          catch_variable;
          catch_right_paren;
          catch_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_finally_clause
        finally_keyword
        finally_body
      =
        let syntax = FinallyClause {
          finally_keyword;
          finally_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_do_statement
        do_keyword
        do_body
        do_while_keyword
        do_left_paren
        do_condition
        do_right_paren
        do_semicolon
      =
        let syntax = DoStatement {
          do_keyword;
          do_body;
          do_while_keyword;
          do_left_paren;
          do_condition;
          do_right_paren;
          do_semicolon;
        } in
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
        for_body
      =
        let syntax = ForStatement {
          for_keyword;
          for_left_paren;
          for_initializer;
          for_first_semicolon;
          for_control;
          for_second_semicolon;
          for_end_of_loop;
          for_right_paren;
          for_body;
        } in
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
        foreach_body
      =
        let syntax = ForeachStatement {
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
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_statement
        switch_keyword
        switch_left_paren
        switch_expression
        switch_right_paren
        switch_left_brace
        switch_sections
        switch_right_brace
      =
        let syntax = SwitchStatement {
          switch_keyword;
          switch_left_paren;
          switch_expression;
          switch_right_paren;
          switch_left_brace;
          switch_sections;
          switch_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_section
        switch_section_labels
        switch_section_statements
        switch_section_fallthrough
      =
        let syntax = SwitchSection {
          switch_section_labels;
          switch_section_statements;
          switch_section_fallthrough;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_switch_fallthrough
        fallthrough_keyword
        fallthrough_semicolon
      =
        let syntax = SwitchFallthrough {
          fallthrough_keyword;
          fallthrough_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_case_label
        case_keyword
        case_expression
        case_colon
      =
        let syntax = CaseLabel {
          case_keyword;
          case_expression;
          case_colon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_default_label
        default_keyword
        default_colon
      =
        let syntax = DefaultLabel {
          default_keyword;
          default_colon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_return_statement
        return_keyword
        return_expression
        return_semicolon
      =
        let syntax = ReturnStatement {
          return_keyword;
          return_expression;
          return_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_goto_label
        goto_label_name
        goto_label_colon
      =
        let syntax = GotoLabel {
          goto_label_name;
          goto_label_colon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_goto_statement
        goto_statement_keyword
        goto_statement_label_name
        goto_statement_semicolon
      =
        let syntax = GotoStatement {
          goto_statement_keyword;
          goto_statement_label_name;
          goto_statement_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_throw_statement
        throw_keyword
        throw_expression
        throw_semicolon
      =
        let syntax = ThrowStatement {
          throw_keyword;
          throw_expression;
          throw_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_break_statement
        break_keyword
        break_level
        break_semicolon
      =
        let syntax = BreakStatement {
          break_keyword;
          break_level;
          break_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_continue_statement
        continue_keyword
        continue_level
        continue_semicolon
      =
        let syntax = ContinueStatement {
          continue_keyword;
          continue_level;
          continue_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_static_statement
        static_static_keyword
        static_declarations
        static_semicolon
      =
        let syntax = FunctionStaticStatement {
          static_static_keyword;
          static_declarations;
          static_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_static_declarator
        static_name
        static_initializer
      =
        let syntax = StaticDeclarator {
          static_name;
          static_initializer;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_echo_statement
        echo_keyword
        echo_expressions
        echo_semicolon
      =
        let syntax = EchoStatement {
          echo_keyword;
          echo_expressions;
          echo_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_global_statement
        global_keyword
        global_variables
        global_semicolon
      =
        let syntax = GlobalStatement {
          global_keyword;
          global_variables;
          global_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_simple_initializer
        simple_initializer_equal
        simple_initializer_value
      =
        let syntax = SimpleInitializer {
          simple_initializer_equal;
          simple_initializer_value;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_anonymous_function
        anonymous_static_keyword
        anonymous_async_keyword
        anonymous_coroutine_keyword
        anonymous_function_keyword
        anonymous_left_paren
        anonymous_parameters
        anonymous_right_paren
        anonymous_colon
        anonymous_type
        anonymous_use
        anonymous_body
      =
        let syntax = AnonymousFunction {
          anonymous_static_keyword;
          anonymous_async_keyword;
          anonymous_coroutine_keyword;
          anonymous_function_keyword;
          anonymous_left_paren;
          anonymous_parameters;
          anonymous_right_paren;
          anonymous_colon;
          anonymous_type;
          anonymous_use;
          anonymous_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_anonymous_function_use_clause
        anonymous_use_keyword
        anonymous_use_left_paren
        anonymous_use_variables
        anonymous_use_right_paren
      =
        let syntax = AnonymousFunctionUseClause {
          anonymous_use_keyword;
          anonymous_use_left_paren;
          anonymous_use_variables;
          anonymous_use_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_lambda_expression
        lambda_async
        lambda_coroutine
        lambda_signature
        lambda_arrow
        lambda_body
      =
        let syntax = LambdaExpression {
          lambda_async;
          lambda_coroutine;
          lambda_signature;
          lambda_arrow;
          lambda_body;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_lambda_signature
        lambda_left_paren
        lambda_parameters
        lambda_right_paren
        lambda_colon
        lambda_type
      =
        let syntax = LambdaSignature {
          lambda_left_paren;
          lambda_parameters;
          lambda_right_paren;
          lambda_colon;
          lambda_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_cast_expression
        cast_left_paren
        cast_type
        cast_right_paren
        cast_operand
      =
        let syntax = CastExpression {
          cast_left_paren;
          cast_type;
          cast_right_paren;
          cast_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_scope_resolution_expression
        scope_resolution_qualifier
        scope_resolution_operator
        scope_resolution_name
      =
        let syntax = ScopeResolutionExpression {
          scope_resolution_qualifier;
          scope_resolution_operator;
          scope_resolution_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_member_selection_expression
        member_object
        member_operator
        member_name
      =
        let syntax = MemberSelectionExpression {
          member_object;
          member_operator;
          member_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_safe_member_selection_expression
        safe_member_object
        safe_member_operator
        safe_member_name
      =
        let syntax = SafeMemberSelectionExpression {
          safe_member_object;
          safe_member_operator;
          safe_member_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_member_selection_expression
        embedded_member_object
        embedded_member_operator
        embedded_member_name
      =
        let syntax = EmbeddedMemberSelectionExpression {
          embedded_member_object;
          embedded_member_operator;
          embedded_member_name;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_yield_expression
        yield_keyword
        yield_operand
      =
        let syntax = YieldExpression {
          yield_keyword;
          yield_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_yield_from_expression
        yield_from_yield_keyword
        yield_from_from_keyword
        yield_from_operand
      =
        let syntax = YieldFromExpression {
          yield_from_yield_keyword;
          yield_from_from_keyword;
          yield_from_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_prefix_unary_expression
        prefix_unary_operator
        prefix_unary_operand
      =
        let syntax = PrefixUnaryExpression {
          prefix_unary_operator;
          prefix_unary_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_postfix_unary_expression
        postfix_unary_operand
        postfix_unary_operator
      =
        let syntax = PostfixUnaryExpression {
          postfix_unary_operand;
          postfix_unary_operator;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_binary_expression
        binary_left_operand
        binary_operator
        binary_right_operand
      =
        let syntax = BinaryExpression {
          binary_left_operand;
          binary_operator;
          binary_right_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_instanceof_expression
        instanceof_left_operand
        instanceof_operator
        instanceof_right_operand
      =
        let syntax = InstanceofExpression {
          instanceof_left_operand;
          instanceof_operator;
          instanceof_right_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_is_expression
        is_left_operand
        is_operator
        is_right_operand
      =
        let syntax = IsExpression {
          is_left_operand;
          is_operator;
          is_right_operand;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_conditional_expression
        conditional_test
        conditional_question
        conditional_consequence
        conditional_colon
        conditional_alternative
      =
        let syntax = ConditionalExpression {
          conditional_test;
          conditional_question;
          conditional_consequence;
          conditional_colon;
          conditional_alternative;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_eval_expression
        eval_keyword
        eval_left_paren
        eval_argument
        eval_right_paren
      =
        let syntax = EvalExpression {
          eval_keyword;
          eval_left_paren;
          eval_argument;
          eval_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_empty_expression
        empty_keyword
        empty_left_paren
        empty_argument
        empty_right_paren
      =
        let syntax = EmptyExpression {
          empty_keyword;
          empty_left_paren;
          empty_argument;
          empty_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_define_expression
        define_keyword
        define_left_paren
        define_argument_list
        define_right_paren
      =
        let syntax = DefineExpression {
          define_keyword;
          define_left_paren;
          define_argument_list;
          define_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_isset_expression
        isset_keyword
        isset_left_paren
        isset_argument_list
        isset_right_paren
      =
        let syntax = IssetExpression {
          isset_keyword;
          isset_left_paren;
          isset_argument_list;
          isset_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_call_expression
        function_call_receiver
        function_call_left_paren
        function_call_argument_list
        function_call_right_paren
      =
        let syntax = FunctionCallExpression {
          function_call_receiver;
          function_call_left_paren;
          function_call_argument_list;
          function_call_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_function_call_with_type_arguments_expression
        function_call_with_type_arguments_receiver
        function_call_with_type_arguments_type_args
        function_call_with_type_arguments_left_paren
        function_call_with_type_arguments_argument_list
        function_call_with_type_arguments_right_paren
      =
        let syntax = FunctionCallWithTypeArgumentsExpression {
          function_call_with_type_arguments_receiver;
          function_call_with_type_arguments_type_args;
          function_call_with_type_arguments_left_paren;
          function_call_with_type_arguments_argument_list;
          function_call_with_type_arguments_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_parenthesized_expression
        parenthesized_expression_left_paren
        parenthesized_expression_expression
        parenthesized_expression_right_paren
      =
        let syntax = ParenthesizedExpression {
          parenthesized_expression_left_paren;
          parenthesized_expression_expression;
          parenthesized_expression_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_braced_expression
        braced_expression_left_brace
        braced_expression_expression
        braced_expression_right_brace
      =
        let syntax = BracedExpression {
          braced_expression_left_brace;
          braced_expression_expression;
          braced_expression_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_braced_expression
        embedded_braced_expression_left_brace
        embedded_braced_expression_expression
        embedded_braced_expression_right_brace
      =
        let syntax = EmbeddedBracedExpression {
          embedded_braced_expression_left_brace;
          embedded_braced_expression_expression;
          embedded_braced_expression_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_list_expression
        list_keyword
        list_left_paren
        list_members
        list_right_paren
      =
        let syntax = ListExpression {
          list_keyword;
          list_left_paren;
          list_members;
          list_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_collection_literal_expression
        collection_literal_name
        collection_literal_left_brace
        collection_literal_initializers
        collection_literal_right_brace
      =
        let syntax = CollectionLiteralExpression {
          collection_literal_name;
          collection_literal_left_brace;
          collection_literal_initializers;
          collection_literal_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_object_creation_expression
        object_creation_new_keyword
        object_creation_type
        object_creation_left_paren
        object_creation_argument_list
        object_creation_right_paren
      =
        let syntax = ObjectCreationExpression {
          object_creation_new_keyword;
          object_creation_type;
          object_creation_left_paren;
          object_creation_argument_list;
          object_creation_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_array_creation_expression
        array_creation_left_bracket
        array_creation_members
        array_creation_right_bracket
      =
        let syntax = ArrayCreationExpression {
          array_creation_left_bracket;
          array_creation_members;
          array_creation_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_array_intrinsic_expression
        array_intrinsic_keyword
        array_intrinsic_left_paren
        array_intrinsic_members
        array_intrinsic_right_paren
      =
        let syntax = ArrayIntrinsicExpression {
          array_intrinsic_keyword;
          array_intrinsic_left_paren;
          array_intrinsic_members;
          array_intrinsic_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_darray_intrinsic_expression
        darray_intrinsic_keyword
        darray_intrinsic_left_bracket
        darray_intrinsic_members
        darray_intrinsic_right_bracket
      =
        let syntax = DarrayIntrinsicExpression {
          darray_intrinsic_keyword;
          darray_intrinsic_left_bracket;
          darray_intrinsic_members;
          darray_intrinsic_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_dictionary_intrinsic_expression
        dictionary_intrinsic_keyword
        dictionary_intrinsic_left_bracket
        dictionary_intrinsic_members
        dictionary_intrinsic_right_bracket
      =
        let syntax = DictionaryIntrinsicExpression {
          dictionary_intrinsic_keyword;
          dictionary_intrinsic_left_bracket;
          dictionary_intrinsic_members;
          dictionary_intrinsic_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_keyset_intrinsic_expression
        keyset_intrinsic_keyword
        keyset_intrinsic_left_bracket
        keyset_intrinsic_members
        keyset_intrinsic_right_bracket
      =
        let syntax = KeysetIntrinsicExpression {
          keyset_intrinsic_keyword;
          keyset_intrinsic_left_bracket;
          keyset_intrinsic_members;
          keyset_intrinsic_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_varray_intrinsic_expression
        varray_intrinsic_keyword
        varray_intrinsic_left_bracket
        varray_intrinsic_members
        varray_intrinsic_right_bracket
      =
        let syntax = VarrayIntrinsicExpression {
          varray_intrinsic_keyword;
          varray_intrinsic_left_bracket;
          varray_intrinsic_members;
          varray_intrinsic_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_vector_intrinsic_expression
        vector_intrinsic_keyword
        vector_intrinsic_left_bracket
        vector_intrinsic_members
        vector_intrinsic_right_bracket
      =
        let syntax = VectorIntrinsicExpression {
          vector_intrinsic_keyword;
          vector_intrinsic_left_bracket;
          vector_intrinsic_members;
          vector_intrinsic_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_element_initializer
        element_key
        element_arrow
        element_value
      =
        let syntax = ElementInitializer {
          element_key;
          element_arrow;
          element_value;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_subscript_expression
        subscript_receiver
        subscript_left_bracket
        subscript_index
        subscript_right_bracket
      =
        let syntax = SubscriptExpression {
          subscript_receiver;
          subscript_left_bracket;
          subscript_index;
          subscript_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_embedded_subscript_expression
        embedded_subscript_receiver
        embedded_subscript_left_bracket
        embedded_subscript_index
        embedded_subscript_right_bracket
      =
        let syntax = EmbeddedSubscriptExpression {
          embedded_subscript_receiver;
          embedded_subscript_left_bracket;
          embedded_subscript_index;
          embedded_subscript_right_bracket;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_awaitable_creation_expression
        awaitable_async
        awaitable_coroutine
        awaitable_compound_statement
      =
        let syntax = AwaitableCreationExpression {
          awaitable_async;
          awaitable_coroutine;
          awaitable_compound_statement;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_children_declaration
        xhp_children_keyword
        xhp_children_expression
        xhp_children_semicolon
      =
        let syntax = XHPChildrenDeclaration {
          xhp_children_keyword;
          xhp_children_expression;
          xhp_children_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_children_parenthesized_list
        xhp_children_list_left_paren
        xhp_children_list_xhp_children
        xhp_children_list_right_paren
      =
        let syntax = XHPChildrenParenthesizedList {
          xhp_children_list_left_paren;
          xhp_children_list_xhp_children;
          xhp_children_list_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_category_declaration
        xhp_category_keyword
        xhp_category_categories
        xhp_category_semicolon
      =
        let syntax = XHPCategoryDeclaration {
          xhp_category_keyword;
          xhp_category_categories;
          xhp_category_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_enum_type
        xhp_enum_keyword
        xhp_enum_left_brace
        xhp_enum_values
        xhp_enum_right_brace
      =
        let syntax = XHPEnumType {
          xhp_enum_keyword;
          xhp_enum_left_brace;
          xhp_enum_values;
          xhp_enum_right_brace;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_required
        xhp_required_at
        xhp_required_keyword
      =
        let syntax = XHPRequired {
          xhp_required_at;
          xhp_required_keyword;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_class_attribute_declaration
        xhp_attribute_keyword
        xhp_attribute_attributes
        xhp_attribute_semicolon
      =
        let syntax = XHPClassAttributeDeclaration {
          xhp_attribute_keyword;
          xhp_attribute_attributes;
          xhp_attribute_semicolon;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_class_attribute
        xhp_attribute_decl_type
        xhp_attribute_decl_name
        xhp_attribute_decl_initializer
        xhp_attribute_decl_required
      =
        let syntax = XHPClassAttribute {
          xhp_attribute_decl_type;
          xhp_attribute_decl_name;
          xhp_attribute_decl_initializer;
          xhp_attribute_decl_required;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_simple_class_attribute
        xhp_simple_class_attribute_type
      =
        let syntax = XHPSimpleClassAttribute {
          xhp_simple_class_attribute_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_attribute
        xhp_attribute_name
        xhp_attribute_equal
        xhp_attribute_expression
      =
        let syntax = XHPAttribute {
          xhp_attribute_name;
          xhp_attribute_equal;
          xhp_attribute_expression;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_open
        xhp_open_left_angle
        xhp_open_name
        xhp_open_attributes
        xhp_open_right_angle
      =
        let syntax = XHPOpen {
          xhp_open_left_angle;
          xhp_open_name;
          xhp_open_attributes;
          xhp_open_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_expression
        xhp_open
        xhp_body
        xhp_close
      =
        let syntax = XHPExpression {
          xhp_open;
          xhp_body;
          xhp_close;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_xhp_close
        xhp_close_left_angle
        xhp_close_name
        xhp_close_right_angle
      =
        let syntax = XHPClose {
          xhp_close_left_angle;
          xhp_close_name;
          xhp_close_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_constant
        type_constant_left_type
        type_constant_separator
        type_constant_right_type
      =
        let syntax = TypeConstant {
          type_constant_left_type;
          type_constant_separator;
          type_constant_right_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_vector_type_specifier
        vector_type_keyword
        vector_type_left_angle
        vector_type_type
        vector_type_trailing_comma
        vector_type_right_angle
      =
        let syntax = VectorTypeSpecifier {
          vector_type_keyword;
          vector_type_left_angle;
          vector_type_type;
          vector_type_trailing_comma;
          vector_type_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_keyset_type_specifier
        keyset_type_keyword
        keyset_type_left_angle
        keyset_type_type
        keyset_type_trailing_comma
        keyset_type_right_angle
      =
        let syntax = KeysetTypeSpecifier {
          keyset_type_keyword;
          keyset_type_left_angle;
          keyset_type_type;
          keyset_type_trailing_comma;
          keyset_type_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_type_explicit_specifier
        tuple_type_keyword
        tuple_type_left_angle
        tuple_type_types
        tuple_type_right_angle
      =
        let syntax = TupleTypeExplicitSpecifier {
          tuple_type_keyword;
          tuple_type_left_angle;
          tuple_type_types;
          tuple_type_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_varray_type_specifier
        varray_keyword
        varray_left_angle
        varray_type
        varray_trailing_comma
        varray_right_angle
      =
        let syntax = VarrayTypeSpecifier {
          varray_keyword;
          varray_left_angle;
          varray_type;
          varray_trailing_comma;
          varray_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_vector_array_type_specifier
        vector_array_keyword
        vector_array_left_angle
        vector_array_type
        vector_array_right_angle
      =
        let syntax = VectorArrayTypeSpecifier {
          vector_array_keyword;
          vector_array_left_angle;
          vector_array_type;
          vector_array_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_parameter
        type_variance
        type_name
        type_constraints
      =
        let syntax = TypeParameter {
          type_variance;
          type_name;
          type_constraints;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_constraint
        constraint_keyword
        constraint_type
      =
        let syntax = TypeConstraint {
          constraint_keyword;
          constraint_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_darray_type_specifier
        darray_keyword
        darray_left_angle
        darray_key
        darray_comma
        darray_value
        darray_trailing_comma
        darray_right_angle
      =
        let syntax = DarrayTypeSpecifier {
          darray_keyword;
          darray_left_angle;
          darray_key;
          darray_comma;
          darray_value;
          darray_trailing_comma;
          darray_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_map_array_type_specifier
        map_array_keyword
        map_array_left_angle
        map_array_key
        map_array_comma
        map_array_value
        map_array_right_angle
      =
        let syntax = MapArrayTypeSpecifier {
          map_array_keyword;
          map_array_left_angle;
          map_array_key;
          map_array_comma;
          map_array_value;
          map_array_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_dictionary_type_specifier
        dictionary_type_keyword
        dictionary_type_left_angle
        dictionary_type_members
        dictionary_type_right_angle
      =
        let syntax = DictionaryTypeSpecifier {
          dictionary_type_keyword;
          dictionary_type_left_angle;
          dictionary_type_members;
          dictionary_type_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_closure_type_specifier
        closure_outer_left_paren
        closure_coroutine
        closure_function_keyword
        closure_inner_left_paren
        closure_parameter_types
        closure_inner_right_paren
        closure_colon
        closure_return_type
        closure_outer_right_paren
      =
        let syntax = ClosureTypeSpecifier {
          closure_outer_left_paren;
          closure_coroutine;
          closure_function_keyword;
          closure_inner_left_paren;
          closure_parameter_types;
          closure_inner_right_paren;
          closure_colon;
          closure_return_type;
          closure_outer_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_classname_type_specifier
        classname_keyword
        classname_left_angle
        classname_type
        classname_trailing_comma
        classname_right_angle
      =
        let syntax = ClassnameTypeSpecifier {
          classname_keyword;
          classname_left_angle;
          classname_type;
          classname_trailing_comma;
          classname_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_field_specifier
        field_question
        field_name
        field_arrow
        field_type
      =
        let syntax = FieldSpecifier {
          field_question;
          field_name;
          field_arrow;
          field_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_field_initializer
        field_initializer_name
        field_initializer_arrow
        field_initializer_value
      =
        let syntax = FieldInitializer {
          field_initializer_name;
          field_initializer_arrow;
          field_initializer_value;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_shape_type_specifier
        shape_type_keyword
        shape_type_left_paren
        shape_type_fields
        shape_type_ellipsis
        shape_type_right_paren
      =
        let syntax = ShapeTypeSpecifier {
          shape_type_keyword;
          shape_type_left_paren;
          shape_type_fields;
          shape_type_ellipsis;
          shape_type_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_shape_expression
        shape_expression_keyword
        shape_expression_left_paren
        shape_expression_fields
        shape_expression_right_paren
      =
        let syntax = ShapeExpression {
          shape_expression_keyword;
          shape_expression_left_paren;
          shape_expression_fields;
          shape_expression_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_expression
        tuple_expression_keyword
        tuple_expression_left_paren
        tuple_expression_items
        tuple_expression_right_paren
      =
        let syntax = TupleExpression {
          tuple_expression_keyword;
          tuple_expression_left_paren;
          tuple_expression_items;
          tuple_expression_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_generic_type_specifier
        generic_class_type
        generic_argument_list
      =
        let syntax = GenericTypeSpecifier {
          generic_class_type;
          generic_argument_list;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_nullable_type_specifier
        nullable_question
        nullable_type
      =
        let syntax = NullableTypeSpecifier {
          nullable_question;
          nullable_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_soft_type_specifier
        soft_at
        soft_type
      =
        let syntax = SoftTypeSpecifier {
          soft_at;
          soft_type;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_arguments
        type_arguments_left_angle
        type_arguments_types
        type_arguments_right_angle
      =
        let syntax = TypeArguments {
          type_arguments_left_angle;
          type_arguments_types;
          type_arguments_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_type_parameters
        type_parameters_left_angle
        type_parameters_parameters
        type_parameters_right_angle
      =
        let syntax = TypeParameters {
          type_parameters_left_angle;
          type_parameters_parameters;
          type_parameters_right_angle;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_tuple_type_specifier
        tuple_left_paren
        tuple_types
        tuple_right_paren
      =
        let syntax = TupleTypeSpecifier {
          tuple_left_paren;
          tuple_types;
          tuple_right_paren;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_error
        error_error
      =
        let syntax = ErrorSyntax {
          error_error;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

      let make_list_item
        list_item
        list_separator
      =
        let syntax = ListItem {
          list_item;
          list_separator;
        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value



      (* Takes a node and a function from token to token and returns a node with
      the function applied to the leading token, if there is one. *)
      let modify_leading_token node f =
        let rec aux nodes =
          match nodes with
          | [] -> (nodes, false)
          | h :: t ->
            begin
            match get_token h with
            | Some token ->
              let new_token = f token in
              let new_token = make_token new_token in
              ((new_token :: t), true)
            | None ->
              let (new_children, success) = aux (children h) in
              if success then
                let new_head = from_children (kind h) new_children in
                ((new_head :: t), true)
              else
                let (new_tail, success) = aux t in
                if success then
                  ((h :: new_tail), true)
                else
                  (nodes, false)
              end in
        let (results, _) = aux [node] in
        match results with
        | [] -> failwith
          "how did we get a smaller list out than we started with?"
        | h :: [] -> h
        | _ -> failwith
          "how did we get a larger list out than we started with?"

      (* Takes a node and a function from token to token and returns a node with
      the function applied to the trailing token, if there is one. *)
      let modify_trailing_token node f =
        (* We have a list of nodes, reversed, so the rightmost node is first. *)
        let rec aux nodes =
          match nodes with
          | [] -> (nodes, false)
          | h :: t ->
            begin
            match get_token h with
            | Some token ->
              let new_token = f token in
              let new_token = make_token new_token in
              ((new_token :: t), true)
            | None ->
              let (new_children, success) = aux (List.rev (children h)) in
              if success then
                let new_children = List.rev new_children in
                let new_head = from_children (kind h) new_children in
                ((new_head :: t), true)
              else
                let (new_tail, success) = aux t in
                if success then
                  ((h :: new_tail), true)
                else
                  (nodes, false)
              end in
        let (results, _) = aux [node] in
        match results with
        | [] -> failwith
          "how did we get a smaller list out than we started with?"
        | h :: [] -> h
        | _ -> failwith
          "how did we get a larger list out than we started with?"

    end (* WithValueBuilder *)
  end (* WithSyntaxValue *)
end (* WithToken *)
