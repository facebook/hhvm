<?hh

function by_ref(&$ref) {}

function test1() : array {
  $array = [];
  by_ref(&$array);
  return $array;
}

function test2() : string {
  $int = 42;
  by_ref(&$int);
  return $int;
}
<<__EntryPoint>> function main(): void {
var_dump(test1());
var_dump(test2());
}
