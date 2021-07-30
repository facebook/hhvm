<?hh

function some_condition(int $_): bool {
  return true;
}

function flow_typing_example(): void {
  if (3 !== 2 || some_condition(HH\FIXME\UNSAFE_CAST<bool, int>(false))) {
  }
}
