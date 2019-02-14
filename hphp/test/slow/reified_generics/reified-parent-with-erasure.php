<?hh

class D<Ta, reify Tb> {}
class C<reify Ta, Tb, reify Tc> extends D<reify int, Tb>{}

function g<T>() {
  $c = new C<reify int, T, reify bool>();
}

g();
