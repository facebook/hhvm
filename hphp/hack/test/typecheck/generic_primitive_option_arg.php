<?hh // partial

function g(?int $x): void {}

function f<T as int>(T $x): void {
  g($x);
}
