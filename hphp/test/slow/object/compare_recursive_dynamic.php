<?hh

function main() :mixed{
  $recur = new stdClass();
  $recur->recur = $recur;
  $recur2 = new stdClass();
  $recur2->recur = $recur2;
  var_dump($recur);
  var_dump($recur2);
  var_dump($recur2 == $recur);
  var_dump($recur != $recur);
  var_dump($recur === $recur);
  var_dump($recur !== $recur);
}

<<__EntryPoint>>
function main_compare_recursive_dynamic() :mixed{
main();
}
