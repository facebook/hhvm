<?hh // strict

function foo(): array<string> {
  echo "foo\n";
  return varray['hello', 'world'];
}
function foo2(): array<array<string>> {
  return varray[varray[], foo()];
}

function unpack1(): void {
  list($x, $y) = foo();
  echo $x, $y, "\n";
}

function unpack2(): void {
  list($_, list($y, $z)) = foo2();
  echo $y, $z, "\n";

  $a = foo2();
  list($_, list($y, $z)) = $a;
  echo $y, $z, "\n";
}

function f(int $x): int {
  echo $x, "\n";
  return $x;
}

function unpack3(): void {
  $x = darray[];
  list($x[f(0)], $x[f(1)]) = foo();
  var_dump($x);
}

function test(): void {
  unpack1();
  unpack2();
  unpack3();
}
