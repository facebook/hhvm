<?hh

function test($a = 10, ...$more_args) :mixed{
  $args = array_merge(vec[$a], $more_args);
  var_dump(count($args));
  var_dump($args);
}

 <<__EntryPoint>>
function main_20() :mixed{
  test();
  test(1);
  test(1, 2);
}
