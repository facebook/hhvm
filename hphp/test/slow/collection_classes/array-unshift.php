<?hh
function main() {
  $x1 = array();
  var_dump(array_unshift($x1, 1));
  var_dump(array_unshift($x1, 'b', 'c'));
  var_dump($x1);
  $x2 = array('d');
  var_dump(array_unshift($x2, 'e', 'f', 2, 'f'));
  var_dump($x2);
  $x3 = Vector {};
  var_dump(array_unshift($x3, 1));
  var_dump(array_unshift($x3, 'b', 'c'));
  var_dump($x3);
  $x4 = Vector {'d'};
  var_dump(array_unshift($x4, 'e', 'f', 2, 'f'));
  var_dump($x4);
  $x5 = Set {};
  var_dump(array_unshift($x5, 1));
  var_dump(array_unshift($x5, 'b', 'c'));
  var_dump($x5);
  $x6 = Set {'d'};
  var_dump(array_unshift($x6, 'e', 'f', 2, 'f'));
  var_dump($x6);
  $x7 = Set {};
  var_dump(array_unshift($x7, 'h', 3, 'j'));
  var_dump(array_unshift($x7, 3, 'j'));
  var_dump($x7);
}
main();

