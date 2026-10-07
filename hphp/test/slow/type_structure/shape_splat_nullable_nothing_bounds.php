<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Normalizes to shape((?nothing|string)...)
type TNullableThenString = shape(...shape(?nothing...), ...shape(string...));

// Normalizes to shape((string|?nothing)...)
type TStringThenNullable = shape(...shape(string...), ...shape(?nothing...));

function bound(string $alias): dict<arraykey, mixed> {
  return HH\type_structure_for_alias($alias)['variadic_type'];
}

<<__EntryPoint>>
function main(): void {
  $forward = bound(nameof TNullableThenString);
  $reverse = bound(nameof TStringThenNullable);
  invariant(
    $forward['kind'] === TypeStructureKind::OF_UNION &&
      $forward['union_types'][0]['kind'] === TypeStructureKind::OF_NOTHING &&
      $forward['union_types'][0]['nullable'] &&
      $forward['union_types'][1]['kind'] === TypeStructureKind::OF_STRING,
    'nullable nothing was erased from the left bound',
  );
  invariant(
    $reverse['kind'] === TypeStructureKind::OF_UNION &&
      $reverse['union_types'][0]['kind'] === TypeStructureKind::OF_STRING &&
      $reverse['union_types'][1]['kind'] === TypeStructureKind::OF_NOTHING &&
      $reverse['union_types'][1]['nullable'],
    'nullable nothing was erased from the right bound',
  );

  echo 'forward: '.json_encode($forward, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'reverse: '.json_encode($reverse, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
