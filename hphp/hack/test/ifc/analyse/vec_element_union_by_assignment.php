<?hh

class A {}

function testClass(vec<int> $v, A $x): void {
  $v[] = $x;
}

function testPrim(vec<int> $v, string $x): void {
  $v[] = $x;
}
