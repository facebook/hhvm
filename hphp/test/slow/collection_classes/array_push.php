<?hh

function main() :mixed{
  $elts = vec['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'];

  $v = Vector {'a', 'b', 'c', 'd'};

  var_dump(array_push(inout $v, 'e'));
  var_dump(array_push(inout $v, 'f', 'g', 'h'));
  var_dump($v == new Vector($elts));

  $s = Set {'a', 'b', 'c', 'd'};

  var_dump(array_push(inout $s, 'e'));
  var_dump(array_push(inout $s, 'f', 'g', 'h'));
  var_dump($s == new Set($elts));

  $m = Map{};
  array_push(inout $m, 'a');
}

<<__EntryPoint>>
function main_array_push() :mixed{
main();
echo "Done\n";
}
