<?hh

function test(...$args) {
  var_dump($args[0]);
  var_dump($args[1]);
  var_dump($args[2]);
  var_dump($args[3]);
}

 <<__EntryPoint>>
function main_16() {
test(2, 'ok', 0, 'test');
}
