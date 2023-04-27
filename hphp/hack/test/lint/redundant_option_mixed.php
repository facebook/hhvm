<?hh

class C<T>

function g<T>(): void {}

function f<T>(): void {
  g<?mixed>();

  new C<?mixed>();
}
