<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

type TClosed = shape(...shape('a' => int), 'b' => bool);

// The empty shape is the merge identity on both sides.
type TEmptyIdentity = shape(
  ...shape(),
  'value' => int,
  ...shape(),
);

// `nothing` is absorbing regardless of its position in the merge.
type TNothingLeading = shape(...nothing, 'a' => int);
type TNothingMiddle = shape('a' => int, ...nothing, 'b' => bool);
type TNothingTrailing = shape('a' => int, ...nothing);

type TOpenBeforeField = shape(
  ...shape('a' => int, string...),
  'b' => bool,
);

type TMultipleOpen = shape(
  ...shape('a' => int, int...),
  ...shape('b' => bool, string...),
  'c' => float,
);

newtype TResidualOpen<T as shape(...)> = shape(
  ...shape('a' => int, int...),
  ...T,
  ...shape('b' => bool, string...),
);

newtype TOuterOpen<T as shape(...)> = shape(
  'left' => int,
  ...T,
  'right' => bool,
  string...
);

newtype TNested<T as shape(...)> = shape(
  ...shape(...T, 'inner' => int),
  'outer' => bool,
);

// Resolution distributes the surrounding fields into both union branches.
newtype TUnionDistributed<
  TLeft as shape(...),
  TRight as shape(...),
> = shape(
  'before' => int,
  ...(TLeft | TRight),
  'after' => bool,
);

// Assigned text recursively preserves every unresolved nested operand.
newtype TDeeplyNested<T as shape(...)> = shape(
  ...shape(
    'level1' => int,
    ...shape(
      'level2' => string,
      ...shape(...T, 'level3' => bool),
    ),
  ),
  'tail' => float,
);

function show<reify T>(string $alias): void {
  $assigned = (new ReflectionTypeAlias($alias))->getAssignedTypeText();
  echo $alias.":\n";
  echo '  assigned: '.$assigned."\n";
  try {
    null as T;
  } catch (TypeAssertionException $e) {
    $resolved = HH\Lib\Str\strip_prefix($e->getMessage(), 'Expected ');
    $resolved = HH\Lib\Str\strip_suffix($resolved, ', got null');
    echo '  resolved: '.$resolved."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  show<TClosed>(nameof TClosed);
  show<TEmptyIdentity>(nameof TEmptyIdentity);
  show<TNothingLeading>(nameof TNothingLeading);
  show<TNothingMiddle>(nameof TNothingMiddle);
  show<TNothingTrailing>(nameof TNothingTrailing);
  show<TOpenBeforeField>(nameof TOpenBeforeField);
  show<TMultipleOpen>(nameof TMultipleOpen);
  show<TResidualOpen<shape('middle' => float)>>(nameof TResidualOpen);
  show<TOuterOpen<shape('middle' => float)>>(nameof TOuterOpen);
  show<TNested<shape('value' => string)>>(nameof TNested);
  show<
    TUnionDistributed<shape('left' => string), shape('right' => float)>
  >(nameof TUnionDistributed);
  show<TDeeplyNested<shape('center' => num)>>(nameof TDeeplyNested);
}
