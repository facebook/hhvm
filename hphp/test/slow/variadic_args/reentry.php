<?hh

function f_variadic(...$args) {
  echo "* ", __FUNCTION__, "\n";
  var_dump($args);
  return $args;
}

function g1($a1, $a2=null) {
  $args = isset($a2) ? varray[$a1, $a2] : varray[$a1];
  return $args;
}

function g2(...$args) {
  return $args;
}

function main() {
  echo '= Single-arg array_map =', "\n";
  $v = Vector {1, 2, 3, 4};
  $a = $v->toArray();
  $expected = varray[
    varray[1],
    varray[2],
    varray[3],
    varray[4],
  ];

  // this should end up using the hack implementation
  $ret = array_map(fun('f_variadic'), $a);
  var_dump($ret === array_map(fun('g1'), $a));
  var_dump($ret === array_map(fun('g2'), $a));

  // using a collection to break out of hack implementation
  $basic_v = array_map(fun('f_variadic'), $v);
  var_dump($ret === array_map(fun('g1'), $a));
  var_dump($ret === array_map(fun('g2'), $a));

  echo "\n\n = Multi-arg ArrayMap =\n";
  $ret = array_map(fun('f_variadic'), $a, varray['a', 'b', 'c', 'd']);
  var_dump($ret === array_map(fun('g1'), $a, varray['a', 'b', 'c', 'd']));
  var_dump($ret === array_map(fun('g2'), $a, varray['a', 'b', 'c', 'd']));
}


<<__EntryPoint>>
function main_reentry() {
main();
}
