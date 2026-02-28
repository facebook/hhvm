<?hh

function f() :mixed{
  $a = 0;
  $b = 0;
  $c = 0;
  $d = 0;
  try { array_chunk($a = 1, $b = 2, $c = 3, $d = 4); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($a, $b, $c, $d);
}

<<__EntryPoint>>
function main_1715() :mixed{
f();
}
