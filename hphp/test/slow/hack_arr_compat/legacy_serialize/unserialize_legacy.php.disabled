<?hh

function decode(string $input, bool $legacy): mixed {
  return unserialize(
    $input,
    darray[
      'legacy_hack_arrays' => $legacy,
    ],
  );
}

function check(string $val): void {
  check_impl(
    decode($val, true),
    true,
  );
  check_impl(
    decode($val, false),
    false,
  );
}

function check_impl(mixed $val, bool $should_be_legacy): void {
  if (HH\is_any_array($val)) {
    var_dump($val);
    invariant(
      $should_be_legacy === HH\is_marked_legacy_hack_array($val),
      'Legacy expectation mismatch!',
    );
    foreach ($val as $v) {
      check_impl($v, $should_be_legacy);
    }
  }
}

function cases(): vec<mixed> {
  return vec[
    vec[],
    dict[],
    vec[1, 2, 3],
    1,
    dict["foobar" => null],
    vec[dict[]],
    dict["crosby" => dict["bing" => null]],
  ];
}

<<__EntryPoint>>
function test(): void {
  foreach (cases() as $c) {
    check(serialize($c));
  }
}
