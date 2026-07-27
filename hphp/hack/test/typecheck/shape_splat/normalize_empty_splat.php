<?hh
<<file: __EnableUnstableFeatures('shape_splat_concrete')>>

// Splatting an empty shape is an identity
type Nope = shape('_placeholder' => nothing);
type Base = shape('x' => int);

type LeftEmpty = shape(...Nope, ...Base);

function test_empty_identity(LeftEmpty $l, Base $b): void {
  // LeftEmpty normalizes to shape('x' => int, ?'_placeholder' => nothing)
  // which is a supertype of Base
  hh_expect<shape('_placeholder' => nothing, 'x' => int)>($l);
}
