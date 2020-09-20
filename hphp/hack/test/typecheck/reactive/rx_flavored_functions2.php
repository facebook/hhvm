<?hh // strict

<<__RxShallow>>
function f1(RxShallow<(function(): int)> $f): int {
  return $f();
}

<<__RxShallow>>
function f3(): int {
  // ERROR
  return f1(<<__RxLocal>> () ==> 1);
}
