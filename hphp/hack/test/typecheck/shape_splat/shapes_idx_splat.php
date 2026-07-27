<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Shapes::idx on a splat-composed shape
type Base = shape('x' => int, 'y' => string);
type Extra = shape(?'z' => bool);
type Combined = shape(...Base, ...Extra);

function test_shapes_idx(Combined $s): void {
  // Shapes::idx always returns nullable
  $x = Shapes::idx($s, 'x');
  hh_expect_equivalent<?int>($x);

  $y = Shapes::idx($s, 'y');
  hh_expect_equivalent<?string>($y);

  // z is optional
  $z = Shapes::idx($s, 'z');
  hh_expect_equivalent<?bool>($z);
}
