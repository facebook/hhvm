<?hh

class D<reify Ta, reify Tb> {}
class C<reify Ta, <<__Soft>> reify Tb> extends D<Ta, Tb> {}

$c = new C<int, string>();
