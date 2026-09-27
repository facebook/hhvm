<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The scrutinee's concrete field 'a' is int; the closed hint requires 'a' to be
// string. No value can satisfy both, so the true branch is `nothing` (the splat
// is now split precisely instead of leaving an inert intersection).
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => string)) {
    hh_expect_equivalent<nothing>($x);
  }
}
