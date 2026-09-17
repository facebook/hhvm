<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// A splat tparam's bound is spreadable when every member of a union bound is,
// mirroring the distribution in Typing_shape_normalize.merge.

// Members carrying distinct tparams cannot be joined, so the bound survives as a
// real union -- the same condition under which merge distributes (see
// union_distribute_distinct.php).
function distinct_bound<
  T1 as shape(...),
  T2 as shape(...),
  T as (T1 | T2),
>(shape(...T, 'x' => int) $s): void {
  hh_show($s);
}

// `~shape(...)` is `Tunion [dynamic; shape]` after localization, so this is a
// union bound too, and neither member alone matches the shape-or-dynamic test.
function like_shape<T as (shape('a' => int) | dynamic)>(shape(...T, 'x' => int) $s): void {
  hh_show($s);
}

// A union of param-free shapes is joined during localization, with each side's
// fields optional, so it never reaches the union rule at all.
function two_shapes<T as (shape('a' => int) | shape('b' => bool))>(
  shape(...T, 'x' => int) $s,
): void {
  hh_show($s);
}

// REJECT: `int` is not spreadable, so neither is the union containing it.
function with_int<T as (shape('a' => int) | int)>(shape(...T, 'x' => int) $s): void {}


function read_through_union_redundant_splat<
  T1 as shape('a' => int),
  T2 as shape('a' => string),
  T as (shape(...T1) | shape(...T2)),
>(shape(...T) $s): void {
  hh_show($s);
  hh_show($s['a']);
}

function read_through_union_ok<
  T1 as shape('a' => int),
  T2 as shape('a' => string),
  T as (T1 | T2),
>(T $s): void {
  hh_show($s);
  hh_show($s['a']);
}
