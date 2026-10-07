<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

function reified_fields<
  reify T as shape(...),
  reify U as shape(...),
>(): dict<arraykey, mixed> {
  return HH\ReifiedGenerics\get_type_structure<shape(...T, ...U)>()['fields'];
}

function check_reified_multiple<
  reify T as shape(...),
  reify U as shape(...),
>(string $label, mixed $value): void {
  echo $label.":\n";
  var_dump($value is shape(...T, ...U));
  try {
    $value as shape(...T, ...U);
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  // Normalizes to shape('x' => int, 'shared' => string, 'y' => float)
  $fields = reified_fields<
    shape('x' => int, 'shared' => int),
    shape('shared' => string, 'y' => float),
  >();
  invariant(
    $fields['x']['kind'] === TypeStructureKind::OF_INT &&
      $fields['shared']['kind'] === TypeStructureKind::OF_STRING &&
      $fields['y']['kind'] === TypeStructureKind::OF_FLOAT,
    'two reified operands did not merge in order',
  );
  echo 'fields: '.json_encode($fields, JSON_FB_FORCE_HACK_ARRAYS)."\n";

  check_reified_multiple<
    shape('x' => int, 'shared' => int),
    shape('shared' => string, 'y' => float),
  >(
    'match',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'shared' => 'one', 'y' => 1.5),
    ),
  );

  // As above, its rightmost wins to 'shared' has type string
  check_reified_multiple<
    shape('x' => int, 'shared' => int),
    shape('shared' => string, 'y' => float),
  >(
    'left conflict',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'shared' => 1, 'y' => 1.5),
    ),
  );
}
