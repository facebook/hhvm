<?hh

<<__EntryPoint, __Rx>>
function bad() {
  $io = new stdClass();
  $a = array('o' => $io);

  $a['o']->x *= 2;
}
