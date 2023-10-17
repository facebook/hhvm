<?hh

function a_didchange(): int {
  return b_didchange();
}

function b_didchange(): int {
  return 42;
}
