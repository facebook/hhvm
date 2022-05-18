<?hh


function unsafe_cast_nondenotable_lower_bound8(mixed $m): void {
  if($m is Map<_,_>) {
    \HH\FIXME\UNSAFE_CAST<mixed, Map<int,string>>($m); // No lint
  }
}
