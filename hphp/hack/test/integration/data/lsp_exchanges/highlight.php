<?hh

function a_highlight(): int {
  return b_highlight();
}

function b_highlight(): int {
  return 42;
}
