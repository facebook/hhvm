<?hh

<<__SupportDynamicType>>
function f(): (function (int, shape()): void) {
  // during SDT pass when `expected` type is dynamic
  return (int $i, $s) ==> {
    nosdt_int($i);   // $i: (dynamic & int) ; OK
    nosdt_shape($s); // $s: dynamic         ; error
  };
}

function nosdt_int(int $i): void {}

function nosdt_shape(shape() $s): void {}
