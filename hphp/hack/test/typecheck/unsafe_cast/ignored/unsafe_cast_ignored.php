<?hh

function f(string $s): int {
  return \HH\FIXME\UNSAFE_CAST<string, int>($s);
}
