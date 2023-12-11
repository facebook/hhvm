<?hh

function test($a, ...$more_args) :mixed{
  $args = array_merge(vec[$a], $more_args);
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_17() :mixed{
test('test');
 test(1, 2);
 test(1, 2, 3);
}
