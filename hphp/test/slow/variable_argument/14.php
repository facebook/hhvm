<?hh

function test($a, ...$more_args) :mixed{
  var_dump($a);
  var_dump($more_args[0]);
  var_dump($more_args[1]);
}

 <<__EntryPoint>>
function main_14() :mixed{
test(2, 'ok', vec[1]);
}
