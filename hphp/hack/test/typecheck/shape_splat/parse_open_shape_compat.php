<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Verify existing open shape syntax still works unchanged
type TOpen = shape('x' => int, ...);
type TClosed = shape('y' => string);

function test_open(TOpen $s): void {
  hh_show($s);
}

function test_closed(TClosed $s): void {
  hh_show($s);
}
