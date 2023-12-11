<?hh

function test($a, $b, ...$more_args) :mixed{
  $args = array_merge(vec[$a, $b], $more_args);
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_15() :mixed{
test(1, 2);
 test(1, 2, 3);
 test(1, 2, 3, 4);
}
