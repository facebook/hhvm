<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function check_open(string $label, mixed $value): void {
  echo $label.":\n";
  // test shape normalizes to shape('x' => int, 'y' => string, mixed...)
  var_dump(
    $value is shape(
      ...shape('x' => int, ...),
      'y' => string,
    ),
  );
  try {
    $value as shape(
      ...shape('x' => int, ...),
      'y' => string,
    );
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  check_open(
    'exact',
    __hhvm_intrinsics\launder_value(shape('x' => 1, 'y' => 'one')),
  );
  check_open(
    'extra',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'y' => 'one', 'extra' => null),
    ),
  );
  check_open(
    'wrong type',
    __hhvm_intrinsics\launder_value(shape('x' => 'one', 'y' => 'one')),
  );
  check_open(
    'wrong explicit field',
    __hhvm_intrinsics\launder_value(shape('x' => 1, 'y' => 1)),
  );
  check_open(
    'missing field',
    __hhvm_intrinsics\launder_value(shape('x' => 1)),
  );
}
