<?hh

<<__EntryPoint>>
function main_1316() :mixed{
  $x = 1;
  switch ($x++ ?: -1) {
    default: break;
  }
  var_dump($x);
}
