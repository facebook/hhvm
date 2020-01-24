<?hh

function foo<reify T>(T $a): void {
}

function test(): void {
  // This should be an error. Expect reified types to be expected
  $x = foo<>;
}
