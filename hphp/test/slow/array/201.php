<?hh

<<__EntryPoint>>
function main_201() :mixed{
  $a = varray[2,1];
  try { var_dump($a[3]); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
