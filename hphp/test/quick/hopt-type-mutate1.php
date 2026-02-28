<?hh

function run(inout $a, inout $b) :mixed{
  $a = 1;
  $a = true;

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(inout $a, inout $a));
}
