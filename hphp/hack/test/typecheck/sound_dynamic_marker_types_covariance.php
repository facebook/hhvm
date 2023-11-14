<?hh

function f(
  HH\FIXME\POISON_MARKER<arraykey> $_poison,
): void {}

function g(
  HH\FIXME\POISON_MARKER<string> $poison,
): void {
  f($poison);
}
