<?hh // strict

function byRef(inout $x) {}

function test() {
  $x = null;
  byRef(inout $x?->y); // error
}
