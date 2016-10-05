(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*)
type t =
| Token
| Missing
| SyntaxList
| ScriptHeader
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
| NamespaceUseDeclaration
| NamespaceGroupUseDeclaration
| NamespaceUseClause
| FunctionDeclaration
| FunctionDeclarationHeader
| MethodishDeclaration
| ClassishDeclaration
| ClassishBody
| TraitUse
| RequireClause
| ConstDeclaration
| ConstantDeclarator
| TypeConstDeclaration
| DecoratedExpression
| ParameterDeclaration
| AttributeSpecification
| Attribute
| InclusionExpression
| InclusionDirective
| CompoundStatement
| ExpressionStatement
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
| CaseStatement
| DefaultStatement
| ReturnStatement
| ThrowStatement
| BreakStatement
| ContinueStatement
| FunctionStaticStatement
| StaticDeclarator
| EchoStatement
| SimpleInitializer
| AnonymousFunction
| AnonymousFunctionUseClause
| LambdaExpression
| LambdaSignature
| CastExpression
| ScopeResolutionExpression
| MemberSelectionExpression
| SafeMemberSelectionExpression
| YieldExpression
| PrintExpression
| PrefixUnaryExpression
| PostfixUnaryExpression
| BinaryExpression
| InstanceofExpression
| ConditionalExpression
| FunctionCallExpression
| ParenthesizedExpression
| BracedExpression
| ListExpression
| CollectionLiteralExpression
| ObjectCreationExpression
| ArrayCreationExpression
| ArrayIntrinsicExpression
| ElementInitializer
| SubscriptExpression
| AwaitableCreationExpression
| XHPChildrenDeclaration
| XHPCategoryDeclaration
| XHPEnumType
| XHPRequired
| XHPClassAttributeDeclaration
| XHPClassAttribute
| XHPAttribute
| XHPOpen
| XHPExpression
| XHPClose
| TypeConstant
| VectorTypeSpecifier
| TypeParameter
| TypeConstraint
| MapTypeSpecifier
| ClosureTypeSpecifier
| ClassnameTypeSpecifier
| FieldSpecifier
| FieldInitializer
| ShapeTypeSpecifier
| ShapeExpression
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
  | Missing -> "missing"
  | Token -> "token"
  | SyntaxList -> "list"
  | ScriptHeader -> "header"
  | Script -> "script"
  | SimpleTypeSpecifier -> "simple_type_specifier"
  | LiteralExpression -> "literal"
  | VariableExpression -> "variable"
  | QualifiedNameExpression -> "qualified_name"
  | PipeVariableExpression -> "pipe_variable"
  | EnumDeclaration -> "enum_declaration"
  | Enumerator -> "enumerator"
  | AliasDeclaration -> "alias_declaration"
  | PropertyDeclaration -> "property_declaration"
  | PropertyDeclarator -> "property_declarator"
  | NamespaceDeclaration -> "namespace_declaration"
  | NamespaceBody -> "namespace_body"
  | NamespaceUseDeclaration -> "namespace_use_declaration"
  | NamespaceGroupUseDeclaration -> "namespace_group_use_declaration"
  | NamespaceUseClause -> "namespace_use_clause"
  | FunctionDeclaration -> "function_declaration"
  | FunctionDeclarationHeader -> "function_declaration_header"
  | MethodishDeclaration -> "methodish_declaration"
  | ClassishDeclaration -> "classish_declaration"
  | ClassishBody -> "classish_body"
  | TraitUse -> "trait_use"
  | RequireClause -> "require_clause"
  | ConstDeclaration -> "const_declaration"
  | ConstantDeclarator -> "constant_declarator"
  | TypeConstDeclaration -> "type_const_declaration"
  | DecoratedExpression -> "decorated_expression"
  | ParameterDeclaration -> "parameter_declaration"
  | AttributeSpecification -> "attribute_specification"
  | Attribute -> "attribute"
  | InclusionExpression -> "inclusion_expression"
  | InclusionDirective -> "inclusion_directive"
  | CompoundStatement -> "compound_statement"
  | ExpressionStatement -> "expression_statement"
  | WhileStatement -> "while_statement"
  | IfStatement -> "if_statement"
  | ElseifClause -> "elseif_clause"
  | ElseClause -> "else_clause"
  | TryStatement -> "try_statement"
  | CatchClause -> "catch_clause"
  | FinallyClause -> "finally_clause"
  | DoStatement -> "do_statement"
  | ForStatement -> "for_statement"
  | ForeachStatement -> "foreach_statement"
  | SwitchStatement -> "switch_statement"
  | CaseStatement -> "case_statement"
  | DefaultStatement -> "default_statement"
  | ReturnStatement -> "return_statement"
  | ThrowStatement -> "throw_statement"
  | BreakStatement -> "break_statement"
  | ContinueStatement -> "continue_statement"
  | FunctionStaticStatement -> "function_static_statement"
  | StaticDeclarator -> "static_declarator"
  | EchoStatement -> "echo_statement"
  | SimpleInitializer -> "simple_initializer"
  | AnonymousFunction -> "anonymous_function"
  | AnonymousFunctionUseClause -> "anonymous_function_use_clause"
  | LambdaExpression -> "lambda_expression"
  | LambdaSignature -> "lambda_signature"
  | CastExpression -> "cast_expression"
  | ScopeResolutionExpression -> "scope_resolution_expression"
  | MemberSelectionExpression -> "member_selection_expression"
  | SafeMemberSelectionExpression -> "safe_member_selection_expression"
  | YieldExpression -> "yield_expression"
  | PrintExpression -> "print_expression"
  | PrefixUnaryExpression -> "prefix_unary_expression"
  | PostfixUnaryExpression -> "postfix_unary_expression"
  | BinaryExpression -> "binary_expression"
  | InstanceofExpression -> "instanceof_expression"
  | ConditionalExpression -> "conditional_expression"
  | FunctionCallExpression -> "function_call_expression"
  | ParenthesizedExpression -> "parenthesized_expression"
  | BracedExpression -> "braced_expression"
  | ListExpression -> "list_expression"
  | CollectionLiteralExpression -> "collection_literal_expression"
  | ObjectCreationExpression -> "object_creation_expression"
  | ArrayCreationExpression -> "array_creation_expression"
  | ArrayIntrinsicExpression -> "array_intrinsic_expression"
  | ElementInitializer -> "element_initializer"
  | SubscriptExpression -> "subscript_expression"
  | AwaitableCreationExpression -> "awaitable_creation_expression"
  | XHPChildrenDeclaration -> "xhp_children_declaration"
  | XHPCategoryDeclaration -> "xhp_category_declaration"
  | XHPEnumType -> "xhp_enum_type"
  | XHPRequired -> "xhp_required"
  | XHPClassAttributeDeclaration -> "xhp_class_attribute_declaration"
  | XHPClassAttribute -> "xhp_class_attribute"
  | XHPAttribute -> "xhp_attribute"
  | XHPOpen -> "xhp_open"
  | XHPExpression -> "xhp_expression"
  | XHPClose -> "xhp_close"
  | TypeConstant -> "type_constant"
  | VectorTypeSpecifier -> "vector_type_specifier"
  | TypeParameter -> "type_parameter"
  | TypeConstraint -> "type_constraint"
  | MapTypeSpecifier -> "map_type_specifier"
  | ClosureTypeSpecifier -> "closure_type_specifier"
  | ClassnameTypeSpecifier -> "classname_type_specifier"
  | FieldSpecifier -> "field_specifier"
  | FieldInitializer -> "field_initializer"
  | ShapeTypeSpecifier -> "shape_type_specifier"
  | ShapeExpression -> "shape_expression"
  | GenericTypeSpecifier -> "generic_type_specifier"
  | NullableTypeSpecifier -> "nullable_type_specifier"
  | SoftTypeSpecifier -> "soft_type_specifier"
  | TypeArguments -> "type_arguments"
  | TypeParameters -> "type_parameters"
  | TupleTypeSpecifier -> "tuple_type_specifier"
  | ErrorSyntax -> "error"
  | ListItem -> "list_item"
