<?hh

class C<reify Ta, reify Tb> {}
class D<reify T> extends C<T> {}

new D<int>();
