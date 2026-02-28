<?hh

function test1(): (string, string) {
  $f = (vec<_> $x, vec<_> $y): (string, string) ==> {
    return tuple($x[0], $y[0]);
  };
  // This should be illegal - we've turned an int into a string!
  return $f(vec[2], vec[3]);
}

function test2(): string {
  $f = (vec<_> ...$x): string ==> $x[0][0];
  // This should also be illegal
  return $f(vec[2]);
}

function getVecString(): vec<string> {
  return vec["A", "B"];
}
function getVecInt(): vec<int> {
  return vec[2, 3];
}

function expectVecArraykey(vec<arraykey> $_): void {}
function expectInt(int $_): void {}

function test3(): string {
  $x = getVecInt();
  $f = (inout vec<_> $x) ==> {
    expectVecArraykey($x);
  };
  $f(inout $x);
  return $x[0];
}
