<?hh

class ClsMethTest { static function fn() :mixed{} }

// Run a function and print either the result or the error thrown.
function run($fn) :mixed{
  try {
    print(json_encode($fn())."\n");
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

// Display $x in a way that distinguishes arrays, varrays, and darrays.
function display($x) :mixed{
  if ($x === ClsMethTest::fn<>) return 'clsmeth';
  $result = __hhvm_intrinsics\serialize_keep_dvarrays($x)[0];
  $lookup = dict[
    'a'=>'array',
    'y'=>'varray',
    'Y'=>'darray',
    'v'=>'vec',
    'D'=>'dict',
  ];
  return $lookup[$result];
}

// Test that as/is shape/tuple does dvarray checks.
function test_as_is_shape_tuple() :mixed{
  print("\n=====================================\nas/is shape/tuple:\n");
  print("\n");
  $darray = dict['a' => 17, 'b' => false];
  $varray = varray($darray);
  foreach (vec[$varray, $darray] as $input) {
    print(display($input).' as shape: ');
    run(() ==> $input as shape('a' => int, 'b' => bool));
    print(display($input).' is shape: ');
    run(() ==> $input is shape('a' => int, 'b' => bool));
  }
  print("\n");
  $varray = vec['a', false];
  $darray = darray($varray);
  foreach (vec[$varray, $darray] as $input) {
    print(display($input).' as tuple: ');
    run(() ==> $input as (string, bool));
    print(display($input).' is tuple: ');
    run(() ==> $input is (string, bool));
  }
}

// Test that we print specialized types in builtin errors.
function test_builtin_error_messages() :mixed{
  print("\n=====================================\nBuiltin errors:\n");
  print('Passing boolean to darray: ');
  run(() ==> __hhvm_intrinsics\dummy_darray_builtin(false));
  print('Passing darray to boolean: ');
  run(() ==> json_decode('[]', dict[]));
}

// Test that builtins enforce dvarray-ness of inputs.
function test_builtin_enforcement() :mixed{
  print("\n=====================================\nBuiltins:\n");
  $clsmeth = ClsMethTest::fn<>;
  foreach (vec[vec[], dict[], $clsmeth] as $input) {
    print('Passing '.display($input).' to varray: ');
    run(() ==> __hhvm_intrinsics\dummy_varray_builtin($input));
    print('Passing '.display($input).' to darray: ');
    run(() ==> __hhvm_intrinsics\dummy_darray_builtin($input));
  }
}

// Test all possible triples of (a1 type, a2 type, comparison operator).
function test_darray_varray_comparisons() :mixed{
  print("\n=====================================\nComparisons:\n");
  $varray = vec[2, 3, 5];
  $darray = dict[0 => 2, 1 => 3, 2 => 5];
  $vec = vec[2, 3, 5];
  $dict = dict[0 => 2, 1 => 3, 2 => 5];
  foreach (vec[$varray, $darray, $vec, $dict] as $a1) {
    foreach (vec[$varray, $darray, $vec, $dict] as $a2) {
      print('  Comparing '.display($a1).' and '.display($a2).":\n");
      print('    === '); run(() ==> $a1 === $a2);
      print('    !== '); run(() ==> $a1 !== $a2);
      print('    <   '); run(() ==> $a1 < $a2);
      print('    >   '); run(() ==> $a1 > $a2);
      print('    <=  '); run(() ==> $a1 <= $a2);
      print('    >=  '); run(() ==> $a1 >= $a2);
      print('    <=> '); run(() ==> $a1 <=> $a2);
    }
  }
}

// A direct test of the "implicit append" behavior. Note that we also test it
// in test_varray_ops when $x is vec[2, 3, 5] and we set $x[3] to 'oh no'.
function test_varray_implicit_append() :mixed{
  print("\n=====================================\nImplicit append:\n");
  run(() ==> { $x = vec[]; $x[0] = 'implicit append'; return $x; });
}

// We use arrays of three different lengths, so that we can test varray ops
// with "valid index before the last index", "last index", and "OOB index".
//
// This test tests varray unset, varray set string, and implicit append.
function test_varray_ops() :mixed{
  $inputs = vec[
    vec[2, 3],
    vec[2, 3, 5],
    vec[2, 3, 5, 7],
  ];
  foreach ($inputs as $i => $x) {
    print("\n=====================================\nTest $i:\n");
    print(json_encode($x)."\n");
    run(() ==> { unset($x[2]); return $x; });
    run(() ==> { unset($x[2]); $x[] = 'appended'; return $x; });
    run(() ==> { $x[3] = 'oh no'; return $x; });
    run(() ==> { $x['whoops'] = false; return $x; });
  }
}

// We're testing many fatal errors here; we do so by keeping a count and
// running the test multiple times (since a fatal ends the test...)
function get_count() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  $count = $count ? $count : 0;
  apc_store('count', $count + 1);
  return $count;
}

function takes_varray(varray $x) :mixed{ return 'varray to varray: OK!'; }
function takes_darray(darray $x) :mixed{ return 'darray to darray: OK!'; }
function returns_varray($fn): varray { return $fn(); }
function returns_darray($fn): darray { return $fn(); }
class C {
  public ?varray $v = null;
  public ?darray $d = null;
}
class D extends C {
  public ?varray $d = null;
}

// Test that regular Hack function typehints are enforced.
function test_typehint_enforcement(int $count) :mixed{
  if (!$count) print("\n=====================================\nTypehints:\n");
  $clsmeth = ClsMethTest::fn<>;
  foreach (vec[vec[], dict[], $clsmeth] as $input) {
    if (!($count--)) run(() ==> takes_varray($input));
    if (!($count--)) run(() ==> takes_darray($input));
  }
  foreach (vec[vec[], dict[], $clsmeth] as $input) {
    if (!($count--)) run(() ==> returns_varray(() ==> $input));
    if (!($count--)) run(() ==> returns_darray(() ==> $input));
  }
  $c = new C();
  foreach (vec[vec[], dict[], $clsmeth] as $input) {
    if (!($count--)) run(() ==> $c->v = $input);
    if (!($count--)) run(() ==> $c->d = $input);
  }
  if (!($count--)) $d = new D();
}

<<__EntryPoint>>
function main() :mixed{
  $count = get_count();
  if (!$count) {
    test_as_is_shape_tuple();
    test_builtin_error_messages();
    test_builtin_enforcement();
    test_darray_varray_comparisons();
    test_varray_implicit_append();
    test_varray_ops();
  }
  test_typehint_enforcement($count);
}
