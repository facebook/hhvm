<?hh

class C<reified Ta, reified Tb> {}
class D extends C<reified int> {}

new D();
