<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

interface ShapeSplatReflexivePrefixer<TPrefix as shape()> {
  public function prepend<TSuffix as shape(...)>(
    TSuffix $suffix,
  ): shape(...TPrefix, ...TSuffix);
}

interface ShapeSplatReflexiveBox<+T> {
  public function get(): T;
}

function accept_shape_splat_identity<T as shape()>(
  T $_witness,
  (function(ShapeSplatReflexiveBox<T>): T) $_identity,
): void {}

function test_shape_splat_reflexive<TPrefix as shape()>(
  ShapeSplatReflexivePrefixer<TPrefix> $prefixer,
): void {
  $identity = (ShapeSplatReflexiveBox<shape(..._)> $box) ==>
    $prefixer->prepend($box->get());
  accept_shape_splat_identity(shape(), $identity);
}
