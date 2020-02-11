<?hh

function f() {
  $vals = array();
  $vals[] = 10;
  printf("%016x\n", 1 << $vals[0]);
}

<<__EntryPoint>>
function main() {
  f();
  f();
}
