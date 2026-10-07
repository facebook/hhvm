<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function check_optional(string $label, mixed $value): void {
  echo $label.":\n";
  // test type normalizes to shape(?'x' => (int|string))
  var_dump(
    $value is shape(
      ...shape(?'x' => int),
      ...shape(?'x' => string),
    ),
  );
  try {
    $value as shape(
      ...shape(?'x' => int),
      ...shape(?'x' => string),
    );
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  check_optional('missing', __hhvm_intrinsics\launder_value(shape()));
  check_optional('int', __hhvm_intrinsics\launder_value(shape('x' => 1)));
  check_optional(
    'string',
    __hhvm_intrinsics\launder_value(shape('x' => 'one')),
  );
  check_optional('bool', __hhvm_intrinsics\launder_value(shape('x' => true)));
}
