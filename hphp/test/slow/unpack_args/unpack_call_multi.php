<?hh

function f($a, $b, $c) {
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function main() {
  $args = varray['a', 'b', 'c'];
  f(...$args, ...$args);
}


<<__EntryPoint>>
function main_unpack_call_multi() {
main();
}
