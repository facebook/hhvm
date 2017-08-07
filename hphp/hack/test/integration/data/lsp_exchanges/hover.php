<?hh  //strict

function a_hover(): int {
  return b_hover();
}

function b_hover(): int {
  return 42;
}
