<?hh

function rxshallow(): void {}


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
