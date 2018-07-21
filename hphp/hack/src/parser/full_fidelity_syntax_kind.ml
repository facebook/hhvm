(**
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
 *)

type t =
  | Token of Full_fidelity_token_kind.t
  | Missing
  | SyntaxList
  | EndOfFile
  | Script
  | QualifiedName
  | SimpleTypeSpecifier
  | LiteralExpression
  | PrefixedStringExpression
  | VariableExpression
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
  | AlternateLoopStatement
  | ExpressionStatement
  | MarkupSection
  | MarkupSuffix
  | UnsetStatement
  | LetStatement
  | UsingStatementBlockScoped
  | UsingStatementFunctionScoped
  | DeclareDirectiveStatement
  | DeclareBlockStatement
  | WhileStatement
  | IfStatement
  | ElseifClause
  | ElseClause
  | AlternateIfStatement
  | AlternateElseifClause
  | AlternateElseClause
  | TryStatement
  | CatchClause
  | FinallyClause
  | DoStatement
  | ForStatement
  | ForeachStatement
  | SwitchStatement
  | AlternateSwitchStatement
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
  | AnonymousClass
  | AnonymousFunction
  | Php7AnonymousFunction
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
  | IsExpression
  | AsExpression
  | NullableAsExpression
  | ConditionalExpression
  | EvalExpression
  | EmptyExpression
  | DefineExpression
  | HaltCompilerExpression
  | IssetExpression
  | FunctionCallExpression
  | FunctionCallWithTypeArgumentsExpression
  | ParenthesizedExpression
  | BracedExpression
  | EmbeddedBracedExpression
  | ListExpression
  | CollectionLiteralExpression
  | ObjectCreationExpression
  | ConstructorCall
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
  | XHPSimpleAttribute
  | XHPSpreadAttribute
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
  | ClosureParameterTypeSpecifier
  | ClassnameTypeSpecifier
  | FieldSpecifier
  | FieldInitializer
  | ShapeTypeSpecifier
  | ShapeExpression
  | TupleExpression
  | GenericTypeSpecifier
  | NullableTypeSpecifier
  | SoftTypeSpecifier
  | ReifiedTypeArgument
  | TypeArguments
  | TypeParameters
  | TupleTypeSpecifier
  | ErrorSyntax
  | ListItem

  [@@deriving show]

