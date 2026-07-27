<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Spreading `dynamic` into a shape contributes an open row whose unknown
// fields have type `dynamic`.

function dyn_alone(shape(...dynamic) $s): void {
  hh_show($s);
}

// `dynamic` on the left: the concrete field on the right wins.
function dyn_left(shape(...dynamic, 'x' => int) $s): void {
  hh_show($s);
  hh_expect<int>($s['x']);
}

// `dynamic` on the right: the concrete field is unioned with `dynamic`.
function dyn_right(shape('x' => int, ...dynamic) $s): void {
  hh_show($s);
}

// `dynamic` merged with a concrete shape splat.
type TBase = shape('x' => int, 'y' => string);

function dyn_with_shape(shape(...TBase, ...dynamic) $s): void {
  hh_show($s);
}
