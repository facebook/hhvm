<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Inline shape splat in function param combined with open shape
type TBase = shape('x' => int);

function accepts_open(shape(...TBase, 'y' => string, ...) $s): void {
  // The trailing `...` groups with 'y' into an open shape spread
  // (`...shape('y' => string, ...)`), which widens the base field 'x' to mixed.
  hh_expect_equivalent<mixed>($s['x']);
  hh_expect_equivalent<string>($s['y']);
}

function test_inline_open(): void {
  $s = shape('x' => 1, 'y' => 'hello', 'extra' => true);
  accepts_open($s);
}
