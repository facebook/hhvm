<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// `absent 'x'` is surface sugar for `?'x' => nothing`: the field is declared
// definitely-not-present. This test ensures the two are equivalent.

type WithAbsent = shape('y' => int, absent 'x');
type WithOptNothing = shape('y' => int, ?'x' => nothing);

// Mutual assignability => the two shape types are equal.
function a2b(WithAbsent $a): WithOptNothing {
  return $a;
}
function b2a(WithOptNothing $b): WithAbsent {
  return $b;
}

// `absent` is a contextual keyword: still usable as an ordinary identifier.
function absent(int $x): int {
  return $x;
}

function show_absent(WithAbsent $a): void {
  hh_show($a);
}
