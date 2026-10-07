<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

/* `T as shape(...)` is statically valid as a splat operand. Reflecting this
    generic alias by name provides no concrete argument for `T`, so runtime
    resolution preserves it as an unbound `T_typevar` residual (kind 13). */
newtype TInner<T as shape(...)> = shape('inner' => int, ...T);

/* Resolving and flattening the nested `TInner<T>` produces:

   shape(
     ...shape('outer' => bool, 'inner' => int), // kind 14; fields 2 and 1
     ...T,                                      // kind 13
     ...shape('tail' => string),                // kind 14; field 4
   )

   The adjacent concrete shape fragments before `T` merge. The residual `T`
   remains a barrier, so the concrete suffix stays a separate element. */
newtype TOuter<T as shape(...)> =
  shape('outer' => bool, ...TInner<T>, 'tail' => string);

<<__EntryPoint>>
function main(): void {
  $ts = HH\type_structure_for_alias(nameof TOuter);
  $elements = $ts['splat_elem_types'];

  invariant(count($elements) === 3, 'nested residual was not flattened');
  invariant(
    $elements[0]['fields']['outer']['kind'] === TypeStructureKind::OF_BOOL &&
      $elements[0]['fields']['inner']['kind'] === TypeStructureKind::OF_INT,
    'nested prefix fields were not merged',
  );
  invariant(
    $elements[1]['name'] === 'T' && !array_key_exists('fields', $elements[1]),
    'residual type parameter was lost',
  );
  invariant(
    $elements[2]['fields']['tail']['kind'] === TypeStructureKind::OF_STRING,
    'suffix after nested residual was lost',
  );

  echo json_encode($elements, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
