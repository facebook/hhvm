<?hh

class D<reify Ta, reify Tb> {}
class C<reify Ta, <<__Soft>> reify Tb> extends D<reify Ta, reify Tb> {}

$c = new C<reify int, string>();
