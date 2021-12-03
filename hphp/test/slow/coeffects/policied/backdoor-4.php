<?hh

function zoned_with()[zoned_with] {}
function defaults() {}

function defaults_f() {
  // fails, because there's no way we can obtain implicit context
  HH\Coeffects\backdoor(()[zoned_with] ==> { zoned_with(); });
}

function zoned_with_f()[zoned_with] {
  // still fails, because lambda doesn't capture the context of the
  // parent scope; it just happens to be zoned_with, but the intention
  // may have been to work with a different policy
  HH\Coeffects\backdoor(()[zoned_with] ==> { zoned_with(); });

  // succeeds, because the lambda defaults to parent scope,
  // so it should capture context and implicit policy
  HH\Coeffects\backdoor(() ==> { zoned_with(); });
}

function unknown(mixed $arg)[ctx $arg] {
  // behaves the same like above
  HH\Coeffects\backdoor(() ==> { zoned_with(); });
}

<<__EntryPoint>>
function main() {
  defaults_f();
  HH\Coeffects\_Private\enter_zoned_with(zoned_with_f<>);
  unknown(defaults<>);
  unknown(zoned_with<>);
}
