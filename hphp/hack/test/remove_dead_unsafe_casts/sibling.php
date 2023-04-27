<?hh

function f(int $i): void {
  // Both of these are redundant
  HH\FIXME\UNSAFE_CAST<mixed,int>($i) + HH\FIXME\UNSAFE_CAST<mixed,int>($i);
}
