<?hh

function four() { return 4; }
function arr() { return darray['x' => four(), 'something' => new stdClass()]; }
function go() {
  $x = arr();
  $x['something']->hahaha = "yeah";
  return $x;
}
function main() {
  $x = go();
  var_dump($x['something']);
  var_dump(is_object($x['something']));
}

<<__EntryPoint>>
function main_array_018() {
main();
}
