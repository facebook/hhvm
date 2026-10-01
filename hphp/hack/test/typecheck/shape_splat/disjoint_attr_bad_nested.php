<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('nested' => int);
type B = shape(...A, 'other' => bool);

<<__DisjointShapeSplat>>
type Bad = shape(...B, 'nested' => string);

// REGRESSION (previously a false negative): an outer splat must not hide an
// overlap entirely within its nested shape.
<<__DisjointShapeSplat>>
type BadInternal =
  shape(
    ...shape(...shape('internal' => int), 'internal' => string),
    'outer' => bool,
  );

// SOUND ACCEPTANCE CONTROL: every level of the nested shape is disjoint.
<<__DisjointShapeSplat>>
type GoodInternal =
  shape(
    ...shape(...shape('inner' => int), 'middle' => string),
    'outer_good' => bool,
  );
