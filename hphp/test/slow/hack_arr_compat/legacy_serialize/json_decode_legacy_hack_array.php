<?hh

function decode(string $input, bool $legacy): mixed {
  return json_decode(
    $input,
    true,
    512,
    $legacy ? JSON_FB_LEGACY_HACK_ARRAYS : 0,
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
    var_dump($val, $should_be_legacy === HH\is_marked_legacy_hack_array($val));
    foreach ($val as $v) {
      check_impl($v, $should_be_legacy);
    }
  }
}

function cases(): vec<string> {
  return vec[
    '[]',
    '{}',
    '[1, 2, 3]',
    '1',
    '{"foobar": null}',
    '[[]]',
    '{"crosby": {"bing": null}}'
  ];
}

<<__EntryPoint>>
function test(): void {
  foreach (cases() as $c) {
    check($c);
  }
}
