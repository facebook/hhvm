<?hh

function run(&$a, &$b, &$c) {
  $a = 1;
  $b = true;
  $c = 3;

  return $a;
}

$a = 5;
var_dump(run(&$a, &$a, &$a));
