<?hh

function test($a, ...$more_args) {
  $args = varray[$a];
  $args = array_merge($args, $more_args);
  $n = count($args);
  var_dump($n);
  var_dump($args);
}

 <<__EntryPoint>>
function main_13() {
test(1);
 test(1, 2);
 test(1, 2, 3);
}
