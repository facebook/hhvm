(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t =
| Token
| Error
| Missing
| SyntaxList
| ListItem

(* Declarations *)
| ScriptHeader
| Script
| NamespaceDeclaration
| NamespaceBody
| NamespaceGroupUseDeclaration
| NamespaceUseDeclaration
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
| InclusionDirective
| EnumDeclaration
| Enumerator
| AliasDeclaration
| PropertyDeclaration
| PropertyDeclarator

(* Statements *)
| CompoundStatement
| ExpressionStatement
| WhileStatement
| DoStatement
| ForStatement
| ForeachStatement
| IfStatement
| ElseifClause
| ElseClause
| TryStatement
| CatchClause
| FinallyClause
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

(* Expressions *)
| InstanceofExpression
| InclusionExpression
| MemberSelectionExpression
| SafeMemberSelectionExpression
| ScopeResolutionExpression
| YieldExpression
| PrintExpression
| CastExpression
| LambdaExpression
| LambdaSignature
| AnonymousFunction
| AnonymousFunctionUseClause
| LiteralExpression
| VariableExpression
| QualifiedNameExpression
| PipeVariableExpression
| PrefixUnaryOperator
| PostfixUnaryOperator
| BinaryOperator
| ConditionalExpression
| FunctionCallExpression
| ParenthesizedExpression
| BracedExpression
| ListExpression
| CollectionLiteralExpression
| ObjectCreationExpression
| ShapeExpression
| FieldInitializer
| ArrayCreationExpression
| ArrayIntrinsicExpression
| ElementInitializer
| SubscriptExpression
| AwaitableCreationExpression
| XHPChildrenDeclaration
| XHPCategoryDeclaration
| XHPExpression
| XHPOpen
| XHPAttribute
| XHPClose
| XHPClassAttributeDeclaration
| XHPClassAttribute
| XHPEnumType
| XHPRequired

(* Types *)
| SoftTypeSpecifier
| SimpleTypeSpecifier
| NullableTypeSpecifier
| TypeConstraint
| TypeParameter
| TypeConstant
| GenericTypeSpecifier
| TypeArguments
| TypeParameters
| TupleTypeSpecifier
| VectorTypeSpecifier
| MapTypeSpecifier
| ClosureTypeSpecifier
| ClassnameTypeSpecifier
| ShapeTypeSpecifier
| FieldSpecifier

let to_string kind =
  match kind with
  | Missing -> "missing"
  | Token -> "token"
  | MemberSelectionExpression -> "member_selection_expression"
  | SafeMemberSelectionExpression -> "safe_member_selection_expression"
  | ScopeResolutionExpression -> "scope_resolution_expression"
  | YieldExpression -> "yield_expression"
  | PrintExpression -> "print_expression"
  | CastExpression -> "cast_expression"
  | LambdaExpression -> "lambda_expression"
  | LambdaSignature -> "lambda_signature"
  | AnonymousFunction -> "anonymous_function"
  | AnonymousFunctionUseClause -> "anonymous_function_use_clause"
  | LiteralExpression -> "literal"
  | VariableExpression -> "variable"
  | QualifiedNameExpression -> "qualified_name"
  | PipeVariableExpression -> "pipe_variable"
  | Error -> "error"
  | SyntaxList -> "list"
  | ListItem -> "list_item"
  | ScriptHeader -> "header"
  | Script -> "script"
  | NamespaceDeclaration -> "namespace_declaration"
  | NamespaceBody -> "namespace_body"
  | NamespaceGroupUseDeclaration -> "namespace_group_use_declaration"
  | NamespaceUseDeclaration -> "namespace_use_declaration"
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
  | CompoundStatement -> "compound_statement"
  | ExpressionStatement -> "expression_statement"
  | WhileStatement -> "while_statement"
  | DoStatement -> "do_statement"
  | ForStatement -> "for_statement"
  | ForeachStatement -> "foreach_statement"
  | IfStatement -> "if_statement"
  | ElseifClause -> "elseif_clause"
  | ElseClause -> "else_clause"
  | TryStatement -> "try_statement"
  | CatchClause -> "catch_clause"
  | FinallyClause -> "finally_clause"
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
  | PrefixUnaryOperator -> "prefix_unary_operator"
  | PostfixUnaryOperator -> "postfix_unary_operator"
  | BinaryOperator -> "binary_operator"
  | ConditionalExpression -> "conditional_expression"
  | FunctionCallExpression -> "function_call_expression"
  | ParenthesizedExpression -> "parenthesized_expression"
  | BracedExpression -> "braced_expression"
  | ListExpression -> "list_expression"
  | CollectionLiteralExpression -> "collection_literal_expression"
  | ObjectCreationExpression -> "object_creation_expression"
  | ShapeExpression -> "shape_expression"
  | FieldInitializer -> "field_initializer"
  | ArrayCreationExpression -> "array_creation_expression"
  | ArrayIntrinsicExpression -> "array_intrinsic_expression"
  | ElementInitializer -> "element_initializer"
  | SubscriptExpression -> "subscript_expression"
  | TypeConstant -> "type_constant"
  | SimpleTypeSpecifier -> "simple_type_specifier"
  | SoftTypeSpecifier -> "soft_type_specifier"
  | TypeConstraint -> "type_constraint"
  | TypeParameter -> "type_parameter"
  | NullableTypeSpecifier -> "nullable_type_specifier"
  | GenericTypeSpecifier -> "generic_type_specifier"
  | TupleTypeSpecifier -> "tuple_type_specifier"
  | VectorTypeSpecifier -> "vector_type_specifier"
  | MapTypeSpecifier -> "map_type_specifier"
  | ClosureTypeSpecifier -> "closure_type_specifier"
  | ClassnameTypeSpecifier -> "classname_type_specifier"
  | ShapeTypeSpecifier -> "shape_type_specifier"
  | FieldSpecifier -> "field_specifier"
  | TypeArguments -> "type_arguments"
  | TypeParameters -> "type_parameters"
  | InclusionDirective -> "inclusion_directive"
  | InstanceofExpression -> "instanceof_expression"
  | InclusionExpression -> "inclusion_expression"
  | EnumDeclaration -> "enum_declaration"
  | Enumerator -> "enumerator"
  | AliasDeclaration -> "alias_declaration"
  | PropertyDeclaration -> "property_declaration"
  | PropertyDeclarator -> "property_declarator"
  | AwaitableCreationExpression -> "awaitable_creation_expression"
  | XHPChildrenDeclaration -> "xhp_children_declaration"
  | XHPCategoryDeclaration -> "xhp_category_declaration"
  | XHPExpression -> "xhp_expression"
  | XHPOpen -> "xhp_open"
  | XHPAttribute -> "xhp_attribute"
  | XHPClose -> "xhp_close"
  | XHPClassAttributeDeclaration -> "xhp_class_attribute_declaration"
  | XHPClassAttribute -> "xhp_class_attribute"
  | XHPEnumType -> "xhp_enum_type"
  | XHPRequired -> "xhp_required"
