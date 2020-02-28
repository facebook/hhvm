<?hh

<<__EntryPoint, __Rx>>
function bad() {
  $io = new stdClass();
  $io->x = 1;
  $a = darray['o' => $io];

  unset($a['o']->x);
}
