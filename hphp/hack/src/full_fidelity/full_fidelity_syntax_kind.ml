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
| SimpleInitializer

(* Expressions *)
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
| ObjectCreationExpression
| ShapeExpression
| FieldInitializer
| ArrayCreationExpression
| ArrayIntrinsicExpression
| XHPExpression
| XHPOpen
| XHPAttribute
| XHPClose

(* Types *)
| SimpleTypeSpecifier
| NullableTypeSpecifier
| TypeConstraint
| TypeParameter
| TypeConstant
| GenericTypeSpecifier
| TypeArguments
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
  | SimpleInitializer -> "simple_initializer"
  | PrefixUnaryOperator -> "prefix_unary_operator"
  | PostfixUnaryOperator -> "postfix_unary_operator"
  | BinaryOperator -> "binary_operator"
  | ConditionalExpression -> "conditional_expression"
  | FunctionCallExpression -> "function_call_expression"
  | ParenthesizedExpression -> "parenthesized_expression"
  | BracedExpression -> "braced_expression"
  | ListExpression -> "list_expression"
  | ObjectCreationExpression -> "object_creation_expression"
  | ShapeExpression -> "shape_expression"
  | FieldInitializer -> "field_initializer"
  | ArrayCreationExpression -> "array_creation_expression"
  | ArrayIntrinsicExpression -> "array_intrinsic_expression"
  | TypeConstant -> "type_constant"
  | SimpleTypeSpecifier -> "simple_type_specifier"
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
  | InclusionDirective -> "inclusion_directive"
  | EnumDeclaration -> "enum_declaration"
  | Enumerator -> "enumerator"
  | AliasDeclaration -> "alias_declaration"
  | PropertyDeclaration -> "property_declaration"
  | PropertyDeclarator -> "property_declarator"
  | XHPExpression -> "xhp_expression"
  | XHPOpen -> "xhp_open"
  | XHPAttribute -> "xhp_attribute"
  | XHPClose -> "xhp_close"
