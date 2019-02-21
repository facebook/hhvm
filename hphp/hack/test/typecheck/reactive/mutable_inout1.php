<?hh // partial

<<__Rx>>
function function_1(<<__Mutable>>inout int $x): void {
  function_2(inout $x);
}

<<__Rx>>
function function_2(<<__Mutable>>inout int $x): void {
}
