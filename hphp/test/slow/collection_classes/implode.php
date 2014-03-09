<?hh

function main() {
  $arr = array('a', 'b', 'c');

  $v = Vector {'a', 'b', 'c'};
  $m = Map {0 => 'a', 1 => 'b', 2 => 'c'};
  $s = Set {'a', 'b', 'c'};

  $arr_implode = implode(', ', $arr);

  var_dump(implode(', ', $v));
  var_dump(implode($v, ', '));
  var_dump(implode(', ', $v) === $arr_implode);

  var_dump(implode(', ', $m));
  var_dump(implode($m, ', '));
  var_dump(implode(', ', $v) === $arr_implode);

  var_dump(join(', ', $v));
  var_dump(join($v, ', '));
  var_dump(implode(', ', $v) === $arr_implode);

  $set_implode = implode(', ', $s);
  var_dump(strlen($set_implode) === strlen($arr_implode));
  var_dump(new Set(explode(', ', $set_implode)) == $s);
}
main();
