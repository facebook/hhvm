<?hh

function main() {
  $d = dict['foo' => 1];
  foo($d['foo']);
}


<<__EntryPoint>>
function main_hack_array_fpass() {
if (isset($g)) {
  function foo(&$x) {}
} else {
  function foo($x) { var_dump($x); }
}

main();
}
