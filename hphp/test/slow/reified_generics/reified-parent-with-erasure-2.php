<?hh

class D<reify Ta> {}
class C<reify Ta, Tb, reify Tc> extends D<int, string>{}

$c = new C<int, string, bool>();
