<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);

// Missing comma after a shape splat element: must be rejected rather than
// silently parsed as `...A` followed by a `'y' => int` field.
type Bad = shape(...A 'y' => int);
