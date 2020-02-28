<?hh

function test($a, ...$more_args) {
  var_dump($a);
  var_dump($more_args[0]);
  var_dump($more_args[1]);
}

 <<__EntryPoint>>
function main_14() {
test(2, 'ok', varray[1]);
}
