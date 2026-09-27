<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter's upper bound has a *required* field 't', so every
// value of `T` (and hence of the scrutinee) has 't'. A closed hint that omits
// 't' can therefore never match: the true branch is `nothing`. This is the
// disjointness the bound-aware splat view detects; the concrete part alone does
// not force it.
function f<T as shape('t' => bool, ...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
