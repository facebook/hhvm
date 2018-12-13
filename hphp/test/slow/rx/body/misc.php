<?hh

<<__Rx>>
function test() {
  // CheckProp and InitProp are only emitted in 86pinits
  $x = new stdClass();
  $y = clone $x;          // Clone
  $z = @new stdClass();   // Silence
  eval('echo "Evil\n";'); // Eval
  echo "Goodbye\n";       // Print
  exit(0);                // Exit
}

<<__EntryPoint>>
function main() {
  test();
  // test exits so we should not get here
  echo "FAIL\n";
}
