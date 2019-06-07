<?hh

function test(...$args) {
  var_dump($args[0]);
  var_dump($args[1]);
}

 <<__EntryPoint>>
function main_12() {
test(2, 'ok');
}
