<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

// Empty shape is identity for merge so this normalizes to shape(...T)
newtype TTrailingEmpty<T as shape(...)> = shape(...T, ...shape());

/* Normalizes to
   shape(
     ...shape('a' => int),              - kind 14, fields ('a', kind 1)
     ...T,                              - kind 13
     ...shape('b' => bool, 'c' => float) - kind 14, fields ('b', kind 2), ('c', kind 3)
     ...U,                              - kind 13
     ...shape('d' => string),           - kind 14, fields ('d', kind 4)
   ) */
newtype TAligned<T as shape(...), U as shape(...)> = shape(
  'a' => int,
  ...T,
  'b' => bool,
  ...shape('c' => float),
  ...U,
  'd' => string,
);

<<__EntryPoint>>
function main(): void {
  $trailing =
    HH\type_structure_for_alias(nameof TTrailingEmpty)['splat_elem_types'];
  invariant(
    count($trailing) === 1 && $trailing[0]['name'] === 'T',
    'empty shape after a residual was not removed',
  );

  $aligned = HH\type_structure_for_alias(nameof TAligned)['splat_elem_types'];
  invariant(count($aligned) === 5, 'residual fragments were not aligned');
  invariant(
    $aligned[0]['fields']['a']['kind'] === TypeStructureKind::OF_INT &&
      $aligned[1]['name'] === 'T' &&
      $aligned[2]['fields']['b']['kind'] === TypeStructureKind::OF_BOOL &&
      $aligned[2]['fields']['c']['kind'] === TypeStructureKind::OF_FLOAT &&
      $aligned[3]['name'] === 'U' &&
      $aligned[4]['fields']['d']['kind'] === TypeStructureKind::OF_STRING,
    'concrete fragments were lost around residual operands',
  );

  echo 'trailing: '.json_encode($trailing, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  echo 'aligned: '.json_encode($aligned, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
