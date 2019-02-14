<?hh

class C<reify T> {}
class D<reify T> extends C<reify T, reify int> {}

new D<int>();
