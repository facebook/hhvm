<?hh

function run(&$a, &$b) {
  $b = 3;
  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(&$a, &$a));
}
