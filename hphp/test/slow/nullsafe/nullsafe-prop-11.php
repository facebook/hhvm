<?hh // strict

function something($x) { return null; }

function test() {
  echo 'not reached';
  $x =& something(($k = 2))?->bar; // emit error
  echo 'not reached';
}


<<__EntryPoint>>
function main_nullsafe_prop_11() {
test();
}
