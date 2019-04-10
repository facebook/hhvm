<?hh

echo "multiple argument\n";
class C {
  <<__Memoize>>
  public static function f<reify Ta, Tb, reify Tc>($x, $y, $z) {
    var_dump("hi");
  }
}

C::f<int, string, bool>(1,2,3); // print
C::f<int, string, bool>(1,2,3); // nope
C::f<string, string, bool>(1,2,3); // print
C::f<string, string, bool>(1,2,3); // nope
C::f<string, string, string>(1,2,3); // print
C::f<int, string, bool>(1,2,3); // nope
C::f<int, string, bool>(1,1,3); // print
C::f<int, string, bool>(1,2,3); // nope


echo "no argument\n";
class D {
  <<__Memoize>>
  public static function g<reify Ta, Tb, reify Tc>() {
    var_dump("hi");
  }
}

D::g<int, string, bool>(); // print
D::g<int, string, bool>(); // nope
D::g<string, string, bool>(); // print
D::g<string, string, bool>(); // nope
D::g<string, string, string>(); // print
D::g<int, string, bool>(); // nope
D::g<int, string, bool>(); // nope

echo "traits\n";

trait T {
  <<__Memoize>>
  public static function h<reify Ta, Tb, reify Tc>(mixed $x) {
    var_dump("hi");
  }
}

class E {
  use T;
}

E::h<int, string, bool>(1); // print
E::h<int, string, bool>(1); // nope
E::h<string, string, bool>(1); // print
E::h<string, string, bool>(1); // nope
E::h<string, string, string>(1); // print
E::h<int, string, bool>(1); // nope
E::h<int, string, bool>(1); // nope
E::h<int, string, bool>(2); // print
E::h<int, string, bool>(1); // nope
