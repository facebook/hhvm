<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// PERFORMANCE REGRESSION: repeated generic bounds must reuse cached label
// bounds instead of being recomputed exponentially.
<<__DisjointShapeSplat>>
newtype RepeatedBounds<
  T0 as shape(...T1, ...T1),
  T1 as shape(...T2, ...T2),
  T2 as shape(...T3, ...T3),
  T3 as shape(...T4, ...T4),
  T4 as shape(...T5, ...T5),
  T5 as shape(...T6, ...T6),
  T6 as shape(...T7, ...T7),
  T7 as shape(...T8, ...T8),
  T8 as shape(...T9, ...T9),
  T9 as shape(...T10, ...T10),
  T10 as shape(...T11, ...T11),
  T11 as shape(...T12, ...T12),
  T12 as shape(...T13, ...T13),
  T13 as shape(...T14, ...T14),
  T14 as shape(...T15, ...T15),
  T15 as shape(...T16, ...T16),
  T16 as shape('a' => int),
> = shape(...T0, 'b' => int);
