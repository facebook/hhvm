<?hh

class C<reify T> {}
class D<reify T> extends C<T, int> {}

new D<int>();
