<?hh

function a_bad_call(): int {
  return b_bad_call();
}

function b_bad_call(): int {
  return 42;
}
