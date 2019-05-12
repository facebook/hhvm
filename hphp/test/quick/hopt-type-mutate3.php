<?hh

function run(&$a, &$b) {
  $a = 1;
  $a = true;
  $b = 3;

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(&$a, &$a));
}
