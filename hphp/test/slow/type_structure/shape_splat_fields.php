<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

final class LeftKey {
  const string SHARED = 'shared';
}

final class RightKey {
  const string SHARED = 'shared';
}

type TRequiredLeft = shape('shared' => int, 'left' => bool);
type TRequiredRight = shape('shared' => string, 'right' => float);

/* Normalized to
   shape('shared' => string, 'left' => bool, 'right' => float)
                       4                2                3 */
type TRequiredWins = shape(...TRequiredLeft, ...TRequiredRight);

/* Normalized to
   shape('shared' => (int|string))
                      (31:(1|4)) */
type TRequiredOptional = shape(
  ...shape('shared' => int),
  ...shape(?'shared' => string),
);

/* Normalized to
   shape(?'shared' => (int|string))
                       (31:(1|4)) */
type TOptionalOptional = shape(
  ...shape(?'shared' => int),
  ...shape(?'shared' => string),
);

type TSymbolicInt = shape(LeftKey::SHARED => int);
type TSymbolicString = shape(RightKey::SHARED => string);

/* Normalized to
   shape('shared' => string)
                       4 */
type TSymbolicConflict = shape(...TSymbolicInt, ...TSymbolicString);

function show_fields(string $alias): void {
  $ts = HH\type_structure_for_alias($alias);
  echo $alias.': '.json_encode($ts['fields'], JSON_FB_FORCE_HACK_ARRAYS)."\n";
}

<<__EntryPoint>>
function main(): void {
  $required = HH\type_structure_for_alias(nameof TRequiredWins);
  invariant(count($required['fields']) === 3, 'disjoint fields were lost');
  invariant(
    $required['fields']['shared']['kind'] === TypeStructureKind::OF_STRING,
    'required right field did not win',
  );

  $required_optional =
    HH\type_structure_for_alias(nameof TRequiredOptional)['fields']['shared'];
  invariant(
    $required_optional['kind'] === TypeStructureKind::OF_UNION,
    'required/optional fields did not union',
  );
  invariant(
    !($required_optional['optional_shape_field'] ?? false),
    'required/optional field became optional',
  );

  $optional_optional =
    HH\type_structure_for_alias(nameof TOptionalOptional)['fields']['shared'];
  invariant(
    $optional_optional['kind'] === TypeStructureKind::OF_UNION,
    'optional/optional fields did not union',
  );
  invariant(
    $optional_optional['optional_shape_field'] ?? false,
    'optional/optional field became required',
  );

  $symbolic = HH\type_structure_for_alias(nameof TSymbolicConflict);
  invariant(
    count($symbolic['fields']) === 1 &&
      $symbolic['fields']['shared']['kind'] === TypeStructureKind::OF_STRING,
    'class-constant field names did not collide',
  );

  show_fields(nameof TRequiredWins);
  show_fields(nameof TRequiredOptional);
  show_fields(nameof TOptionalOptional);
  show_fields(nameof TSymbolicConflict);
}
