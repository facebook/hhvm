<?hh
<<file:__EnableUnstableFeatures('shape_splat_expression')>>

// Enabling expression splats must not also enable type-level splats.
type T = shape(...shape('x' => int));
