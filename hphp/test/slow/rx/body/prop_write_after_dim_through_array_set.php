<?hh

<<__EntryPoint, __Rx>>
function bad() {
  $io = new stdClass();
  $a = darray['o' => $io];

  $a['o']->x = 2;
}
