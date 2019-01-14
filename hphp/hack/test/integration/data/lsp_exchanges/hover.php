<?hh  //strict
// comment
function a_hover(): int {
  return b_hover();
}

# A comment describing b_hover.
function b_hover(): int {
  return 42;
}
