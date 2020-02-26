<?hh

function run(inout $a, inout $b, inout $c) {
  $a = "hello";
  $b = 2;
  $c = varray[];

  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
var_dump(run(inout $a, inout $a, inout $a));
}
