<?hh

<<__Rx>>
function test() {
  return array(
    'a' => &$x, // AddElemV
    &$y,        // AddNewElemV
  );
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
