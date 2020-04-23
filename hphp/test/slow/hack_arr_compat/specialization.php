<?hh

function run($input, $fn) {
  try {
    print(json_encode($fn($input))."\n");
  } catch (Exception $e) {
    print($e->getMessage()."\n");
  }
}

<<__EntryPoint>>
function main() {
  // A direct test of the "implicit append" behavior. Note that we also test
  // it below when we $input is varray[2, 3, 5] and we set $x[3] to 'oh no'.
  print("\n=====================================\nImplicit append:\n");
  run(varray[], $x ==> { $x[0] = 'implicit append'; return $x; });

  // We use arrays of three different lengths, so that we can test varray ops
  // with "valid index before the last index", "last index", and "OOB index".
  $inputs = vec[
    varray[2, 3],
    varray[2, 3, 5],
    varray[2, 3, 5, 7],
  ];
  foreach ($inputs as $i => $input) {
    print("\n=====================================\nTest $i:\n");
    print(json_encode($input)."\n");
    run($input, $x ==> { unset($x[2]); return $x; });
    run($input, $x ==> { unset($x[2]); $x[] = 'appended'; return $x; });
    run($input, $x ==> { $x[3] = 'oh no'; return $x; });
    run($input, $x ==> { $x['whoops'] = false; return $x; });
  }
}
