<?hh

class C<reify Ta, reify Tb> {}
class D extends C<int> {}

new D();
