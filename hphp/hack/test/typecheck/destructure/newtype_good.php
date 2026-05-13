//// def.php
<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

newtype ShapeAlias as shape('x' => int, ...) = shape('x' => int, 'y' => string);

// Same file: newtype is transparent, 'y' is accessible
function test_same_file(ShapeAlias $s): void {
  shape('x' => $x, 'y' => $y) = $s;
  hh_expect_equivalent<int>($x);
  hh_expect_equivalent<string>($y);
}

//// use.php
<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring')>>

// Different file: the upper bound shape('x' => int, ...) is visible
function test_cross_file(ShapeAlias $s): void {
  shape('x' => $x, ...) = $s;
  hh_expect_equivalent<int>($x);
}
