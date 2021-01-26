<?hh

<<__EntryPoint>>
function bad()[rx] {
  $io = new stdClass();
  $a = darray['o' => $io];

  $a['o']->x++;
}
