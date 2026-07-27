<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

// Subtyping: extended is a subtype of a shape with matching fields
function takes_base(shape('x' => int, 'y' => string, 'z' => bool) $s): void {}

function test(TExtended $s): void {
  // This should work — TExtended normalizes to shape('x' => int, 'y' => string, 'z' => bool)
  takes_base($s);
}

// Subtyping with open shapes
function takes_open(shape('x' => int, ...) $s): void {}

function test_open(TExtended $s): void {
  // Closed <: open with subset of fields
  takes_open($s);
}
