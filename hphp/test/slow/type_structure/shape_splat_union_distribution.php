<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

/*  Union distribute and we don't simplify general unions in the type structure
    representation so the following normalizes to
    (shape('x' => string) | shape('x' => string))
*/
type TUnionSuffix = shape(
  ...(shape('x' => int) | shape('x' => bool)),
  'x' => string,
);

/* We DO simplify unions with nothing in type structure so the following
   normalizes to
   shape('a' => int, 'b' => int)
*/
type TUnionNothing = shape(
  'a' => int,
  ...(shape('b' => int) | nothing),
);

// Normalizes to (shape('x' => int) | shape('x' => bool))
type TSameFieldUnion = shape(...(shape('x' => int) | shape('x' => bool)));

// Normalizes to (shape('a' => bool) | (shape('b' => int) | shape('c' => string)))
type TNestedUnion = shape(
  ...(shape('a' => bool) | (shape('b' => int) | shape('c' => string))),
);

<<__EntryPoint>>
function main(): void {
  $suffix = HH\type_structure_for_alias(nameof TUnionSuffix);
  invariant(
    $suffix['kind'] === TypeStructureKind::OF_UNION &&
      count($suffix['union_types']) === 2 &&
      $suffix['union_types'][0]['fields']['x']['kind'] ===
        TypeStructureKind::OF_STRING &&
      $suffix['union_types'][1]['fields']['x']['kind'] ===
        TypeStructureKind::OF_STRING,
    'suffix was not merged into every branch',
  );

  $bottom = HH\type_structure_for_alias(nameof TUnionNothing);
  invariant(
    $bottom['kind'] === TypeStructureKind::OF_SHAPE &&
      count($bottom['fields']) === 2,
    'bottom branch was not removed',
  );

  $same_field = HH\type_structure_for_alias(nameof TSameFieldUnion);
  invariant(
    $same_field['kind'] === TypeStructureKind::OF_UNION &&
      count($same_field['union_types']) === 2,
    'same-field union branches were not preserved',
  );

  $nested = HH\type_structure_for_alias(nameof TNestedUnion);
  invariant(
    $nested['kind'] === TypeStructureKind::OF_UNION &&
      count($nested['union_types']) === 3,
    'nested union was not flattened',
  );

  foreach (dict[
    'suffix' => $suffix,
    'bottom' => $bottom,
    'same field' => $same_field,
    'nested' => $nested,
  ] as $label => $ts) {
    echo $label.': '.json_encode($ts, JSON_FB_FORCE_HACK_ARRAYS)."\n";
  }
}
