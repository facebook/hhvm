<?hh // strict

function byRef(&$x) {}

function test() {
  $x = null;
  byRef(&$x?->y); // error
}


<<__EntryPoint>>
function main_nullsafe_prop_13() {
test();
}
