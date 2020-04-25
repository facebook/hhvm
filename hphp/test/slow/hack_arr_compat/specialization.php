<?hh

function run($fn) {
  try {
    print(json_encode($fn())."\n");
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

// Display $x in a way that distinguishes arrays, varrays, and darrays.
function display($x) {
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

// Test all possible triples of (a1 type, a2 type, comparison operator).
function test_darray_varray_comparisons() {
  print("\n=====================================\nComparisons:\n");
  $varray = varray[2, 3, 5];
  $darray = darray[0 => 2, 1 => 3, 2 => 5];
  $array = __hhvm_intrinsics\dummy_array_builtin($varray);
  $array[] = 2; $array[] = 3; $array[] = 5;
  $vec = vec[2, 3, 5];
  $dict = dict[0 => 2, 1 => 3, 2 => 5];
  foreach (vec[$array, $varray, $darray, $vec, $dict] as $a1) {
    foreach (vec[$array, $varray, $darray, $vec, $dict] as $a2) {
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
// in test_varray_ops when $x is varray[2, 3, 5] and we set $x[3] to 'oh no'.
function test_varray_implicit_append() {
  print("\n=====================================\nImplicit append:\n");
  run(() ==> { $x = varray[]; $x[0] = 'implicit append'; return $x; });
}

// We use arrays of three different lengths, so that we can test varray ops
// with "valid index before the last index", "last index", and "OOB index".
//
// This test tests varray unset, varray set string, and implicit append.
function test_varray_ops() {
  $inputs = vec[
    varray[2, 3],
    varray[2, 3, 5],
    varray[2, 3, 5, 7],
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

<<__EntryPoint>>
function main() {
  test_darray_varray_comparisons();
  test_varray_implicit_append();
  test_varray_ops();
}
