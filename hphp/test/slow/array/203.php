<?hh

<<__EntryPoint>>
function main_203() :mixed{
  $a = dict['b' => 2, 'a' => 1];
  try { var_dump($a['bogus']); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
}
