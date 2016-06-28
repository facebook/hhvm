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
| FunctionDeclaration
| ParameterDeclaration
| DefaultArgumentSpecifier

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

(* Expressions *)
| LiteralExpression
| VariableExpression
| QualifiedNameExpression
| PrefixUnaryOperator
| PostfixUnaryOperator
| BinaryOperator
| ConditionalExpression
| ParenthesizedExpression
| BracedExpression
| ListExpression
| ObjectCreationExpression
| XHPExpression
| XHPOpen
| XHPAttribute

(* Types *)
| SimpleTypeSpecifier
| NullableTypeSpecifier
| TypeConstant
| GenericTypeSpecifier
| TypeArguments
| TupleTypeSpecifier
| VectorTypeSpecifier
| MapTypeSpecifier
| ClosureTypeSpecifier

let to_string kind =
  match kind with
  | Missing -> "missing"
  | Token -> "token"
  | LiteralExpression -> "literal"
  | VariableExpression -> "variable"
  | QualifiedNameExpression -> "qualified_name"
  | Error -> "error"
  | SyntaxList -> "list"
  | ListItem -> "list_item"
  | ScriptHeader -> "header"
  | Script -> "script"
  | FunctionDeclaration -> "function_declaration"
  | ParameterDeclaration -> "parameter_declaration"
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
  | PrefixUnaryOperator -> "prefix_unary_operator"
  | PostfixUnaryOperator -> "postfix_unary_operator"
  | BinaryOperator -> "binary_operator"
  | ConditionalExpression -> "conditional_expression"
  | ParenthesizedExpression -> "parenthesized_expression"
  | BracedExpression -> "braced_expression"
  | ListExpression -> "list_expression"
  | ObjectCreationExpression -> "object_creation_expression"
  | TypeConstant -> "type_constant"
  | SimpleTypeSpecifier -> "simple_type_specifier"
  | NullableTypeSpecifier -> "nullable_type_specifier"
  | GenericTypeSpecifier -> "generic_type_specifier"
  | TupleTypeSpecifier -> "tuple_type_specifier"
  | VectorTypeSpecifier -> "vector_type_specifier"
  | MapTypeSpecifier -> "map_type_specifier"
  | ClosureTypeSpecifier -> "closure_type_specifier"
  | TypeArguments -> "type_arguments"
  | DefaultArgumentSpecifier -> "default_argument_specifier"
  | XHPExpression -> "xhp_expression"
  | XHPOpen -> "xhp_open"
  | XHPAttribute -> "xhp_attribute"
