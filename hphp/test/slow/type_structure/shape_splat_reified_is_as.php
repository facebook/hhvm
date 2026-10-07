<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

type TReifiedBase = shape('x' => int, 'shared' => int);
type TReifiedOverride = shape('shared' => string, 'y' => float);

function show_reified<reify T as shape(...)>(): void {
  $ts = HH\ReifiedGenerics\get_type_structure<shape(...T, 'z' => bool)>();
  echo json_encode($ts['fields'], JSON_FB_FORCE_HACK_ARRAYS)."\n";
}

function check_reified<reify T as shape(...)>(string $label, mixed $value): void {
  echo $label.":\n";
  var_dump($value is shape(...T, 'z' => bool));
  try {
    $value as shape(...T, 'z' => bool);
    echo "as: pass\n";
  } catch (TypeAssertionException $_) {
    echo "as: fail\n";
  }
}

<<__EntryPoint>>
function main(): void {
  // Within show reified, normalizes to
  // shape('x' => int, 'shared' => string, 'y' => float, 'z' => bool)
  show_reified<shape(...TReifiedBase, ...TReifiedOverride)>();

  check_reified<shape(...TReifiedBase, ...TReifiedOverride)>(
    'match',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'shared' => 'yes', 'y' => 1.5, 'z' => true),
    ),
  );
  check_reified<shape(...TReifiedBase, ...TReifiedOverride)>(
    'wrong type',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'shared' => 2, 'y' => 1.5, 'z' => true),
    ),
  );
  check_reified<shape(...TReifiedBase, ...TReifiedOverride)>(
    'missing field',
    __hhvm_intrinsics\launder_value(
      shape('x' => 1, 'shared' => 'yes', 'y' => 1.5),
    ),
  );
  check_reified<shape(...TReifiedBase, ...TReifiedOverride)>(
    'extra field',
    __hhvm_intrinsics\launder_value(
      shape(
        'x' => 1,
        'shared' => 'yes',
        'y' => 1.5,
        'z' => true,
        'extra' => null,
      ),
    ),
  );
}
