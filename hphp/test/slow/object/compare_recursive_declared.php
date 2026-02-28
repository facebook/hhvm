<?hh

class R { public $recur; }

function main() :mixed{
  $recur = new R();
  $recur->recur = $recur;
  $recur2 = new R();
  $recur2->recur = $recur2;
  var_dump($recur);
  var_dump($recur2);
  var_dump($recur2 == $recur);
  var_dump($recur != $recur);
  var_dump($recur === $recur);
  var_dump($recur !== $recur);
}

<<__EntryPoint>>
function main_compare_recursive_declared() :mixed{
main();
}
