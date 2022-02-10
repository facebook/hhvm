<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function unsafe_cast_nondenotable_lower_bound((int | bool) $m): void {
  \HH\FIXME\UNSAFE_CAST<mixed, int>($m); // No lint
}
