<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function check_nullable_nothing(string $label, mixed $value): void {
  echo $label.":\n";
  // test type normalizes to shape('x' => (?nothing|int)) = shape('x' => ?int)
  // since 'x' is required in the left hand merge operand
  var_dump(
    $value is shape(
      ...shape('x' => ?nothing),
      ...shape(?'x' => int),
    ),
  );

  try {
    $value as shape(
      ...shape('x' => ?nothing),
      ...shape(?'x' => int),
    );
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  check_nullable_nothing(
    'null',
    __hhvm_intrinsics\launder_value(shape('x' => null)),
  );

  check_nullable_nothing(
    'int',
    __hhvm_intrinsics\launder_value(shape('x' => 1)),
  );

  check_nullable_nothing(
    'bool',
    __hhvm_intrinsics\launder_value(shape('x' => true)),
  );

  // Should fail - 'x' must be present in the normalized shape
  check_nullable_nothing(
    'missing',
    __hhvm_intrinsics\launder_value(shape()),
  );
}
