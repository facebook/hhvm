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

// comment describing the "pad_left" function and its parameters
function pad_left(string $s, int $i, string $pad): string {
  return $s;
}

// comment describing "return_a_string" function
function return_a_string(): string {
  $pad = "hello";
  $x = pad_left("StringToPad", 20, $pad);
  return $x;
}
