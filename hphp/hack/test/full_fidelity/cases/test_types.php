<?hh
function f(
  (function (array<int, array<(double, string)>>, string,) : void) $a,
  shape(bar => int) $b,
  @classname<abc<def>> $c
) : void {}
