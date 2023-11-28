<?hh

function f(): ~int {
  return \HH\FIXME\UNSAFE_CAST<string, int>();
}

function g(dynamic $m1, dynamic $m2): ~int {
  return \HH\FIXME\UNSAFE_CAST<dynamic, int>($m1, $m2);
}
