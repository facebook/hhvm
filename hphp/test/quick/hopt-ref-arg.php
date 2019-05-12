<?hh

function set(&$b) {
  $b = 3;
}

function run(&$a) {
  set(&$a);
  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
run(&$a);
var_dump($a);
}
