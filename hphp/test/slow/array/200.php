<?hh

<<__EntryPoint>>
function main_200() {
  $a = varray[2,1];
  try { var_dump($a[-1]); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
