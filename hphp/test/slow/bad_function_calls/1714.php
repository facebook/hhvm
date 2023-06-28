<?hh

function f() :mixed{
  $a = 0;
  try { array_chunk($a = 1); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($a);
}

<<__EntryPoint>>
function main_1714() :mixed{
f();
}
