<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// SOUND ACCEPTANCE CONTROL: intersection support must not conservatively
// reject an inhabited splat which is provably disjoint from `x`.
<<__DisjointShapeSplat>>
newtype Good<
  TA as shape('a' => int),
  TB as shape('a' => int),
> = shape(...(TA & TB), 'x' => int);

type Witness = Good<shape('a' => int), shape('a' => int)>;

// A closed conjunct rules out `x` even when the other conjunct is open.
<<__DisjointShapeSplat>>
newtype ClosedAndOpen<
  TA as shape('a' => int),
  TB as shape(...),
> = shape(...(TA & TB), 'x' => int);

type ClosedAndOpenWitness = ClosedAndOpen<
  shape('a' => int),
  shape('a' => int, ...),
>;
