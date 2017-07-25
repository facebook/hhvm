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
 *)

type t =
  | Token
  | Missing
  | SyntaxList
  | EndOfFile
  | Script
  | SimpleTypeSpecifier
  | LiteralExpression
  | VariableExpression
  | QualifiedNameExpression
  | PipeVariableExpression
  | EnumDeclaration
  | Enumerator
  | AliasDeclaration
  | PropertyDeclaration
  | PropertyDeclarator
  | NamespaceDeclaration
  | NamespaceBody
  | NamespaceEmptyBody
  | NamespaceUseDeclaration
  | NamespaceGroupUseDeclaration
  | NamespaceUseClause
  | FunctionDeclaration
  | FunctionDeclarationHeader
  | WhereClause
  | WhereConstraint
  | MethodishDeclaration
  | ClassishDeclaration
  | ClassishBody
  | TraitUsePrecedenceItem
  | TraitUseAliasItem
  | TraitUseConflictResolution
  | TraitUse
  | RequireClause
  | ConstDeclaration
  | ConstantDeclarator
  | TypeConstDeclaration
  | DecoratedExpression
  | ParameterDeclaration
  | VariadicParameter
  | AttributeSpecification
  | Attribute
  | InclusionExpression
  | InclusionDirective
  | CompoundStatement
  | ExpressionStatement
  | MarkupSection
  | MarkupSuffix
  | UnsetStatement
  | WhileStatement
  | IfStatement
  | ElseifClause
  | ElseClause
  | TryStatement
  | CatchClause
  | FinallyClause
  | DoStatement
  | ForStatement
  | ForeachStatement
  | SwitchStatement
  | SwitchSection
  | SwitchFallthrough
  | CaseLabel
  | DefaultLabel
  | ReturnStatement
  | GotoLabel
  | GotoStatement
  | ThrowStatement
  | BreakStatement
  | ContinueStatement
  | FunctionStaticStatement
  | StaticDeclarator
  | EchoStatement
  | GlobalStatement
  | SimpleInitializer
  | AnonymousFunction
  | AnonymousFunctionUseClause
  | LambdaExpression
  | LambdaSignature
  | CastExpression
  | ScopeResolutionExpression
  | MemberSelectionExpression
  | SafeMemberSelectionExpression
  | EmbeddedMemberSelectionExpression
  | YieldExpression
  | YieldFromExpression
  | PrefixUnaryExpression
  | PostfixUnaryExpression
  | BinaryExpression
  | InstanceofExpression
  | ConditionalExpression
  | EvalExpression
  | EmptyExpression
  | DefineExpression
  | IssetExpression
  | FunctionCallExpression
  | ParenthesizedExpression
  | BracedExpression
  | EmbeddedBracedExpression
  | ListExpression
  | CollectionLiteralExpression
  | ObjectCreationExpression
  | ArrayCreationExpression
  | ArrayIntrinsicExpression
  | DarrayIntrinsicExpression
  | DictionaryIntrinsicExpression
  | KeysetIntrinsicExpression
  | VarrayIntrinsicExpression
  | VectorIntrinsicExpression
  | ElementInitializer
  | SubscriptExpression
  | EmbeddedSubscriptExpression
  | AwaitableCreationExpression
  | XHPChildrenDeclaration
  | XHPChildrenParenthesizedList
  | XHPCategoryDeclaration
  | XHPEnumType
  | XHPRequired
  | XHPClassAttributeDeclaration
  | XHPClassAttribute
  | XHPSimpleClassAttribute
  | XHPAttribute
  | XHPOpen
  | XHPExpression
  | XHPClose
  | TypeConstant
  | VectorTypeSpecifier
  | KeysetTypeSpecifier
  | TupleTypeExplicitSpecifier
  | VarrayTypeSpecifier
  | VectorArrayTypeSpecifier
  | TypeParameter
  | TypeConstraint
  | DarrayTypeSpecifier
  | MapArrayTypeSpecifier
  | DictionaryTypeSpecifier
  | ClosureTypeSpecifier
  | ClassnameTypeSpecifier
  | FieldSpecifier
  | FieldInitializer
  | ShapeTypeSpecifier
  | ShapeExpression
  | TupleExpression
  | GenericTypeSpecifier
  | NullableTypeSpecifier
  | SoftTypeSpecifier
  | TypeArguments
  | TypeParameters
  | TupleTypeSpecifier
  | ErrorSyntax
  | ListItem


