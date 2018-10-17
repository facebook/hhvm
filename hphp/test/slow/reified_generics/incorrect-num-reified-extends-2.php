<?hh

class C<reified Ta, reified Tb> {}
class D<reified T> extends C<reified T> {}

new D<reified int>();
