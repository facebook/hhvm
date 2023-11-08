<?hh

class C {}
function f<T as C>(): void {
  nameof T;
}
