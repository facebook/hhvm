<?hh

// Expression-level splat without feature flag should error
function test_ungated(): void {
  $a = shape('x' => 1);
  $s = shape(...$a, 'y' => 2);
}
