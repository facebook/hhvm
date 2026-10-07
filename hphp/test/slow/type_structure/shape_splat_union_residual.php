<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

/* Normalizes to
   ( shape(...T, ...shape('a' => int))
   | shape(...T, ...shape('b' => string))
   )
*/
newtype TResidualUnion<T as shape(...)> = shape(
  ...T,
  ...(shape('a' => int) | shape('b' => string)),
);

<<__EntryPoint>>
function main(): void {
  $ts = HH\type_structure_for_alias(nameof TResidualUnion);
  invariant(
    $ts['kind'] === TypeStructureKind::OF_UNION &&
      count($ts['union_types']) === 2 &&
      $ts['union_types'][0]['splat_elem_types'][0]['name'] === 'T' &&
      $ts['union_types'][1]['splat_elem_types'][0]['name'] === 'T',
    'union did not distribute across the whole residual row',
  );

  echo json_encode($ts, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
