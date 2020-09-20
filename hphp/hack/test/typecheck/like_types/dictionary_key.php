<?hh

function f(mixed $k): void {
  dict[$k as ~arraykey => true];
}
