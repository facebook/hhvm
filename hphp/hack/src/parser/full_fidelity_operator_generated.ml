(*
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
module type Sig = sig
  type t =
    | DollarOperator
    | ScopeResolutionOperator
    | IndexingOperator
    | NullSafeMemberSelectionOperator
    | MemberSelectionOperator
    | NewOperator
    | FunctionCallOperator
    | CloneOperator
    | PostfixIncrementOperator
    | PostfixDecrementOperator
    | PrefixIncrementOperator
    | PrefixDecrementOperator
    | ExponentOperator
    | ErrorControlOperator
    | CastOperator
    | NullableAsOperator
    | IsOperator
    | InstanceofOperator
    | AsOperator
    | UpcastOperator
    | UnaryPlusOperator
    | UnaryMinusOperator
    | NotOperator
    | LogicalNotOperator
    | RemainderOperator
    | MultiplicationOperator
    | DivisionOperator
    | SubtractionOperator
    | ConcatenationOperator
    | AdditionOperator
    | RightShiftOperator
    | LeftShiftOperator
    | SpaceshipOperator
    | LessThanOrEqualOperator
    | LessThanOperator
    | GreaterThanOrEqualOperator
    | GreaterThanOperator
    | StrictNotEqualOperator
    | StrictEqualOperator
    | NotEqualOperator
    | EqualOperator
    | AndOperator
    | ExclusiveOrOperator
    | OrOperator
    | LogicalAndOperator
    | LogicalOrOperator
    | CoalesceOperator
    | DegenerateConditionalOperator
    | ConditionalQuestionOperator
    | ConditionalColonOperator
    | PipeOperator
    | SubtractionAssignmentOperator
    | RightShiftAssignmentOperator
    | RemainderAssignmentOperator
    | OrAssignmentOperator
    | MultiplicationAssignmentOperator
    | LeftShiftAssignmentOperator
    | ExponentiationAssignmentOperator
    | ExclusiveOrAssignmentOperator
    | DivisionAssignmentOperator
    | ConcatenationAssignmentOperator
    | CoalesceAssignmentOperator
    | AssignmentOperator
    | AndAssignmentOperator
    | AdditionAssignmentOperator
    | PrintOperator
    | RequireOperator
    | RequireOnceOperator
    | IncludeOperator
    | IncludeOnceOperator
    | AwaitOperator
    | ReadonlyOperator
    | EnumClassLabelOperator
    | PackageOperator
    | NameofOperator
end

module Impl : Sig = struct
  type t =
    | DollarOperator
    | ScopeResolutionOperator
    | IndexingOperator
    | NullSafeMemberSelectionOperator
    | MemberSelectionOperator
    | NewOperator
    | FunctionCallOperator
    | CloneOperator
    | PostfixIncrementOperator
    | PostfixDecrementOperator
    | PrefixIncrementOperator
    | PrefixDecrementOperator
    | ExponentOperator
    | ErrorControlOperator
    | CastOperator
    | NullableAsOperator
    | IsOperator
    | InstanceofOperator
    | AsOperator
    | UpcastOperator
    | UnaryPlusOperator
    | UnaryMinusOperator
    | NotOperator
    | LogicalNotOperator
    | RemainderOperator
    | MultiplicationOperator
    | DivisionOperator
    | SubtractionOperator
    | ConcatenationOperator
    | AdditionOperator
    | RightShiftOperator
    | LeftShiftOperator
    | SpaceshipOperator
    | LessThanOrEqualOperator
    | LessThanOperator
    | GreaterThanOrEqualOperator
    | GreaterThanOperator
    | StrictNotEqualOperator
    | StrictEqualOperator
    | NotEqualOperator
    | EqualOperator
    | AndOperator
    | ExclusiveOrOperator
    | OrOperator
    | LogicalAndOperator
    | LogicalOrOperator
    | CoalesceOperator
    | DegenerateConditionalOperator
    | ConditionalQuestionOperator
    | ConditionalColonOperator
    | PipeOperator
    | SubtractionAssignmentOperator
    | RightShiftAssignmentOperator
    | RemainderAssignmentOperator
    | OrAssignmentOperator
    | MultiplicationAssignmentOperator
    | LeftShiftAssignmentOperator
    | ExponentiationAssignmentOperator
    | ExclusiveOrAssignmentOperator
    | DivisionAssignmentOperator
    | ConcatenationAssignmentOperator
    | CoalesceAssignmentOperator
    | AssignmentOperator
    | AndAssignmentOperator
    | AdditionAssignmentOperator
    | PrintOperator
    | RequireOperator
    | RequireOnceOperator
    | IncludeOperator
    | IncludeOnceOperator
    | AwaitOperator
    | ReadonlyOperator
    | EnumClassLabelOperator
    | PackageOperator
    | NameofOperator
end
