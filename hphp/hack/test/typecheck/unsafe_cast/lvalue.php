<?hh

function f(mixed $m): void {
  HH\FIXME\UNSAFE_CAST<mixed, dict<int, string>>($m)[0] = "lvalue";
}
