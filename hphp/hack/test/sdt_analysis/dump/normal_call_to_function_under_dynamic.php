<?hh

class C<T> {}

function f(C<int> $_): void {}

function main(C<int> $c): void {
  f($c); // f NeedsSDT
}
