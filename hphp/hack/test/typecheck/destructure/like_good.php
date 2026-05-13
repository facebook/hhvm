<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'like_type_hints')>>

function test_like_shape(~shape('x' => int) $s): void {
  shape('x' => $x) = $s;
  hh_expect_equivalent<~int>($x);
}

function test_like_tuple(~(int, string) $t): void {
  tuple($a, $b) = $t;
  hh_expect_equivalent<~int>($a);
  hh_expect_equivalent<~string>($b);
}

function test_like_optional(~shape('x' => int, ?'y' => string) $s): void {
  shape('x' => $x, ?'y' => $y, ...) = $s;
  hh_expect_equivalent<~int>($x);
  hh_expect_equivalent<~?string>($y);
}
