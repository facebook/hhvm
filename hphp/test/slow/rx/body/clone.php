<?hh

<<__EntryPoint, __Rx>>
function bad() {
  $x = new stdClass();

  $y = clone $x;
}
