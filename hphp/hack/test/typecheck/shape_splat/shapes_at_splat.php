<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Shapes::at on a splat-composed shape
type A = shape('x' => int);
type B = shape('y' => string);
type AB = shape(...A, ...B);

function test_shapes_at(AB $s): void {
  $x = Shapes::at($s, 'x');
  hh_expect<int>($x);

  $y = Shapes::at($s, 'y');
  hh_expect<string>($y);
}
