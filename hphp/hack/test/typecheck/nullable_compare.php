<?hh

function id<T>(T $x): T {
  return $x;
}

function test(int $x): bool {
  return id($x) < null;
}