let to_string kind =
  match kind with
  | Token _                           -> "token"
  | Missing                           -> "missing"
  | SyntaxList                        -> "list"
  | EndOfFile                               -> "end_of_file"
  | Script                                  -> "script"
  | QualifiedName                           -> "qualified_name"
  | SimpleTypeSpecifier                     -> "simple_type_specifier"
  | LiteralExpression                       -> "literal"
  | PrefixedStringExpression                -> "prefixed_string"
  | VariableExpression                      -> "variable"
  | PipeVariableExpression                  -> "pipe_variable"
  | EnumDeclaration                         -> "enum_declaration"
  | Enumerator                              -> "enumerator"
  | AliasDeclaration                        -> "alias_declaration"
  | PropertyDeclaration                     -> "property_declaration"
  | PropertyDeclarator                      -> "property_declarator"
  | NamespaceDeclaration                    -> "namespace_declaration"
  | NamespaceBody                           -> "namespace_body"
  | NamespaceEmptyBody                      -> "namespace_empty_body"
  | NamespaceUseDeclaration                 -> "namespace_use_declaration"
  | NamespaceGroupUseDeclaration            -> "namespace_group_use_declaration"
  | NamespaceUseClause                      -> "namespace_use_clause"
  | FunctionDeclaration                     -> "function_declaration"
  | FunctionDeclarationHeader               -> "function_declaration_header"
  | WhereClause                             -> "where_clause"
  | WhereConstraint                         -> "where_constraint"
  | MethodishDeclaration                    -> "methodish_declaration"
  | ClassishDeclaration                     -> "classish_declaration"
  | ClassishBody                            -> "classish_body"
  | TraitUsePrecedenceItem                  -> "trait_use_precedence_item"
  | TraitUseAliasItem                       -> "trait_use_alias_item"
  | TraitUseConflictResolution              -> "trait_use_conflict_resolution"
  | TraitUse                                -> "trait_use"
  | RequireClause                           -> "require_clause"
  | ConstDeclaration                        -> "const_declaration"
  | ConstantDeclarator                      -> "constant_declarator"
  | TypeConstDeclaration                    -> "type_const_declaration"
  | DecoratedExpression                     -> "decorated_expression"
  | ParameterDeclaration                    -> "parameter_declaration"
  | VariadicParameter                       -> "variadic_parameter"
  | AttributeSpecification                  -> "attribute_specification"
  | Attribute                               -> "attribute"
  | InclusionExpression                     -> "inclusion_expression"
  | InclusionDirective                      -> "inclusion_directive"
  | CompoundStatement                       -> "compound_statement"
  | AlternateLoopStatement                  -> "alternate_loop_statement"
  | ExpressionStatement                     -> "expression_statement"
  | MarkupSection                           -> "markup_section"
  | MarkupSuffix                            -> "markup_suffix"
  | UnsetStatement                          -> "unset_statement"
  | LetStatement                            -> "let_statement"
  | UsingStatementBlockScoped               -> "using_statement_block_scoped"
  | UsingStatementFunctionScoped            -> "using_statement_function_scoped"
  | DeclareDirectiveStatement               -> "declare_directive_statement"
  | DeclareBlockStatement                   -> "declare_block_statement"
  | WhileStatement                          -> "while_statement"
  | IfStatement                             -> "if_statement"
  | ElseifClause                            -> "elseif_clause"
  | ElseClause                              -> "else_clause"
  | AlternateIfStatement                    -> "alternate_if_statement"
  | AlternateElseifClause                   -> "alternate_elseif_clause"
  | AlternateElseClause                     -> "alternate_else_clause"
  | TryStatement                            -> "try_statement"
  | CatchClause                             -> "catch_clause"
  | FinallyClause                           -> "finally_clause"
  | DoStatement                             -> "do_statement"
  | ForStatement                            -> "for_statement"
  | ForeachStatement                        -> "foreach_statement"
  | SwitchStatement                         -> "switch_statement"
  | AlternateSwitchStatement                -> "alternate_switch_statement"
  | SwitchSection                           -> "switch_section"
  | SwitchFallthrough                       -> "switch_fallthrough"
  | CaseLabel                               -> "case_label"
  | DefaultLabel                            -> "default_label"
  | ReturnStatement                         -> "return_statement"
  | GotoLabel                               -> "goto_label"
  | GotoStatement                           -> "goto_statement"
  | ThrowStatement                          -> "throw_statement"
  | BreakStatement                          -> "break_statement"
  | ContinueStatement                       -> "continue_statement"
  | FunctionStaticStatement                 -> "function_static_statement"
  | StaticDeclarator                        -> "static_declarator"
  | EchoStatement                           -> "echo_statement"
  | GlobalStatement                         -> "global_statement"
  | SimpleInitializer                       -> "simple_initializer"
  | AnonymousClass                          -> "anonymous_class"
  | AnonymousFunction                       -> "anonymous_function"
  | Php7AnonymousFunction                   -> "php7_anonymous_function"
  | AnonymousFunctionUseClause              -> "anonymous_function_use_clause"
  | LambdaExpression                        -> "lambda_expression"
  | LambdaSignature                         -> "lambda_signature"
  | CastExpression                          -> "cast_expression"
  | ScopeResolutionExpression               -> "scope_resolution_expression"
  | MemberSelectionExpression               -> "member_selection_expression"
  | SafeMemberSelectionExpression           -> "safe_member_selection_expression"
  | EmbeddedMemberSelectionExpression       -> "embedded_member_selection_expression"
  | YieldExpression                         -> "yield_expression"
  | YieldFromExpression                     -> "yield_from_expression"
  | PrefixUnaryExpression                   -> "prefix_unary_expression"
  | PostfixUnaryExpression                  -> "postfix_unary_expression"
  | BinaryExpression                        -> "binary_expression"
  | InstanceofExpression                    -> "instanceof_expression"
  | IsExpression                            -> "is_expression"
  | AsExpression                            -> "as_expression"
  | NullableAsExpression                    -> "nullable_as_expression"
  | ConditionalExpression                   -> "conditional_expression"
  | EvalExpression                          -> "eval_expression"
  | EmptyExpression                         -> "empty_expression"
  | DefineExpression                        -> "define_expression"
  | HaltCompilerExpression                  -> "halt_compiler_expression"
  | IssetExpression                         -> "isset_expression"
  | FunctionCallExpression                  -> "function_call_expression"
  | FunctionCallWithTypeArgumentsExpression -> "function_call_with_type_arguments_expression"
  | ParenthesizedExpression                 -> "parenthesized_expression"
  | BracedExpression                        -> "braced_expression"
  | EmbeddedBracedExpression                -> "embedded_braced_expression"
  | ListExpression                          -> "list_expression"
  | CollectionLiteralExpression             -> "collection_literal_expression"
  | ObjectCreationExpression                -> "object_creation_expression"
  | ConstructorCall                         -> "constructor_call"
  | ArrayCreationExpression                 -> "array_creation_expression"
  | ArrayIntrinsicExpression                -> "array_intrinsic_expression"
  | DarrayIntrinsicExpression               -> "darray_intrinsic_expression"
  | DictionaryIntrinsicExpression           -> "dictionary_intrinsic_expression"
  | KeysetIntrinsicExpression               -> "keyset_intrinsic_expression"
  | VarrayIntrinsicExpression               -> "varray_intrinsic_expression"
  | VectorIntrinsicExpression               -> "vector_intrinsic_expression"
  | ElementInitializer                      -> "element_initializer"
  | SubscriptExpression                     -> "subscript_expression"
  | EmbeddedSubscriptExpression             -> "embedded_subscript_expression"
  | AwaitableCreationExpression             -> "awaitable_creation_expression"
  | XHPChildrenDeclaration                  -> "xhp_children_declaration"
  | XHPChildrenParenthesizedList            -> "xhp_children_parenthesized_list"
  | XHPCategoryDeclaration                  -> "xhp_category_declaration"
  | XHPEnumType                             -> "xhp_enum_type"
  | XHPRequired                             -> "xhp_required"
  | XHPClassAttributeDeclaration            -> "xhp_class_attribute_declaration"
  | XHPClassAttribute                       -> "xhp_class_attribute"
  | XHPSimpleClassAttribute                 -> "xhp_simple_class_attribute"
  | XHPSimpleAttribute                      -> "xhp_simple_attribute"
  | XHPSpreadAttribute                      -> "xhp_spread_attribute"
  | XHPOpen                                 -> "xhp_open"
  | XHPExpression                           -> "xhp_expression"
  | XHPClose                                -> "xhp_close"
  | TypeConstant                            -> "type_constant"
  | VectorTypeSpecifier                     -> "vector_type_specifier"
  | KeysetTypeSpecifier                     -> "keyset_type_specifier"
  | TupleTypeExplicitSpecifier              -> "tuple_type_explicit_specifier"
  | VarrayTypeSpecifier                     -> "varray_type_specifier"
  | VectorArrayTypeSpecifier                -> "vector_array_type_specifier"
  | TypeParameter                           -> "type_parameter"
  | TypeConstraint                          -> "type_constraint"
  | DarrayTypeSpecifier                     -> "darray_type_specifier"
  | MapArrayTypeSpecifier                   -> "map_array_type_specifier"
  | DictionaryTypeSpecifier                 -> "dictionary_type_specifier"
  | ClosureTypeSpecifier                    -> "closure_type_specifier"
  | ClosureParameterTypeSpecifier           -> "closure_parameter_type_specifier"
  | ClassnameTypeSpecifier                  -> "classname_type_specifier"
  | FieldSpecifier                          -> "field_specifier"
  | FieldInitializer                        -> "field_initializer"
  | ShapeTypeSpecifier                      -> "shape_type_specifier"
  | ShapeExpression                         -> "shape_expression"
  | TupleExpression                         -> "tuple_expression"
  | GenericTypeSpecifier                    -> "generic_type_specifier"
  | NullableTypeSpecifier                   -> "nullable_type_specifier"
  | SoftTypeSpecifier                       -> "soft_type_specifier"
  | ReifiedTypeArgument                     -> "reified_type_argument"
  | TypeArguments                           -> "type_arguments"
  | TypeParameters                          -> "type_parameters"
  | TupleTypeSpecifier                      -> "tuple_type_specifier"
  | ErrorSyntax                             -> "error"
  | ListItem                                -> "list_item"
