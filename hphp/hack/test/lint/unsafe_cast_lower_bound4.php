<?hh

function f(arraykey $m): void {
  \HH\FIXME\UNSAFE_CAST<mixed, int>($m); // Lint that mixed is too loose
}

function g(num $m): void {
  \HH\FIXME\UNSAFE_CAST<mixed, int>($m); // Lint that mixed is too loose
}
