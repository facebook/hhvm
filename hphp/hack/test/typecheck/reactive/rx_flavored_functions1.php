<?hh // strict

<<__RxShallow>>
function f1(RxShallow<(function(): int)> $f): int {
  // OK
  return $f();
}

<<__RxLocal>>
function f2(RxLocal<(function(): int)> $f): int {
  // OK
  return $f();
}

<<__RxShallow>>
function f3(): int {
  // OK
  return f1(<<__RxShallow>> () ==> 1);
}

<<__RxLocal>>
function f4(): int {
  // OK
  return f2(<<__RxLocal>> () ==> 1);
}
