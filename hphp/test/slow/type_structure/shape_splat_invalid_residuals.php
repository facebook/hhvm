<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

/* mixed might not be a shape, so it cannot be merged and must remain in
   splat_elem_types as a residual element with kind 9 */
type TMixedResidual = shape(...mixed);

/* nullable dynamic may be null, so it cannot be merged and must remain in
   splat_elem_types as a residual element with kind 30 with `nullable` true */
type TNullableDynamicResidual = shape(...?dynamic);

function elements(string $alias): vec<dict<arraykey, mixed>> {
  return HH\type_structure_for_alias($alias)['splat_elem_types'];
}

<<__EntryPoint>>
function main(): void {
  $mixed = elements(nameof TMixedResidual);
  invariant(
    count($mixed) === 1 &&
      $mixed[0]['kind'] === TypeStructureKind::OF_MIXED,
    'mixed was not retained as an invalid residual',
  );

  $nullable_dynamic = elements(nameof TNullableDynamicResidual);
  invariant(
    count($nullable_dynamic) === 1 &&
      $nullable_dynamic[0]['kind'] === TypeStructureKind::OF_DYNAMIC &&
      $nullable_dynamic[0]['nullable'],
    'nullable dynamic was not retained as an invalid residual',
  );

  echo 'mixed: '.json_encode($mixed, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'nullable dynamic: '.json_encode(
    $nullable_dynamic,
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
