<?hh

<<__Rx>>
function by_ref(&$x) {}

<<__Rx>>
function test() {
  // vgetl
  by_ref(&$x);
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
