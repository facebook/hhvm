(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type asssociativity =
  | Left
  | Right
  | None

type operator_node = {
  name: string;
  associativity: asssociativity;
  precedence: int;
  is_assignment: bool;
  is_comparison: bool;
}

let make_operator
    name
    associativity
    precedence
    ?(is_assignment = false)
    ?(is_comparison = false)
    () =
  { name; associativity; precedence; is_assignment; is_comparison }

let operators =
  [
    make_operator "Dollar" Right 32 ();
    make_operator "ScopeResolution" Left 31 ();
    make_operator "Indexing" Left 30 ();
    make_operator "NullSafeMemberSelection" Left 29 ();
    make_operator "MemberSelection" Left 29 ();
    make_operator "New" None 28 ();
    make_operator "FunctionCall" Left 27 ();
    make_operator "Reference" Right 26 ();
    make_operator "Clone" None 24 ();
    make_operator "PostfixIncrement" Right 23 ();
    make_operator "PostfixDecrement" Right 23 ();
    make_operator "PrefixIncrement" Right 22 ();
    make_operator "PrefixDecrement" Right 22 ();
    make_operator "Exponent" Right 22 ();
    make_operator "ErrorControl" Right 22 ();
    make_operator "Cast" Right 22 ();
    make_operator "NullableAs" Left 21 ();
    make_operator "Is" Left 21 ();
    make_operator "Instanceof" None 21 ();
    make_operator "As" Left 21 ();
    make_operator "UnaryPlus" Right 20 ();
    make_operator "UnaryMinus" Right 20 ();
    make_operator "Not" Right 20 ();
    make_operator "LogicalNot" Right 20 ();
    make_operator "Suspend" Right 19 ();
    make_operator "Remainder" Left 19 ();
    make_operator "Multiplication" Left 19 ();
    make_operator "Division" Left 19 ();
    make_operator "Subtraction" Left 18 ();
    make_operator "Concatenation" Left 18 ();
    make_operator "Addition" Left 18 ();
    make_operator "RightShift" Left 17 ();
    make_operator "LeftShift" Left 17 ();
    make_operator "Spaceship" None 16 ~is_comparison:true ();
    make_operator "LessThanOrEqual" None 16 ~is_comparison:true ();
    make_operator "LessThan" None 16 ~is_comparison:true ();
    make_operator "GreaterThanOrEqual" None 16 ~is_comparison:true ();
    make_operator "GreaterThan" None 16 ~is_comparison:true ();
    make_operator "StrictNotEqual" None 15 ~is_comparison:true ();
    make_operator "StrictEqual" None 15 ~is_comparison:true ();
    make_operator "PhpNotEqual" None 15 ~is_comparison:true ();
    make_operator "NotEqual" None 15 ~is_comparison:true ();
    make_operator "Equal" None 15 ~is_comparison:true ();
    make_operator "And" Left 14 ();
    make_operator "ExclusiveOr" Left 13 ();
    make_operator "Or" Left 12 ();
    make_operator "LogicalAnd" Left 11 ();
    make_operator "LogicalOr" Left 10 ();
    make_operator "Coalesce" Right 9 ();
    make_operator "DegenerateConditional" Left 8 ();
    make_operator "ConditionalQuestion" Left 8 ();
    make_operator "ConditionalColon" Left 8 ();
    make_operator "Pipe" Left 7 ();
    make_operator "SubtractionAssignment" Right 6 ~is_assignment:true ();
    make_operator "RightShiftAssignment" Right 6 ~is_assignment:true ();
    make_operator "RemainderAssignment" Right 6 ~is_assignment:true ();
    make_operator "OrAssignment" Right 6 ~is_assignment:true ();
    make_operator "MultiplicationAssignment" Right 6 ~is_assignment:true ();
    make_operator "LeftShiftAssignment" Right 6 ~is_assignment:true ();
    make_operator "ExponentiationAssignment" Right 6 ~is_assignment:true ();
    make_operator "ExclusiveOrAssignment" Right 6 ~is_assignment:true ();
    make_operator "DivisionAssignment" Right 6 ~is_assignment:true ();
    make_operator "ConcatenationAssignment" Right 6 ~is_assignment:true ();
    make_operator "CoalesceAssignment" Right 6 ~is_assignment:true ();
    make_operator "Assignment" Right 6 ~is_assignment:true ();
    make_operator "AndAssignment" Right 6 ~is_assignment:true ();
    make_operator "AdditionAssignment" Right 6 ~is_assignment:true ();
    make_operator "Print" Right 5 ();
    make_operator "Require" Left 1 ();
    make_operator "RequireOnce" Left 1 ();
    make_operator "Include" Left 1 ();
    make_operator "IncludeOnce" Left 1 ();
    make_operator "Await" None 1 ();
  ]
