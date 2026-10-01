<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
newtype BadNullable<
  TU as ?shape('x' => int),
  T as TU as shape(...),
> = shape(...T, 'x' => bool);

type NullableWitness = BadNullable<shape('x' => int), shape('x' => int)>;

<<__DisjointShapeSplat>>
newtype BadNonnull<T as nonnull as shape(...)> =
  shape(...T, 'x' => bool);

type NonnullWitness = BadNonnull<shape('x' => int)>;
