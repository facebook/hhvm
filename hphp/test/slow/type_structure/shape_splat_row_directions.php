<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Normalizes to shape((int|string)...)
type TBoundsForward = shape(...shape(int...), ...shape(string...));

// Normalizes to shape((string|int)...)
type TBoundsReverse = shape(...shape(string...), ...shape(int...));

/* Normalizes to shape('x' => string, 'y' => bool, mixed...). An untyped-open
   shape is encoded with `allows_unknown_fields` true and no `variadic_type`. */
type TOpenLeft = shape(
  ...shape('x' => int, ...),
  'x' => string,
  'y' => bool,
);

/* Spreading dynamic gives you an open shape with dynamic as the upper bound
   for all unknown fields so this normalizes to
   shape('x' => (int|dynamic), dynamic...) */
type TDynamicRight = shape(...shape('x' => int), ...dynamic);

/* Rightmost wins so this normalizes to
   shape('x' => int, dynamic...) */
type TDynamicLeft = shape(...dynamic, ...shape('x' => int));

function row(string $alias): dict<arraykey, mixed> {
  $ts = HH\type_structure_for_alias($alias);
  $result = dict[];
  if (array_key_exists('fields', $ts)) {
    $result['fields'] = $ts['fields'];
  }
  if (array_key_exists('allows_unknown_fields', $ts)) {
    $result['allows_unknown_fields'] = $ts['allows_unknown_fields'];
  }
  if (array_key_exists('variadic_type', $ts)) {
    $result['variadic_type'] = $ts['variadic_type'];
  }
  return $result;
}

<<__EntryPoint>>
function main(): void {
  $forward = row(nameof TBoundsForward);
  $reverse = row(nameof TBoundsReverse);
  invariant(
    $forward['variadic_type']['kind'] === TypeStructureKind::OF_UNION &&
      $reverse['variadic_type']['kind'] === TypeStructureKind::OF_UNION,
    'typed-open bounds did not union in both orders',
  );

  $open_left = row(nameof TOpenLeft);
  invariant(
    $open_left['allows_unknown_fields'] &&
      $open_left['fields']['x']['kind'] === TypeStructureKind::OF_STRING &&
      $open_left['fields']['y']['kind'] === TypeStructureKind::OF_BOOL,
    'required fields did not override an open left operand',
  );

  $dynamic_right = row(nameof TDynamicRight);
  invariant(
    $dynamic_right['variadic_type']['kind'] === TypeStructureKind::OF_DYNAMIC &&
      $dynamic_right['fields']['x']['kind'] === TypeStructureKind::OF_UNION,
    'right dynamic did not widen the preceding field',
  );

  $dynamic_left = row(nameof TDynamicLeft);
  invariant(
    $dynamic_left['variadic_type']['kind'] === TypeStructureKind::OF_DYNAMIC &&
      $dynamic_left['fields']['x']['kind'] === TypeStructureKind::OF_INT,
    'required field did not override left dynamic',
  );

  foreach (vec[
    nameof TBoundsForward,
    nameof TBoundsReverse,
    nameof TOpenLeft,
    nameof TDynamicRight,
    nameof TDynamicLeft,
  ] as $alias) {
    echo $alias.': '.json_encode(row($alias), JSON_FB_FORCE_HACK_ARRAYS)."\n";
  }
}
