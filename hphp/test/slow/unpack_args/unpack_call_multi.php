<?hh

function f($a, $b, $c) :mixed{
  echo __FUNCTION__, "\n";
  var_dump($a, $b, $c);
}

function main() :mixed{
  $args = vec['a', 'b', 'c'];
  f(...$args, ...$args);
}


<<__EntryPoint>>
function main_unpack_call_multi() :mixed{
main();
}
