<?hh

function test1(): string {
  $f = (vec<_> $x): string ==> $x[0];
  return $f(vec["A"]);
}

function test2(): string {
  $f = (vec<_> ...$xs): string ==> $xs[0][0];
  return $f(vec["A"]);
}

function getVecString(): vec<string> {
  return vec["A", "B"];
}

function test3(): string {
  $v = getVecString();
  $f = (bool $b, inout vec<_> $x, string $y): string ==> {
    if ($b) $x[0] = $y;
    return $x[1];
  };
  return $f(false, inout $v, "C");
}
