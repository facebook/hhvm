<?hh

function by_ref(inout $ref) {}

function test1() : varray {
  $array = varray[];
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
