<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Regression test: simple shapes on either side of a nested-splat boundary must
// merge into a single canonical element, not stay as separate fragments. `T` is
// kept generic so the result remains an open splat.
//
// The nested form `shape(...(shape(...T, 'foo' => int)), 'bar' => int)` and the
// flat form `shape(...T, 'foo' => int, 'bar' => int)` must normalise to the SAME
// shape: 'foo' and 'bar' in one merged element.

type TWith<T> = shape(...T, 'foo' => int);

function nested<T as shape(...)>(shape(...TWith<T>, 'bar' => int) $s): void {
  hh_expect_equivalent<shape(...T, 'foo' => int, 'bar' => int)>($s);
}

function flat<T as shape(...)>(shape(...T, 'foo' => int, 'bar' => int) $s): void {
  hh_expect_equivalent<shape(...T, 'foo' => int, 'bar' => int)>($s);
}
