<?hh

function main() :mixed{
  $arr = vec['a', 'b', 'c'];

  $v = vec['a', 'b', 'c'];
  $d = dict[0 => 'a', 1 => 'b', 2 => 'c'];
  $k = keyset['a', 'b', 'c'];

  $arr_implode = implode(', ', $arr);

  var_dump(implode(', ', $v));
  var_dump(implode($v, ', '));
  var_dump(implode(', ', $v) === $arr_implode);

  var_dump(implode(', ', $d));
  var_dump(implode($d, ', '));
  var_dump(implode(', ', $d) === $arr_implode);

  var_dump(join(', ', $v));
  var_dump(join($v, ', '));
  var_dump(join(', ', $v) === $arr_implode);

  $keyset_implode = implode(', ', $k);
  var_dump(strlen($keyset_implode) === strlen($arr_implode));
  var_dump(keyset(explode(', ', $keyset_implode)) == $k);
}

<<__EntryPoint>>
function main_implode_hack_array() :mixed{
main();
}
