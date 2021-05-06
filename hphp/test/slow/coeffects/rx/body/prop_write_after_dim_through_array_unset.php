<?hh

<<__EntryPoint>>
function bad()[rx] {
  $io = new stdClass();
  $io->x = 1;
  $a = darray['o' => $io];

  unset($a['o']->x);
}
