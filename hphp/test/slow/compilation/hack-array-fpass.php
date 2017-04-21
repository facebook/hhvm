<?hh

if (isset($g)) {
  function foo(&$x) {}
} else {
  function foo($x) { var_dump($x); }
}

function main() {
  $d = dict['foo' => 1];
  foo($d['foo']);
}

main();
