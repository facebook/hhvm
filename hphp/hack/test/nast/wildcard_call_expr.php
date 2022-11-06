<?hh

function my_id<T>(T $x): T {
  return $x;
}

function wildcard_call(): void {
  my_id<_>(42);
}
