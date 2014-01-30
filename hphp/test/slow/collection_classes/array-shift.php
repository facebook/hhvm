<?hh
function main() {
  $x1 = array('a', 'b', 'c');
  var_dump(array_shift($x1));
  var_dump($x1);
  $x1[] = 'a';
  var_dump($x1);

  $x2 = Vector {'a', 'b', 'c'};
  var_dump(array_shift($x2));
  var_dump($x2);
  $x2[] = 'a';
  var_dump($x2);

  $x3 = Map {11 => 'a', 22 => 'b', 33 => 'c'};
  var_dump(array_shift($x3));
  var_dump($x3);
  $x3[11] = 'a';
  var_dump($x3);

  $x4 = Set {'a', 'b', 'c'};
  var_dump(array_shift($x4));
  var_dump($x4);
  $x4[] = 'a';
  var_dump($x4);

  $x5 = array();
  var_dump(array_shift($x5));

  $x6 = Vector {};
  var_dump(array_shift($x6));

  $x7 = Map {};
  var_dump(array_shift($x7));

  $x8 = Set {};
  var_dump(array_shift($x8));
}
main();

