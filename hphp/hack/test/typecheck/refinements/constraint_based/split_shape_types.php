<?hh

function should_understand_disjoint_shapes_too_many(mixed $x): void {
  if ($x is shape('x' => int, 'y' => int)) {
    if ($x is shape('x' => int, 'y' => int, 'z' => int)) {
      hh_expect<nothing>($x);
    }
  }
}
function should_understand_disjoint_shapes_different_fields(mixed $x): void {
  if ($x is shape('x' => int, 'y' => int)) {
    if ($x is shape('x' => int, 'z' => int)) {
      hh_expect<nothing>($x);
    }
  }
}

function should_understand_disjoint_tuples_one_type_off(mixed $x): void {
  if ($x is shape('x' => int, 'y' => int)) {
    if ($x is shape('x' => int, 'y' => string)) {
      hh_expect<nothing>($x);
    }
  }
}

function subsplit_tuples(
  shape('x' => int, 'y' => string, 'z' => num) $x,
): void {
  if ($x is shape('x' => int, 'y' => string, 'z' => int)) {
    hh_expect_equivalent<shape('x' => int, 'y' => string, 'z' => int)>($x);
  } else {
    hh_expect_equivalent<shape('x' => int, 'y' => string, 'z' => float)>($x);
  }
}
