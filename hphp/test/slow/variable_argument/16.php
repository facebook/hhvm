<?hh

function test(...$args) :mixed{
  var_dump($args[0]);
  var_dump($args[1]);
  var_dump($args[2]);
  var_dump($args[3]);
}

 <<__EntryPoint>>
function main_16() :mixed{
test(2, 'ok', 0, 'test');
}
