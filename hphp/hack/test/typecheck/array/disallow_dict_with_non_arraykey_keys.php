<?hh // strict

class C {
  const float PI = 3.14;
}

function test(): void {
  $x = dict[C::PI => 1];
}
