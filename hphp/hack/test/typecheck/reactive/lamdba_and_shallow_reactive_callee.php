<?hh // partial

<<__RxShallow>>
function rxshallow(): void {}

<<__RxLocal>>
function rxlocal(): void {}

function nonreactive(): void {}

function test2(): void {
  // OK
  $x = () ==> rxshallow();
  // OK
  $x = () ==> rxlocal();
  // OK
  $x = () ==> nonreactive();
}
