<?hh

class C<reified T> {}
class D<reified T> extends C<reified T, reified int> {}

new D<reified int>();
