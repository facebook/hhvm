<?hh

function takes_int(int $_): void {}

function f(int $i): void {
  takes_int(HH\FIXME\UNSAFE_CAST<mixed, int>($i)); // Redundant
  1 + (HH\FIXME\UNSAFE_CAST<mixed, int>($i) + 2); // Redundant
}
