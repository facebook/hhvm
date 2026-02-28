<?hh

function cases(): vec<mixed> {
  return vec[
    42,
    "foo",
    null,
    false,
    vec['bar'],
    dict[7 => 13],
    new stdClass(),
    cases<>,
    ExternalThreadEventWaitHandle::setOnCreateCallback<>,
    stdClass::class,
  ];
}

<<__EntryPoint>>
function main(): void {
  foreach (cases() as $case) {
    __hhvm_intrinsics\launder_value_inout(inout $case);
    var_dump($case);
  }
}
