<?hh
<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfRxFunc>>(function(): int) $f): void {
  $a = <<__RxOfScope>> () ==> $f();
  $a();
}
