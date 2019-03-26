<?hh

function run(&$a, &$b) {
  $a = 1;
  $a = true;
  $b = 3;

  return $a;
}

$a = 5;
var_dump(run(&$a, &$a));
