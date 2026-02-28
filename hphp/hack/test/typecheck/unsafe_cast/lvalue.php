<?hh

function f(dynamic $m): void {
  HH\FIXME\UNSAFE_CAST<dynamic, dict<int, string>>($m)[0] = "lvalue";
}
