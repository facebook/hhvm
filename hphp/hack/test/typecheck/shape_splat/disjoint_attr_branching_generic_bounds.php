<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// PERFORMANCE REGRESSION: each level reaches the next bound directly and
// through another type parameter. Caching by the full expansion path makes the
// number of traversals exponential.
<<__DisjointShapeSplat>>
newtype BranchingBounds<
  T0 as shape(...T1, ...TU0),
  TU0 as T1,
  T1 as shape(...T2, ...TU1),
  TU1 as T2,
  T2 as shape(...T3, ...TU2),
  TU2 as T3,
  T3 as shape(...T4, ...TU3),
  TU3 as T4,
  T4 as shape(...T5, ...TU4),
  TU4 as T5,
  T5 as shape(...T6, ...TU5),
  TU5 as T6,
  T6 as shape(...T7, ...TU6),
  TU6 as T7,
  T7 as shape(...T8, ...TU7),
  TU7 as T8,
  T8 as shape(...T9, ...TU8),
  TU8 as T9,
  T9 as shape(...T10, ...TU9),
  TU9 as T10,
  T10 as shape(...T11, ...TU10),
  TU10 as T11,
  T11 as shape(...T12, ...TU11),
  TU11 as T12,
  T12 as shape('a' => int),
> = shape(...T0, 'b' => int);
