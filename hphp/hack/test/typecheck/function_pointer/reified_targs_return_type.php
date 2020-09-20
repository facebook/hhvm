<?hh

function foo<reify T>(T $x): T {
  return $x;
}

function test(): (function(string): string) {
  return foo<int>;
}
