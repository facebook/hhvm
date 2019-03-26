<?hh

function run(&$ref, &$ref2, $recur, $recur2) {
  $ref = $recur;
  $ref2 = $recur2;
  var_dump($recur);
  var_dump($recur2);
  var_dump($recur2 == $recur);
  var_dump($recur != $recur);
  var_dump($recur === $recur);
  var_dump($recur !== $recur);
}

<<__EntryPoint>>
function main() {
  $recur = array();
  $recur2 = array();
  run(&$recur[], &$recur2[], $recur, $recur2);
}
