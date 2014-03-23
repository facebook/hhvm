<?hh

function f_variadic(...$args) {
  echo "* ", __FUNCTION__, "\n";
  var_dump($args);
  return $args;
}

function g1($a1, $a2=null) {
  $args = isset($a2) ? [$a1, $a2] : [$a1];
  return $args;
}

function g2() {
  $args = func_get_args();
  return $args;
}

function main() {
  echo '= Single-arg array_map =', "\n";
  $v = Vector {1, 2, 3, 4};
  $a = $v->toArray();
  $expected = array(
    array(1),
    array(2),
    array(3),
    array(4),
  );

  // this should end up using the cufiter (hhas) implementation
  $ret = array_map('f_variadic', $a);
  var_dump($ret === array_map('g1', $a));
  var_dump($ret === array_map('g2', $a));

  // using a collection to break out of hhas implementation
  $basic_v = array_map('f_variadic', $v);
  var_dump($ret === array_map('g1', $a));
  var_dump($ret === array_map('g2', $a));

  echo "\n\n = Multi-arg ArrayMap =\n";
  $ret = array_map('f_variadic', $a, array('a', 'b', 'c', 'd'));
  var_dump($ret === array_map('g1', $a, array('a', 'b', 'c', 'd')));
  var_dump($ret === array_map('g2', $a, array('a', 'b', 'c', 'd')));
}

main();
