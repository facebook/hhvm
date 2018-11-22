<?hh

class D<reify Ta> {}
class C<reify Ta, Tb, reify Tc> extends D<reify int, string>{}

$c = new C<reify int, string, reify bool>();
