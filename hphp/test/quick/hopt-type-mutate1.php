<?hh

function run(&$a, &$b) {
  $a = 1;
  $a = true;

  return $a;
}

$a = 5;
var_dump(run(&$a, &$a));
