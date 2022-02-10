<?hh

function foo(): void {
  $x = ExampleDsl`() ==> { $x = 1; if (true) { $x = 2; } }`;
}

function bar(): void {}
