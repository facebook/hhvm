<?php /* ref in array */


function main() {
  $recur = array();
  $recur[] = &$recur;
  $recur2 = array();
  $recur2[] = &$recur2;
  var_dump($recur);
  var_dump($recur2);
  var_dump($recur2 == $recur);
  var_dump($recur != $recur);
  var_dump($recur === $recur);
  var_dump($recur !== $recur);
}

<<__EntryPoint>>
function main_compare_recursive_array() {
main();
}
