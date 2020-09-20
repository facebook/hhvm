<?hh

function foo<reify T>(T $x): T {
  return $x;
}

function test(): (function(int): int) {
  $y = foo<int>;

  $y(4);

  return $y;
}
