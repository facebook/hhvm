<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// A shape literal that splats an inline shape literal which *itself* contains
// a splat. With everything concrete, normalization must fully flatten this to
// a single simple shape -- no Shape_splat survives as an element of another.
function nested_concrete(
  shape('x' => int, ...shape('y' => bool, ...shape('z' => string))) $s,
): void {
  hh_expect<shape('x' => int, 'y' => bool, 'z' => string)>($s);
  hh_expect<int>($s['x']);
  hh_expect<bool>($s['y']);
  hh_expect<string>($s['z']);
}
