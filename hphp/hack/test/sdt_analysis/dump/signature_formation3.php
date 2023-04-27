<?hh

class C<T> {}

function f<T>(): C<T> {
  return new C();
}
