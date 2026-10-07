<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

function check_operand_union(string $label, mixed $value): void {
  echo $label.":\n";
  // test type normalizes to
  // ( shape('prefix' => bool, 'b' => int)
  // | shape('prefix' => bool, 'c' => string)
  // )
  var_dump(
    $value is shape(
      'prefix' => bool,
      ...(shape('b' => int) | shape('c' => string)),
    ),
  );
  try {
    $value as shape(
      'prefix' => bool,
      ...(shape('b' => int) | shape('c' => string)),
    );
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  check_operand_union(
    'first branch',
    __hhvm_intrinsics\launder_value(shape('prefix' => true, 'b' => 1)),
  );

  check_operand_union(
    'second branch',
    __hhvm_intrinsics\launder_value(shape('prefix' => true, 'c' => 'one')),
  );

  // neither 'b' or 'c' is present so this doesn't correspond to either element
  // of the expanded union
  check_operand_union(
    'neither branch',
    __hhvm_intrinsics\launder_value(shape('prefix' => true)),
  );

  // both 'b' and 'c' are present so this also doesn't correspond to either
  // element of the expanded union
  check_operand_union(
    'both branches',
    __hhvm_intrinsics\launder_value(
      shape('prefix' => true, 'b' => 1, 'c' => 'one'),
    ),
  );
}
