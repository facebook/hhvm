<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

function unsafe_cast_nondenotable_lower_bound7((function((int | bool)): string) $m): void {
  \HH\FIXME\UNSAFE_CAST<mixed, (function(mixed): string)>($m); // No lint
}
