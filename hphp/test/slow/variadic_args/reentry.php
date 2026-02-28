<?hh

function f_variadic(...$args) :mixed{
  echo "* ", __FUNCTION__, "\n";
  var_dump($args);
  return $args;
}

function g1($a1, $a2=null) :mixed{
  $args = isset($a2) ? vec[$a1, $a2] : vec[$a1];
  return $args;
}

function g2(...$args) :mixed{
  return $args;
}

function main() :mixed{
  echo '= Single-arg array_map =', "\n";
  $v = Vector {1, 2, 3, 4};
  $a = $v->toVArray();
  $expected = vec[
    vec[1],
    vec[2],
    vec[3],
    vec[4],
  ];

  // this should end up using the hack implementation
  $ret = array_map(f_variadic<>, $a);
  var_dump($ret === array_map(g1<>, $a));
  var_dump($ret === array_map(g2<>, $a));

  // using a collection to break out of hack implementation
  $basic_v = array_map(f_variadic<>, $v);
  var_dump($ret === array_map(g1<>, $a));
  var_dump($ret === array_map(g2<>, $a));

  echo "\n\n = Multi-arg ArrayMap =\n";
  $ret = array_map(f_variadic<>, $a, vec['a', 'b', 'c', 'd']);
  var_dump($ret === array_map(g1<>, $a, vec['a', 'b', 'c', 'd']));
  var_dump($ret === array_map(g2<>, $a, vec['a', 'b', 'c', 'd']));
}


<<__EntryPoint>>
function main_reentry() :mixed{
main();
}
