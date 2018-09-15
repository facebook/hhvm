<?hh // strict

<<__RxLocal>>
function test(): void {}

<<__Rx>>
function test2(): void {
  // Rx cannot call RxLocal
  test();
}
