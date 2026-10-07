<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Normalizes to shape('x' => string) ( kind 4 )
type TOptionalRequired = shape(
  ...shape(?'x' => int),
  ...shape('x' => string),
);

// Normalizes to shape(?'x' => ((int|bool)|float)) ( kind 31:(31:(1|2)|3) )
type TThreeOptional = shape(
  ...shape(?'x' => int),
  ...shape(?'x' => bool),
  ...shape(?'x' => float),
);

function field(string $alias): dict<arraykey, mixed> {
  return HH\type_structure_for_alias($alias)['fields']['x'];
}

<<__EntryPoint>>
function main(): void {
  $rightmost = field(nameof TOptionalRequired);
  invariant(
    $rightmost['kind'] === TypeStructureKind::OF_STRING &&
      !($rightmost['optional_shape_field'] ?? false),
    'required right field did not replace the optional left field',
  );

  $three = field(nameof TThreeOptional);
  $members = $three['union_types'];
  invariant(
    $three['kind'] === TypeStructureKind::OF_UNION &&
      $three['optional_shape_field'] &&
      count($members) === 2 &&
      $members[0]['kind'] === TypeStructureKind::OF_UNION &&
      count($members[0]['union_types']) === 2 &&
      $members[1]['kind'] === TypeStructureKind::OF_FLOAT,
    'merging an existing field union lost a member',
  );

  echo 'rightmost: '.json_encode($rightmost, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'three: '.json_encode($three, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
