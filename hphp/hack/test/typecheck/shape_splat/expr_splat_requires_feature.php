<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// An expression-level splat requires `shape_splat_expression` specifically,
// even when the type-level shape-splat features are enabled. This should error.
function f(): void {
  $a = shape('x' => 1);
  $s = shape(...$a, 'y' => 2);
}
