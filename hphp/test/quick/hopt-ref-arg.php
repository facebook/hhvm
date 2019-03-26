<?hh

function set(&$b) {
  $b = 3;
}

function run(&$a) {
  set(&$a);
  return $a;
}

$a = 5;
run(&$a);
var_dump($a);