let to_string kind =
  match kind with
  | Missing                           -> "missing"
  | Token                             -> "token"
  | SyntaxList                        -> "list"
  | EndOfFile                         -> "end_of_file"
  | Script                            -> "script"
  | SimpleTypeSpecifier               -> "simple_type_specifier"
  | LiteralExpression                 -> "literal"
  | VariableExpression                -> "variable"
  | QualifiedNameExpression           -> "qualified_name"
  | PipeVariableExpression            -> "pipe_variable"
  | EnumDeclaration                   -> "enum_declaration"
  | Enumerator                        -> "enumerator"
  | AliasDeclaration                  -> "alias_declaration"
  | PropertyDeclaration               -> "property_declaration"
  | PropertyDeclarator                -> "property_declarator"
  | NamespaceDeclaration              -> "namespace_declaration"
  | NamespaceBody                     -> "namespace_body"
  | NamespaceEmptyBody                -> "namespace_empty_body"
  | NamespaceUseDeclaration           -> "namespace_use_declaration"
  | NamespaceGroupUseDeclaration      -> "namespace_group_use_declaration"
  | NamespaceUseClause                -> "namespace_use_clause"
  | FunctionDeclaration               -> "function_declaration"
  | FunctionDeclarationHeader         -> "function_declaration_header"
  | WhereClause                       -> "where_clause"
  | WhereConstraint                   -> "where_constraint"
  | MethodishDeclaration              -> "methodish_declaration"
  | ClassishDeclaration               -> "classish_declaration"
  | ClassishBody                      -> "classish_body"
  | TraitUsePrecedenceItem            -> "trait_use_precedence_item"
  | TraitUseAliasItem                 -> "trait_use_alias_item"
  | TraitUseConflictResolution        -> "trait_use_conflict_resolution"
  | TraitUse                          -> "trait_use"
  | RequireClause                     -> "require_clause"
  | ConstDeclaration                  -> "const_declaration"
  | ConstantDeclarator                -> "constant_declarator"
  | TypeConstDeclaration              -> "type_const_declaration"
  | DecoratedExpression               -> "decorated_expression"
  | ParameterDeclaration              -> "parameter_declaration"
  | VariadicParameter                 -> "variadic_parameter"
  | AttributeSpecification            -> "attribute_specification"
  | Attribute                         -> "attribute"
  | InclusionExpression               -> "inclusion_expression"
  | InclusionDirective                -> "inclusion_directive"
  | CompoundStatement                 -> "compound_statement"
  | ExpressionStatement               -> "expression_statement"
  | MarkupSection                     -> "markup_section"
  | MarkupSuffix                      -> "markup_suffix"
  | UnsetStatement                    -> "unset_statement"
  | WhileStatement                    -> "while_statement"
  | IfStatement                       -> "if_statement"
  | ElseifClause                      -> "elseif_clause"
  | ElseClause                        -> "else_clause"
  | TryStatement                      -> "try_statement"
  | CatchClause                       -> "catch_clause"
  | FinallyClause                     -> "finally_clause"
  | DoStatement                       -> "do_statement"
  | ForStatement                      -> "for_statement"
  | ForeachStatement                  -> "foreach_statement"
  | SwitchStatement                   -> "switch_statement"
  | SwitchSection                     -> "switch_section"
  | SwitchFallthrough                 -> "switch_fallthrough"
  | CaseLabel                         -> "case_label"
  | DefaultLabel                      -> "default_label"
  | ReturnStatement                   -> "return_statement"
  | GotoLabel                         -> "goto_label"
  | GotoStatement                     -> "goto_statement"
  | ThrowStatement                    -> "throw_statement"
  | BreakStatement                    -> "break_statement"
  | ContinueStatement                 -> "continue_statement"
  | FunctionStaticStatement           -> "function_static_statement"
  | StaticDeclarator                  -> "static_declarator"
  | EchoStatement                     -> "echo_statement"
  | GlobalStatement                   -> "global_statement"
  | SimpleInitializer                 -> "simple_initializer"
  | AnonymousFunction                 -> "anonymous_function"
  | AnonymousFunctionUseClause        -> "anonymous_function_use_clause"
  | LambdaExpression                  -> "lambda_expression"
  | LambdaSignature                   -> "lambda_signature"
  | CastExpression                    -> "cast_expression"
  | ScopeResolutionExpression         -> "scope_resolution_expression"
  | MemberSelectionExpression         -> "member_selection_expression"
  | SafeMemberSelectionExpression     -> "safe_member_selection_expression"
  | EmbeddedMemberSelectionExpression -> "embedded_member_selection_expression"
  | YieldExpression                   -> "yield_expression"
  | YieldFromExpression               -> "yield_from_expression"
  | PrefixUnaryExpression             -> "prefix_unary_expression"
  | PostfixUnaryExpression            -> "postfix_unary_expression"
  | BinaryExpression                  -> "binary_expression"
  | InstanceofExpression              -> "instanceof_expression"
  | ConditionalExpression             -> "conditional_expression"
  | EvalExpression                    -> "eval_expression"
  | EmptyExpression                   -> "empty_expression"
  | DefineExpression                  -> "define_expression"
  | IssetExpression                   -> "isset_expression"
  | FunctionCallExpression            -> "function_call_expression"
  | ParenthesizedExpression           -> "parenthesized_expression"
  | BracedExpression                  -> "braced_expression"
  | EmbeddedBracedExpression          -> "embedded_braced_expression"
  | ListExpression                    -> "list_expression"
  | CollectionLiteralExpression       -> "collection_literal_expression"
  | ObjectCreationExpression          -> "object_creation_expression"
  | ArrayCreationExpression           -> "array_creation_expression"
  | ArrayIntrinsicExpression          -> "array_intrinsic_expression"
  | DarrayIntrinsicExpression         -> "darray_intrinsic_expression"
  | DictionaryIntrinsicExpression     -> "dictionary_intrinsic_expression"
  | KeysetIntrinsicExpression         -> "keyset_intrinsic_expression"
  | VarrayIntrinsicExpression         -> "varray_intrinsic_expression"
  | VectorIntrinsicExpression         -> "vector_intrinsic_expression"
  | ElementInitializer                -> "element_initializer"
  | SubscriptExpression               -> "subscript_expression"
  | EmbeddedSubscriptExpression       -> "embedded_subscript_expression"
  | AwaitableCreationExpression       -> "awaitable_creation_expression"
  | XHPChildrenDeclaration            -> "xhp_children_declaration"
  | XHPChildrenParenthesizedList      -> "xhp_children_parenthesized_list"
  | XHPCategoryDeclaration            -> "xhp_category_declaration"
  | XHPEnumType                       -> "xhp_enum_type"
  | XHPRequired                       -> "xhp_required"
  | XHPClassAttributeDeclaration      -> "xhp_class_attribute_declaration"
  | XHPClassAttribute                 -> "xhp_class_attribute"
  | XHPSimpleClassAttribute           -> "xhp_simple_class_attribute"
  | XHPAttribute                      -> "xhp_attribute"
  | XHPOpen                           -> "xhp_open"
  | XHPExpression                     -> "xhp_expression"
  | XHPClose                          -> "xhp_close"
  | TypeConstant                      -> "type_constant"
  | VectorTypeSpecifier               -> "vector_type_specifier"
  | KeysetTypeSpecifier               -> "keyset_type_specifier"
  | TupleTypeExplicitSpecifier        -> "tuple_type_explicit_specifier"
  | VarrayTypeSpecifier               -> "varray_type_specifier"
  | VectorArrayTypeSpecifier          -> "vector_array_type_specifier"
  | TypeParameter                     -> "type_parameter"
  | TypeConstraint                    -> "type_constraint"
  | DarrayTypeSpecifier               -> "darray_type_specifier"
  | MapArrayTypeSpecifier             -> "map_array_type_specifier"
  | DictionaryTypeSpecifier           -> "dictionary_type_specifier"
  | ClosureTypeSpecifier              -> "closure_type_specifier"
  | ClassnameTypeSpecifier            -> "classname_type_specifier"
  | FieldSpecifier                    -> "field_specifier"
  | FieldInitializer                  -> "field_initializer"
  | ShapeTypeSpecifier                -> "shape_type_specifier"
  | ShapeExpression                   -> "shape_expression"
  | TupleExpression                   -> "tuple_expression"
  | GenericTypeSpecifier              -> "generic_type_specifier"
  | NullableTypeSpecifier             -> "nullable_type_specifier"
  | SoftTypeSpecifier                 -> "soft_type_specifier"
  | TypeArguments                     -> "type_arguments"
  | TypeParameters                    -> "type_parameters"
  | TupleTypeSpecifier                -> "tuple_type_specifier"
  | ErrorSyntax                       -> "error"
  | ListItem                          -> "list_item"
