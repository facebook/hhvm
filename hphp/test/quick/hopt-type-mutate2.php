<?hh

function run(inout $a, inout $b, inout $c) :mixed{
  $a = 1;
  $b = true;
  $c = 3;

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(inout $a, inout $a, inout $a));
}
