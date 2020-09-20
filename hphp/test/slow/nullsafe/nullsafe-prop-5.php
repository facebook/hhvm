<?hh // strict

function byVal($x) {
  echo 'byVal is called, $x is: ';
  var_dump($x);
}

function test() {
  $x = null;
  byVal($x?->y); // ok
}


<<__EntryPoint>>
function main_nullsafe_prop_5() {
test();
}
