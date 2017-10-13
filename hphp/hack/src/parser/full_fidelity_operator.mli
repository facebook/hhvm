(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module TokenKind : sig
  type t = Full_fidelity_token_kind.t
end

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

val precedence : t -> int

val precedence_for_assignment_in_expressions : int

val associativity : t -> assoc

val prefix_unary_from_token : TokenKind.t -> t

val is_trailing_operator_token : TokenKind.t -> bool

val trailing_from_token : TokenKind.t -> t

val is_binary_operator_token : TokenKind.t -> bool

val is_assignment : t -> bool

val is_comparison : t -> bool

val to_string : t -> string
