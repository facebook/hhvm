(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type operator_node = {
  name: string;
  is_assignment: bool;
  is_comparison: bool;
}

let make_operator ?(is_assignment = false) ?(is_comparison = false) name =
  { name; is_assignment; is_comparison }

let operators =
  [
    make_operator "Dollar";
    make_operator "ScopeResolution";
    make_operator "Indexing";
    make_operator "NullSafeMemberSelection";
    make_operator "MemberSelection";
    make_operator "New";
    make_operator "FunctionCall";
    make_operator "Clone";
    make_operator "PostfixIncrement";
    make_operator "PostfixDecrement";
    make_operator "PrefixIncrement";
    make_operator "PrefixDecrement";
    make_operator "Exponent";
    make_operator "ErrorControl";
    make_operator "Cast";
    make_operator "NullableAs";
    make_operator "Is";
    make_operator "Instanceof";
    make_operator "As";
    make_operator "Upcast";
    make_operator "UnaryPlus";
    make_operator "UnaryMinus";
    make_operator "Not";
    make_operator "LogicalNot";
    make_operator "Remainder";
    make_operator "Multiplication";
    make_operator "Division";
    make_operator "Subtraction";
    make_operator "Concatenation";
    make_operator "Addition";
    make_operator "RightShift";
    make_operator "LeftShift";
    make_operator "Spaceship" ~is_comparison:true;
    make_operator "LessThanOrEqual" ~is_comparison:true;
    make_operator "LessThan" ~is_comparison:true;
    make_operator "GreaterThanOrEqual" ~is_comparison:true;
    make_operator "GreaterThan" ~is_comparison:true;
    make_operator "StrictNotEqual" ~is_comparison:true;
    make_operator "StrictEqual" ~is_comparison:true;
    make_operator "NotEqual" ~is_comparison:true;
    make_operator "Equal" ~is_comparison:true;
    make_operator "And";
    make_operator "ExclusiveOr";
    make_operator "Or";
    make_operator "LogicalAnd";
    make_operator "LogicalOr";
    make_operator "Coalesce";
    make_operator "DegenerateConditional";
    make_operator "ConditionalQuestion";
    make_operator "ConditionalColon";
    make_operator "Pipe";
    make_operator "SubtractionAssignment" ~is_assignment:true;
    make_operator "RightShiftAssignment" ~is_assignment:true;
    make_operator "RemainderAssignment" ~is_assignment:true;
    make_operator "OrAssignment" ~is_assignment:true;
    make_operator "MultiplicationAssignment" ~is_assignment:true;
    make_operator "LeftShiftAssignment" ~is_assignment:true;
    make_operator "ExponentiationAssignment" ~is_assignment:true;
    make_operator "ExclusiveOrAssignment" ~is_assignment:true;
    make_operator "DivisionAssignment" ~is_assignment:true;
    make_operator "ConcatenationAssignment" ~is_assignment:true;
    make_operator "CoalesceAssignment" ~is_assignment:true;
    make_operator "Assignment" ~is_assignment:true;
    make_operator "AndAssignment" ~is_assignment:true;
    make_operator "AdditionAssignment" ~is_assignment:true;
    make_operator "Print";
    make_operator "Require";
    make_operator "RequireOnce";
    make_operator "Include";
    make_operator "IncludeOnce";
    make_operator "Await";
    make_operator "Readonly";
    make_operator "EnumClassLabel";
    make_operator "Package";
    make_operator "Nameof";
  ]
