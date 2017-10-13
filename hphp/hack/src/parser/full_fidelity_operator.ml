(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TokenKind = Full_fidelity_token_kind

type t =
| DollarOperator
(* TODO: Is there a better name? Operators should be named as what they do,
not how they look on the page. *)
| IndexingOperator
| FunctionCallOperator
| AwaitOperator
| SuspendOperator
| PipeOperator
| ConditionalQuestionOperator
| ConditionalColonOperator
| CoalesceOperator
| PHPOrOperator
| PHPExclusiveOrOperator
| PHPAndOperator
| PrintOperator
| LogicalOrOperator
| ExclusiveOrOperator
| LogicalAndOperator
| OrOperator
| AndOperator
| EqualOperator
| StrictEqualOperator
| NotEqualOperator
| PhpNotEqualOperator
| StrictNotEqualOperator
| SpaceshipOperator
| LessThanOperator
| LessThanOrEqualOperator
| GreaterThanOperator
| GreaterThanOrEqualOperator
| LeftShiftOperator
| RightShiftOperator
| AdditionOperator
| SubtractionOperator
| ConcatenationOperator
| MultiplicationOperator
| DivisionOperator
| RemainderOperator
| LogicalNotOperator
| InstanceofOperator
| NotOperator
| PrefixIncrementOperator
| PrefixDecrementOperator
| PostfixIncrementOperator
| PostfixDecrementOperator
| CastOperator
| ExponentOperator
| ReferenceOperator
| ErrorControlOperator
| NewOperator
| CloneOperator
| AssignmentOperator
| AdditionAssignmentOperator
| SubtractionAssignmentOperator
| MultiplicationAssignmentOperator
| DivisionAssignmentOperator
| ExponentiationAssignmentOperator
| ConcatenationAssignmentOperator
| RemainderAssignmentOperator
| AndAssignmentOperator
| OrAssignmentOperator
| ExclusiveOrAssignmentOperator
| LeftShiftAssignmentOperator
| RightShiftAssignmentOperator
| MemberSelectionOperator
| NullSafeMemberSelectionOperator
| ScopeResolutionOperator
| UnaryPlusOperator
| UnaryMinusOperator
| IncludeOperator
| IncludeOnceOperator
| RequireOperator
| RequireOnceOperator

type assoc =
| LeftAssociative
| RightAssociative
| NotAssociative

let precedence operator =
  (* TODO: eval *)
  (* TODO: Comma *)
  (* TODO: elseif *)
  (* TODO: else *)
  (* TODO: endif *)
  (* TODO: variable operator $ *)
  match operator with
  | IncludeOperator | IncludeOnceOperator | RequireOperator
  | RequireOnceOperator | AwaitOperator -> 1
  | PHPOrOperator -> 2
  | PHPExclusiveOrOperator -> 3
  | PHPAndOperator -> 4
  | PrintOperator -> 5
  | AssignmentOperator | AdditionAssignmentOperator
  | SubtractionAssignmentOperator | MultiplicationAssignmentOperator
  | DivisionAssignmentOperator | ExponentiationAssignmentOperator
  | RemainderAssignmentOperator | ConcatenationAssignmentOperator
  | AndAssignmentOperator
  | OrAssignmentOperator | ExclusiveOrAssignmentOperator
  | LeftShiftAssignmentOperator | RightShiftAssignmentOperator
    -> 6
  | PipeOperator -> 7
  | ConditionalQuestionOperator | ConditionalColonOperator -> 8
  | CoalesceOperator -> 9
  | LogicalOrOperator -> 10
  | LogicalAndOperator -> 11
  | OrOperator -> 12
  | ExclusiveOrOperator -> 13
  | AndOperator -> 14
  | EqualOperator | StrictEqualOperator
  | PhpNotEqualOperator | NotEqualOperator | StrictNotEqualOperator -> 15
  | SpaceshipOperator | LessThanOperator | LessThanOrEqualOperator
  | GreaterThanOperator | GreaterThanOrEqualOperator -> 16
  | LeftShiftOperator | RightShiftOperator -> 17
  | AdditionOperator | SubtractionOperator | ConcatenationOperator -> 18
  | MultiplicationOperator | DivisionOperator | RemainderOperator | SuspendOperator -> 19
  | CastOperator
  | ReferenceOperator | ErrorControlOperator
  | PrefixIncrementOperator | PrefixDecrementOperator
  | LogicalNotOperator| NotOperator
  | UnaryPlusOperator | UnaryMinusOperator -> 20
  | InstanceofOperator -> 21
  | ExponentOperator -> 22
  | PostfixIncrementOperator | PostfixDecrementOperator -> 23
  | CloneOperator -> 24
  | FunctionCallOperator -> 25
  | NewOperator -> 26
  (* value 27 is reserved for assignment that appear in expressions *)
  | IndexingOperator -> 28
  | MemberSelectionOperator | NullSafeMemberSelectionOperator -> 29
  | ScopeResolutionOperator -> 30
  | DollarOperator -> 31

let precedence_for_assignment_in_expressions = 27

let associativity operator =
  match operator with
  | EqualOperator | StrictEqualOperator | NotEqualOperator | PhpNotEqualOperator
  | StrictNotEqualOperator | LessThanOperator | LessThanOrEqualOperator
  | GreaterThanOperator | GreaterThanOrEqualOperator | InstanceofOperator
  | NewOperator | CloneOperator | AwaitOperator | SpaceshipOperator
    -> NotAssociative

  | PipeOperator | ConditionalQuestionOperator | ConditionalColonOperator
  | LogicalOrOperator | ExclusiveOrOperator | LogicalAndOperator
  | OrOperator | AndOperator | LeftShiftOperator | RightShiftOperator
  | AdditionOperator | SubtractionOperator | ConcatenationOperator
  | MultiplicationOperator | DivisionOperator | RemainderOperator
  | MemberSelectionOperator | NullSafeMemberSelectionOperator
  | ScopeResolutionOperator | FunctionCallOperator | IndexingOperator
  | IncludeOperator | IncludeOnceOperator | RequireOperator
  | RequireOnceOperator | PHPAndOperator | PHPOrOperator
  | PHPExclusiveOrOperator
  (* eval *)
  (* Comma *)
  (* elseif *)
  (* else *)
  (* endif *)
    -> LeftAssociative
  | CoalesceOperator| LogicalNotOperator | NotOperator | CastOperator
  | DollarOperator | UnaryPlusOperator | UnaryMinusOperator  (* TODO: Correct? *)
  | ErrorControlOperator | ReferenceOperator (* TODO: Correct? *)
  | PostfixIncrementOperator | PostfixDecrementOperator
  | PrefixIncrementOperator | PrefixDecrementOperator | ExponentOperator
  | AssignmentOperator | AdditionAssignmentOperator
  | SubtractionAssignmentOperator | MultiplicationAssignmentOperator
  | DivisionAssignmentOperator | ExponentiationAssignmentOperator
  | ConcatenationAssignmentOperator
  | RemainderAssignmentOperator | AndAssignmentOperator
  | OrAssignmentOperator | ExclusiveOrAssignmentOperator
  | LeftShiftAssignmentOperator | RightShiftAssignmentOperator
  | PrintOperator | SuspendOperator
    -> RightAssociative

let prefix_unary_from_token token =
  match token with
  | TokenKind.Suspend -> SuspendOperator
  | TokenKind.Await -> AwaitOperator
  | TokenKind.Exclamation -> LogicalNotOperator
  | TokenKind.Tilde -> NotOperator
  | TokenKind.PlusPlus -> PrefixIncrementOperator
  | TokenKind.MinusMinus -> PrefixDecrementOperator
  | TokenKind.Dollar -> DollarOperator
  | TokenKind.Plus -> UnaryPlusOperator
  | TokenKind.Minus -> UnaryMinusOperator
  | TokenKind.Ampersand -> ReferenceOperator
  | TokenKind.At -> ErrorControlOperator
  | TokenKind.New -> NewOperator
  | TokenKind.Clone -> CloneOperator
  | TokenKind.Include -> IncludeOperator
  | TokenKind.Include_once -> IncludeOnceOperator
  | TokenKind.Require -> RequireOperator
  | TokenKind.Require_once -> RequireOnceOperator
  | TokenKind.Print -> PrintOperator
  | _ -> failwith "not a unary operator"

(* Is this a token that can appear after an expression? *)
let is_trailing_operator_token token =
  match token with
  | TokenKind.And
  | TokenKind.Or
  | TokenKind.Xor
  | TokenKind.PlusPlus
  | TokenKind.MinusMinus
  | TokenKind.LeftParen
  | TokenKind.LeftBracket
  | TokenKind.LeftBrace
  | TokenKind.Plus
  | TokenKind.Minus
  | TokenKind.Ampersand
  | TokenKind.BarGreaterThan
  | TokenKind.Question
  | TokenKind.QuestionQuestion
  | TokenKind.BarBar
  | TokenKind.Carat
  | TokenKind.AmpersandAmpersand
  | TokenKind.Bar
  | TokenKind.EqualEqual
  | TokenKind.EqualEqualEqual
  | TokenKind.LessThanGreaterThan
  | TokenKind.ExclamationEqual
  | TokenKind.ExclamationEqualEqual
  | TokenKind.LessThanEqualGreaterThan
  | TokenKind.LessThan
  | TokenKind.LessThanEqual
  | TokenKind.GreaterThan
  | TokenKind.GreaterThanEqual
  | TokenKind.LessThanLessThan
  | TokenKind.GreaterThanGreaterThan
  | TokenKind.Dot
  | TokenKind.Star
  | TokenKind.Slash
  | TokenKind.Percent
  | TokenKind.Instanceof
  | TokenKind.StarStar
  | TokenKind.Equal
  | TokenKind.PlusEqual
  | TokenKind.MinusEqual
  | TokenKind.StarEqual
  | TokenKind.SlashEqual
  | TokenKind.StarStarEqual
  | TokenKind.DotEqual
  | TokenKind.PercentEqual
  | TokenKind.AmpersandEqual
  | TokenKind.BarEqual
  | TokenKind.CaratEqual
  | TokenKind.LessThanLessThanEqual
  | TokenKind.GreaterThanGreaterThanEqual
  | TokenKind.MinusGreaterThan
  | TokenKind.QuestionMinusGreaterThan
  | TokenKind.ColonColon -> true
  | _ -> false

let trailing_from_token token =
  match token with
  | TokenKind.And -> PHPAndOperator
  | TokenKind.Or -> PHPOrOperator
  | TokenKind.Xor -> PHPExclusiveOrOperator
  | TokenKind.BarGreaterThan -> PipeOperator
  | TokenKind.Question -> ConditionalQuestionOperator
  | TokenKind.Colon -> ConditionalColonOperator
  | TokenKind.QuestionQuestion -> CoalesceOperator
  | TokenKind.BarBar -> LogicalOrOperator
  | TokenKind.Carat -> ExclusiveOrOperator
  | TokenKind.AmpersandAmpersand -> LogicalAndOperator
  | TokenKind.Bar -> OrOperator
  | TokenKind.Ampersand -> AndOperator
  | TokenKind.EqualEqual -> EqualOperator
  | TokenKind.EqualEqualEqual -> StrictEqualOperator
  | TokenKind.ExclamationEqual -> NotEqualOperator
  | TokenKind.LessThanGreaterThan -> PhpNotEqualOperator
  | TokenKind.ExclamationEqualEqual -> StrictNotEqualOperator
  | TokenKind.LessThan -> LessThanOperator
  | TokenKind.LessThanEqualGreaterThan -> SpaceshipOperator
  | TokenKind.LessThanEqual -> LessThanOrEqualOperator
  | TokenKind.GreaterThan -> GreaterThanOperator
  | TokenKind.GreaterThanEqual -> GreaterThanOrEqualOperator
  | TokenKind.LessThanLessThan -> LeftShiftOperator
  | TokenKind.GreaterThanGreaterThan -> RightShiftOperator
  | TokenKind.Plus -> AdditionOperator
  | TokenKind.Minus -> SubtractionOperator
  | TokenKind.Dot -> ConcatenationOperator
  | TokenKind.Star -> MultiplicationOperator
  | TokenKind.Slash -> DivisionOperator
  | TokenKind.Percent -> RemainderOperator
  | TokenKind.Instanceof -> InstanceofOperator
  | TokenKind.StarStar -> ExponentOperator
  | TokenKind.Equal -> AssignmentOperator
  | TokenKind.PlusEqual -> AdditionAssignmentOperator
  | TokenKind.MinusEqual -> SubtractionAssignmentOperator
  | TokenKind.StarEqual -> MultiplicationAssignmentOperator
  | TokenKind.SlashEqual -> DivisionAssignmentOperator
  | TokenKind.StarStarEqual -> ExponentiationAssignmentOperator
  | TokenKind.DotEqual -> ConcatenationAssignmentOperator
  | TokenKind.PercentEqual -> RemainderAssignmentOperator
  | TokenKind.AmpersandEqual -> AndAssignmentOperator
  | TokenKind.BarEqual -> OrAssignmentOperator
  | TokenKind.CaratEqual -> ExclusiveOrAssignmentOperator
  | TokenKind.LessThanLessThanEqual -> LeftShiftAssignmentOperator
  | TokenKind.GreaterThanGreaterThanEqual -> RightShiftAssignmentOperator
  | TokenKind.MinusGreaterThan -> MemberSelectionOperator
  | TokenKind.QuestionMinusGreaterThan -> NullSafeMemberSelectionOperator
  | TokenKind.ColonColon -> ScopeResolutionOperator
  | TokenKind.PlusPlus -> PostfixIncrementOperator
  | TokenKind.MinusMinus -> PostfixDecrementOperator
  | TokenKind.LeftParen -> FunctionCallOperator
  | TokenKind.LeftBracket -> IndexingOperator
  | TokenKind.LeftBrace -> IndexingOperator
  | _ -> failwith (Printf.sprintf "%s is not a trailing operator"
                    (Full_fidelity_token_kind.to_string token))

let is_binary_operator_token token =
  match token with
  | TokenKind.And
  | TokenKind.Or
  | TokenKind.Xor
  | TokenKind.Plus
  | TokenKind.Minus
  | TokenKind.Ampersand
  | TokenKind.BarGreaterThan
  | TokenKind.QuestionQuestion
  | TokenKind.BarBar
  | TokenKind.Carat
  | TokenKind.AmpersandAmpersand
  | TokenKind.Bar
  | TokenKind.EqualEqual
  | TokenKind.EqualEqualEqual
  | TokenKind.ExclamationEqual
  | TokenKind.LessThanGreaterThan
  | TokenKind.ExclamationEqualEqual
  | TokenKind.LessThanEqualGreaterThan
  | TokenKind.LessThan
  | TokenKind.LessThanEqual
  | TokenKind.GreaterThan
  | TokenKind.GreaterThanEqual
  | TokenKind.LessThanLessThan
  | TokenKind.GreaterThanGreaterThan
  | TokenKind.Dot
  | TokenKind.Star
  | TokenKind.Slash
  | TokenKind.Percent
  | TokenKind.StarStar
  | TokenKind.Equal
  | TokenKind.PlusEqual
  | TokenKind.MinusEqual
  | TokenKind.StarEqual
  | TokenKind.SlashEqual
  | TokenKind.DotEqual
  | TokenKind.PercentEqual
  | TokenKind.AmpersandEqual
  | TokenKind.BarEqual
  | TokenKind.CaratEqual
  | TokenKind.LessThanLessThanEqual
  | TokenKind.GreaterThanGreaterThanEqual
  | TokenKind.MinusGreaterThan
  | TokenKind.QuestionMinusGreaterThan -> true
  | _ -> false

let is_assignment operator =
  match operator with
  | AssignmentOperator
  | AdditionAssignmentOperator
  | SubtractionAssignmentOperator
  | MultiplicationAssignmentOperator
  | DivisionAssignmentOperator
  | ExponentiationAssignmentOperator
  | ConcatenationAssignmentOperator
  | RemainderAssignmentOperator
  | AndAssignmentOperator
  | OrAssignmentOperator
  | ExclusiveOrAssignmentOperator
  | LeftShiftAssignmentOperator
  | RightShiftAssignmentOperator -> true
  | _ -> false

let is_comparison operator =
  match operator with
  | EqualOperator
  | StrictEqualOperator
  | NotEqualOperator
  | PhpNotEqualOperator
  | StrictNotEqualOperator
  | LessThanOperator
  | LessThanOrEqualOperator
  | GreaterThanOperator
  | GreaterThanOrEqualOperator
  | SpaceshipOperator -> true
  | _ -> false

let to_string kind =
  match kind with
  | PHPAndOperator -> "php_and"
  | PHPOrOperator -> "php_or"
  | PHPExclusiveOrOperator -> "php_exclusive_or"
  | IndexingOperator -> "indexing"
  | FunctionCallOperator -> "function_call"
  | AwaitOperator -> "await"
  | SuspendOperator -> "suspend"
  | PipeOperator -> "pipe"
  | ConditionalQuestionOperator -> "conditional"
  | ConditionalColonOperator -> "colon"
  | CoalesceOperator -> "coalesce"
  | LogicalOrOperator -> "logical_or"
  | ExclusiveOrOperator -> "exclusive_or"
  | LogicalAndOperator -> "logical_and"
  | OrOperator -> "or"
  | AndOperator -> "and"
  | EqualOperator -> "equal"
  | StrictEqualOperator -> "strict_equal"
  | NotEqualOperator -> "not_equal"
  | PhpNotEqualOperator -> "php_not_equal"
  | StrictNotEqualOperator -> "strict_not_equal"
  | LessThanOperator -> "less_than"
  | SpaceshipOperator -> "spaceship"
  | LessThanOrEqualOperator -> "less_than_or_equal"
  | GreaterThanOperator -> "greater_than"
  | GreaterThanOrEqualOperator -> "greater_than_or_equal"
  | LeftShiftOperator -> "left_shift"
  | RightShiftOperator -> "right_shift"
  | AdditionOperator -> "addition"
  | SubtractionOperator -> "subtraction"
  | ConcatenationOperator -> "concatenation"
  | MultiplicationOperator -> "multiplication"
  | DivisionOperator -> "division"
  | RemainderOperator -> "remainder"
  | LogicalNotOperator -> "logical_not"
  | InstanceofOperator -> "instanceof"
  | NotOperator -> "not"
  | PrefixIncrementOperator -> "prefix_increment"
  | PrefixDecrementOperator -> "prefix_decrement"
  | PostfixIncrementOperator -> "postfix_increment"
  | PostfixDecrementOperator -> "postfix_decrement"
  | CastOperator -> "cast"
  | ExponentOperator -> "exponentiation"
  | ReferenceOperator -> "reference"
  | ErrorControlOperator -> "error_control"
  | NewOperator -> "new"
  | CloneOperator -> "clone"
  | AssignmentOperator -> "assignment"
  | AdditionAssignmentOperator -> "addition_assignment"
  | SubtractionAssignmentOperator -> "subtraction_assignment"
  | MultiplicationAssignmentOperator -> "multiplication_assignment"
  | DivisionAssignmentOperator -> "division_assignment"
  | ExponentiationAssignmentOperator -> "exponentiation_assignment"
  | ConcatenationAssignmentOperator -> "concatenation_assignment"
  | RemainderAssignmentOperator -> "reminder_assignment"
  | AndAssignmentOperator -> "and_assignment"
  | OrAssignmentOperator -> "or_assignment"
  | ExclusiveOrAssignmentOperator -> "exclusive_or_assignment"
  | LeftShiftAssignmentOperator -> "left_shift_assignment"
  | RightShiftAssignmentOperator -> "right_shift_assignment"
  | MemberSelectionOperator -> "member_selection"
  | NullSafeMemberSelectionOperator -> "null_safe_member_selection"
  | ScopeResolutionOperator -> "scope_resolution"
  | DollarOperator -> "dollar"
  | UnaryPlusOperator -> "unary_plus"
  | UnaryMinusOperator -> "unary_minus"
  | IncludeOperator -> "include"
  | IncludeOnceOperator -> "include_once"
  | RequireOperator -> "require"
  | RequireOnceOperator -> "require_once"
  | PrintOperator -> "print"
