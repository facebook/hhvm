<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

interface ShapeSplatCycleBox<+T> {
  public function get(): T;
}

// Function-parameter contravariance and the return type produce the cycle
// `T <: V <: shape(...TPrefix, ...T)`, which uses the general mid-row fallback.
function infer_prefixed_identity<
  TPrefix as shape('x' => int),
  T as shape(),
>(
  shape(...TPrefix) $_prefix,
  (function(ShapeSplatCycleBox<shape(...T)>): shape(...TPrefix, ...T)) $_f,
): T {
  throw new Exception();
}

function trigger_same_var_fallback<TPrefix as shape('x' => int)>(
  shape(...TPrefix) $prefix,
): void {
  $identity = (ShapeSplatCycleBox<shape(..._)> $box) ==> $box->get();
  $inferred = infer_prefixed_identity($prefix, $identity);
  hh_expect<shape('x' => int)>($inferred);
}
