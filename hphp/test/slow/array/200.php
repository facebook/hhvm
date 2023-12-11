<?hh

<<__EntryPoint>>
function main_200() :mixed{
  $a = vec[2,1];
  try { var_dump($a[-1]); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
