<?hh

class C<reify T> {}
class D<reify T> extends C<T, int> {}

function f(): void {
  new D<int>();
}
