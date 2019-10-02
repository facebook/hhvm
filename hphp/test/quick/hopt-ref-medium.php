<?hh

function run(inout $a, inout $b, inout $c) {
  $a = 1;
  $b = 2;
  $c = 3;

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(inout $a, inout $a, inout $a));
}
