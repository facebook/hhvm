<?hh // strict

function test() {
  $foo = null;
  echo 'not reached';
  $x =& $foo?->bar; // emit error
  echo 'not reached';
}


<<__EntryPoint>>
function main_nullsafe_prop_8() {
test();
}
