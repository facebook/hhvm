<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

/* Spreading nothing gives you the bottom row; bottom + any = bottom, then
   lifting back into a type with shape gives you nothing so both aliases normalize
   to nothing */
type TUnionThenBottom = shape(
  ...(shape('a' => int) | shape('b' => string)),
  ...nothing,
);
type TBottomThenUnion = shape(
  ...nothing,
  ...(shape('a' => int) | shape('b' => string)),
);

<<__EntryPoint>>
function main(): void {
  $union_then_bottom = HH\type_structure_for_alias(nameof TUnionThenBottom);
  $bottom_then_union = HH\type_structure_for_alias(nameof TBottomThenUnion);

  invariant(
    $union_then_bottom['kind'] === TypeStructureKind::OF_NOTHING,
    'bottom did not absorb a preceding union',
  );
  invariant(
    $bottom_then_union['kind'] === TypeStructureKind::OF_NOTHING,
    'bottom did not absorb a following union',
  );

  echo json_encode(
    dict[
      'union_then_bottom' => $union_then_bottom['kind'],
      'bottom_then_union' => $bottom_then_union['kind'],
    ],
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
