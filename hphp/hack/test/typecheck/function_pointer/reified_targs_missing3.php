<?hh

function foo<reify T, reify T2>(T $a): void {
}

function test(): void {
  $x = foo<int, _>;
}
