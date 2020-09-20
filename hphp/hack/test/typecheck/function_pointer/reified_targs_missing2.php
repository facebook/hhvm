<?hh

function foo<reify T, reify T2>(T $a): void {
}

function test(): void {
  // This should be an error. Expect reified types to be expected
  $x = foo<int>;
}
