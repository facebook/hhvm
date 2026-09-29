<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function infer_super<T as shape(...)>(
  shape(?'before' => int, ...T, ?'after' => int) $_,
): void {}

function infer_sub<T as shape(...)>(
  ?T $_ = null,
): shape(?'before' => int, ...T, ?'after' => int) {
  throw new Exception();
}

function accept<T as shape(...)>(
  shape(?'before' => int, ...T, ?'after' => int) $_,
): void {}

function exercise<
  T1 as shape(...),
  T2 as shape(...),
>(shape('left' => int, ...T1, 'x' => int) $value): shape(?'y' => string, 'x' => int)
where
  T1 as shape(...T2, ?'left' => int),
  T2 as shape(...T1, ?'right' => int) {
  infer_super($value);
  accept<T1>(infer_sub());
  hh_expect_equivalent<int>($value['x']);
  hh_expect_equivalent<mixed>($value['left']);
  return $value;
}
