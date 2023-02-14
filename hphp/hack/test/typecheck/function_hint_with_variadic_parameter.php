<?hh //strict

function variadic_int(int ...$y): void {}

function hint_no_args((function(): void) $f): void {}

function hint_single_int((function(int): void) $f): void {}

function hint_multiple_int((function(int, int, int): void) $f): void {}

function hint_variadic_int((function(int...): void) $f): void {}

function test(): void {
  hint_no_args(variadic_int<>);
  hint_single_int(variadic_int<>);
  hint_multiple_int(variadic_int<>);
  hint_variadic_int(variadic_int<>);

  $closure = function(int ...$y): void {};
  hint_no_args($closure);
  hint_single_int($closure);
  hint_multiple_int($closure);
  hint_variadic_int($closure);

  $lambda = (int ...$y) ==> {
  };
  hint_no_args($lambda);
  hint_single_int($lambda);
  hint_multiple_int($lambda);
  hint_variadic_int($lambda);

  $lambda = (...$y) ==> {
  };
  hint_no_args($lambda);
  hint_single_int($lambda);
  hint_multiple_int($lambda);
  hint_variadic_int($lambda);
}
