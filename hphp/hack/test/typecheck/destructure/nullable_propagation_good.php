<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Optional ancestor, required child: $x gets ?int
function test_opt_ancestor_req_child(
  shape('a' => shape('b' => int, ...),
        ?'c' => shape('d' => int, ...),
  ) $s,
): void {
  shape(
    'a' => shape('b' => $b, ...),
    ?'c' => shape('d' => $d, ...),
  ) = $s;
  hh_expect_equivalent<int>($b);
  hh_expect_equivalent<?int>($d);
}

// Required ancestor, optional child: $x gets ?int
function test_req_ancestor_opt_child(
  shape('a' => shape('x' => int, ?'y' => int, ...)) $s,
): void {
  shape('a' => shape('x' => $x, ?'y' => $y, ...)) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?int>($y);
}

// Double optional: outer optional, inner optional: $x gets ?int (not ??int)
function test_double_optional(
  shape(?'a' => shape(?'b' => int, ...)) $s,
): void {
  shape(?'a' => shape(?'b' => $x, ...)) = $s;
  hh_expect_equivalent<?int>($x);
}

// 3-level: mixed optional
function test_three_level(
  shape(?'l1' => shape('l2' => shape(?'l3' => int))) $s,
): void {
  shape(?'l1' => shape('l2' => shape(?'l3' => $val))) = $s;
  hh_expect_equivalent<?int>($val);
}

// Optional shape with tuple child: nullable into tuple
function test_opt_shape_tuple_child(
  shape(?'pair' => (int, string)) $s,
): void {
  shape(?'pair' => tuple($a, $b)) = $s;
  hh_expect_equivalent<?int>($a);
  hh_expect_equivalent<?string>($b);
}

// Tuple with shape child with optional field
function test_tuple_shape_opt(
  (shape('x' => int, ?'y' => string), bool) $t,
): void {
  tuple(shape('x' => $x, ?'y' => $y), $z) = $t;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<?string>($y);
  hh_expect_equivalent<bool>($z);
}
