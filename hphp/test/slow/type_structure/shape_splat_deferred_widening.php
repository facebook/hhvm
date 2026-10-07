<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

/* Normalizes to:
   shape(?'a' => ((?nothing|resource)|bool), ((num|resource)|float)...)
                      29       5        2       6     5       3 */
type TDeferredWidening = shape(
  ...shape(?'a' => ?nothing, num...),
  ...shape(resource...),
  ...shape(?'a' => bool, float...),
);

<<__EntryPoint>>
function main(): void {
  $ts = HH\type_structure_for_alias(nameof TDeferredWidening);
  $field = $ts['fields']['a'];

  invariant(
    $field['kind'] === TypeStructureKind::OF_UNION &&
      $field['optional_shape_field'],
    'the optional field was not preserved',
  );
  invariant(
    $ts['allows_unknown_fields'] &&
      $ts['variadic_type']['kind'] === TypeStructureKind::OF_UNION,
    'the explicit unknown bounds were not preserved',
  );

  echo 'field: '.json_encode($field, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'bound: '.json_encode(
    $ts['variadic_type'],
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
