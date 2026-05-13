<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

function test_wildcard_not_usable(shape('x' => int, 'y' => int) $s): void {
  shape('x' => $x, 'y' => _) = $s;
  hh_expect_equivalent<int>($x);
}

function test_wildcard_under_optional(
  shape('x' => int, ?'y' => string) $s,
): void {
  shape('x' => $x, ?'y' => _, ...) = $s;
  hh_expect_equivalent<int>($x);
}

function test_all_wildcards(shape('x' => int, 'y' => string, 'z' => bool) $s): void {
  shape('x' => _, 'y' => _, 'z' => _) = $s;
}

function test_wildcard_multiple(
  shape('a' => int, 'b' => string, 'c' => bool) $s,
): void {
  shape('a' => _, 'b' => _, 'c' => $c) = $s;
  hh_expect_equivalent<bool>($c);
}
