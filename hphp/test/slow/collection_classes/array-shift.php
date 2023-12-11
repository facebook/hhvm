<?hh
function main() :mixed{
  $x1 = vec['a', 'b', 'c'];
  var_dump(array_shift(inout $x1));
  var_dump($x1);
  $x1[] = 'a';
  var_dump($x1);

  $x2 = Vector {'a', 'b', 'c'};
  var_dump(array_shift(inout $x2));
  var_dump($x2);
  $x2[] = 'a';
  var_dump($x2);

  $x3 = Map {11 => 'a', 22 => 'b', 33 => 'c'};
  var_dump(array_shift(inout $x3));
  var_dump($x3);
  $x3[11] = 'a';
  var_dump($x3);

  $x4 = Set {'a', 'b', 'c'};
  var_dump(array_shift(inout $x4));
  var_dump($x4);
  $x4[] = 'a';
  var_dump($x4);

  $x5 = vec[];
  var_dump(array_shift(inout $x5));

  $x6 = Vector {};
  var_dump(array_shift(inout $x6));

  $x7 = Map {};
  var_dump(array_shift(inout $x7));

  $x8 = Set {};
  var_dump(array_shift(inout $x8));
}


<<__EntryPoint>>
function main_array_shift() :mixed{
main();
}
