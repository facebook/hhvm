<?hh

function four() :mixed{ return 4; }
function arr() :mixed{ return dict['x' => four(), 'something' => new stdClass()]; }
function go() :mixed{
  $x = arr();
  $x['something']->hahaha = "yeah";
  return $x;
}
function main() :mixed{
  $x = go();
  var_dump($x['something']);
  var_dump(is_object($x['something']));
}

<<__EntryPoint>>
function main_array_018() :mixed{
main();
}
