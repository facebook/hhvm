<?hh  //strict

function a_definition(): int {
  return b_definition();
}

function b_definition(): int {
  return 42;
}
