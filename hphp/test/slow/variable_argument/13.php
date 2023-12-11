<?hh

function test($a, ...$more_args) :mixed{
  $args = vec[$a];
  $args = array_merge($args, $more_args);
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_13() :mixed{
test(1);
 test(1, 2);
 test(1, 2, 3);
}
