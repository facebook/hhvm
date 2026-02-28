<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function get_like_nonnull(): ~nonnull {
  return 0;
}

function foo(): void {
  $like_nonnull = get_like_nonnull();
  hh_expect_equivalent<~nonnull>($like_nonnull);
  if ($like_nonnull is null) {
    hh_expect_equivalent<(dynamic & null)>($like_nonnull);
  } else {
    hh_expect_equivalent<~nonnull>($like_nonnull);
  }
}
