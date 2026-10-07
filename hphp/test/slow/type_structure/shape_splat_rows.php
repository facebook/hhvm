<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TRowBase = shape('x' => int);

// empty shape is left and right identity so this normalizes to shape('x' => int)
type TEmptyIdentity = shape(...shape(), ...TRowBase, ...shape());

type TUntypedOpenOperand = shape('y' => string, ...);

/* open shape is right merge operand so 'x' maybe be at the type of the unknown
   field; normalizes to shape('x' => mixed, 'y' => string, mixed...)
   we purposely simplify unions with mixed sot he type structure reprsentation
   is canonical for untyped open shapes, i.e. no 'variadic_type' field is needed */
type TUntypedOpen = shape(...TRowBase, ...TUntypedOpenOperand);

type TTypedOpenOperand = shape('y' => string, int...);

// Normalizes to shape('x' => (int|int), y => string, int...)
type TTypedOpen = shape(...TRowBase, ...TTypedOpenOperand);

/* Spreading nothing gives you the bottom row then lifting with shape gives you
   the bottom type, i.e. nothing */
type TBottom = shape(...TRowBase, ...nothing, ...shape('after' => bool));

function structure(string $alias): dict<arraykey, mixed> {
  $ts = HH\type_structure_for_alias($alias);
  $result = dict['kind' => $ts['kind']];
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

function show_row(string $alias): void {
  echo $alias.': '.json_encode(
    structure($alias),
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}

<<__EntryPoint>>
function main(): void {
  $base = HH\type_structure_for_alias(nameof TRowBase);
  $empty = HH\type_structure_for_alias(nameof TEmptyIdentity);
  invariant($empty['fields'] === $base['fields'], 'empty shape was not identity');
  invariant(
    !($empty['allows_unknown_fields'] ?? false),
    'empty identity opened a closed row',
  );

  $open = HH\type_structure_for_alias(nameof TUntypedOpen);
  invariant(
    $open['allows_unknown_fields'] && !array_key_exists('variadic_type', $open),
    'untyped open row was not represented by mixed',
  );
  invariant(
    $open['fields']['x']['kind'] === TypeStructureKind::OF_MIXED,
    'untyped open row did not widen a preceding field',
  );

  $typed = HH\type_structure_for_alias(nameof TTypedOpen);
  invariant(
    $typed['variadic_type']['kind'] === TypeStructureKind::OF_INT,
    'typed open bound was not preserved',
  );
  invariant(
    $typed['fields']['x']['kind'] === TypeStructureKind::OF_UNION,
    'typed open bound did not union with a preceding field',
  );

  $bottom = structure(nameof TBottom);
  invariant(
    $bottom === dict['kind' => TypeStructureKind::OF_NOTHING],
    'nothing did not absorb its prefix and suffix and discard their shape data',
  );

  show_row(nameof TEmptyIdentity);
  show_row(nameof TUntypedOpen);
  show_row(nameof TTypedOpen);
  show_row(nameof TBottom);
}
