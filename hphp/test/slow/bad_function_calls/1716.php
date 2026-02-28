<?hh

<<__EntryPoint>>
function f() :mixed{
  $arr = vec[null];
  try { sort(inout $arr,0,0,0,0,0,0,0,0); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($arr);
}
