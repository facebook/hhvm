<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Normalizes to shape('x' => (?nothing|int)) = shape('x' => ?int)
type TNullableNothingField = shape(
  ...shape('x' => ?nothing),
  ...shape(?'x' => int),
);

// This is a shape in which any fields which are present must have type null
type TNullableNothingRow = shape(?nothing...);

<<__EntryPoint>>
function main(): void {
  $field = HH\type_structure_for_alias(nameof TNullableNothingField)['fields']['x'];
  $members = $field['union_types'];
  invariant(
    $field['kind'] === TypeStructureKind::OF_UNION &&
      !($field['optional_shape_field'] ?? false) &&
      count($members) === 2 &&
      $members[0]['kind'] === TypeStructureKind::OF_NOTHING &&
      $members[0]['nullable'] &&
      $members[1]['kind'] === TypeStructureKind::OF_INT,
    'nullable nothing was treated as bottom in a field union',
  );

  $row = HH\type_structure_for_alias(nameof TNullableNothingRow);
  invariant(
    $row['allows_unknown_fields'] &&
      $row['variadic_type']['kind'] === TypeStructureKind::OF_NOTHING &&
      $row['variadic_type']['nullable'],
    'nullable nothing closed the row',
  );

  echo 'field: '.json_encode($field, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'row: '.json_encode(
    dict[
      'allows_unknown_fields' => $row['allows_unknown_fields'],
      'variadic_type' => $row['variadic_type'],
    ],
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
