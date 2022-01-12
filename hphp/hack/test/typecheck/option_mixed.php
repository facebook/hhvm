<?hh

class C<T> {}

function f<T>(): void {
  f<?mixed>();

  new C<?mixed>();
}
