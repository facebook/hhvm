<?hh // strict

class C {
  const type T = int;
}

function test(): void {
  $x = C::T;
}
