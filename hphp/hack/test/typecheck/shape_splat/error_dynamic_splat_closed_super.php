<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

// BAD: expected to fail subtyping. A `...dynamic` operand gives the parameter's
// bound an open `_ => dynamic` unknown tail, which is not a subtype of a CLOSED
// super (unknown tail `nothing`).

function sink_closed(shape('x' => int) $_): void {}

function dyn_closed<T as shape(...dynamic, 'x' => int)>(shape(...T) $s): void {
  sink_closed($s);
}
