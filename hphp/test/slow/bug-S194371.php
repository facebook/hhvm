<?hh

function f() :mixed{
  $vals = varray[];
  $vals[] = 10;
  printf("%016x\n", 1 << $vals[0]);
}

<<__EntryPoint>>
function main() :mixed{
  f();
  f();
}
