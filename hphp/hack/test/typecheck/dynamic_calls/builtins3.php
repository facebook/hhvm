<?hh

function foo(mixed $x): mixed {return $x;}

function test(): void {
  $mixed = foo(10);
  $foo_dynamic = HH\dynamic_fun($mixed);
}
