<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'union_intersection_type_hints')>>

function test_union_shapes(
  (shape('x' => int) | shape('x' => string)) $s,
): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<(int | string)>($x);
}

function test_union_mixed_optionality(
  (shape('a' => int, ?'b' => string) | shape('a' => float, 'b' => bool)) $s,
): void {
  shape('a' => $a, ?'b' => $b) = $s;
  hh_expect_equivalent<num>($a);
  hh_expect_equivalent<?(bool | string)>($b);
}

function test_union_tuples(
  ((int, bool) | (string, float)) $t,
): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<(int | string)>($a);
  hh_expect_equivalent<(bool | float)>($b);
}

function test_union_disjoint_open(
  (shape('x' => int, 'y' => bool) | shape('x' => string, 'z' => float)) $s,
): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<(int | string)>($x);
}

function test_union_all_common_closed(
  (shape('a' => int, 'b' => string) | shape('a' => float, 'b' => bool)) $s,
): void {
  shape('a' => $a, 'b' => $b) = $s;
  hh_expect_equivalent<num>($a);
  hh_expect_equivalent<(bool | string)>($b);
}

function test_union_both_optional(
  (shape('a' => int, ?'b' => string) | shape('a' => float, ?'b' => bool)) $s,
): void {
  shape('a' => $a, ?'b' => $b) = $s;
  hh_expect_equivalent<num>($a);
  hh_expect_equivalent<?(bool | string)>($b);
}

function test_union_omit_optional_closed(
  (shape('a' => int, ?'c' => string) | shape('a' => float, ?'c' => bool)) $s,
): void {
  shape('a' => $a) = $s;
  hh_expect_equivalent<num>($a);
}

function test_three_way_union(
  (shape('x' => int) | shape('x' => string) | shape('x' => bool)) $s,
): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<(int | bool | string)>($x);
}

function test_union_with_nothing(
  (shape('x' => int) | nothing) $s,
): void {
  shape('x' => $x) = $s;
  hh_expect_equivalent<int>($x);
}

function test_union_with_dynamic(
  (shape('x' => int) | dynamic) $s,
): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<(int | dynamic)>($x);
}

// Disjoint extra fields, CLOSED pattern.
// The solver unifies the union into shape('a'=>num, ?'b'=>string, ?'c'=>bool),
// making 'b' and 'c' optional. We allow closed patterns to omit optional
// fields, so this succeeds. Whether this is desirable is a judgment call:
// fwiw I could see a case for rejecting the code, forcing the dev to handle
// each branch, but this would require changing how we solve or hacks.
function test_union_disjoint_closed(
  (shape('a' => int, 'b' => string) | shape('a' => float, 'c' => bool)) $s,
): void {
  shape('a' => $a) = $s; // no error: unified type has 'b','c' as optional
  hh_expect_equivalent<num>($a);
}

// Field absent in one closed branch.
// The solver unifies to shape('a'=>num, ?'b'=>string, ...) -- 'b' becomes
// optional in the unified type. Using ?'b' on an optional field is valid.
// Judgment call: per-member, 'b' is absent in member2 (a closed shape with
// no 'b' at all). But the unified type says ?'b' is available.
function test_union_field_absent_closed(
  (shape('a' => int, 'b' => string) | shape('a' => float)) $s,
): void {
  shape('a' => $a, ?'b' => $b, ...) = $s; // no error: unified type has ?'b'
  hh_expect_equivalent<num>($a);
  hh_expect_equivalent<?string>($b);
}

// Field absent in one OPEN branch.
// Same situation: solver unifies so 'b' is optional in the result.
// Judgment call: per-member, 'b' is not a KNOWN field in member2 (open shape).
// But the unified type includes ?'b'.
function test_union_field_absent_open(
  (shape('a' => int, 'b' => string) | shape('a' => float, ...)) $s,
): void {
  shape('a' => $a, ?'b' => $b, ...) = $s; // no error: unified type has ?'b'
}

// Tuples with disjoint arity, open pattern: OK -- only access known positions.
function test_union_tuples_open_prefix(
  ((int, int) | (int, string, string)) $t,
): void {
  tuple($a, ...) = $t;
  hh_expect_equivalent<int>($a);
}

// Another open tuple with different arities:
function test_union_tuples_open_prefix_2(
  ((int, string) | (int, string, bool)) $t,
): void {
  tuple($a, $b, ...) = $t;
  hh_expect_equivalent<int>($a);
  hh_expect_equivalent<string>($b);
}

// Union where both members have the same type for a field
function test_union_same_type(bool $b): void {
  if ($b) {
    $u = shape('x' => 1, 'y' => 'a');
  } else {
    $u = shape('x' => 2, 'y' => 'b');
  }
  shape('x' => $x, 'y' => $y) = $u;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

// Union of tuples with different element types
function test_union_tuples_diff_types(
  ((int, string) | (float, bool)) $t,
): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<(int | float)>($a);
  hh_expect_equivalent<(string | bool)>($b);
}
