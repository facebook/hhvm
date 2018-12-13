<?hh

<<__Rx>>
function &returnByRef1(): int {
  return 1;
}

<<__Rx>>
function &returnByRef2() {
  return returnByRef1();
}

<<__EntryPoint>>
function main() {
  returnByRef1();
  echo "Done\n";
}
