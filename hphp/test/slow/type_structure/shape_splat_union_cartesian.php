<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

/* Unions distribute so this normalizes to:
   ( shape('a' => int, 'y' => int)
   | shape('a' => int, 'z' => int)
   | shape('b' => int, 'y' => int)
   | shape('b' => int, 'z' => int)
   )
*/
type TTwoUnions = shape(
  ...(shape('a' => int) | shape('b' => int)),
  ...(shape('y' => int) | shape('z' => int)),
);

<<__EntryPoint>>
function main(): void {
  $ts = HH\type_structure_for_alias(nameof TTwoUnions);
  invariant(
    $ts['kind'] === TypeStructureKind::OF_UNION &&
      count($ts['union_types']) === 4,
    'two unions did not form four branches',
  );

  echo json_encode($ts, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
