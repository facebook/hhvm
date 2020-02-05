<?hh

function good_cases(): vec<Container<mixed>> {
  $init = vec[
    varray[],
    darray[],
    vec[1, 2, 3],
    dict['foo' =>  null, 'bar' => null],
  ];

  // we also want some non-static arrays:
  foreach ($init as $e) {
    $f = $e;
    $f[] = 42;
    $init[] = $f;
  }
  // now mark them
  foreach ($init as $e) {
    $init[] = HH\mark_legacy_hack_array($e);
  }
  return $init;
}

function bad_cases(): vec<mixed> {
  return varray[
    keyset[],
    42,
    null,
  ];
}

function test(Container<mixed> $cases): void {
  foreach ($cases as $case) {
    var_dump(HH\is_marked_legacy_hack_array($case));
  }
}


<<__EntryPoint>>
function main(): void {
  test(good_cases());
  test(bad_cases());
}
