<?hh

function a_nomethod(): int {
  return b_nomethod();
}

function b_nomethod(): int {
  return 42;
}
