<?hh

function run(&$a, &$b, &$c) {
  $a = "hello";
  $b = 2;
  $c = array();

  return $a;
}

$a = 5;
var_dump(run(&$a, &$a, &$a));
