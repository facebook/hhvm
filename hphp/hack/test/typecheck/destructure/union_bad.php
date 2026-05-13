<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'union_intersection_type_hints')>>

function test_union_one_open_err(
  (shape('x' => int) | shape('x' => int, ...)) $s,
): void {
  shape('x' => $x) = $s; // error: member2 is open, missing ...
}

function test_union_req_on_optional_err(
  (shape('a' => int, ?'b' => string) | shape('a' => float, 'b' => bool)) $s,
): void {
  shape('a' => $a, 'b' => $b) = $s; // error: 'b' optional in member1
}

function test_union_tuples_arity_mismatch_err(
  ((int, string) | (int, string, bool)) $t,
): void {
  tuple($a, $b) = $t; // error: arity
}

function test_union_tuples_closed_arity_err(
  ((int,) | (int, string)) $t,
): void {
  tuple($a, $b) = $t; // error: arity
}

function test_union_missing_field(
  (shape('a' => string) | shape('a' => int, 'b' => int)) $s
): void {
  shape('a' => $a, 'b' => $b) = $s; // error: 'b' not in first member
}

function test_union_field_absent_in_member(
  (shape('x' => int, 'y' => string) | shape('x' => float)) $s,
): void {
  shape('x' => $x, 'y' => $y, ...) = $s; // error: 'y' not in second member
}
