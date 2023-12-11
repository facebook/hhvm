<?hh

function takes_arraylike(AnyArray<arraykey, mixed> $a) :mixed{
  var_dump($a);
}

function ret_array(): AnyArray<arraykey, mixed> {
  return darray(dict[42 => 'lol']);
}
function ret_varray(): AnyArray<arraykey, mixed> {
  return vec['lol'];
}
function ret_darray(): AnyArray<arraykey, mixed> {
  return dict[42 => 'lol'];
}
function ret_vec(): AnyArray<arraykey, mixed> {
  return vec['lol'];
}
function ret_dict(): AnyArray<arraykey, mixed> {
  return dict[42 => 'lol'];
}
function ret_keyset(): AnyArray<arraykey, mixed> {
  return keyset['lol'];
}

function call_arraylike_builtin($x) :mixed{
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin($x);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
}

function test_builtins($a, $b, $c, $d, $e, $f, $g) :mixed{
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
    __hhvm_intrinsics\dummy_arraylike_builtin(vec[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(dict[]);
  } catch (Exception $e) {
    echo "Exception: {$e->getMessage()}\n";
  }
  try {
    __hhvm_intrinsics\dummy_arraylike_builtin(vec[]);
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

function main() :mixed{
  takes_arraylike(darray(dict[42 => 'lol']));
  takes_arraylike(vec['lol']);
  takes_arraylike(dict[42 => 'lol']);
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
    vec[],
    dict[],
    vec[]
  );
}


<<__EntryPoint>>
function main_arraylike() :mixed{
main();
}
