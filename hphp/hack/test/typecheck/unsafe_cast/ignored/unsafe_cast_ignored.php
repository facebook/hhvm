<?hh

function f(string $s): int {
  return unsafe_cast<string, int>($s);
}
