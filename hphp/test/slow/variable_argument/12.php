<?hh

function test(...$args) :mixed{
  var_dump($args[0]);
  var_dump($args[1]);
}

 <<__EntryPoint>>
function main_12() :mixed{
test(2, 'ok');
}
