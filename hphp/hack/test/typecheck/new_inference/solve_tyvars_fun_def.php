<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(int $x, string $y): vec<arraykey> {
  $v = vec[$x, $y];
  return $v;
  // This function doesn't contain any calls that would trigger
  // solving of constraints, so we need to solve any outstanding
  // constraints (e.g., that the type of $v is a subtype of the
  // declared return type vec<arraykey>) after type-checking the
  // function definition.
}
