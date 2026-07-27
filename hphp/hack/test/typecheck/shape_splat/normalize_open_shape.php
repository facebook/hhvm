<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TClosed = shape('x' => int);
type TOpen = shape('y' => string, ...);

// Splatting an open shape produces an open shape
type TMerged = shape(...TClosed, ...TOpen);

function test(TMerged $s): void {
  hh_expect<shape('x' => mixed, 'y' => string, ...)>($s);
}

// Closed + closed = closed
type TBothClosed = shape(...TClosed, ...shape('z' => bool));
function test_closed(TBothClosed $s): void {
  hh_expect<shape('x' => int, 'z' => bool)>($s);
}
