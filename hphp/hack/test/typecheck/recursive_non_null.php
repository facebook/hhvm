<?hh

function opt<T>(T $x): ?T {
  return $x;
}

function f(): void {
  $v = Vector {};
  $x = $v[0];
  $v[] = opt($x);
  $x = $v[0];
  $v[] = opt($x);

  $x = $v[0];
  if ($x) {}
}
