<?hh

function policied_of()[policied_of] {}
function defaults() {}

function defaults_f() {
  // fails, because there's no way we can obtain implicit context
  HH\Coeffects\backdoor(()[policied_of] ==> { policied_of(); });
}

function policied_of_f()[policied_of] {
  // still fails, because lambda doesn't capture the context of the
  // parent scope; it just happens to be policied_of, but the intention
  // may have been to work with a different policy
  HH\Coeffects\backdoor(()[policied_of] ==> { policied_of(); });

  // succeeds, because the lambda defaults to parent scope,
  // so it should capture context and implicit policy
  HH\Coeffects\backdoor(() ==> { policied_of(); });
}

function unknown(mixed $arg)[ctx $arg] {
  // behaves the same like above
  HH\Coeffects\backdoor(() ==> { policied_of(); });
}

<<__EntryPoint>>
function main() {
  defaults_f();
  HH\Coeffects\enter_policied_of(policied_of_f<>);
  unknown(defaults<>);
  unknown(policied_of<>);
}
