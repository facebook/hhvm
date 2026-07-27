<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Chain where each level overrides a field from the prior level
type V1 = shape('x' => int, 'y' => string);
type V2 = shape(...V1, 'x' => string);  // x changes int -> string
type V3 = shape(...V2, 'y' => bool);    // y changes string -> bool

function test_override_chain(V3 $s): void {
  // After chain: x => string (from V2), y => bool (from V3)
  hh_expect<string>($s['x']);
  hh_expect<bool>($s['y']);
}
