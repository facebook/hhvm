<?hh
function main() :mixed{
  $x1 = vec[];
  var_dump(array_unshift(inout $x1, 1));
  var_dump(array_unshift(inout $x1, 'b', 'c'));
  var_dump($x1);
  $x2 = vec['d'];
  var_dump(array_unshift(inout $x2, 'e', 'f', 2, 'f'));
  var_dump($x2);
  $x3 = Vector {};
  var_dump(array_unshift(inout $x3, 1));
  var_dump(array_unshift(inout $x3, 'b', 'c'));
  var_dump($x3);
  $x4 = Vector {'d'};
  var_dump(array_unshift(inout $x4, 'e', 'f', 2, 'f'));
  var_dump($x4);
  $x5 = Set {};
  var_dump(array_unshift(inout $x5, 1));
  var_dump(array_unshift(inout $x5, 'b', 'c'));
  var_dump($x5);
  $x6 = Set {'d'};
  var_dump(array_unshift(inout $x6, 'e', 'f', 2, 'f'));
  var_dump($x6);
  $x7 = Set {};
  var_dump(array_unshift(inout $x7, 'h', 3, 'j'));
  var_dump(array_unshift(inout $x7, 3, 'j'));
  var_dump($x7);
}


<<__EntryPoint>>
function main_array_unshift() :mixed{
main();
}
