<?hh

<<__Rx>>
function test() {
  // box, bindl, popv
  $x =& new stdClass();

  // vgetl, bindl, unbox
  $z = $y =& $x;
  return tuple($x, $y, $z);
}

<<__EntryPoint>>
function main() {
  test();
  echo "Done\n";
}
