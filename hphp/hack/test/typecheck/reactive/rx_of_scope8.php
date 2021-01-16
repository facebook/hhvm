<?hh
<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): void {
  $a = () ==> $f();
  $a();
}
