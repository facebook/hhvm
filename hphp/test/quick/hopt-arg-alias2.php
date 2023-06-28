<?hh

function run(inout $a, inout $b) :mixed{
  $b = 2;
  $a = 3;
  return $b;
}
<<__EntryPoint>> function main(): void {
$a = 5;
$b = 4;
var_dump(run(inout $a, inout $b));
var_dump($a);
var_dump($b);
}
