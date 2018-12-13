<?hh

<<__Rx>>
function test() {
  $x = array();
  foreach ($x as &$v) { /* do nothing */ }
  foreach ($x as $k => &$v) { /* do nothing */ }
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
