<?hh

function run(&$a, &$b, &$c) {
  $a = 1;
  $b = 2;
  $c = 3;

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(&$a, &$a, &$a));
}
