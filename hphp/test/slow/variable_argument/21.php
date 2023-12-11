<?hh

function test($a, $b = 10, ...$more_args) :mixed{
  var_dump($a);
  var_dump($b);
  var_dump(array_merge(vec[$a, $b], $more_args));
}

 <<__EntryPoint>>
function main_21() :mixed{
  test(1);
  test(1, 2);
  test(1, 2, 3);
}
