<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// An intersection operand distributes, dually to a union: a value that is every
// member spreads to a row that is every member's row.
//
// Typing_intersection folds most intersections before normalization sees them --
// two shapes intersect into one row, a shape and a class prove empty. What
// survives is an intersection whose members are opaque to it, typically type
// parameters, which merge carries as residual elements without consulting their
// bounds.

// The direct case: two shape-bounded type parameters with no alias.
function direct<TA as shape(...), TB as shape(...)>(
  shape('x' => int, ...(TA & TB)) $s,
): void {
  hh_expect_equivalent<(
    shape(...shape('x' => int), ...TA) & shape(...shape('x' => int), ...TB)
  )>($s);
}

newtype Isect<T1 as shape(...), T2 as shape(...)> =
  shape('x' => int, ...(T1 & T2));

// Simplified before distribution: the two shapes intersect into one row.
function concrete(
  Isect<shape('a' => int, ...), shape('b' => string, ...)> $s,
): void {
  hh_expect_equivalent<shape('a' => int, 'b' => string, 'x' => mixed, ...)>($s);
}

// Through a transparent newtype, which behaves no differently here.
function generic<TA as shape(...), TB as shape(...)>(Isect<TA, TB> $s): void {
  hh_expect_equivalent<(
    shape(...shape('x' => int), ...TA) & shape(...shape('x' => int), ...TB)
  )>($s);
}

// `(a & b) <: c` holds when either branch does, so the intersection converts to
// either branch.
function to_branch<TA as shape(...), TB as shape(...)>(
  Isect<TA, TB> $s,
): shape('x' => int, ...TA) {
  return $s;
}

function to_other_branch<TA as shape(...), TB as shape(...)>(
  Isect<TA, TB> $s,
): shape('x' => int, ...TB) {
  return $s;
}
