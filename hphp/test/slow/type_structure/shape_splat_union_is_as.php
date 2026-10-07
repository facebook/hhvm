<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function check_union(string $label, mixed $value): void {
  echo $label.":\n";
  // Test type normalizes to shape('x' => (int|string))
  var_dump(
    $value is shape(
      ...shape('x' => int),
      ...shape(?'x' => string),
    ),
  );
  try {
    $value as shape(
      ...shape('x' => int),
      ...shape(?'x' => string),
    );
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  check_union('int', __hhvm_intrinsics\launder_value(shape('x' => 1)));
  check_union('string', __hhvm_intrinsics\launder_value(shape('x' => 'one')));
  // Fail bool </: (int|string)
  check_union('bool', __hhvm_intrinsics\launder_value(shape('x' => true)));
  // Fail 'x' is required
  check_union('missing', __hhvm_intrinsics\launder_value(shape()));
}
