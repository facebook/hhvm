<?hh // strict

function test() {
  $foo = null;
  echo 'not reached';
  $x =& $foo?->bar[1]; // parse error
  echo 'not reached';
}


<<__EntryPoint>>
function main_nullsafe_prop_6() {
test();
}
