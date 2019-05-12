<?hh

function run(&$a, &$b) {
  $b = 2;
  $a = 3;
  return $b;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(&$a, &$a));
var_dump($a);
}
