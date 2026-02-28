<?hh

function should_understand_disjoint_tuples_mismatched_arity(mixed $x): void {
  if ($x is (int, int)) {
    if ($x is (int, int, int)) {
      hh_expect<nothing>($x);
    }
  }
}

function should_understand_disjoint_tuples_one_type_off(mixed $x): void {
  if ($x is (int, int, int)) {
    if ($x is (int, int, string)) {
      hh_expect<nothing>($x);
    }
  }
}

function subsplit_tuples((int, string, num) $x): void {
  if ($x is (int, string, int)) {
    hh_expect_equivalent<(int, string, int)>($x);
  } else {
    hh_expect_equivalent<(int, string, float)>($x);
  }
}
