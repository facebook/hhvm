<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TAliasBase = shape('a' => int, 'shared' => int, string...);

/* Normalizes to
  shape('a' => int, 'shared' => int, 'b' => bool, string... )
                1                1           2      4 */
type TAliasLevel1 = shape(...TAliasBase, 'b' => bool);

/* Normalizes to
  shape('a' => int, 'shared' => int, 'b' => bool, 'c' => float, string... )
                1                1           2            3      4 */
type TAliasLevel2 = shape(...TAliasLevel1, 'c' => float);

/* Normalizes to
  shape('a' => int, 'shared' => arraykey, 'b' => bool, 'c' => float, 'd' => num, string... )
                1                  7              2            3            6     4 */
type TAliasNested = shape(...TAliasLevel2, 'shared' => arraykey, 'd' => num);

// Should be equal to normalized TAliasNested
type TAliasFlat = shape(
  'a' => int,
  'shared' => arraykey,
  'b' => bool,
  'c' => float,
  'd' => num,
  string...
);

/* Normalizes to
  shape('base' => int, 'shared' => arraykey, 'one' => bool, 'two' => float, 'three' => num)
                   1                  7                2              3                6 */
type TLiteralNested = shape(
  ...shape(
    ...shape(
      ...shape('base' => int, 'shared' => int),
      'one' => bool,
      'shared' => string,
    ),
    'two' => float,
  ),
  'three' => num,
  'shared' => arraykey,
);

// Should be equal to normalized TLiteralNested
type TLiteralFlat = shape(
  'base' => int,
  'shared' => arraykey,
  'one' => bool,
  'two' => float,
  'three' => num,
);

<<__EntryPoint>>
function main(): void {
  $alias_nested = HH\type_structure_for_alias(nameof TAliasNested);
  $alias_flat = HH\type_structure_for_alias(nameof TAliasFlat);
  invariant(
    $alias_nested['fields'] === $alias_flat['fields'],
    'nested aliases did not match their flattened form',
  );
  invariant(
    $alias_nested['allows_unknown_fields'] &&
      $alias_nested['variadic_type']['kind'] === TypeStructureKind::OF_STRING,
    'nested aliases did not preserve their typed-open bound',
  );

  $literal_nested = HH\type_structure_for_alias(nameof TLiteralNested);
  $literal_flat = HH\type_structure_for_alias(nameof TLiteralFlat);
  invariant(
    $literal_nested['fields'] === $literal_flat['fields'],
    'nested splat literals did not match their flattened form',
  );
  invariant(
    !($literal_nested['allows_unknown_fields'] ?? false),
    'nested closed literals became open',
  );

  echo 'alias: '.json_encode(
    $alias_nested['fields'],
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
  echo 'literal: '.json_encode(
    $literal_nested['fields'],
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
