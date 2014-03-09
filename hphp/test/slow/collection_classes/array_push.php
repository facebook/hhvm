<?hh

function main() {
  $elts = array('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h');

  $v = Vector {'a', 'b', 'c', 'd'};

  var_dump(array_push($v, 'e'));
  var_dump(array_push($v, 'f', 'g', 'h'));
  var_dump($v == new Vector($elts));

  $s = Set {'a', 'b', 'c', 'd'};

  var_dump(array_push($s, 'e'));
  var_dump(array_push($s, 'f', 'g', 'h'));
  var_dump($s == new Set($elts));

  $m = Map{};
  array_push($m, 'a');
}
main();
echo "Done\n";
