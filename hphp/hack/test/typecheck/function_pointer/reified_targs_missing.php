<?hh

function foo<reify T>(T $a): void {
}

function test(): void {
  $x = foo<>;
}
