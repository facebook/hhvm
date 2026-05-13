<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'shape_field_punning')>>

function test_basic(int $x, string $y): void {
  $s = shape($x, $y);
  hh_expect_equivalent<shape('x' => int, 'y' => string)>($s);
}

function test_mixed_punned_and_explicit(int $x, string $y): void {
  $s = shape($x, 'y' => $y);
  hh_expect_equivalent<shape('x' => int, 'y' => string)>($s);
}

function test_nested(int $y, string $z): void {
  $s = shape('x' => shape($y, $z));
  hh_expect_equivalent<shape('x' => shape('y' => int, 'z' => string))>($s);
}

// Punning on both sides
function test_both_sides(): void {
  $x = 1;
  $y = 'hello';
  shape($x, $y) = shape($x, $y);
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}
