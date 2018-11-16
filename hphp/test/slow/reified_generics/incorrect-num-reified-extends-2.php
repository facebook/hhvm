<?hh

class C<reify Ta, reify Tb> {}
class D<reify T> extends C<reify T> {}

new D<reify int>();
