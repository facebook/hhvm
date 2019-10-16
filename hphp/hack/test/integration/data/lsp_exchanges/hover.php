<?hh  //strict
// comment
function a_hover(): int {
  return b_hover();
}

# A comment describing b_hover.
function b_hover(): int {
  return 42;
}

// A comment describing THE_ANSWER
const int THE_ANSWER = 42;

function return_the_answer(): int {
  return THE_ANSWER;
}
