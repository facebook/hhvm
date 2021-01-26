<?hh

<<__EntryPoint>>
function bad()[rx] {
  $x = new stdClass();

  $y = clone $x;
}
