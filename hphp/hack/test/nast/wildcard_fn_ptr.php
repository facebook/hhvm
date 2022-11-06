<?hh

function my_id<T>(T $x): T {
  return $x;
}

function wildcard_fn_ptr(): void {
  $x = my_id<_>;
}
