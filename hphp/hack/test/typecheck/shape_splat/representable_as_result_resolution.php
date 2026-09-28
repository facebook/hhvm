<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

function takes_overwritten(
  \HH\Runtime\RepresentableAs<dict<string, int>> $_,
): void {}

function rightmost<T as shape('x' => string)>(
  shape(...T, 'x' => int) $shape,
): void {
  takes_overwritten($shape);
}

function takes_shape(
  \HH\Runtime\RepresentableAs<shape('x' => int, ...)> $_,
): void {}

function shape_alternative<T as shape(...)>(
  shape(...T, 'x' => int) $shape,
): void {
  takes_shape($shape);
}

function takes_transitive(
  \HH\Runtime\RepresentableAs<dict<string, arraykey>> $_,
): void {}

function transitive<
  T0 as shape('x' => int),
  T1 as shape(...T0, ...shape()),
>(
  shape(...T1, 'y' => string) $shape,
): void {
  takes_transitive($shape);
}
