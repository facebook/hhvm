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
 * This module contains the type describing the structure of a syntax tree.
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
      | WhileStatement                          _ -> SyntaxKind.WhileStatement
      | IfStatement                             _ -> SyntaxKind.IfStatement
      | ElseifClause                            _ -> SyntaxKind.ElseifClause
      | ElseClause                              _ -> SyntaxKind.ElseClause
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
    let is_while_statement                              = has_kind SyntaxKind.WhileStatement
    let is_if_statement                                 = has_kind SyntaxKind.IfStatement
    let is_elseif_clause                                = has_kind SyntaxKind.ElseifClause
    let is_else_clause                                  = has_kind SyntaxKind.ElseClause
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
      parameter_type;
      parameter_name;
      parameter_default_value;
    } = (
      parameter_attribute,
      parameter_visibility,
      parameter_type,
      parameter_name,
      parameter_default_value
    )

    let get_variadic_parameter_children {
      variadic_parameter_ellipsis;
    } = (
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



    (* The order that the children are returned in should match the order
       that they appear in the source text *)
    let children node =
      match node.syntax with
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
        parameter_type;
        parameter_name;
        parameter_default_value;
      } -> [
        parameter_attribute;
        parameter_visibility;
        parameter_type;
        parameter_name;
        parameter_default_value;
      ]
      | VariadicParameter {
        variadic_parameter_ellipsis;
      } -> [
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
        parameter_type;
        parameter_name;
        parameter_default_value;
      } -> [
        "parameter_attribute";
        "parameter_visibility";
        "parameter_type";
        "parameter_name";
        "parameter_default_value";
      ]
      | VariadicParameter {
        variadic_parameter_ellipsis;
      } -> [
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
          parameter_type;
          parameter_name;
          parameter_default_value;
        ]) ->
        ParameterDeclaration {
          parameter_attribute;
          parameter_visibility;
          parameter_type;
          parameter_name;
          parameter_default_value;
        }
      | (SyntaxKind.VariadicParameter, [
          variadic_parameter_ellipsis;
        ]) ->
        VariadicParameter {
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
      from_children SyntaxKind.EndOfFile [
        end_of_file_token;
      ]

    let make_script
      script_declarations
    =
      from_children SyntaxKind.Script [
        script_declarations;
      ]

    let make_simple_type_specifier
      simple_type_specifier
    =
      from_children SyntaxKind.SimpleTypeSpecifier [
        simple_type_specifier;
      ]

    let make_literal_expression
      literal_expression
    =
      from_children SyntaxKind.LiteralExpression [
        literal_expression;
      ]

    let make_variable_expression
      variable_expression
    =
      from_children SyntaxKind.VariableExpression [
        variable_expression;
      ]

    let make_qualified_name_expression
      qualified_name_expression
    =
      from_children SyntaxKind.QualifiedNameExpression [
        qualified_name_expression;
      ]

    let make_pipe_variable_expression
      pipe_variable_expression
    =
      from_children SyntaxKind.PipeVariableExpression [
        pipe_variable_expression;
      ]

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
      from_children SyntaxKind.EnumDeclaration [
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

    let make_enumerator
      enumerator_name
      enumerator_equal
      enumerator_value
      enumerator_semicolon
    =
      from_children SyntaxKind.Enumerator [
        enumerator_name;
        enumerator_equal;
        enumerator_value;
        enumerator_semicolon;
      ]

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
      from_children SyntaxKind.AliasDeclaration [
        alias_attribute_spec;
        alias_keyword;
        alias_name;
        alias_generic_parameter;
        alias_constraint;
        alias_equal;
        alias_type;
        alias_semicolon;
      ]

    let make_property_declaration
      property_modifiers
      property_type
      property_declarators
      property_semicolon
    =
      from_children SyntaxKind.PropertyDeclaration [
        property_modifiers;
        property_type;
        property_declarators;
        property_semicolon;
      ]

    let make_property_declarator
      property_name
      property_initializer
    =
      from_children SyntaxKind.PropertyDeclarator [
        property_name;
        property_initializer;
      ]

    let make_namespace_declaration
      namespace_keyword
      namespace_name
      namespace_body
    =
      from_children SyntaxKind.NamespaceDeclaration [
        namespace_keyword;
        namespace_name;
        namespace_body;
      ]

    let make_namespace_body
      namespace_left_brace
      namespace_declarations
      namespace_right_brace
    =
      from_children SyntaxKind.NamespaceBody [
        namespace_left_brace;
        namespace_declarations;
        namespace_right_brace;
      ]

    let make_namespace_empty_body
      namespace_semicolon
    =
      from_children SyntaxKind.NamespaceEmptyBody [
        namespace_semicolon;
      ]

    let make_namespace_use_declaration
      namespace_use_keyword
      namespace_use_kind
      namespace_use_clauses
      namespace_use_semicolon
    =
      from_children SyntaxKind.NamespaceUseDeclaration [
        namespace_use_keyword;
        namespace_use_kind;
        namespace_use_clauses;
        namespace_use_semicolon;
      ]

    let make_namespace_group_use_declaration
      namespace_group_use_keyword
      namespace_group_use_kind
      namespace_group_use_prefix
      namespace_group_use_left_brace
      namespace_group_use_clauses
      namespace_group_use_right_brace
      namespace_group_use_semicolon
    =
      from_children SyntaxKind.NamespaceGroupUseDeclaration [
        namespace_group_use_keyword;
        namespace_group_use_kind;
        namespace_group_use_prefix;
        namespace_group_use_left_brace;
        namespace_group_use_clauses;
        namespace_group_use_right_brace;
        namespace_group_use_semicolon;
      ]

    let make_namespace_use_clause
      namespace_use_clause_kind
      namespace_use_name
      namespace_use_as
      namespace_use_alias
    =
      from_children SyntaxKind.NamespaceUseClause [
        namespace_use_clause_kind;
        namespace_use_name;
        namespace_use_as;
        namespace_use_alias;
      ]

    let make_function_declaration
      function_attribute_spec
      function_declaration_header
      function_body
    =
      from_children SyntaxKind.FunctionDeclaration [
        function_attribute_spec;
        function_declaration_header;
        function_body;
      ]

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
      from_children SyntaxKind.FunctionDeclarationHeader [
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

    let make_where_clause
      where_clause_keyword
      where_clause_constraints
    =
      from_children SyntaxKind.WhereClause [
        where_clause_keyword;
        where_clause_constraints;
      ]

    let make_where_constraint
      where_constraint_left_type
      where_constraint_operator
      where_constraint_right_type
    =
      from_children SyntaxKind.WhereConstraint [
        where_constraint_left_type;
        where_constraint_operator;
        where_constraint_right_type;
      ]

    let make_methodish_declaration
      methodish_attribute
      methodish_modifiers
      methodish_function_decl_header
      methodish_function_body
      methodish_semicolon
    =
      from_children SyntaxKind.MethodishDeclaration [
        methodish_attribute;
        methodish_modifiers;
        methodish_function_decl_header;
        methodish_function_body;
        methodish_semicolon;
      ]

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
      from_children SyntaxKind.ClassishDeclaration [
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

    let make_classish_body
      classish_body_left_brace
      classish_body_elements
      classish_body_right_brace
    =
      from_children SyntaxKind.ClassishBody [
        classish_body_left_brace;
        classish_body_elements;
        classish_body_right_brace;
      ]

    let make_trait_use_precedence_item
      trait_use_precedence_item_name
      trait_use_precedence_item_keyword
      trait_use_precedence_item_removed_names
    =
      from_children SyntaxKind.TraitUsePrecedenceItem [
        trait_use_precedence_item_name;
        trait_use_precedence_item_keyword;
        trait_use_precedence_item_removed_names;
      ]

    let make_trait_use_alias_item
      trait_use_alias_item_aliasing_name
      trait_use_alias_item_keyword
      trait_use_alias_item_visibility
      trait_use_alias_item_aliased_name
    =
      from_children SyntaxKind.TraitUseAliasItem [
        trait_use_alias_item_aliasing_name;
        trait_use_alias_item_keyword;
        trait_use_alias_item_visibility;
        trait_use_alias_item_aliased_name;
      ]

    let make_trait_use_conflict_resolution
      trait_use_conflict_resolution_keyword
      trait_use_conflict_resolution_names
      trait_use_conflict_resolution_left_brace
      trait_use_conflict_resolution_clauses
      trait_use_conflict_resolution_right_brace
    =
      from_children SyntaxKind.TraitUseConflictResolution [
        trait_use_conflict_resolution_keyword;
        trait_use_conflict_resolution_names;
        trait_use_conflict_resolution_left_brace;
        trait_use_conflict_resolution_clauses;
        trait_use_conflict_resolution_right_brace;
      ]

    let make_trait_use
      trait_use_keyword
      trait_use_names
      trait_use_semicolon
    =
      from_children SyntaxKind.TraitUse [
        trait_use_keyword;
        trait_use_names;
        trait_use_semicolon;
      ]

    let make_require_clause
      require_keyword
      require_kind
      require_name
      require_semicolon
    =
      from_children SyntaxKind.RequireClause [
        require_keyword;
        require_kind;
        require_name;
        require_semicolon;
      ]

    let make_const_declaration
      const_abstract
      const_keyword
      const_type_specifier
      const_declarators
      const_semicolon
    =
      from_children SyntaxKind.ConstDeclaration [
        const_abstract;
        const_keyword;
        const_type_specifier;
        const_declarators;
        const_semicolon;
      ]

    let make_constant_declarator
      constant_declarator_name
      constant_declarator_initializer
    =
      from_children SyntaxKind.ConstantDeclarator [
        constant_declarator_name;
        constant_declarator_initializer;
      ]

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
      from_children SyntaxKind.TypeConstDeclaration [
        type_const_abstract;
        type_const_keyword;
        type_const_type_keyword;
        type_const_name;
        type_const_type_constraint;
        type_const_equal;
        type_const_type_specifier;
        type_const_semicolon;
      ]

    let make_decorated_expression
      decorated_expression_decorator
      decorated_expression_expression
    =
      from_children SyntaxKind.DecoratedExpression [
        decorated_expression_decorator;
        decorated_expression_expression;
      ]

    let make_parameter_declaration
      parameter_attribute
      parameter_visibility
      parameter_type
      parameter_name
      parameter_default_value
    =
      from_children SyntaxKind.ParameterDeclaration [
        parameter_attribute;
        parameter_visibility;
        parameter_type;
        parameter_name;
        parameter_default_value;
      ]

    let make_variadic_parameter
      variadic_parameter_ellipsis
    =
      from_children SyntaxKind.VariadicParameter [
        variadic_parameter_ellipsis;
      ]

    let make_attribute_specification
      attribute_specification_left_double_angle
      attribute_specification_attributes
      attribute_specification_right_double_angle
    =
      from_children SyntaxKind.AttributeSpecification [
        attribute_specification_left_double_angle;
        attribute_specification_attributes;
        attribute_specification_right_double_angle;
      ]

    let make_attribute
      attribute_name
      attribute_left_paren
      attribute_values
      attribute_right_paren
    =
      from_children SyntaxKind.Attribute [
        attribute_name;
        attribute_left_paren;
        attribute_values;
        attribute_right_paren;
      ]

    let make_inclusion_expression
      inclusion_require
      inclusion_filename
    =
      from_children SyntaxKind.InclusionExpression [
        inclusion_require;
        inclusion_filename;
      ]

    let make_inclusion_directive
      inclusion_expression
      inclusion_semicolon
    =
      from_children SyntaxKind.InclusionDirective [
        inclusion_expression;
        inclusion_semicolon;
      ]

    let make_compound_statement
      compound_left_brace
      compound_statements
      compound_right_brace
    =
      from_children SyntaxKind.CompoundStatement [
        compound_left_brace;
        compound_statements;
        compound_right_brace;
      ]

    let make_expression_statement
      expression_statement_expression
      expression_statement_semicolon
    =
      from_children SyntaxKind.ExpressionStatement [
        expression_statement_expression;
        expression_statement_semicolon;
      ]

    let make_markup_section
      markup_prefix
      markup_text
      markup_suffix
      markup_expression
    =
      from_children SyntaxKind.MarkupSection [
        markup_prefix;
        markup_text;
        markup_suffix;
        markup_expression;
      ]

    let make_markup_suffix
      markup_suffix_less_than_question
      markup_suffix_name
    =
      from_children SyntaxKind.MarkupSuffix [
        markup_suffix_less_than_question;
        markup_suffix_name;
      ]

    let make_unset_statement
      unset_keyword
      unset_left_paren
      unset_variables
      unset_right_paren
      unset_semicolon
    =
      from_children SyntaxKind.UnsetStatement [
        unset_keyword;
        unset_left_paren;
        unset_variables;
        unset_right_paren;
        unset_semicolon;
      ]

    let make_while_statement
      while_keyword
      while_left_paren
      while_condition
      while_right_paren
      while_body
    =
      from_children SyntaxKind.WhileStatement [
        while_keyword;
        while_left_paren;
        while_condition;
        while_right_paren;
        while_body;
      ]

    let make_if_statement
      if_keyword
      if_left_paren
      if_condition
      if_right_paren
      if_statement
      if_elseif_clauses
      if_else_clause
    =
      from_children SyntaxKind.IfStatement [
        if_keyword;
        if_left_paren;
        if_condition;
        if_right_paren;
        if_statement;
        if_elseif_clauses;
        if_else_clause;
      ]

    let make_elseif_clause
      elseif_keyword
      elseif_left_paren
      elseif_condition
      elseif_right_paren
      elseif_statement
    =
      from_children SyntaxKind.ElseifClause [
        elseif_keyword;
        elseif_left_paren;
        elseif_condition;
        elseif_right_paren;
        elseif_statement;
      ]

    let make_else_clause
      else_keyword
      else_statement
    =
      from_children SyntaxKind.ElseClause [
        else_keyword;
        else_statement;
      ]

    let make_try_statement
      try_keyword
      try_compound_statement
      try_catch_clauses
      try_finally_clause
    =
      from_children SyntaxKind.TryStatement [
        try_keyword;
        try_compound_statement;
        try_catch_clauses;
        try_finally_clause;
      ]

    let make_catch_clause
      catch_keyword
      catch_left_paren
      catch_type
      catch_variable
      catch_right_paren
      catch_body
    =
      from_children SyntaxKind.CatchClause [
        catch_keyword;
        catch_left_paren;
        catch_type;
        catch_variable;
        catch_right_paren;
        catch_body;
      ]

    let make_finally_clause
      finally_keyword
      finally_body
    =
      from_children SyntaxKind.FinallyClause [
        finally_keyword;
        finally_body;
      ]

    let make_do_statement
      do_keyword
      do_body
      do_while_keyword
      do_left_paren
      do_condition
      do_right_paren
      do_semicolon
    =
      from_children SyntaxKind.DoStatement [
        do_keyword;
        do_body;
        do_while_keyword;
        do_left_paren;
        do_condition;
        do_right_paren;
        do_semicolon;
      ]

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
      from_children SyntaxKind.ForStatement [
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
      from_children SyntaxKind.ForeachStatement [
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

    let make_switch_statement
      switch_keyword
      switch_left_paren
      switch_expression
      switch_right_paren
      switch_left_brace
      switch_sections
      switch_right_brace
    =
      from_children SyntaxKind.SwitchStatement [
        switch_keyword;
        switch_left_paren;
        switch_expression;
        switch_right_paren;
        switch_left_brace;
        switch_sections;
        switch_right_brace;
      ]

    let make_switch_section
      switch_section_labels
      switch_section_statements
      switch_section_fallthrough
    =
      from_children SyntaxKind.SwitchSection [
        switch_section_labels;
        switch_section_statements;
        switch_section_fallthrough;
      ]

    let make_switch_fallthrough
      fallthrough_keyword
      fallthrough_semicolon
    =
      from_children SyntaxKind.SwitchFallthrough [
        fallthrough_keyword;
        fallthrough_semicolon;
      ]

    let make_case_label
      case_keyword
      case_expression
      case_colon
    =
      from_children SyntaxKind.CaseLabel [
        case_keyword;
        case_expression;
        case_colon;
      ]

    let make_default_label
      default_keyword
      default_colon
    =
      from_children SyntaxKind.DefaultLabel [
        default_keyword;
        default_colon;
      ]

    let make_return_statement
      return_keyword
      return_expression
      return_semicolon
    =
      from_children SyntaxKind.ReturnStatement [
        return_keyword;
        return_expression;
        return_semicolon;
      ]

    let make_goto_label
      goto_label_name
      goto_label_colon
    =
      from_children SyntaxKind.GotoLabel [
        goto_label_name;
        goto_label_colon;
      ]

    let make_goto_statement
      goto_statement_keyword
      goto_statement_label_name
      goto_statement_semicolon
    =
      from_children SyntaxKind.GotoStatement [
        goto_statement_keyword;
        goto_statement_label_name;
        goto_statement_semicolon;
      ]

    let make_throw_statement
      throw_keyword
      throw_expression
      throw_semicolon
    =
      from_children SyntaxKind.ThrowStatement [
        throw_keyword;
        throw_expression;
        throw_semicolon;
      ]

    let make_break_statement
      break_keyword
      break_level
      break_semicolon
    =
      from_children SyntaxKind.BreakStatement [
        break_keyword;
        break_level;
        break_semicolon;
      ]

    let make_continue_statement
      continue_keyword
      continue_level
      continue_semicolon
    =
      from_children SyntaxKind.ContinueStatement [
        continue_keyword;
        continue_level;
        continue_semicolon;
      ]

    let make_function_static_statement
      static_static_keyword
      static_declarations
      static_semicolon
    =
      from_children SyntaxKind.FunctionStaticStatement [
        static_static_keyword;
        static_declarations;
        static_semicolon;
      ]

    let make_static_declarator
      static_name
      static_initializer
    =
      from_children SyntaxKind.StaticDeclarator [
        static_name;
        static_initializer;
      ]

    let make_echo_statement
      echo_keyword
      echo_expressions
      echo_semicolon
    =
      from_children SyntaxKind.EchoStatement [
        echo_keyword;
        echo_expressions;
        echo_semicolon;
      ]

    let make_global_statement
      global_keyword
      global_variables
      global_semicolon
    =
      from_children SyntaxKind.GlobalStatement [
        global_keyword;
        global_variables;
        global_semicolon;
      ]

    let make_simple_initializer
      simple_initializer_equal
      simple_initializer_value
    =
      from_children SyntaxKind.SimpleInitializer [
        simple_initializer_equal;
        simple_initializer_value;
      ]

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
      from_children SyntaxKind.AnonymousFunction [
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

    let make_anonymous_function_use_clause
      anonymous_use_keyword
      anonymous_use_left_paren
      anonymous_use_variables
      anonymous_use_right_paren
    =
      from_children SyntaxKind.AnonymousFunctionUseClause [
        anonymous_use_keyword;
        anonymous_use_left_paren;
        anonymous_use_variables;
        anonymous_use_right_paren;
      ]

    let make_lambda_expression
      lambda_async
      lambda_coroutine
      lambda_signature
      lambda_arrow
      lambda_body
    =
      from_children SyntaxKind.LambdaExpression [
        lambda_async;
        lambda_coroutine;
        lambda_signature;
        lambda_arrow;
        lambda_body;
      ]

    let make_lambda_signature
      lambda_left_paren
      lambda_parameters
      lambda_right_paren
      lambda_colon
      lambda_type
    =
      from_children SyntaxKind.LambdaSignature [
        lambda_left_paren;
        lambda_parameters;
        lambda_right_paren;
        lambda_colon;
        lambda_type;
      ]

    let make_cast_expression
      cast_left_paren
      cast_type
      cast_right_paren
      cast_operand
    =
      from_children SyntaxKind.CastExpression [
        cast_left_paren;
        cast_type;
        cast_right_paren;
        cast_operand;
      ]

    let make_scope_resolution_expression
      scope_resolution_qualifier
      scope_resolution_operator
      scope_resolution_name
    =
      from_children SyntaxKind.ScopeResolutionExpression [
        scope_resolution_qualifier;
        scope_resolution_operator;
        scope_resolution_name;
      ]

    let make_member_selection_expression
      member_object
      member_operator
      member_name
    =
      from_children SyntaxKind.MemberSelectionExpression [
        member_object;
        member_operator;
        member_name;
      ]

    let make_safe_member_selection_expression
      safe_member_object
      safe_member_operator
      safe_member_name
    =
      from_children SyntaxKind.SafeMemberSelectionExpression [
        safe_member_object;
        safe_member_operator;
        safe_member_name;
      ]

    let make_embedded_member_selection_expression
      embedded_member_object
      embedded_member_operator
      embedded_member_name
    =
      from_children SyntaxKind.EmbeddedMemberSelectionExpression [
        embedded_member_object;
        embedded_member_operator;
        embedded_member_name;
      ]

    let make_yield_expression
      yield_keyword
      yield_operand
    =
      from_children SyntaxKind.YieldExpression [
        yield_keyword;
        yield_operand;
      ]

    let make_yield_from_expression
      yield_from_yield_keyword
      yield_from_from_keyword
      yield_from_operand
    =
      from_children SyntaxKind.YieldFromExpression [
        yield_from_yield_keyword;
        yield_from_from_keyword;
        yield_from_operand;
      ]

    let make_prefix_unary_expression
      prefix_unary_operator
      prefix_unary_operand
    =
      from_children SyntaxKind.PrefixUnaryExpression [
        prefix_unary_operator;
        prefix_unary_operand;
      ]

    let make_postfix_unary_expression
      postfix_unary_operand
      postfix_unary_operator
    =
      from_children SyntaxKind.PostfixUnaryExpression [
        postfix_unary_operand;
        postfix_unary_operator;
      ]

    let make_binary_expression
      binary_left_operand
      binary_operator
      binary_right_operand
    =
      from_children SyntaxKind.BinaryExpression [
        binary_left_operand;
        binary_operator;
        binary_right_operand;
      ]

    let make_instanceof_expression
      instanceof_left_operand
      instanceof_operator
      instanceof_right_operand
    =
      from_children SyntaxKind.InstanceofExpression [
        instanceof_left_operand;
        instanceof_operator;
        instanceof_right_operand;
      ]

    let make_conditional_expression
      conditional_test
      conditional_question
      conditional_consequence
      conditional_colon
      conditional_alternative
    =
      from_children SyntaxKind.ConditionalExpression [
        conditional_test;
        conditional_question;
        conditional_consequence;
        conditional_colon;
        conditional_alternative;
      ]

    let make_eval_expression
      eval_keyword
      eval_left_paren
      eval_argument
      eval_right_paren
    =
      from_children SyntaxKind.EvalExpression [
        eval_keyword;
        eval_left_paren;
        eval_argument;
        eval_right_paren;
      ]

    let make_empty_expression
      empty_keyword
      empty_left_paren
      empty_argument
      empty_right_paren
    =
      from_children SyntaxKind.EmptyExpression [
        empty_keyword;
        empty_left_paren;
        empty_argument;
        empty_right_paren;
      ]

    let make_define_expression
      define_keyword
      define_left_paren
      define_argument_list
      define_right_paren
    =
      from_children SyntaxKind.DefineExpression [
        define_keyword;
        define_left_paren;
        define_argument_list;
        define_right_paren;
      ]

    let make_isset_expression
      isset_keyword
      isset_left_paren
      isset_argument_list
      isset_right_paren
    =
      from_children SyntaxKind.IssetExpression [
        isset_keyword;
        isset_left_paren;
        isset_argument_list;
        isset_right_paren;
      ]

    let make_function_call_expression
      function_call_receiver
      function_call_left_paren
      function_call_argument_list
      function_call_right_paren
    =
      from_children SyntaxKind.FunctionCallExpression [
        function_call_receiver;
        function_call_left_paren;
        function_call_argument_list;
        function_call_right_paren;
      ]

    let make_function_call_with_type_arguments_expression
      function_call_with_type_arguments_receiver
      function_call_with_type_arguments_type_args
      function_call_with_type_arguments_left_paren
      function_call_with_type_arguments_argument_list
      function_call_with_type_arguments_right_paren
    =
      from_children SyntaxKind.FunctionCallWithTypeArgumentsExpression [
        function_call_with_type_arguments_receiver;
        function_call_with_type_arguments_type_args;
        function_call_with_type_arguments_left_paren;
        function_call_with_type_arguments_argument_list;
        function_call_with_type_arguments_right_paren;
      ]

    let make_parenthesized_expression
      parenthesized_expression_left_paren
      parenthesized_expression_expression
      parenthesized_expression_right_paren
    =
      from_children SyntaxKind.ParenthesizedExpression [
        parenthesized_expression_left_paren;
        parenthesized_expression_expression;
        parenthesized_expression_right_paren;
      ]

    let make_braced_expression
      braced_expression_left_brace
      braced_expression_expression
      braced_expression_right_brace
    =
      from_children SyntaxKind.BracedExpression [
        braced_expression_left_brace;
        braced_expression_expression;
        braced_expression_right_brace;
      ]

    let make_embedded_braced_expression
      embedded_braced_expression_left_brace
      embedded_braced_expression_expression
      embedded_braced_expression_right_brace
    =
      from_children SyntaxKind.EmbeddedBracedExpression [
        embedded_braced_expression_left_brace;
        embedded_braced_expression_expression;
        embedded_braced_expression_right_brace;
      ]

    let make_list_expression
      list_keyword
      list_left_paren
      list_members
      list_right_paren
    =
      from_children SyntaxKind.ListExpression [
        list_keyword;
        list_left_paren;
        list_members;
        list_right_paren;
      ]

    let make_collection_literal_expression
      collection_literal_name
      collection_literal_left_brace
      collection_literal_initializers
      collection_literal_right_brace
    =
      from_children SyntaxKind.CollectionLiteralExpression [
        collection_literal_name;
        collection_literal_left_brace;
        collection_literal_initializers;
        collection_literal_right_brace;
      ]

    let make_object_creation_expression
      object_creation_new_keyword
      object_creation_type
      object_creation_left_paren
      object_creation_argument_list
      object_creation_right_paren
    =
      from_children SyntaxKind.ObjectCreationExpression [
        object_creation_new_keyword;
        object_creation_type;
        object_creation_left_paren;
        object_creation_argument_list;
        object_creation_right_paren;
      ]

    let make_array_creation_expression
      array_creation_left_bracket
      array_creation_members
      array_creation_right_bracket
    =
      from_children SyntaxKind.ArrayCreationExpression [
        array_creation_left_bracket;
        array_creation_members;
        array_creation_right_bracket;
      ]

    let make_array_intrinsic_expression
      array_intrinsic_keyword
      array_intrinsic_left_paren
      array_intrinsic_members
      array_intrinsic_right_paren
    =
      from_children SyntaxKind.ArrayIntrinsicExpression [
        array_intrinsic_keyword;
        array_intrinsic_left_paren;
        array_intrinsic_members;
        array_intrinsic_right_paren;
      ]

    let make_darray_intrinsic_expression
      darray_intrinsic_keyword
      darray_intrinsic_left_bracket
      darray_intrinsic_members
      darray_intrinsic_right_bracket
    =
      from_children SyntaxKind.DarrayIntrinsicExpression [
        darray_intrinsic_keyword;
        darray_intrinsic_left_bracket;
        darray_intrinsic_members;
        darray_intrinsic_right_bracket;
      ]

    let make_dictionary_intrinsic_expression
      dictionary_intrinsic_keyword
      dictionary_intrinsic_left_bracket
      dictionary_intrinsic_members
      dictionary_intrinsic_right_bracket
    =
      from_children SyntaxKind.DictionaryIntrinsicExpression [
        dictionary_intrinsic_keyword;
        dictionary_intrinsic_left_bracket;
        dictionary_intrinsic_members;
        dictionary_intrinsic_right_bracket;
      ]

    let make_keyset_intrinsic_expression
      keyset_intrinsic_keyword
      keyset_intrinsic_left_bracket
      keyset_intrinsic_members
      keyset_intrinsic_right_bracket
    =
      from_children SyntaxKind.KeysetIntrinsicExpression [
        keyset_intrinsic_keyword;
        keyset_intrinsic_left_bracket;
        keyset_intrinsic_members;
        keyset_intrinsic_right_bracket;
      ]

    let make_varray_intrinsic_expression
      varray_intrinsic_keyword
      varray_intrinsic_left_bracket
      varray_intrinsic_members
      varray_intrinsic_right_bracket
    =
      from_children SyntaxKind.VarrayIntrinsicExpression [
        varray_intrinsic_keyword;
        varray_intrinsic_left_bracket;
        varray_intrinsic_members;
        varray_intrinsic_right_bracket;
      ]

    let make_vector_intrinsic_expression
      vector_intrinsic_keyword
      vector_intrinsic_left_bracket
      vector_intrinsic_members
      vector_intrinsic_right_bracket
    =
      from_children SyntaxKind.VectorIntrinsicExpression [
        vector_intrinsic_keyword;
        vector_intrinsic_left_bracket;
        vector_intrinsic_members;
        vector_intrinsic_right_bracket;
      ]

    let make_element_initializer
      element_key
      element_arrow
      element_value
    =
      from_children SyntaxKind.ElementInitializer [
        element_key;
        element_arrow;
        element_value;
      ]

    let make_subscript_expression
      subscript_receiver
      subscript_left_bracket
      subscript_index
      subscript_right_bracket
    =
      from_children SyntaxKind.SubscriptExpression [
        subscript_receiver;
        subscript_left_bracket;
        subscript_index;
        subscript_right_bracket;
      ]

    let make_embedded_subscript_expression
      embedded_subscript_receiver
      embedded_subscript_left_bracket
      embedded_subscript_index
      embedded_subscript_right_bracket
    =
      from_children SyntaxKind.EmbeddedSubscriptExpression [
        embedded_subscript_receiver;
        embedded_subscript_left_bracket;
        embedded_subscript_index;
        embedded_subscript_right_bracket;
      ]

    let make_awaitable_creation_expression
      awaitable_async
      awaitable_coroutine
      awaitable_compound_statement
    =
      from_children SyntaxKind.AwaitableCreationExpression [
        awaitable_async;
        awaitable_coroutine;
        awaitable_compound_statement;
      ]

    let make_xhp_children_declaration
      xhp_children_keyword
      xhp_children_expression
      xhp_children_semicolon
    =
      from_children SyntaxKind.XHPChildrenDeclaration [
        xhp_children_keyword;
        xhp_children_expression;
        xhp_children_semicolon;
      ]

    let make_xhp_children_parenthesized_list
      xhp_children_list_left_paren
      xhp_children_list_xhp_children
      xhp_children_list_right_paren
    =
      from_children SyntaxKind.XHPChildrenParenthesizedList [
        xhp_children_list_left_paren;
        xhp_children_list_xhp_children;
        xhp_children_list_right_paren;
      ]

    let make_xhp_category_declaration
      xhp_category_keyword
      xhp_category_categories
      xhp_category_semicolon
    =
      from_children SyntaxKind.XHPCategoryDeclaration [
        xhp_category_keyword;
        xhp_category_categories;
        xhp_category_semicolon;
      ]

    let make_xhp_enum_type
      xhp_enum_keyword
      xhp_enum_left_brace
      xhp_enum_values
      xhp_enum_right_brace
    =
      from_children SyntaxKind.XHPEnumType [
        xhp_enum_keyword;
        xhp_enum_left_brace;
        xhp_enum_values;
        xhp_enum_right_brace;
      ]

    let make_xhp_required
      xhp_required_at
      xhp_required_keyword
    =
      from_children SyntaxKind.XHPRequired [
        xhp_required_at;
        xhp_required_keyword;
      ]

    let make_xhp_class_attribute_declaration
      xhp_attribute_keyword
      xhp_attribute_attributes
      xhp_attribute_semicolon
    =
      from_children SyntaxKind.XHPClassAttributeDeclaration [
        xhp_attribute_keyword;
        xhp_attribute_attributes;
        xhp_attribute_semicolon;
      ]

    let make_xhp_class_attribute
      xhp_attribute_decl_type
      xhp_attribute_decl_name
      xhp_attribute_decl_initializer
      xhp_attribute_decl_required
    =
      from_children SyntaxKind.XHPClassAttribute [
        xhp_attribute_decl_type;
        xhp_attribute_decl_name;
        xhp_attribute_decl_initializer;
        xhp_attribute_decl_required;
      ]

    let make_xhp_simple_class_attribute
      xhp_simple_class_attribute_type
    =
      from_children SyntaxKind.XHPSimpleClassAttribute [
        xhp_simple_class_attribute_type;
      ]

    let make_xhp_attribute
      xhp_attribute_name
      xhp_attribute_equal
      xhp_attribute_expression
    =
      from_children SyntaxKind.XHPAttribute [
        xhp_attribute_name;
        xhp_attribute_equal;
        xhp_attribute_expression;
      ]

    let make_xhp_open
      xhp_open_left_angle
      xhp_open_name
      xhp_open_attributes
      xhp_open_right_angle
    =
      from_children SyntaxKind.XHPOpen [
        xhp_open_left_angle;
        xhp_open_name;
        xhp_open_attributes;
        xhp_open_right_angle;
      ]

    let make_xhp_expression
      xhp_open
      xhp_body
      xhp_close
    =
      from_children SyntaxKind.XHPExpression [
        xhp_open;
        xhp_body;
        xhp_close;
      ]

    let make_xhp_close
      xhp_close_left_angle
      xhp_close_name
      xhp_close_right_angle
    =
      from_children SyntaxKind.XHPClose [
        xhp_close_left_angle;
        xhp_close_name;
        xhp_close_right_angle;
      ]

    let make_type_constant
      type_constant_left_type
      type_constant_separator
      type_constant_right_type
    =
      from_children SyntaxKind.TypeConstant [
        type_constant_left_type;
        type_constant_separator;
        type_constant_right_type;
      ]

    let make_vector_type_specifier
      vector_type_keyword
      vector_type_left_angle
      vector_type_type
      vector_type_trailing_comma
      vector_type_right_angle
    =
      from_children SyntaxKind.VectorTypeSpecifier [
        vector_type_keyword;
        vector_type_left_angle;
        vector_type_type;
        vector_type_trailing_comma;
        vector_type_right_angle;
      ]

    let make_keyset_type_specifier
      keyset_type_keyword
      keyset_type_left_angle
      keyset_type_type
      keyset_type_trailing_comma
      keyset_type_right_angle
    =
      from_children SyntaxKind.KeysetTypeSpecifier [
        keyset_type_keyword;
        keyset_type_left_angle;
        keyset_type_type;
        keyset_type_trailing_comma;
        keyset_type_right_angle;
      ]

    let make_tuple_type_explicit_specifier
      tuple_type_keyword
      tuple_type_left_angle
      tuple_type_types
      tuple_type_right_angle
    =
      from_children SyntaxKind.TupleTypeExplicitSpecifier [
        tuple_type_keyword;
        tuple_type_left_angle;
        tuple_type_types;
        tuple_type_right_angle;
      ]

    let make_varray_type_specifier
      varray_keyword
      varray_left_angle
      varray_type
      varray_trailing_comma
      varray_right_angle
    =
      from_children SyntaxKind.VarrayTypeSpecifier [
        varray_keyword;
        varray_left_angle;
        varray_type;
        varray_trailing_comma;
        varray_right_angle;
      ]

    let make_vector_array_type_specifier
      vector_array_keyword
      vector_array_left_angle
      vector_array_type
      vector_array_right_angle
    =
      from_children SyntaxKind.VectorArrayTypeSpecifier [
        vector_array_keyword;
        vector_array_left_angle;
        vector_array_type;
        vector_array_right_angle;
      ]

    let make_type_parameter
      type_variance
      type_name
      type_constraints
    =
      from_children SyntaxKind.TypeParameter [
        type_variance;
        type_name;
        type_constraints;
      ]

    let make_type_constraint
      constraint_keyword
      constraint_type
    =
      from_children SyntaxKind.TypeConstraint [
        constraint_keyword;
        constraint_type;
      ]

    let make_darray_type_specifier
      darray_keyword
      darray_left_angle
      darray_key
      darray_comma
      darray_value
      darray_trailing_comma
      darray_right_angle
    =
      from_children SyntaxKind.DarrayTypeSpecifier [
        darray_keyword;
        darray_left_angle;
        darray_key;
        darray_comma;
        darray_value;
        darray_trailing_comma;
        darray_right_angle;
      ]

    let make_map_array_type_specifier
      map_array_keyword
      map_array_left_angle
      map_array_key
      map_array_comma
      map_array_value
      map_array_right_angle
    =
      from_children SyntaxKind.MapArrayTypeSpecifier [
        map_array_keyword;
        map_array_left_angle;
        map_array_key;
        map_array_comma;
        map_array_value;
        map_array_right_angle;
      ]

    let make_dictionary_type_specifier
      dictionary_type_keyword
      dictionary_type_left_angle
      dictionary_type_members
      dictionary_type_right_angle
    =
      from_children SyntaxKind.DictionaryTypeSpecifier [
        dictionary_type_keyword;
        dictionary_type_left_angle;
        dictionary_type_members;
        dictionary_type_right_angle;
      ]

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
      from_children SyntaxKind.ClosureTypeSpecifier [
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

    let make_classname_type_specifier
      classname_keyword
      classname_left_angle
      classname_type
      classname_trailing_comma
      classname_right_angle
    =
      from_children SyntaxKind.ClassnameTypeSpecifier [
        classname_keyword;
        classname_left_angle;
        classname_type;
        classname_trailing_comma;
        classname_right_angle;
      ]

    let make_field_specifier
      field_question
      field_name
      field_arrow
      field_type
    =
      from_children SyntaxKind.FieldSpecifier [
        field_question;
        field_name;
        field_arrow;
        field_type;
      ]

    let make_field_initializer
      field_initializer_name
      field_initializer_arrow
      field_initializer_value
    =
      from_children SyntaxKind.FieldInitializer [
        field_initializer_name;
        field_initializer_arrow;
        field_initializer_value;
      ]

    let make_shape_type_specifier
      shape_type_keyword
      shape_type_left_paren
      shape_type_fields
      shape_type_ellipsis
      shape_type_right_paren
    =
      from_children SyntaxKind.ShapeTypeSpecifier [
        shape_type_keyword;
        shape_type_left_paren;
        shape_type_fields;
        shape_type_ellipsis;
        shape_type_right_paren;
      ]

    let make_shape_expression
      shape_expression_keyword
      shape_expression_left_paren
      shape_expression_fields
      shape_expression_right_paren
    =
      from_children SyntaxKind.ShapeExpression [
        shape_expression_keyword;
        shape_expression_left_paren;
        shape_expression_fields;
        shape_expression_right_paren;
      ]

    let make_tuple_expression
      tuple_expression_keyword
      tuple_expression_left_paren
      tuple_expression_items
      tuple_expression_right_paren
    =
      from_children SyntaxKind.TupleExpression [
        tuple_expression_keyword;
        tuple_expression_left_paren;
        tuple_expression_items;
        tuple_expression_right_paren;
      ]

    let make_generic_type_specifier
      generic_class_type
      generic_argument_list
    =
      from_children SyntaxKind.GenericTypeSpecifier [
        generic_class_type;
        generic_argument_list;
      ]

    let make_nullable_type_specifier
      nullable_question
      nullable_type
    =
      from_children SyntaxKind.NullableTypeSpecifier [
        nullable_question;
        nullable_type;
      ]

    let make_soft_type_specifier
      soft_at
      soft_type
    =
      from_children SyntaxKind.SoftTypeSpecifier [
        soft_at;
        soft_type;
      ]

    let make_type_arguments
      type_arguments_left_angle
      type_arguments_types
      type_arguments_right_angle
    =
      from_children SyntaxKind.TypeArguments [
        type_arguments_left_angle;
        type_arguments_types;
        type_arguments_right_angle;
      ]

    let make_type_parameters
      type_parameters_left_angle
      type_parameters_parameters
      type_parameters_right_angle
    =
      from_children SyntaxKind.TypeParameters [
        type_parameters_left_angle;
        type_parameters_parameters;
        type_parameters_right_angle;
      ]

    let make_tuple_type_specifier
      tuple_left_paren
      tuple_types
      tuple_right_paren
    =
      from_children SyntaxKind.TupleTypeSpecifier [
        tuple_left_paren;
        tuple_types;
        tuple_right_paren;
      ]

    let make_error
      error_error
    =
      from_children SyntaxKind.ErrorSyntax [
        error_error;
      ]

    let make_list_item
      list_item
      list_separator
    =
      from_children SyntaxKind.ListItem [
        list_item;
        list_separator;
      ]



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
