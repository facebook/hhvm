<?hh

function run(&$a, &$b) {
  $b = 3;
  return $a;
}

$a = 5;
var_dump(run(&$a, &$a));
