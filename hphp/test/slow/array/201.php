<?hh

<<__EntryPoint>>
function main_201() :mixed{
  $a = vec[2,1];
  try { var_dump($a[3]); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
