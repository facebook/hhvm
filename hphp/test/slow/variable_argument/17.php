<?hh

function test($a, ...$more_args) {
  $args = array_merge(varray[$a], $more_args);
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_17() {
test('test');
 test(1, 2);
 test(1, 2, 3);
}
