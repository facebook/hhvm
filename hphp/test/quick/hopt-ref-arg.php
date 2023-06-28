<?hh

function set(inout $b) :mixed{
  $b = 3;
}

function run(inout $a) :mixed{
  set(inout $a);
  return $a;
}
<<__EntryPoint>> function main(): void {
$a = 5;
run(inout $a);
var_dump($a);
}
