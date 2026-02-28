<?hh

function run(inout $a, inout $b, inout $c) :mixed{
  $a = "hello";
  $b = 2;
  $c = vec[];

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(inout $a, inout $a, inout $a));
}
