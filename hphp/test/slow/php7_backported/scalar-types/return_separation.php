<?hh

function by_ref(inout $ref) :mixed{}

function test1() : varray {
  $array = vec[];
  by_ref(inout $array);
  return $array;
}

function test2() : string {
  $int = 42;
  by_ref(inout $int);
  return $int;
}
<<__EntryPoint>> function main(): void {
var_dump(test1());
var_dump(test2());
}
