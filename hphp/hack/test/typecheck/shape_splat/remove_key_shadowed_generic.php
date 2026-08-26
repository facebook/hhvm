<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `Shapes::removeKey` on a field supplied by a generic (here also shadowed by
// `T` to its right). The field is required by `T`, which can't be edited, and
// because `removeKey` takes the shape `inout` its type is invariant — so `T`
// can't be widened to its upper bound and the removal can't be represented. It
// is therefore rejected rather than silently dropped.
function shadowed<T as shape('a' => int)>(shape('a' => string, ...T) $s): void {
  Shapes::removeKey(inout $s, 'a');
}
