<?hh

function foo(): void {
  foo(); // stack overflow, produces lint
}
function foo2(): void {
  foo2(); // stack overflow, produces lint
  $x = 1;
}
function foo3(): void {
  $x = 1; //should not produce lint
  foo3();
}

function foo4(int $x): void {
  foo4($x); //should not produce lint
}
function foo5(): void {
  foo3(); //should not produce lint
}
