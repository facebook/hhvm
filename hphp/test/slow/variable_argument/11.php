<?hh

function test(...$args) {
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_11() {
test();
 test(1);
 test(1, 2);
}
