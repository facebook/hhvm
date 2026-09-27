<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// Hint requires a field ('q') that only the abstract splat could supply. The
// intersection cannot be simplified against `...T`, but the true branch is
// still a subtype of both the original splat shape and the hint.
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => int, 'q' => bool)) {
    hh_expect<shape(...T, 'a' => int)>($x);
    hh_expect<shape('a' => int, 'q' => bool)>($x);
  }
}
