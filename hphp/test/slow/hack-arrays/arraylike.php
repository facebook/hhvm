<?hh

function takes_arraylike(arraylike<arraykey, mixed> $a) {
  var_dump($a);
}

function ret_array(): arraylike<arraykey, mixed> {
  return __hhvm_intrinsics\dummy_cast_to_kindofarray(dict[42 => 'lol']);
}
function ret_varray(): arraylike<arraykey, mixed> {
  return varray['lol'];
}
function ret_darray(): arraylike<arraykey, mixed> {
  return darray[42 => 'lol'];
}
function ret_vec(): arraylike<arraykey, mixed> {
  return vec['lol'];
}
function ret_dict(): arraylike<arraykey, mixed> {
  return dict[42 => 'lol'];
}
function ret_keyset(): arraylike<arraykey, mixed> {
  return keyset['lol'];
}

function call_arraylike_builtin($x) {
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin($x);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
}

function test_builtins($a, $b, $c, $d, $e, $f, $g) {
  $a = __hhvm_intrinsics\launder_value($a);
  $b = __hhvm_intrinsics\launder_value($b);
  $c = __hhvm_intrinsics\launder_value($c);
  $d = __hhvm_intrinsics\launder_value($d);
  $e = __hhvm_intrinsics\launder_value($e);
  $f = __hhvm_intrinsics\launder_value($f);
  $g = __hhvm_intrinsics\launder_value($g);

  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(null);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(42);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(tuple(true, false));
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(shape('a' => true, 'b' => false));
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(varray[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(darray[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(varray[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }

  call_arraylike_builtin($a);
  call_arraylike_builtin($b);
  call_arraylike_builtin($c);
  call_arraylike_builtin($d);
  call_arraylike_builtin($e);
  call_arraylike_builtin($f);
  call_arraylike_builtin($g);
}

function main() {
  takes_arraylike(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict[42 => 'lol']));
  takes_arraylike(varray['lol']);
  takes_arraylike(darray[42 => 'lol']);
  takes_arraylike(vec['lol']);
  takes_arraylike(dict[42 => 'lol']);
  takes_arraylike(keyset['lol']);

  var_dump(ret_array());
  var_dump(ret_varray());
  var_dump(ret_darray());
  var_dump(ret_vec());
  var_dump(ret_dict());
  var_dump(ret_keyset());

  test_builtins(
    null,
    42,
    tuple(true, false),
    shape('a' => true, 'b' => false),
    varray[],
    darray[],
    varray[]
  );
}


<<__EntryPoint>>
function main_arraylike() {
main();
}
