<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function fail($a, $a2) {
  echo "=========== Comparison Failure ==========\n";
  var_dump($a);
  var_dump(is_varray($a));
  var_dump(is_darray($a));
  var_dump(is_array($a));
  var_dump(is_vec($a));
  var_dump(is_dict($a));

  var_dump($a2);
  var_dump(is_varray($a2));
  var_dump(is_darray($a2));
  var_dump(is_array($a2));
  var_dump(is_vec($a2));
  var_dump(is_dict($a2));
}

function get($key, $a) {
  $a2 = __hhvm_intrinsics\apc_fetch_no_check($key);
  if ($a2 === false) {
    echo "=============== Error fetching $key ==============\n";
    return;
  }
  if ($a !== $a2) {
    if (count($a) != count($a2) ||
        count($a) == 0 ||
        !is_object($a[0]) ||
        !is_object($a2[0])) {
      return fail($a, $a2);
    }
  }
  if (is_varray($a) !== is_varray($a2)) return fail($a, $a2);
  if (is_darray($a) !== is_darray($a2)) return fail($a, $a2);
  if (is_array($a)  !== is_array($a2))  return fail($a, $a2);
  if (is_vec($a)    !== is_vec($a2))    return fail($a, $a2);
  if (is_dict($a)   !== is_dict($a2))   return fail($a, $a2);
}

function set_all($values) {
  foreach ($values as $k => $v) apc_store($k, $v);
}

function get_all($values) {
  foreach ($values as $k => $v) get($k, $v);
}

function launder($v) { return __hhvm_intrinsics\launder_value($v); }

function make_circular_varray() {
  $x = new stdClass;
  $v = vec[$x];
  $x->foo = $v;
  return $v;
}

function make_circular_darray() {
  $x = new stdClass;
  $d = dict[0 => $x];
  $x->foo = $d;
  return $d;
}

function test() {
  $values = dict[
    'varray1' => vec[],
    'varray2' => vec[1, 2, 3],
    'varray3' => vec['a', 'b', 'c'],
    'varray4' => vec[launder(7), launder(8), launder(9)],
    'varray5' => make_circular_varray(),
    'darray1' => dict[],
    'darray2' => dict[0 => 'a', 1 => 'b', 2 => 'c'],
    'darray3' => dict['a' => 100, 'b' => 200],
    'darray4' => dict[0 => launder('x'), 1 => launder('y'), 2 => launder('z')],
    'darray5' => dict[launder('j') => 11, launder('k') => 222],
    'darray6' => make_circular_darray()
  ];

  if (__hhvm_intrinsics\apc_fetch_no_check('second-time') === false) {
    set_all($values);
    apc_store('second-time', true);
  } else {
    get_all($values);
    echo "============ DONE ================\n";
  }
}
