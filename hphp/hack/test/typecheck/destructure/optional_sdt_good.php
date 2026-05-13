<?hh
<<file:__EnableUnstableFeatures('shape_and_tuple_destructuring', 'like_type_hints')>>

// Plain optional field
function test_plain(shape(?'x' => int) $s): void {
  shape(?'x' => $x, ...) = $s;
  hh_expect_equivalent<?int>($x);
}

// Like type with optional field
function test_like(~shape(?'x' => int) $s): void {
  shape(?'x' => $x, ...) = $s;
  hh_expect_equivalent<~?int>($x);
}

// supportdyn with optional field
function test_supportdyn(supportdyn<shape(?'x' => int)> $s): void {
  shape(?'x' => $x, ...) = $s;
  hh_expect_equivalent<?int>($x);
}

// supportdyn of like with optional field
function test_supportdyn_like(supportdyn<~shape(?'x' => int)> $s): void {
  shape(?'x' => $x, ...) = $s;
  hh_expect_equivalent<~?int>($x);
}
