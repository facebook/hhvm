<?hh // strict

class C {
  const type T = int;
}

function test(): void {
  hh_show(C::T);
}
