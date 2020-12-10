<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $f): void {
  $a = () ==> $f();
  $a();
}
