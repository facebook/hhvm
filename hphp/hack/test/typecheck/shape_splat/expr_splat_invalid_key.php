<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Class-constant keys are validated both with and without a splat.

class C {
  const string K = 'k';
}

function bad_const_key_with_splat(shape('a' => int) $base): void {
  $s = shape(...$base, C::NO_SUCH => 1);
}

function bad_const_key_no_splat(): void {
  $s = shape(C::NO_SUCH => 1);
}
